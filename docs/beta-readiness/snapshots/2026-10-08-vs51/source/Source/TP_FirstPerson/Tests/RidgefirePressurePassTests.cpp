#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "RidgefireCharacterMovement.h"
#include "RidgefireWeaponProgression.h"
#include "RidgefireGameMode.h"
#include "Variant_Shooter/ShooterPlayerController.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "Variant_Shooter/AI/ShooterNPC.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "InputKeyEventArgs.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "HAL/PlatformTime.h"
#include "Misc/Paths.h"
#include "Misc/App.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPressurePassTest,"Ridgefire.Pressure.MovementAimAndCombat",EAutomationTestFlags::ClientContext|EAutomationTestFlags::EngineFilter)

class FPressurePassChecks
{
public:
 static float NormalFOV(AShooterCharacter* P){return P->RidgefireNormalFOV;}
 static FVector Muzzle(AShooterCharacter* P){return P->GetRidgefireMuzzleWorldLocation();}
 static int32 PendingBurst(AShooterCharacter* P){return P->RidgefireBurstRemaining;}
 static FRidgefireWeaponRoll Weapon(AShooterCharacter* P){return P->RidgefireWeapon;}
 static void Check(FAutomationTestBase* T,AShooterPlayerController* PC,AShooterCharacter* P)
 {
  auto* M=CastChecked<URidgefireCharacterMovement>(P->GetCharacterMovement());
  const FVector OriginalLocation=P->GetActorLocation(),OriginalVelocity=M->Velocity;
  const float OriginalHP=P->CurrentHP;
  const auto OriginalWeapon=P->RidgefireWeapon;
  const int32 Ammo=P->RidgefireAmmo,Reserve=P->RidgefireReserveAmmo;
  const float Standing=M->GetMaxSpeed();
  M->SetMovementMode(MOVE_Walking);M->RequestSprint(true);
  T->TestTrue(TEXT("predicted sprint increases effective movement speed"),M->GetMaxSpeed()>Standing*1.4f);
  M->Velocity=FVector(M->MaxWalkSpeed*1.5f,0,0);
  PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::C,IE_Pressed,1));
  M->UpdateCharacterStateBeforeMovement(0.016f);
  T->TestTrue(TEXT("actual hold-crouch input starts grounded momentum slide"),P->IsRidgefireSliding() && P->bIsCrouched);
  const float Before=M->Velocity.Size2D();M->Acceleration=FVector(-M->MaxAcceleration,0,0);
  M->CalcVelocity(0.1f,8,false,2048);
  T->TestTrue(TEXT("slide spends momentum rather than adding boost"),M->Velocity.Size2D()<Before);
  T->TestTrue(TEXT("reverse input cannot reverse slide"),M->Velocity.X>0 && FMath::Abs(M->Velocity.Rotation().Yaw)<=3.6f);
  const float Hip=P->GetRidgefireShotSpread();
  PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::RightMouseButton,IE_Pressed,1));
  PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::RightMouseButton,IE_Repeat,1));
  T->TestTrue(TEXT("held aim survives repeated input events while sliding"),P->IsRidgefireAiming());
  T->TestTrue(TEXT("sliding ADS is less accurate than hipfire"),P->GetRidgefireShotSpread()>Hip);
  FRidgefireSavedMove Saved;
  auto* Data=static_cast<FNetworkPredictionData_Client_Character*>(M->GetPredictionData_Client());
  Saved.SetMoveFor(P,0.016f,M->Acceleration,*Data);
  const uint8 Flags=Saved.GetCompressedFlags();
  M->bCrouchHeld=M->bAimRequested=false;M->UpdateFromCompressedFlags(Flags);
  T->TestTrue(TEXT("movement packet carries crouch and aim intent"),M->bCrouchHeld && M->bAimRequested);
  M->bSliding=false;Saved.PrepMoveFor(P);
  T->TestTrue(TEXT("correction replay restores slide state"),M->bSliding);
  PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::C,IE_Released,0));
  M->UpdateCharacterStateBeforeMovement(0.016f);
  T->TestFalse(TEXT("early release ends slide"),P->IsRidgefireSliding());
  T->TestFalse(TEXT("hold-crouch release stands when clear"),P->bIsCrouched);
  M->SlideRecovery=0;M->bPreviousCrouchHeld=false;M->Velocity=FVector(M->MaxWalkSpeed*1.5f,0,0);
  P->StartRidgefireCrouch();M->UpdateCharacterStateBeforeMovement(0.016f);
  M->Velocity=FVector(M->MaxWalkSpeed*0.3f,0,0);M->UpdateCharacterStateBeforeMovement(0.016f);
  T->TestTrue(TEXT("held late slide remains crouched after momentum spent"),P->bIsCrouched && !P->IsRidgefireSliding());
  P->StopRidgefireCrouch();M->UpdateCharacterStateBeforeMovement(0.016f);
  T->TestFalse(TEXT("late release is also hold-based"),P->bIsCrouched);
  P->StartRidgefireCrouch();M->UpdateCharacterStateBeforeMovement(0.016f);M->RequestSprint(true);M->UpdateCharacterStateBeforeMovement(0.016f);
  T->TestFalse(TEXT("sprinting clears crouched stance"),P->bIsCrouched);
  M->RequestSprint(false);M->RequestAim(false);M->bPreviousCrouchHeld=false;M->SlideRecovery=0;
  M->SetMovementMode(MOVE_Falling);M->Velocity=FVector(M->MaxWalkSpeed*1.5f,0,0);
  P->StartRidgefireCrouch();M->UpdateCharacterStateBeforeMovement(0.016f);
  T->TestFalse(TEXT("airborne crouch cannot create slide"),P->IsRidgefireSliding());
  P->StopRidgefireCrouch();M->SetMovementMode(MOVE_Walking);M->UpdateCharacterStateBeforeMovement(0.016f);
  const float NormalHip=P->GetRidgefireShotSpread();M->RequestAim(true);
  T->TestTrue(TEXT("normal ADS tightens actual trace cone"),P->GetRidgefireShotSpread()<NormalHip);
  T->TestTrue(TEXT("normal ADS reduces effective movement speed"),M->GetMaxSpeed()<Standing);
  M->RequestAim(false);
  FRidgefireWeaponRoll Light,Heavy=Light,Fast=Light;
  Heavy.Mode=ERidgefireFireMode::Phasma;Heavy.DamageScale=2.8f;Fast.RefireDelay=0.055f;
  T->TestTrue(TEXT("heavy gun has stronger actual recoil"),RidgefireWeaponProgression::RecoilDegrees(Heavy)>RidgefireWeaponProgression::RecoilDegrees(Light));
  T->TestTrue(TEXT("very high fire rate increases recoil"),RidgefireWeaponProgression::RecoilDegrees(Fast)>RidgefireWeaponProgression::RecoilDegrees(Light));
  UClass* NPCClass=LoadClass<AShooterNPC>(nullptr,TEXT("/Game/Variant_Shooter/Blueprints/AI/BP_ShooterNPC.BP_ShooterNPC_C"));
  auto* Enemy=NPCClass ? P->GetWorld()->SpawnActor<AShooterNPC>(NPCClass,OriginalLocation+FVector(0,0,5000),FRotator::ZeroRotator) : nullptr;
  if(T->TestNotNull(TEXT("production NPC pressure fixture"),Enemy))
  {
   Enemy->bRidgefireSentry=true;Enemy->SetActorTickEnabled(false);Enemy->CurrentHP=1000;
   Enemy->ConfigurePressureTactics(21,0,1,0,1);const bool Smart=Enemy->IsStrategicSentry();const int32 Pattern=Enemy->PressurePattern;
   Enemy->ConfigurePressureTactics(21,0,1,0,1);
   T->TestTrue(TEXT("same run inputs reproduce strategic assignment"),Smart==Enemy->IsStrategicSentry() && Pattern==Enemy->PressurePattern);
   int32 Changed=0,Short=0;
   for(int32 Seed=22;Seed<122;++Seed){Enemy->ConfigurePressureTactics(Seed,Seed%4,2,4,2);if(Enemy->IsStrategicSentry()!=Smart)++Changed;if(Enemy->PressurePattern==0)++Short;}
   T->TestTrue(TEXT("strategy varies with run and arena"),Changed>0 && Changed<100);
   T->TestTrue(TEXT("burst-led mix includes other patterns"),Short>50 && Short<90);
   FDamageEvent D;Enemy->TakeDamage(80,D,PC,P);
   const float Interrupted=Enemy->PressureInterruptUntil;
   T->TestTrue(TEXT("heavy hit interrupts ordinary sentry"),Interrupted>P->GetWorld()->GetTimeSeconds());
   Enemy->TakeDamage(80,D,PC,P);
   T->TestEqual(TEXT("repeat hit cannot extend interruption during refractory window"),Enemy->PressureInterruptUntil,Interrupted);
   Enemy->EnableRidgefireBrute();Enemy->PressureInterruptUntil=0;Enemy->PressureInterruptReadyAt=0;Enemy->TakeDamage(100,D,PC,P);
   T->TestEqual(TEXT("brute resists interruption"),Enemy->PressureInterruptUntil,0.0f);
   Enemy->SetActorLocation(OriginalLocation+FVector(600,0,0));Enemy->SetActorRotation(FRotator(0,180,0));Enemy->bPressureStrategic=false;
   const auto Response=P->GetCapsuleComponent()->GetCollisionResponseToChannel(ECC_Visibility);
   P->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
   P->SetActorLocation(OriginalLocation);P->CurrentHP=P->MaxHP;
   Enemy->LockPressureAim(P);Enemy->FireSentryShot(P,5);
   T->TestTrue(TEXT("stationary control is hit by production enemy trace"),P->CurrentHP<P->MaxHP);
   const float AfterHit=P->CurrentHP;P->SetActorLocation(OriginalLocation+FVector(0,450,0));Enemy->FireSentryShot(P,5);
   T->TestEqual(TEXT("moving off committed aim avoids subsequent shot without invulnerability"),P->CurrentHP,AfterHit);
   P->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility,Response);
   Enemy->Destroy();
  }
  P->SetActorLocation(OriginalLocation);P->CurrentHP=OriginalHP;M->Velocity=OriginalVelocity;M->Acceleration=FVector::ZeroVector;
  M->bPreviousCrouchHeld=false;M->SlideRecovery=0;
  P->EquipRidgefireWeapon(OriginalWeapon,Ammo,Reserve);
  UE_LOG(LogTemp,Display,TEXT("IRON SUN PRESSURE CHECK: input, momentum, steering, aim cones, packet/replay intent, mixed tactics, interruption and committed shot traces checked"));
 }
};
namespace {
class FWaitForPressure : public IAutomationLatentCommand
{
 FAutomationTestBase* T;double Started=FPlatformTime::Seconds();
public:
 explicit FWaitForPressure(FAutomationTestBase* Test):T(Test){}
 bool Update() override
 {
  if(FPlatformTime::Seconds()-Started>90){T->AddError(TEXT("Pressure fixture startup timed out"));return true;}
  UWorld* W=nullptr;for(const auto& C:GEngine->GetWorldContexts())if(C.World() && C.World()->IsGameWorld())W=C.World();
  auto* PC=W ? Cast<AShooterPlayerController>(W->GetFirstPlayerController()) : nullptr;
  auto* P=PC ? Cast<AShooterCharacter>(PC->GetPawn()) : nullptr;
  auto* Mode=W ? W->GetAuthGameMode<ARidgefireGameMode>() : nullptr;
  if(!P || !Mode)return false;
  if(Mode->IsArmoryOpen() || Mode->GetWave()<1)
  {if(Mode->GetSetupStage()==ERidgefireSetupStage::Secondary)Mode->SelectSetupSecondary(PC,2);UGameplayStatics::SetGlobalTimeDilation(W,20);return false;}
  UGameplayStatics::SetGlobalTimeDilation(W,1);
  for(TActorIterator<AShooterNPC> It(W);It;++It){It->SetActorTickEnabled(false);It->GetCharacterMovement()->DisableMovement();}
  FPressurePassChecks::Check(T,PC,P);return true;
 }
};
}
bool FPressurePassTest::RunTest(const FString&){ADD_LATENT_AUTOMATION_COMMAND(FWaitForPressure(this));return true;}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPressurePresentationTest,"Ridgefire.Pressure.VisualAimAndTimedBurst",EAutomationTestFlags::ClientContext|EAutomationTestFlags::EngineFilter)
namespace {
class FPressurePresentation : public IAutomationLatentCommand
{
 FAutomationTestBase* T;double Started=FPlatformTime::Seconds(),At=0;int32 Stage=0,Ammo=0,Reserve=0;
 FRidgefireWeaponRoll Original;FVector Location;FRotator Rotation;float InitialFOV=0,InitialDamage=0;
 TWeakObjectPtr<AShooterNPC> Target;FString Folder;
public:
 explicit FPressurePresentation(FAutomationTestBase* Test):T(Test){}
 bool Update() override
 {
  if(FPlatformTime::Seconds()-Started>100){T->AddError(TEXT("Presentation/burst fixture timed out"));if(Target.IsValid())Target->Destroy();return true;}
  UWorld* W=nullptr;for(const auto& C:GEngine->GetWorldContexts())if(C.World() && C.World()->IsGameWorld())W=C.World();
  auto* PC=W ? Cast<AShooterPlayerController>(W->GetFirstPlayerController()) : nullptr;
  auto* P=PC ? Cast<AShooterCharacter>(PC->GetPawn()) : nullptr;auto* Mode=W ? W->GetAuthGameMode<ARidgefireGameMode>() : nullptr;
  if(!P || !Mode)return false;
  if(Mode->IsArmoryOpen() || Mode->GetWave()<1)
  {if(Mode->GetSetupStage()==ERidgefireSetupStage::Secondary)Mode->SelectSetupSecondary(PC,2);UGameplayStatics::SetGlobalTimeDilation(W,20);return false;}
  UGameplayStatics::SetGlobalTimeDilation(W,1);
  for(TActorIterator<AShooterNPC> It(W);It;++It){It->SetActorTickEnabled(false);It->GetCharacterMovement()->DisableMovement();}
  const double Now=FPlatformTime::Seconds();
  if(Stage==0)
  {
   Original=FPressurePassChecks::Weapon(P);Ammo=P->GetRidgefireAmmo();Reserve=P->GetRidgefireReserveAmmo();Location=P->GetActorLocation();Rotation=PC->GetControlRotation();InitialFOV=FPressurePassChecks::NormalFOV(P);P->SetRidgefireAiming(false);
   Folder=FPaths::ProjectSavedDir()/TEXT("Automation/PressureVisual")/FDateTime::Now().ToString(TEXT("%Y%m%d-%H%M%S"));IFileManager::Get().MakeDirectory(*Folder,true);
   Stage=1;At=Now;return false;
  }
  if(Now-At<0.5)return false;
  if(Stage==1){if(P->GetFirstPersonCameraComponent()->FieldOfView<InitialFOV-0.5f && Now-At<3)return false;T->TestTrue(TEXT("hip view settles back to configured FOV"),P->GetFirstPersonCameraComponent()->FieldOfView>=InitialFOV-1);if(FApp::CanEverRender())FScreenshotRequest::RequestScreenshot(Folder/TEXT("Hip.png"),false,false);P->SetRidgefireAiming(true);Stage=2;At=Now;return false;}
  if(Stage==2)
  {
   if(P->GetFirstPersonCameraComponent()->FieldOfView>InitialFOV-10 && Now-At<3)return false;
   T->TestTrue(TEXT("sight aiming changes real camera FOV"),P->GetFirstPersonCameraComponent()->FieldOfView<InitialFOV-8);
   if(FApp::CanEverRender())FScreenshotRequest::RequestScreenshot(Folder/TEXT("ADS.png"),false,false);
   P->SetRidgefireAiming(false);Stage=3;At=Now;return false;
  }
  if(Stage==3)
  {
   UClass* Class=LoadClass<AShooterNPC>(nullptr,TEXT("/Game/Variant_Shooter/Blueprints/AI/BP_ShooterNPC.BP_ShooterNPC_C"));
   FActorSpawnParameters Params;Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
   auto* Enemy=Class ? W->SpawnActor<AShooterNPC>(Class,FVector(200000,200000,5000),FRotator::ZeroRotator,Params) : nullptr;
   if(!T->TestNotNull(TEXT("timed burst target"),Enemy))return true;Target=Enemy;Enemy->SetActorTickEnabled(false);Enemy->GetCharacterMovement()->DisableMovement();Enemy->CurrentHP=10000;
   UStaticMeshComponent* Part=nullptr;float Largest=0;TInlineComponentArray<UStaticMeshComponent*> Parts(Enemy);
   for(auto* C:Parts)if(C->IsRegistered() && !C->bHiddenInGame && C->GetCollisionResponseToChannel(ECC_Visibility)==ECR_Block && C->GetStaticMesh() && C->GetStaticMesh()->GetPathName().Contains(TEXT("BasicShapes/Cube"))){const float Volume=C->Bounds.BoxExtent.X*C->Bounds.BoxExtent.Y*C->Bounds.BoxExtent.Z;if(Volume>Largest){Largest=Volume;Part=C;}}
   if(!T->TestNotNull(TEXT("timed burst actual visible hit component"),Part)){Enemy->Destroy();return true;}
   P->SetActorLocation(P->GetActorLocation()+Part->Bounds.Origin-P->GetFirstPersonCameraComponent()->GetComponentLocation()-FVector(500,0,0));
   P->GetCharacterMovement()->DisableMovement();PC->SetControlRotation(FRotator::ZeroRotator);P->GetFirstPersonCameraComponent()->SetWorldRotation(FRotator::ZeroRotator);
   auto Burst=Original;Burst.Mode=ERidgefireFireMode::Burst;Burst.RefireDelay=0.28f;Burst.DamageScale=1;Burst.Payload=ERidgefirePayload::Plain;Burst.Condition=ERidgefireCondition::Steady;Burst.UpgradeBehavior=ERidgefireUpgradeBehavior::None;
   P->EquipRidgefireWeapon(Burst,5,0);
   FHitResult Proof;FCollisionQueryParams Q(SCENE_QUERY_STAT(PressureBurstProof),true,P);const FVector Eye=P->GetFirstPersonCameraComponent()->GetComponentLocation();const FVector Forward=P->GetFirstPersonCameraComponent()->GetForwardVector();W->LineTraceSingleByChannel(Proof,Eye,Eye+Forward*10000,ECC_Visibility,Q);
   UE_LOG(LogTemp,Display,TEXT("PRESSURE BURST PROOF actor=%s component=%s direction=%s muzzle=%s"),*GetNameSafe(Proof.GetActor()),*GetNameSafe(Proof.GetComponent()),*Forward.ToCompactString(),*FPressurePassChecks::Muzzle(P).ToCompactString());
   P->DoStartFiring();P->DoStopFiring();InitialDamage=10000-Enemy->CurrentHP;
   UE_LOG(LogTemp,Display,TEXT("PRESSURE BURST target=%s part=%s point=%s camera=%s damage=%.2f"),*Enemy->GetName(),*Part->GetName(),*Part->Bounds.Origin.ToCompactString(),*P->GetFirstPersonCameraComponent()->GetComponentLocation().ToCompactString(),InitialDamage);
   T->TestTrue(TEXT("first pulse hits actual visible target"),InitialDamage>0);
   T->TestEqual(TEXT("burst spends one charge"),P->GetRidgefireAmmo(),4);
   T->TestEqual(TEXT("two pulses are pending, not simultaneous hits"),FPressurePassChecks::PendingBurst(P),2);
   Stage=4;At=Now;return false;
  }
  if(Stage==4)
  {
   if(Target.IsValid())T->TestTrue(TEXT("real timer callbacks land later burst pulses"),10000-Target->CurrentHP>InitialDamage*1.5f);
   T->TestEqual(TEXT("burst callbacks finish after trigger release"),FPressurePassChecks::PendingBurst(P),0);
   T->TestEqual(TEXT("follow-up pulses do not spend extra charges"),P->GetRidgefireAmmo(),4);
   auto Burst=Original;Burst.Mode=ERidgefireFireMode::Burst;P->EquipRidgefireWeapon(Burst,5,0);P->DoStartFiring();P->DoStopFiring();P->EquipRidgefireWeapon(Original,Ammo,Reserve);
   T->TestEqual(TEXT("weapon switch cancels delayed old-weapon pulses"),FPressurePassChecks::PendingBurst(P),0);
   Stage=5;At=Now;return false;
  }
  if(Stage==5)
  {
   if(Target.IsValid())Target->Destroy();P->SetActorLocation(Location);PC->SetControlRotation(Rotation);P->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
   UE_LOG(LogTemp,Display,TEXT("IRON SUN PRESSURE PRESENTATION PASS camera zoom/timed pulses/switch cancellation; capture=%s"),*Folder);return true;
  }
  return false;
 }
};
}
bool FPressurePresentationTest::RunTest(const FString&){ADD_LATENT_AUTOMATION_COMMAND(FPressurePresentation(this));return true;}
#endif





