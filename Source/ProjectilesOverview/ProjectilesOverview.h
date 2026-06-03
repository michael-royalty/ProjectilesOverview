// Copyright Michael Royalty. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraDataChannelAccessor.h"

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

struct FTraceProjectileDataChannelWriter : public FNDCWriterBase
{
	// This macro gives us a fast data channel writer
	NDCVarWriter(FNiagaraPosition, MuzzleLocation);
	NDCVarWriter(FVector4, ProjectileVelocityAndLifespan);
};