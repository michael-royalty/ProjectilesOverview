// Copyright Michael Royalty. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraDataChannelAccessor.h"
#include "DataDrivenProjectile_Niagara.generated.h"

/**
 *
 */

struct FDataDrivenProjectileDataChannelReader : public FNDCReaderBase
{
	// This macro gives us a fast data channel reader
	NDCVarReader(int32, NiagaraIndex);
};

struct FDataDrivenProjectileDataChannelWriter : public FNDCWriterBase
{
	// This macro gives us a fast data channel writer
	NDCVarWriter(int32, NiagaraIndex);
	NDCVarWriter(FNiagaraPosition, MuzzleLocation);
	NDCVarWriter(FVector, ProjectileVelocity);
};

UCLASS()
class PROJECTILESOVERVIEW_API ADataDrivenProjectile_Niagara : public AActor
{
	GENERATED_BODY()

	ADataDrivenProjectile_Niagara();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX", meta = (AllowPrivateAccess = "true"))
	UNiagaraComponent* NiagaraComponent;

	UFUNCTION(BlueprintCallable, Category = "Data Driven Projectile")
	void CreateProjectile(
		UNiagaraDataChannelAsset* DataChannelAsset,
		const TArray<FVector>& MuzzleLocations,
		const TArray<FVector>& MuzzleDirections,
		float MuzzleVelocity,
		int32 Count,
		float ConeHalfAngle);

	UFUNCTION(BlueprintCallable, Category = "Data Driven Projectile")
	void UpdateProjectiles(float DeltaSeconds);

	void RetireProjectile(int32 Index);

	TArray<int32> GetNiagaraIndexesFromPool(int32 NumIndexes);

	UFUNCTION(BlueprintCallable, Category = "Data Driven Projectile")
	void ReadDeadParticles(const FNiagaraDataChannelUpdateContext& Context);

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup", meta = (ExposeOnSpawn = true))
	AActor* Player;

private:

	TArray<FVector> Locations;
	TArray<FVector> Velocities;
	TArray<float> Ages;

	TArray<int32> PooledIndexes;
	TArray<int32> NiagaraIndexes;
	int32 NextNiagaraIndex = 0;
};
