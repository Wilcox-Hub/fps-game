#if WITH_EDITOR
#include "Tickable.h"
#include "RidgefireCharacterMovement.h"
#include "RidgefireGameMode.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "Variant_Shooter/ShooterPlayerController.h"
#include "Variant_Shooter/AI/ShooterNPC.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "InputKeyEventArgs.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "HAL/PlatformTime.h"
#include "TimerManager.h"

// Explicit opt-in Editor-only two-process fixture; never active in Shipping or ordinary play.
class FPressureNetworkFixture : public FTickableGameObject
{
 bool Done=false,Placed=false,SawSlide=false,SawAim=false;
 int32 Stage=0;double At=0;FVector Start=FVector::ZeroVector;
public:
 virtual TStatId GetStatId() const override {RETURN_QUICK_DECLARE_CYCLE_STAT(FPressureNetworkFixture,STATGROUP_Tickables);}
 virtual bool IsTickable() const override {return !Done && FParse::Param(FCommandLine::Get(),TEXT("RidgefirePressureNetworkFixture"));}
 virtual void Tick(float DT) override
 {
  UWorld* W=nullptr;
  if(GEngine)for(const auto& C:GEngine->GetWorldContexts())if(C.World() && C.World()->IsGameWorld())W=C.World();
  if(!W)return;
  auto* Local=Cast<AShooterPlayerController>(W->GetFirstPlayerController());
  if(!Local || !Local->GetPawn())return;
  const double Now=FPlatformTime::Seconds();
  if(W->GetNetMode()==NM_ListenServer)
  {
   auto* Mode=W->GetAuthGameMode<ARidgefireGameMode>();
   AShooterPlayerController* Remote=nullptr;
   for(FConstPlayerControllerIterator It=W->GetPlayerControllerIterator();It;++It)
    if(auto* P=Cast<AShooterPlayerController>(It->Get());P && !P->IsLocalController())Remote=P;
   if(!Mode || !Remote || !Remote->GetPawn())return;
   if(Mode->IsArmoryOpen())
   {
    if(Mode->GetSetupStage()==ERidgefireSetupStage::Secondary){Mode->SelectSetupSecondary(Local,2);Mode->SelectSetupSecondary(Remote,2);}
    UGameplayStatics::SetGlobalTimeDilation(W,8);return;
   }
   UGameplayStatics::SetGlobalTimeDilation(W,1);
   if(Mode->GetWave()<1)return;
   for(TActorIterator<AShooterNPC> It(W);It;++It){It->SetActorTickEnabled(false);It->GetCharacterMovement()->DisableMovement();if(It->GetController())It->GetController()->SetActorTickEnabled(false);}
   auto* P=Cast<AShooterCharacter>(Remote->GetPawn());if(!P)return;
   auto* M=CastChecked<URidgefireCharacterMovement>(P->GetCharacterMovement());
   if(!Placed)
   {
    FActorSpawnParameters Params;Params.Name=TEXT("PressureNetworkFloor");
    auto* Floor=W->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),P->GetActorLocation()+FVector(0,0,2000),FRotator::ZeroRotator,Params);
    if(!Floor)return;
    Floor->SetReplicates(true);Floor->bAlwaysRelevant=true;
    Floor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
    Floor->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));
    Floor->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));Floor->GetStaticMeshComponent()->SetIsReplicated(true);
    Floor->SetActorScale3D(FVector(60,60,0.2f));Floor->SetReplicateMovement(true);
    P->SetActorLocation(Floor->GetActorLocation()+FVector(-800,0,10+P->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()+2));
    P->GetCharacterMovement()->SetMovementMode(MOVE_Walking);Start=P->GetActorLocation();Placed=true;At=Now;
    Floor->ForceNetUpdate();P->ForceNetUpdate();
    const TWeakObjectPtr<AShooterCharacter> Pawn(P);FTimerHandle Placement;
    W->GetTimerManager().SetTimer(Placement,FTimerDelegate::CreateWeakLambda(Remote,[Remote,Pawn](){if(Pawn.IsValid())Remote->ClientSetLocation(Pawn->GetActorLocation(),FRotator::ZeroRotator);}),1.5f,false);return;
   }
   SawSlide|=M->bSliding;SawAim|=M->bSliding && M->bAimRequested;
   if(SawSlide && SawAim && !M->bCrouchHeld && !M->bSliding && !P->bIsCrouched && FVector::Dist2D(Start,P->GetActorLocation())>150)
   {UE_LOG(LogTemp,Display,TEXT("IRON SUN PRESSURE NETWORK PASS host: remote authoritative slide/ADS/release displacement=%.1f"),FVector::Dist2D(Start,P->GetActorLocation()));Done=true;}
   else if(Now-At>25){UE_LOG(LogTemp,Error,TEXT("IRON SUN PRESSURE NETWORK FAIL host slide=%d aim=%d crouch=%d distance=%.1f"),SawSlide,SawAim,P->bIsCrouched,FVector::Dist2D(Start,P->GetActorLocation()));Done=true;}
  }
  else if(W->GetNetMode()==NM_Client)
  {
   auto* P=Cast<AShooterCharacter>(Local->GetPawn());if(!P || Local->GetRidgefireRunState().bArmoryOpen)return;
   auto* M=CastChecked<URidgefireCharacterMovement>(P->GetCharacterMovement());
   bool FloorReady=false;for(TActorIterator<AStaticMeshActor> It(W);It;++It)if(FMath::IsNearlyEqual(It->GetActorScale3D().X,60.0f) && P->GetActorLocation().Z>It->GetActorLocation().Z)FloorReady=true;
   if(!FloorReady || (Stage==0 && !M->IsMovingOnGround()))return;
   if(Stage==0){Local->SetControlRotation(FRotator::ZeroRotator);M->RequestSprint(true);At=Now;Start=P->GetActorLocation();Stage=1;}
   if(Stage==1)
   {
    P->DoMove(0,1);
    if(Now-At>0.4 && M->Velocity.Size2D()>=M->MaxWalkSpeed*1.28f)
    {Local->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::C,IE_Pressed,1));Stage=2;At=Now;}
    else if(Now-At>8){UE_LOG(LogTemp,Error,TEXT("IRON SUN PRESSURE NETWORK FAIL client sprint velocity %.1f"),M->Velocity.Size2D());Done=true;}
   }
   else if(Stage==2)
   {
    SawSlide|=P->IsRidgefireSliding();
    if(Now-At>0.08)Local->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::RightMouseButton,IE_Pressed,1));
    SawAim|=P->IsRidgefireSliding() && P->IsRidgefireAiming();
    if(Now-At>0.30)
    {Local->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::C,IE_Released,0));Local->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::RightMouseButton,IE_Released,0));Stage=3;At=Now;}
   }
   else if(Stage==3 && Now-At>0.5)
   {
    const bool Passed=SawSlide && SawAim && !P->bIsCrouched && !P->IsRidgefireSliding() && FVector::Dist2D(Start,P->GetActorLocation())>150;
    if(Passed){UE_LOG(LogTemp,Display,TEXT("IRON SUN PRESSURE NETWORK PASS client: predicted slide/ADS/release distance=%.1f"),FVector::Dist2D(Start,P->GetActorLocation()));}else{UE_LOG(LogTemp,Error,TEXT("IRON SUN PRESSURE NETWORK FAIL client: slide=%d aim=%d crouch=%d distance=%.1f"),SawSlide,SawAim,P->bIsCrouched,FVector::Dist2D(Start,P->GetActorLocation()));}Done=true;
   }
  }
 }
};
static FPressureNetworkFixture GPressureNetworkFixture;
#endif


