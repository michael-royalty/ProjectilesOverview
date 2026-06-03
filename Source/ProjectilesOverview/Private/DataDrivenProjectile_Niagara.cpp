// Copyright Michael Royalty. All Rights Reserved.

#include "DataDrivenProjectile_Niagara.h"
#include "../ProjectilesOverview.h"
#include "NiagaraDataChannelHandler.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "NiagaraTypes.h"
#include "Kismet/KismetMathLibrary.h" // Used for randomized cone angle
#include <numeric> // Used for iota

ADataDrivenProjectile_Niagara::ADataDrivenProjectile_Niagara()
{
	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	NiagaraComponent->SetupAttachment(RootComponent);
	NiagaraComponent->bAutoActivate = true;
}

bool ADataDrivenProjectile_Niagara::BatchCreateProjectiles_Implementation(
	const int ProjectileCount,
	const TArray<FVector>& MuzzleLocations,
	const TArray<FVector>& MuzzleDirections,
	float MuzzleVelocity,
	int32 Count,
	float ConeHalfAngle)
{
	if (ProjectileCount <= 0 || Count <= 0 || MuzzleLocations.Num() == 0 || MuzzleDirections.Num() == 0)
		return false;

	// Replace this age with your own max projectile age
	const float MaxAge = 3.0f;
	const float TargetAge = GetWorld()->GetTimeSeconds() + MaxAge;

	// Used for direct array writing to Niagara
	const FName KillWriteName = ("KillAges");

	const int NumShots = ProjectileCount;
	const int32 TotalProjectiles = ProjectileCount * Count;

	UNiagaraDataChannel* DataChannel = nullptr;

	if (DataChannelAsset)
	{
		DataChannel = DataChannelAsset->Get();
	}

	FNiagaraDataChannelSearchParameters SearchParams;
	SearchParams.Location = FVector::ZeroVector;

	FDataDrivenProjectileDataChannelWriter Writer;

	if (!Writer.BeginWrite(this, DataChannel, SearchParams, TotalProjectiles,
		/* bVisibleToGame = */ false,
		/* bVisibleToCPU  = */ true,
		/* bVisibleToGPU  = */ false))
	{
		UE_LOG(LogTemp, Warning, TEXT("WriteDestructionDataChannel: Failed to begin write."));
		return false;
	}

	const int32 NumToReserve = Locations.Num() + TotalProjectiles;
	Locations.Reserve(NumToReserve);
	Velocities.Reserve(NumToReserve);
	Ages.Reserve(NumToReserve);

	const TArrayView<int32> NewIndexes = GetNiagaraIndexesFromPool(ProjectileCount * Count);

	for (int ShotIndex = 0; ShotIndex < ProjectileCount; ++ShotIndex)
	{
		const FVector MuzzleLocation = MuzzleLocations[ShotIndex];
		const FVector MuzzleDirection = MuzzleDirections[ShotIndex];

		for (int BurstIndex = ShotIndex * Count; BurstIndex < (ShotIndex * Count + Count); ++BurstIndex)
		{
			// Randomize firing angle
			const FVector ProjectileDirection = UKismetMathLibrary::RandomUnitVectorInConeInDegrees(MuzzleDirection, ConeHalfAngle);

			// Set velocity based on the randomized firing angle for this projectile
			const FVector ProjectileVelocity = ProjectileDirection * MuzzleVelocity;

			const int32 NiagaraArrayIndex = NewIndexes[BurstIndex];

			Locations.Add(MuzzleLocation);
			Velocities.Add(ProjectileVelocity);
			Ages.Add(TargetAge);

			// Initialize the KillAges value
			// Uses FName lookup every time, doesn't batch
			UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayFloatValue(
				NiagaraComponent,
				KillWriteName,
				NiagaraArrayIndex,
				MaxAge, // This age is always relative
				true); // Be willing to expand the array

			// Write data to Niagara Data Channel
			Writer.WriteMuzzleLocation(BurstIndex, MuzzleLocation);
			Writer.WriteProjectileVelocity(BurstIndex, ProjectileVelocity);
			Writer.WriteNiagaraIndex(BurstIndex, NiagaraArrayIndex);
		}
	}

	Writer.EndWrite(); // Always call this if not using FNDCScopedWriter

	NiagaraIndexes.Append(NewIndexes);

	return true;
}

void ADataDrivenProjectile_Niagara::UpdateProjectiles(float DeltaSeconds)
{
	if (Locations.IsEmpty())
		return;

	TArray<int32> IndexesToRemove;

	const int32 HighestIndex = Locations.Num() - 1;
	const int32 NumProjectiles = Locations.Num();

	const float CurrentTime = GetWorld()->GetTimeSeconds();

	// Replace this radius with your own projectile radius
	const FCollisionShape SphereCollision = FCollisionShape::MakeSphere(15.0f);
	FCollisionQueryParams QueryParams;
	const FName KillWriteName = ("KillAges");

	struct FProjectileUpdateContext
	{
		TArray<int32> LocalRetireIndexes;
		FHitResult Hit;
	};

	TArray<FProjectileUpdateContext> Contexts;

	ParallelForWithTaskContext<FProjectileUpdateContext>(
		TEXT("UpdateProjectilesNiagara"),
		Contexts,
		NumProjectiles,
		/* Adjust MinBatchSize based on performance */ 16,
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

			const FVector TraceStart = Locations[i];
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
				const float KillAge = 3.0f - (Ages[i] - (CurrentTime + Context.Hit.Time * DeltaSeconds));

				UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayFloatValue(
					NiagaraComponent,
					KillWriteName,
					NiagaraIndexes[i],
					KillAge,
					false
				);

				Context.LocalRetireIndexes.Add(i);
				return;
			}

			Locations[i] = TraceEnd;
		},
		EParallelForFlags::None
	);

	// Merge retire lists back on the game thread
	// Loop in reverse order so we can safely pop and swap
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
}

void ADataDrivenProjectile_Niagara::RetireProjectile(int32 Index)
{
	// Pop and swap old indexes as they're retired

	const int32 Last = Locations.Num() - 1;
	if (Index != Last)
	{
		Locations[Index] = Locations[Last];
		Velocities[Index] = Velocities[Last];
		Ages[Index] = Ages[Last];
		NiagaraIndexes[Index] = NiagaraIndexes[Last];
	}

	Locations.Pop();
	Velocities.Pop();
	Ages.Pop();
	NiagaraIndexes.Pop();
}

TArrayView<int32> ADataDrivenProjectile_Niagara::GetNiagaraIndexesFromPool(int32 NumIndexes)
{
	const int32 NumAbovePool = FMath::Max(NumIndexes - PooledNiagaraIndexesCount, 0);

	if (NumAbovePool > 0)
	{
		if (NumIndexes > PooledNiagaraIndexes.Num())
			PooledNiagaraIndexes.SetNumUninitialized(NumIndexes);

		std::iota(PooledNiagaraIndexes.GetData() + PooledNiagaraIndexesCount,
			PooledNiagaraIndexes.GetData() + NumIndexes,
			NextNiagaraIndex);

		PooledNiagaraIndexesCount = 0;
		NextNiagaraIndex += NumAbovePool;
	}
	else
	{
		PooledNiagaraIndexesCount -= NumIndexes;
	}

	return TArrayView<int32>(PooledNiagaraIndexes.GetData() + PooledNiagaraIndexesCount, NumIndexes);
}

void ADataDrivenProjectile_Niagara::ReadDeadParticles(const FNiagaraDataChannelUpdateContext& Context)
{
	// Reclaim the indexes from Niagara as they're freed

	// The reader we've got through the subscription is already open. Go through some initialization steps to access the data from it.
	UNiagaraDataChannelReader* ContextReader = Context.Reader;

	if (!ContextReader)
		return;

	const UNiagaraDataChannelHandler* Handler = ContextReader->Owner;

	if (!Handler)
		return;

	const UNiagaraDataChannel* DataChannel = Handler->GetDataChannel();

	if (!DataChannel)
		return;

	// Uses a data reader defined by macro
	FDataDrivenProjectileDataChannelReader Reader;
	FNiagaraDataChannelSearchParameters SearchParams;

	if (!Reader.BeginRead(this, DataChannel, SearchParams, false))
		return;

	const int32 TotalNDCCount = Reader.Num();

	const int32 NewNDCCount = Context.NewElementCount;
	const int32 StartNDCIndex = Context.FirstNewDataIndex;
	const int32 EndNDCIndex = Context.LastNewDataIndex;

	const int32 NewPooledIndexesCount = NewNDCCount + PooledNiagaraIndexesCount;

	if (NewPooledIndexesCount > PooledNiagaraIndexes.Num())
		PooledNiagaraIndexes.SetNumUninitialized(NewPooledIndexesCount);

	for (int i = StartNDCIndex; i <= EndNDCIndex; ++i)
	{
		int32 NiagaraIndex;

		if (Reader.ReadNiagaraIndex(i, NiagaraIndex))
		{
			const int32 Index = i + PooledNiagaraIndexesCount;
			PooledNiagaraIndexes[Index] = NiagaraIndex;
		}
	}

	Reader.EndRead();

	PooledNiagaraIndexesCount = NewPooledIndexesCount;
}