// Copyright Michael Royalty. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TurretBase.h"
#include "TurretManager.generated.h"

UCLASS()
class PROJECTILESOVERVIEW_API ATurretManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATurretManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:

	UFUNCTION(BlueprintCallable)
	void DetermineShots();
	void QueueProjectile(FVector Location, FVector Direction);

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ShotsPerSecond;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MuzzleVelocity;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 ProjectilesPerShot;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ConeHalfAngle;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float StartDelay;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float SecondsPerShot;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 TotalTurrets;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FVector> MuzzleLocations;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FVector> MuzzleDirections;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<float> FireTimes;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSubclassOf<ATurretBase> ProjectileManagerClass;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<ATurretBase> ProjectileManager;

private:
	int32 CurrentFiringIndex = 0;
	TArray<FVector> QueuedProjectileDirections;
	TArray<FVector> QueuedProjectileLocations;
	int32 QueuedProjectileCount = 0;
	int32 QueuedProjectileTotal = 0;

};
