// Copyright Michael Royalty. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TurretBase.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraDataChannel.h"
#include "TraceProjectile_Niagara.generated.h"

UCLASS()
class PROJECTILESOVERVIEW_API ATraceProjectile_Niagara : public ATurretBase
{
	GENERATED_BODY()

public:

	ATraceProjectile_Niagara();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX", meta = (AllowPrivateAccess = "true"))
	UNiagaraComponent* NiagaraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	UNiagaraDataChannelAsset* DataChannelAsset;

	bool BatchCreateProjectiles_Implementation(
		const int ProjectileCount,
		const TArray<FVector>& MuzzleLocations,
		const TArray<FVector>& MuzzleDirections,
		float MuzzleVelocity,
		int32 Count,
		float ConeHalfAngle);
};
