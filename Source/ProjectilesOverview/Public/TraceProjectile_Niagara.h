// Copyright Michael Royalty. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraDataChannelAccessor.h"
#include "TraceProjectile_Niagara.generated.h"

struct FTraceProjectileDataChannelWriter : public FNDCWriterBase
{
	// This macro gives us a fast data channel writer
	NDCVarWriter(FNiagaraPosition, MuzzleLocation);
	NDCVarWriter(FVector4, ProjectileVelocityAndLifespan);
};

UCLASS()
class PROJECTILESOVERVIEW_API ATraceProjectile_Niagara : public AActor
{
	GENERATED_BODY()
	
public:	

	ATraceProjectile_Niagara();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX", meta = (AllowPrivateAccess = "true"))
	UNiagaraComponent* NiagaraComponent;

	UFUNCTION(BlueprintCallable, Category = "Tracehit Projectile")
	void CreateProjectile(
		UNiagaraDataChannelAsset* DataChannelAsset,
		const TArray<FVector>& MuzzleLocations,
		const TArray<FVector>& MuzzleDirections,
		float MuzzleVelocity,
		int32 Count,
		float ConeHalfAngle);
};
