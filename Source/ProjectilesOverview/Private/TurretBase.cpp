// Copyright Michael Royalty. All Rights Reserved.

#include "TurretBase.h"

// Sets default values
ATurretBase::ATurretBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATurretBase::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ATurretBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool ATurretBase::BatchCreateProjectiles_Implementation(
	const int32 ProjectileCount,
	const TArray<FVector>& MuzzleLocations,
	const TArray<FVector>& MuzzleDirections,
	float MuzzleVelocity,
	int32 Count,
	float ConeHalfAngle)
{
	return false;
}