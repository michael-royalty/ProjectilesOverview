// Copyright Michael Royalty. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DataDrivenProjectile_ISM.generated.h"

/**
 *
 */
UCLASS()
class PROJECTILESOVERVIEW_API ADataDrivenProjectile_ISM : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADataDrivenProjectile_ISM();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX", meta = (AllowPrivateAccess = "true"))
	UInstancedStaticMeshComponent* ISMComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup", meta = (ExposeOnSpawn = true))
	AActor* Player;

public:

	UFUNCTION(BlueprintCallable, Category = "Pool Manager")
	void CreateProjectile(
		const TArray<FVector>& MuzzleLocations,
		const TArray<FVector>& MuzzleDirections,
		float MuzzleVelocity,
		int32 Count,
		float ConeHalfAngle);

	UFUNCTION(BlueprintCallable, Category = "Pool Manager")
	void UpdateProjectiles(float DeltaSeconds);

	void RetireProjectile(int32 Index);

private:

	TArray<FTransform> Transforms;
	TArray<FVector> Velocities;
	TArray<float> Ages;
};
