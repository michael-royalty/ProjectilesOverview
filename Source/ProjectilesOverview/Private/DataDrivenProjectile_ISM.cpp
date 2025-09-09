// Copyright Michael Royalty. All Rights Reserved.


#include "DataDrivenProjectile_ISM.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ADataDrivenProjectile_ISM::ADataDrivenProjectile_ISM()
{
	ISMComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ISMComponent"));
	ISMComponent->SetupAttachment(RootComponent);
	ISMComponent->bAutoActivate = true;
}

void ADataDrivenProjectile_ISM::CreateProjectile(
	const TArray<FVector>& MuzzleLocations,
	const TArray<FVector>& MuzzleDirections,
	float MuzzleVelocity,
	int32 Count,
	float ConeHalfAngle)
{
	if (Count <= 0 || MuzzleLocations.Num() == 0 || MuzzleDirections.Num() == 0)
		return;

	// Replace this age with your own max projectile age or max distance
	const float MaxAge = 3.0f;
	const float TargetAge = GetWorld()->GetTimeSeconds() + MaxAge;

	const int32 TotalProjectiles = MuzzleLocations.Num() * Count;

	const int32 NumToReserve = Transforms.Num() + TotalProjectiles;

	Transforms.Reserve(NumToReserve);
	Velocities.Reserve(NumToReserve);
	Ages.Reserve(NumToReserve);

	for (int ShotIndex = 0; ShotIndex < MuzzleLocations.Num(); ++ShotIndex)
	{
		const FVector MuzzleLocation = MuzzleLocations[ShotIndex];
		const FVector MuzzleDirection = MuzzleDirections[ShotIndex];

		for (int BurstIndex = 0; BurstIndex < Count; ++BurstIndex)
		{
			const FVector ProjectileDirection = UKismetMathLibrary::RandomUnitVectorInConeInDegrees(MuzzleDirection, ConeHalfAngle);
			const FQuat ProjectileRotation = ProjectileDirection.ToOrientationQuat();
			const FVector ProjectileVelocity = ProjectileDirection * MuzzleVelocity;

			Transforms.Add(FTransform(ProjectileRotation, MuzzleLocation, FVector::OneVector));
			Velocities.Add(ProjectileVelocity);
			Ages.Add(TargetAge);
		}
	}
}

void ADataDrivenProjectile_ISM::UpdateProjectiles(float DeltaSeconds)
{
	{ TRACE_CPUPROFILER_EVENT_SCOPE_STR("Begin"); }

	if (Transforms.IsEmpty())
	{
		if (ISMComponent->GetInstanceCount() > 0)
			ISMComponent->ClearInstances();
		return;
	}

	TArray<int32> IndexesToRemove;

	const int32 HighestIndex = Transforms.Num() - 1;
	const int32 NumProjectiles = Transforms.Num();

	const float CurrentTime = GetWorld()->GetTimeSeconds();

	// Replace this radius with your own projectile radius
	FCollisionShape SphereCollision = FCollisionShape::MakeSphere(15.0f);
	FCollisionQueryParams QueryParams;

	struct FProjectileUpdateContext
	{
		TArray<int32> LocalRetireIndexes;
		FHitResult Hit;
	};

	TArray<FProjectileUpdateContext> Contexts;

	ParallelForWithTaskContext<FProjectileUpdateContext>(
		TEXT("UpdateProjectiles"),
		Contexts,
		NumProjectiles,
		/* Adjust MinBatchSize based on performance */ 16,
		// Body
		[&](FProjectileUpdateContext& Context, int32 i)
		{
			const float ProjectileAge = Ages[i];

			if (ProjectileAge < CurrentTime)
			{
				Context.LocalRetireIndexes.Add(i);
				return;
			}

			const FVector ProjectileVelocity = Velocities[i];
			const FVector TravelDistance = ProjectileVelocity * DeltaSeconds;

			const FTransform ProjectileTransform = Transforms[i];
			const FVector TraceStart = ProjectileTransform.GetLocation();
			const FVector TraceEnd = TraceStart + TravelDistance;

			bool bHit = GetWorld()->SweepSingleByChannel(
				Context.Hit,
				TraceStart,
				TraceEnd,
				FQuat::Identity,
				ECC_Visibility,
				SphereCollision,
				QueryParams
			);

			if (bHit)
			{
				Context.LocalRetireIndexes.Add(i);
				return;
			}

			Transforms[i].SetLocation(TraceEnd);
		},
		EParallelForFlags::None
	);

	// Merge retire lists back on the game thread
	{
		TArray<int32> AllRetireIndexes;
		for (auto& Ctx : Contexts)
		{
			AllRetireIndexes.Append(Ctx.LocalRetireIndexes);
		}

		AllRetireIndexes.Sort(TGreater<int32>());
		for (int32 Index : AllRetireIndexes)
		{
			RetireProjectile(Index);
		}
	}

	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("UpdateISMComponent");

		// We could add batch updates here.
		// Alternatively, swap in ISMHelpers plugin.

		ISMComponent->ClearInstances();
		ISMComponent->AddInstances(Transforms, /*bShouldReturnIndices*/ false, /*bWorldSpace*/ false, /*bUpdateNavigation*/ false);
	}
}

void ADataDrivenProjectile_ISM::RetireProjectile(int32 Index)
{
	// Pop and swap old indexes as they're retired

	const int32 Last = Transforms.Num() - 1;
	if (Index != Last)
	{
		Transforms[Index] = Transforms[Last];
		Velocities[Index] = Velocities[Last];
		Ages[Index] = Ages[Last];
	}

	Transforms.Pop();
	Velocities.Pop();
	Ages.Pop();
}