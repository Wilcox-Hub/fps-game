#include "RidgefireCharacterMovement.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "Variant_Shooter/ShooterPlayerController.h"

bool URidgefireCharacterMovement::CanUseCombatMovement() const
{
    const auto* P=Cast<AShooterCharacter>(CharacterOwner);
    const auto* PC=P ? Cast<AShooterPlayerController>(P->GetController()) : nullptr;
    if(!P || P->IsDead() || P->IsCarryingRidgefirePlayer() || !PC || PC->IsPaused() || PC->IsPersonalPerksOpen()) return false;
    const auto& State=PC->GetRidgefireRunState();
    return !State.bArmoryOpen && !State.bPersonalArsenalOpen && !State.bRunOver;
}
void URidgefireCharacterMovement::RequestSprint(bool Held)
{
    bSprintRequested=Held && CanUseCombatMovement();
    if(bSprintRequested) {bCrouchHeld=false;bSliding=false;bWantsToCrouch=false;bAimRequested=false;CharacterOwner->UnCrouch();}
}
void URidgefireCharacterMovement::RequestCrouch(bool Held)
{
    if(Held && !CanUseCombatMovement()) return;
    bCrouchHeld=Held;
    bWantsToCrouch=Held;
    if(Held) bSprintRequested=false;
    else {if(bSliding) SlideRecovery=0.22f;bSliding=false;CharacterOwner->UnCrouch();}
}
void URidgefireCharacterMovement::RequestAim(bool Held)
{
    bAimRequested=Held && CanUseCombatMovement();
    if(bAimRequested) bSprintRequested=false;
}
float URidgefireCharacterMovement::GetMaxSpeed() const
{
    const auto* P=Cast<AShooterCharacter>(CharacterOwner);
    const float Perk=P ? P->GetPersonalPerkModifiers().MoveSpeed : 1;
    if(P && P->IsCarryingRidgefirePlayer()) return FMath::Min(Super::GetMaxSpeed()*Perk,P->GetRidgefireCarrySpeed());
    if(bSliding) return FMath::Max(MaxWalkSpeed*Perk,Velocity.Size2D());
    const float Factor=bAimRequested ? 0.75f : bSprintRequested && !IsCrouching() ? 1.5f : 1;
    return Super::GetMaxSpeed()*Perk*Factor;
}
void URidgefireCharacterMovement::UpdateFromCompressedFlags(uint8 Flags)
{
    Super::UpdateFromCompressedFlags(Flags);
    bCrouchHeld=(Flags & FSavedMove_Character::FLAG_Custom_0)!=0;
    bSprintRequested=(Flags & FSavedMove_Character::FLAG_Custom_1)!=0;
    bAimRequested=(Flags & FSavedMove_Character::FLAG_Custom_2)!=0;
}
void URidgefireCharacterMovement::UpdateCharacterStateBeforeMovement(float DT)
{
    SlideRecovery=FMath::Max(0.0f,SlideRecovery-DT);
    if(!CanUseCombatMovement()) {bSliding=false;bCrouchHeld=false;bSprintRequested=false;bAimRequested=false;}
    if(bSliding && (!bCrouchHeld || !IsMovingOnGround() || Velocity.Size2D()<MaxWalkSpeed*0.48f))
    {bSliding=false;SlideRecovery=0.22f;}
    if(bCrouchHeld && !bPreviousCrouchHeld && SlideRecovery<=0 && IsMovingOnGround()
       && Velocity.Size2D()>=MaxWalkSpeed*1.28f)
    {bSliding=true;bSprintRequested=false;}
    if(bSprintRequested && !bSliding) bCrouchHeld=false;
    bWantsToCrouch=bCrouchHeld;
    bPreviousCrouchHeld=bCrouchHeld;
    Super::UpdateCharacterStateBeforeMovement(DT);
}
void URidgefireCharacterMovement::CalcVelocity(float DT,float Friction,bool Fluid,float Brake)
{
    if(!bSliding || !IsMovingOnGround()) {Super::CalcVelocity(DT,Friction,Fluid,Brake);return;}
    // No launch impulse or speed boost. Native walking still handles floor, capsule sweeps and walls.
    const float Speed=FMath::Max(0.0f,Velocity.Size2D()-650.0f*DT);
    FVector Direction=Velocity.GetSafeNormal2D();
    if(!Acceleration.IsNearlyZero())
    {
        const float Desired=Acceleration.GetSafeNormal2D().Rotation().Yaw;
        const float Turn=FMath::Clamp(FMath::FindDeltaAngleDegrees(Direction.Rotation().Yaw,Desired),-35.0f*DT,35.0f*DT);
        Direction=Direction.RotateAngleAxis(Turn,FVector::UpVector);
    }
    Velocity=Direction*Speed;
}
FNetworkPredictionData_Client* URidgefireCharacterMovement::GetPredictionData_Client() const
{
    if(!ClientPredictionData) const_cast<URidgefireCharacterMovement*>(this)->ClientPredictionData=new FRidgefirePredictionData(*this);
    return ClientPredictionData;
}
void FRidgefireSavedMove::Clear(){Super::Clear();Held=Sprint=Aim=Sliding=PreviousHeld=false;Recovery=0;}
uint8 FRidgefireSavedMove::GetCompressedFlags() const
{
    return Super::GetCompressedFlags() | (Held ? FLAG_Custom_0 : 0) | (Sprint ? FLAG_Custom_1 : 0) | (Aim ? FLAG_Custom_2 : 0);
}
bool FRidgefireSavedMove::CanCombineWith(const FSavedMovePtr& Move,ACharacter* C,float Delta) const
{
    const auto* Other=static_cast<const FRidgefireSavedMove*>(Move.Get());
    if(Sliding || Other->Sliding || Held!=Other->Held || Sprint!=Other->Sprint || Aim!=Other->Aim || Recovery>0 || Other->Recovery>0) return false;
    return Super::CanCombineWith(Move,C,Delta);
}
void FRidgefireSavedMove::SetMoveFor(ACharacter* C,float Delta,const FVector& Accel,FNetworkPredictionData_Client_Character& Data)
{
    Super::SetMoveFor(C,Delta,Accel,Data);
    const auto* M=CastChecked<URidgefireCharacterMovement>(C->GetCharacterMovement());
    Held=M->bCrouchHeld;Sprint=M->bSprintRequested;Aim=M->bAimRequested;Sliding=M->bSliding;PreviousHeld=M->bPreviousCrouchHeld;Recovery=M->SlideRecovery;
}
void FRidgefireSavedMove::PrepMoveFor(ACharacter* C)
{
    Super::PrepMoveFor(C);
    auto* M=CastChecked<URidgefireCharacterMovement>(C->GetCharacterMovement());
    M->bCrouchHeld=Held;M->bSprintRequested=Sprint;M->bAimRequested=Aim;M->bSliding=Sliding;M->bPreviousCrouchHeld=PreviousHeld;M->SlideRecovery=Recovery;
}
