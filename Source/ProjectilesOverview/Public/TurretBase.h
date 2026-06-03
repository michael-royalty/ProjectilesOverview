// Copyright Michael Royalty. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TurretBase.generated.h"

UCLASS()
class PROJECTILESOVERVIEW_API ATurretBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATurretBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool BatchCreateProjectiles(
		const int32 ProjectileCount,
		const TArray<FVector>& MuzzleLocations,
		const TArray<FVector>& MuzzleDirections,
		float MuzzleVelocity,
		int32 Count,
		float ConeHalfAngle);

};
