// Copyright Michael Royalty. All Rights Reserved.

#include "TurretManager.h"

// Sets default values
ATurretManager::ATurretManager()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATurretManager::BeginPlay()
{
    Super::BeginPlay();

}

// Called every frame
void ATurretManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    DetermineShots();

}

void ATurretManager::DetermineShots()
{
    // cache last valid index of muzzle array
    const int32 HighestIndex = MuzzleLocations.Num() - 1;

    // pull the scheduled fire time for that muzzle
    double ShotTime = FireTimes[CurrentFiringIndex];

    // snapshot current world time once before the loop
    const double GameTime = GetWorld()->GetTimeSeconds();

    // process every overdue shot, advancing FiringIndex each iteration
    while (GameTime > ShotTime)
    {
        // --- then_0: compute predicted muzzle position at fire time and queue it ---
        // direction stored from previous frame (set at loop top before first iteration
        // the blueprint sets Direction = MuzzleDirections[ResolvedIndex] as then_0 first op)
        const FVector Direction = MuzzleDirections[CurrentFiringIndex];

        // elapsed time since the scheduled shot * velocity gives displacement along barrel axis
        const FVector PredictedOffset = Direction * (GameTime - ShotTime) * MuzzleVelocity;

        // world-space predicted spawn location for this shot
        const FVector PredictedLocation = PredictedOffset + MuzzleLocations[CurrentFiringIndex];

        QueueProjectile(PredictedLocation, Direction);

        // set next fire time for this muzzle slot
        FireTimes[CurrentFiringIndex] += SecondsPerShot;

        // --- then_1: advance firing index, wrapping at HighestIndex ---
        if (CurrentFiringIndex < HighestIndex)
        {
            ++CurrentFiringIndex;
        }
        else
        {
            CurrentFiringIndex = 0;
        }

        // --- then_2: re-resolve muzzle and pull its next scheduled time ---
        ShotTime = FireTimes[CurrentFiringIndex];
    }

    ProjectileManager->BatchCreateProjectiles(
        QueuedProjectileCount,
        QueuedProjectileLocations,
        QueuedProjectileDirections,
        MuzzleVelocity,
        ProjectilesPerShot,
        ConeHalfAngle);

    QueuedProjectileCount = 0;
}

void ATurretManager::QueueProjectile(FVector Location, FVector Direction)
{
    if (QueuedProjectileTotal <= QueuedProjectileCount)
    {
        // Expand arrays
        QueuedProjectileTotal += FMath::Max(QueuedProjectileCount / 2, 16);
        QueuedProjectileLocations.SetNumUninitialized(QueuedProjectileTotal);
        QueuedProjectileDirections.SetNumUninitialized(QueuedProjectileTotal);

    }

    QueuedProjectileLocations[QueuedProjectileCount] = Location;
    QueuedProjectileDirections[QueuedProjectileCount] = Direction;

    QueuedProjectileCount++; // By tracking the count, we're also tracking the next index to write to.
}
