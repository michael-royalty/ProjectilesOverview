// Copyright Michael Royalty. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TurretBase.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraDataChannel.h"
#include "DataDrivenProjectile_Niagara.generated.h"

/**
 *
 */

UCLASS()
class PROJECTILESOVERVIEW_API ADataDrivenProjectile_Niagara : public ATurretBase
{
	GENERATED_BODY()

	ADataDrivenProjectile_Niagara();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX", meta = (AllowPrivateAccess = "true"))
	UNiagaraComponent* NiagaraComponent;

	bool BatchCreateProjectiles_Implementation(
		const int ProjectileCount,
		const TArray<FVector>& MuzzleLocations,
		const TArray<FVector>& MuzzleDirections,
		float MuzzleVelocity,
		int32 Count,
		float ConeHalfAngle) override;

	UFUNCTION(BlueprintCallable, Category = "Data Driven Projectile")
	void UpdateProjectiles(float DeltaSeconds);

	void RetireProjectile(int32 Index);

	TArrayView<int32> GetNiagaraIndexesFromPool(int32 NumIndexes);

	UFUNCTION(BlueprintCallable, Category = "Data Driven Projectile")
	void ReadDeadParticles(const FNiagaraDataChannelUpdateContext& Context);

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup", meta = (ExposeOnSpawn = true))
	AActor* Player;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	UNiagaraDataChannelAsset* DataChannelAsset;

private:

	TArray<FVector> Locations;
	TArray<FVector> Velocities;
	TArray<float> Ages;

	TArray<int32> PooledIndexes;
	TArray<int32> NiagaraIndexes;
	int32 NextNiagaraIndex = 0;

	TArray<int32> PooledNiagaraIndexes;
	int32 PooledNiagaraIndexesCount = 0;
};
