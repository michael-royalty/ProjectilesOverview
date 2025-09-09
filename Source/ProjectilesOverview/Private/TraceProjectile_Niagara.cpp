// Copyright Michael Royalty. All Rights Reserved.

#include "TraceProjectile_Niagara.h"
#include "NiagaraDataChannel.h"
#include "NiagaraDataChannelHandler.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "NiagaraTypes.h"
#include "Kismet/KismetMathLibrary.h" // Used for randomized cone angle
#include <numeric> // Used for iota

ATraceProjectile_Niagara::ATraceProjectile_Niagara()
{
	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	NiagaraComponent->SetupAttachment(RootComponent);
	NiagaraComponent->bAutoActivate = true;
}

void ATraceProjectile_Niagara::CreateProjectile(
	UNiagaraDataChannelAsset* DataChannelAsset,
	const TArray<FVector>& MuzzleLocations,
	const TArray<FVector>& MuzzleDirections,
	float MuzzleVelocity,
	int32 Count,
	float ConeHalfAngle)
{
    if (MuzzleLocations.IsEmpty() || MuzzleDirections.IsEmpty() || Count <= 0)
    {
        return;
    }

    // Replace this age with your own max projectile age
	const float MaxAge = 3.0f;

    const int32 NumShots = MuzzleLocations.Num();
    const int32 NumProjectiles = NumShots * Count;

    UNiagaraDataChannel* DataChannel = nullptr;

	if (DataChannelAsset)
	{
		DataChannel = DataChannelAsset->Get();
	}

	// For island-based data channels. Not using, so we can leave everything default/uninitialized
	FNiagaraDataChannelSearchParameters SearchParams;
	SearchParams.Location = FVector::ZeroVector;

	FTraceProjectileDataChannelWriter Writer;

	if (!Writer.BeginWrite(this, DataChannel, SearchParams, NumProjectiles,
		/* bVisibleToGame = */ false,
		/* bVisibleToCPU  = */ true,
		/* bVisibleToGPU  = */ false))
	{
		UE_LOG(LogTemp, Warning, TEXT("Write Data Channel: Failed to begin write."));
		return;
	}

    // Replace with your own projectile radius
    const FCollisionShape SphereCollision = FCollisionShape::MakeSphere(15.0f);
    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(TraceProjectileNiagara), /* bTraceComplex */ false);

    ParallelFor(
        TEXT("TraceProjectileNiagara"),
        NumProjectiles,
        /* Adjust MinBatchSize based on performance */ 16,
        [&](int32 i)
        {
            const int32 ShotIndex = i / Count;
            const FVector& MuzzleLocation = MuzzleLocations[ShotIndex];
            const FVector& MuzzleDirection = MuzzleDirections[ShotIndex];

            // Randomize firing direction within cone
            const FVector ProjectileDirection =
                UKismetMathLibrary::RandomUnitVectorInConeInDegrees(MuzzleDirection, ConeHalfAngle);

            const FVector ProjectileVelocity = ProjectileDirection * MuzzleVelocity;

            // Trace to find hit and adjust lifespan
            FHitResult Hit;
            const FVector TraceEnd = MuzzleLocation + ProjectileVelocity * MaxAge;
            float Lifespan = MaxAge;

            bool bHit = GetWorld()->SweepSingleByChannel(
                Hit,
                MuzzleLocation,
                TraceEnd,
                FQuat::Identity,
                ECC_Visibility,
                SphereCollision,
                QueryParams
            );

            if (bHit)
            {
                Lifespan = Hit.Time * MaxAge;
            }

            FVector4 ProjectileVelocityAndLifespan = FVector4(
                ProjectileVelocity.X,
                ProjectileVelocity.Y,
                ProjectileVelocity.Z,
                Lifespan
            );

            Writer.WriteMuzzleLocation(i, MuzzleLocation);
            Writer.WriteProjectileVelocityAndLifespan(i, ProjectileVelocityAndLifespan);
        },
        EParallelForFlags::None
    );
}