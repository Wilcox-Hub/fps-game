#include "RidgefireSynthWave.h"

URidgefireSynthWave::URidgefireSynthWave(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SampleRate = 44100;
	NumChannels = 1;
}
