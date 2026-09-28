#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundWaveProcedural.h"
#include "RidgefireSynthWave.generated.h"

UCLASS()
class TP_FIRSTPERSON_API URidgefireSynthWave : public USoundWaveProcedural
{
	GENERATED_BODY()

public:
	URidgefireSynthWave(const FObjectInitializer& ObjectInitializer);
};
