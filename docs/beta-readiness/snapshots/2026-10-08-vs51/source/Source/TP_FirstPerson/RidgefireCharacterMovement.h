#pragma once
#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "RidgefireCharacterMovement.generated.h"

// Intent travels with character moves, including corrections/replays, rather than separate RPCs.
UCLASS()
class TP_FIRSTPERSON_API URidgefireCharacterMovement : public UCharacterMovementComponent
{
    GENERATED_BODY()
    friend class FPressurePassChecks;
public:
    bool bCrouchHeld=false, bSprintRequested=false, bAimRequested=false;
    bool bSliding=false, bPreviousCrouchHeld=false;
    float SlideRecovery=0;
    void RequestSprint(bool bHeld);
    void RequestCrouch(bool bHeld);
    void RequestAim(bool bHeld);
    bool CanUseCombatMovement() const;
    virtual float GetMaxSpeed() const override;
    virtual void UpdateFromCompressedFlags(uint8 Flags) override;
    virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
    virtual void CalcVelocity(float DeltaTime,float Friction,bool bFluid,float BrakingDeceleration) override;
    virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
};

class FRidgefireSavedMove : public FSavedMove_Character
{
public:
    using Super=FSavedMove_Character;
    bool Held=false,Sprint=false,Aim=false,Sliding=false,PreviousHeld=false;
    float Recovery=0;
    virtual void Clear() override;
    virtual uint8 GetCompressedFlags() const override;
    virtual bool CanCombineWith(const FSavedMovePtr& NewMove,ACharacter* Character,float MaxDelta) const override;
    virtual void SetMoveFor(ACharacter* C,float Delta,const FVector& Accel,FNetworkPredictionData_Client_Character& Data) override;
    virtual void PrepMoveFor(ACharacter* C) override;
};
class FRidgefirePredictionData : public FNetworkPredictionData_Client_Character
{
public:
    explicit FRidgefirePredictionData(const UCharacterMovementComponent& M):FNetworkPredictionData_Client_Character(M){}
    virtual FSavedMovePtr AllocateNewMove() override {return FSavedMovePtr(new FRidgefireSavedMove());}
};

