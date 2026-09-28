#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "RidgefireHUD.generated.h"

UCLASS()
class TP_FIRSTPERSON_API ARidgefireHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
};
