#if WITH_DEV_AUTOMATION_TESTS
#include "Variant_Shooter/ShooterCharacter.h"
#include "Variant_Shooter/ShooterPlayerController.h"
#include "Variant_Shooter/AI/ShooterNPC.h"
#include "RidgefireGameMode.h"
#include "RidgefireArenaDressing.h"
#include "RidgefirePickup.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HAL/PlatformTime.h"
#include "InputCoreTypes.h"
#include "InputKeyEventArgs.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#if WITH_EDITOR
#include "Tests/AutomationEditorCommon.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRidgefirePIESelectionTest,
    "Ridgefire.Regressions.CombatPIE.ActualShotClearSelectedPrimary",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRidgefirePIEInputTest,
    "Ridgefire.Regressions.CombatPIE.SelectedPrimaryThroughInputProcessing",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
#endif

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRidgefireSelectionBoundaryTest,
    "Ridgefire.Regressions.Combat.SelectedPrimaryAcrossEncounters",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRidgefireVisibleBodyTest,
    "Ridgefire.Regressions.Combat.VisibleBodyAndPhasmaCover",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRidgefireShotSelectionTest,
    "Ridgefire.Regressions.Combat.ActualShotClearSelectedPrimary",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRidgefireInputSelectionTest,
    "Ridgefire.Regressions.Input.SelectedPrimaryThroughInputProcessing",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRidgefireSprintSelectionTest,
    "Ridgefire.Regressions.Input.SprintRetainsSelectedPrimary",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

namespace
{
UWorld* GameWorld()
{
    if (GEngine) for (const FWorldContext& Context : GEngine->GetWorldContexts())
        if (Context.World() && Context.World()->IsGameWorld() && Context.World()->GetNetMode() == NM_Standalone)
            return Context.World();
    return nullptr;
}
void Freeze(AShooterNPC* NPC)
{
    NPC->SetActorTickEnabled(false);
    NPC->GetCharacterMovement()->DisableMovement();
    NPC->GetCharacterMovement()->SetComponentTickEnabled(false);
}
bool PressBinding(UInputComponent* Input, const FKey& Key, EInputEvent Event=IE_Pressed)
{
    if (Input) for (const FInputKeyBinding& Binding : Input->KeyBindings)
        if (Binding.Chord.Key==Key && Binding.KeyEvent==Event) { Binding.KeyDelegate.Execute(Key); return true; }
    return false;
}
UStaticMeshComponent* VisibleShoulder(AShooterNPC* NPC)
{
    TInlineComponentArray<UStaticMeshComponent*> Parts(NPC);
    for (UStaticMeshComponent* Part : Parts)
        if (Part->IsRegistered() && Part->IsVisible() && !Part->bHiddenInGame
            && FMath::Abs(Part->GetRelativeLocation().Y) > 60
            && Part->GetRelativeLocation().Z > 95 && Part->GetRelativeLocation().Z < 120)
            return Part;
    return nullptr;
}
void AimAt(AShooterCharacter* Shooter, const FVector& Point)
{
    Shooter->GetCharacterMovement()->DisableMovement();
    Shooter->GetCharacterMovement()->SetComponentTickEnabled(false);
    UCameraComponent* Camera = Shooter->GetFirstPersonCameraComponent();
    Camera->SetWorldRotation(FRotator::ZeroRotator);
    const FVector CameraLocation = Camera->GetComponentLocation();
    Shooter->SetActorLocation(Shooter->GetActorLocation() + FVector(0, Point.Y-CameraLocation.Y, Point.Z-CameraLocation.Z));
    Camera->SetWorldRotation(FRotator::ZeroRotator);
}

class FSelectionBoundary final : public IAutomationLatentCommand
{
    FAutomationTestBase* Test;
    double Started = FPlatformTime::Seconds();
    int32 FirstWave = 0, ExpectedIndex = INDEX_NONE;
    FString ExpectedName;
    bool bSelected = false;
    int32 CombatStage = 0;
    double NextFirePress = 0;
    bool bShotClear;
public:
    explicit FSelectionBoundary(FAutomationTestBase* InTest, bool bInShotClear=false) : Test(InTest), bShotClear(bInShotClear) {}
    bool Update() override
    {
        if (FPlatformTime::Seconds()-Started > 140) { Test->AddError(TEXT("Selected-primary boundary check timed out")); return true; }
        UWorld* World = GameWorld();
        auto* Mode = World ? World->GetAuthGameMode<ARidgefireGameMode>() : nullptr;
        auto* PC = World ? Cast<AShooterPlayerController>(World->GetFirstPlayerController()) : nullptr;
        auto* Pawn = PC ? Cast<AShooterCharacter>(PC->GetPawn()) : nullptr;
        if (!Mode || !Pawn) return false;
        if (Mode->IsArmoryOpen() || Mode->GetWave() < 1)
        {
            if (Mode->GetSetupStage()==ERidgefireSetupStage::Secondary) Mode->SelectSetupSecondary(PC,2);
            UGameplayStatics::SetGlobalTimeDilation(World, 20.f);
            return false;
        }
        UGameplayStatics::SetGlobalTimeDilation(World, 1.f);
        const auto& State = PC->GetRidgefireRunState();
        if (!bSelected)
        {
            if (!Test->TestTrue(TEXT("normal setup gives one primary"), State.PrimaryWeaponSlots.Num() == 1)) return true;
            ExpectedIndex = 1;
            if (!Test->TestTrue(TEXT("normal Phasma offer exists"), State.WeaponOffers.IsValidIndex(ExpectedIndex))) return true;
            ExpectedName = State.WeaponOffers[ExpectedIndex].Name;
            FVector ArsenalLocation;
            bool bFoundArsenal = false;
            for (TActorIterator<ARidgefireArenaDressing> It(World); It; ++It)
                if (It->GetArsenalStationWorldLocation(ArsenalLocation)) { bFoundArsenal=true; break; }
            if (!Test->TestTrue(TEXT("actual arsenal station exists"), bFoundArsenal)) return true;
            for (TActorIterator<AShooterNPC> It(World); It; ++It) Freeze(*It);
            Pawn->SetActorLocation(ArsenalLocation+FVector(-360,0,96));
            if (!Test->TestTrue(TEXT("actual arsenal permits Phasma acquisition"),Mode->SetPlayerArsenalOpen(PC,true))) return true;
            if(!State.PrimaryWeaponSlots.Contains(ExpectedIndex)) {
                Mode->AwardSharedCash(600); // Controlled funding; acquisition still uses the real paid transaction.
                if(!Test->TestTrue(TEXT("paid arsenal acquires reliability fixture weapon"),Mode->PurchaseWeapon(PC,ExpectedIndex,0))) return true;
            }
            Mode->SelectWeaponOffer(PC,ExpectedIndex,0);
            Mode->SetPlayerArsenalOpen(PC,false);
            const bool bPressed=PressBinding(PC->InputComponent,EKeys::One);
            if (!Test->TestTrue(TEXT("real primary binding exists"), bPressed)
                || !Test->TestEqual(TEXT("primary binding fills the intended primary slot"), State.PrimaryWeaponSlots[0], ExpectedIndex)
                || !Test->TestEqual(TEXT("primary binding actually equips selected offer"), Pawn->GetActiveWeaponName(), ExpectedName)) return true;
            FirstWave = Mode->GetWave();
            bSelected = true;
            Pawn->AddRidgefireReserveAmmo(1);
            if (!Test->TestTrue(TEXT("actual fire binding starts selected primary"),PressBinding(Pawn->InputComponent,EKeys::LeftMouseButton))) return true;
            NextFirePress=FPlatformTime::Seconds()+State.WeaponOffers[ExpectedIndex].RefireDelay+0.1;
            UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE RELIABILITY SELECTED wave=%d offer=%d weapon=%s"), FirstWave, ExpectedIndex, *ExpectedName);
            return false;
        }
        for (TActorIterator<AShooterNPC> It(World); It; ++It) Freeze(*It);
        if (State.ActiveWeaponIndex != ExpectedIndex || Pawn->GetActiveWeaponName() != ExpectedName)
        {
            Test->AddError(FString::Printf(TEXT("Unrequested primary reset: first_wave=%d now_wave=%d expected_offer=%d actual_offer=%d expected=%s actual=%s"),
                FirstWave, Mode->GetWave(), ExpectedIndex, State.ActiveWeaponIndex, *ExpectedName, *Pawn->GetActiveWeaponName()));
            return true;
        }
        if (CombatStage==0)
        {
            if (Pawn->GetRidgefireAmmo()>0)
            {
                if (FPlatformTime::Seconds()>=NextFirePress)
                {
                    PressBinding(Pawn->InputComponent,EKeys::LeftMouseButton,IE_Released);
                    PressBinding(Pawn->InputComponent,EKeys::LeftMouseButton);
                    NextFirePress=FPlatformTime::Seconds()+State.WeaponOffers[ExpectedIndex].RefireDelay+0.1;
                }
                return false;
            }
            PressBinding(Pawn->InputComponent,EKeys::LeftMouseButton,IE_Released);
            if (!Test->TestTrue(TEXT("actual reload binding exists"),PressBinding(PC->InputComponent,EKeys::R) || PressBinding(Pawn->InputComponent,EKeys::R))) return true;
            CombatStage=1;
            return false;
        }
        if (CombatStage==1)
        {
            if (Pawn->IsRidgefireReloading()) return false;
            if (!Test->TestTrue(TEXT("selected primary completes an empty-magazine reload"),Pawn->GetRidgefireAmmo()>0)) return true;
            CombatStage=2;
        }
        if (CombatStage==2)
        {
            auto CheckSelection=[&](const TCHAR* Action)
            {
                return Test->TestEqual(*FString::Printf(TEXT("%s retains selected weapon"),Action),Pawn->GetActiveWeaponName(),ExpectedName)
                    && Test->TestEqual(*FString::Printf(TEXT("%s retains selected offer"),Action),PC->GetRidgefireRunState().ActiveWeaponIndex,ExpectedIndex);
            };
            const FVector PickupPosition=Pawn->GetActorLocation()+FVector(0,500,0);
            auto* AmmoPickup=World->SpawnActor<ARidgefirePickup>(PickupPosition,FRotator::ZeroRotator);
            if (!Test->TestNotNull(TEXT("spawn actual ammunition pickup"),AmmoPickup)) return true;
            AmmoPickup->Initialize(false);
            Pawn->SetActorLocation(PickupPosition);
            const bool bCollected=AmmoPickup->IsCollected();
            AmmoPickup->Destroy();
            if (!Test->TestTrue(TEXT("normal capsule overlap collects ammunition"),bCollected) || !CheckSelection(TEXT("ammo pickup"))) return true;
            UGameplayStatics::ApplyDamage(Pawn,20.f,nullptr,nullptr,UDamageType::StaticClass());
            if (!CheckSelection(TEXT("taking damage"))) return true;
            PressBinding(PC->InputComponent,EKeys::F);
            if (!CheckSelection(TEXT("field patch"))) return true;
            PressBinding(PC->InputComponent,EKeys::E);
            if (!CheckSelection(TEXT("Ion Surge"))) return true;
            PressBinding(PC->InputComponent,EKeys::LeftShift);
            PressBinding(PC->InputComponent,EKeys::LeftShift,IE_Released);
            if (!CheckSelection(TEXT("sprint"))) return true;
            PressBinding(Pawn->InputComponent,EKeys::C);
            if (!CheckSelection(TEXT("bound C action"))) return true;
            CombatStage=3;
        }
        if (Mode->GetWave() >= FirstWave+(bShotClear ? 1 : 3))
        {
            UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE RELIABILITY SELECTION PASS actual primary binding, firing, empty-magazine reload, kill rewards, four reinforcement groups; transitions=%d actual_shot_clear=%d; no selection input after first choice"), bShotClear ? 1 : 3, bShotClear);
            return true;
        }
        TArray<AShooterNPC*> Alive;
        for (TActorIterator<AShooterNPC> It(World); It; ++It)
            if (It->CurrentHP > 0) { Freeze(*It); Alive.Add(*It); }
        if (bShotClear)
        {
            if (Pawn->IsRidgefireReloading() || FPlatformTime::Seconds()<NextFirePress) return false;
            if (Pawn->GetRidgefireAmmo()<=0)
            {
                PressBinding(PC->InputComponent,EKeys::R) || PressBinding(Pawn->InputComponent,EKeys::R);
                return false;
            }
            if (Alive.IsEmpty()) return false;
            AShooterNPC* Target=Alive[0];
            UStaticMeshComponent* Shoulder=VisibleShoulder(Target);
            if (!Test->TestNotNull(TEXT("actual shot-clear target exposes visible armor"),Shoulder)) return true;
            Pawn->GetCharacterMovement()->DisableMovement();
            Pawn->GetCharacterMovement()->SetComponentTickEnabled(false);
            Pawn->SetActorLocation(FVector(150000,150000,50000));
            PC->SetControlRotation(FRotator::ZeroRotator);
            auto* Camera=Pawn->GetFirstPersonCameraComponent();
            Camera->SetWorldRotation(FRotator::ZeroRotator);
            Target->SetActorRotation(FRotator::ZeroRotator);
            Target->SetActorLocation(Target->GetActorLocation()+Camera->GetComponentLocation()+FVector(800,0,0)-Shoulder->GetComponentLocation());
            PressBinding(Pawn->InputComponent,EKeys::LeftMouseButton,IE_Released);
            const float BeforeHP=Target->CurrentHP;
            const int32 BeforeAmmo=Pawn->GetRidgefireAmmo();
            PressBinding(Pawn->InputComponent,EKeys::LeftMouseButton);
            PressBinding(Pawn->InputComponent,EKeys::LeftMouseButton,IE_Released);
            if (!Test->TestEqual(TEXT("normal shot-clear press consumes one round"),Pawn->GetRidgefireAmmo(),BeforeAmmo-1)
                || !Test->TestTrue(TEXT("normal Phasma shot-clear actually damages visible target"),Target->CurrentHP<BeforeHP)) return true;
            NextFirePress=FPlatformTime::Seconds()+State.WeaponOffers[ExpectedIndex].RefireDelay+0.1;
            return false;
        }
        for (AShooterNPC* NPC : Alive)
            UGameplayStatics::ApplyDamage(NPC, NPC->CurrentHP+1000, PC, Pawn, UDamageType::StaticClass());
        return false;
    }
};

class FVisibleBodyAndCover final : public IAutomationLatentCommand
{
    FAutomationTestBase* Test;
    double Started = FPlatformTime::Seconds();
public:
    explicit FVisibleBodyAndCover(FAutomationTestBase* InTest) : Test(InTest) {}
    bool Update() override
    {
        UWorld* World = GameWorld();
        auto* Mode = World ? World->GetAuthGameMode<ARidgefireGameMode>() : nullptr;
        if (!Mode || Mode->IsArmoryOpen() || Mode->GetWave() < 1)
        {
            if (Mode) UGameplayStatics::SetGlobalTimeDilation(World,20.f);
            if (FPlatformTime::Seconds()-Started < 90) return false;
            Test->AddError(TEXT("Visible-body check timed out waiting for gameplay")); return true;
        }
        UGameplayStatics::SetGlobalTimeDilation(World,1.f);
        UClass* NPCClass = LoadClass<AShooterNPC>(nullptr, TEXT("/Game/Variant_Shooter/Blueprints/AI/BP_ShooterNPC.BP_ShooterNPC_C"));
        UClass* PlayerClass = LoadClass<AShooterCharacter>(nullptr, TEXT("/Game/Variant_Shooter/Blueprints/BP_ShooterCharacter.BP_ShooterCharacter_C"));
        UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
        if (!Test->TestNotNull(TEXT("actual NPC Blueprint"), NPCClass) || !Test->TestNotNull(TEXT("actual character Blueprint"), PlayerClass)
            || !Test->TestNotNull(TEXT("solid cover asset"), Cube)) return true;
        TArray<AActor*> Owned;
        ON_SCOPE_EXIT { for (AActor* Actor : Owned) if (IsValid(Actor)) Actor->Destroy(); };
        const FVector Origin(200000, 200000, 50000);
        auto SpawnNPC = [&](FVector Position)
        {
            auto* NPC = World->SpawnActor<AShooterNPC>(NPCClass, Position, FRotator::ZeroRotator);
            if (NPC) { Owned.Add(NPC); Freeze(NPC); }
            return NPC;
        };
        auto* First = SpawnNPC(Origin);
        auto* Shooter = World->SpawnActor<AShooterCharacter>(PlayerClass, Origin-FVector(3000,0,0), FRotator::ZeroRotator);
        if (Shooter) {Owned.Add(Shooter);Shooter->SetRidgefireExactRayFixture(true);}
        // Accuracy now has its own production-cone checks. A ray through a measured
        // empty mesh gap must stay exact here, or spread may legitimately hit nearby armor.
        if (!Test->TestNotNull(TEXT("spawn real sentry"), First) || !Test->TestNotNull(TEXT("spawn real firing character"), Shooter)) return true;
        TInlineComponentArray<UStaticMeshComponent*> Parts(First);
        FCollisionQueryParams Query(SCENE_QUERY_STAT(RidgefireReliability), true, Shooter);
        int32 GhostHits = 0, VisibleHits = 0;
        FVector FirstGhost = FVector::ZeroVector;
        bool bEmptyChosen=false;
        const float Radius = First->GetCapsuleComponent()->GetScaledCapsuleRadius();
        const float Height = First->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
        for (float Y=-Radius*0.85f; Y<=Radius*0.85f; Y+=12)
            for (float Z=-Height*0.85f; Z<=Height*0.85f; Z+=12)
            {
                const FVector Start = Origin+FVector(-600,Y,Z), End=Origin+FVector(600,Y,Z);
                FHitResult Hit;
                const bool bBotHit=World->LineTraceSingleByChannel(Hit,Start,End,ECC_Visibility,Query) && Hit.GetActor()==First;
                bool bVisibleGeometry = false;
                for (UStaticMeshComponent* Part : Parts)
                    if (Part->IsRegistered() && Part->IsVisible() && !Part->bHiddenInGame)
                    {
                        FHitResult PartHit;
                        if (Part->LineTraceComponent(PartHit, Start, End, Query)) { bVisibleGeometry=true; break; }
                    }
                const float CapZ=FMath::Max(0.f,FMath::Abs(Z)-(Height-Radius));
                const bool bInsideMovementCapsule=Y*Y+CapZ*CapZ<Radius*Radius;
                if (!bVisibleGeometry && bInsideMovementCapsule && !bEmptyChosen) { FirstGhost=Origin+FVector(0,Y,Z); bEmptyChosen=true; }
                if (bBotHit && bVisibleGeometry) ++VisibleHits;
                else if (bBotHit)
                {
                    ++GhostHits;
                }
            }
        Test->AddInfo(FString::Printf(TEXT("Visible silhouette scan: registered_parts=%d visible_hits=%d invisible_hits=%d"), Parts.Num(), VisibleHits, GhostHits));
        Test->TestTrue(TEXT("silhouette scan has actual visible positive controls"), VisibleHits>0);
        Test->TestEqual(TEXT("empty space outside visible mechanical parts cannot register a bot hit"), GhostHits, 0);
        if (Test->TestTrue(TEXT("independent mesh scan supplies an actual empty-space shot control"),bEmptyChosen))
        {
            AimAt(Shooter, FirstGhost);
            Shooter->EquipRidgefireWeapon(*Mode->GetOffer(0));
            const float Before=First->CurrentHP;
            Shooter->DoStartFiring(); Shooter->DoStopFiring();
            Test->AddInfo(FString::Printf(TEXT("Empty-space actual normal Gale shot: hp=%.3f->%.3f point=%s"), Before, First->CurrentHP, *FirstGhost.ToString()));
            Test->TestEqual(TEXT("controlled exact-ray Gale shot into empty silhouette space causes no damage"), First->CurrentHP, Before);
        }
        auto* Shoulder = VisibleShoulder(First);
        if (!Test->TestNotNull(TEXT("actual visible shoulder positive control"), Shoulder)) return true;
        const FVector Target=Shoulder->GetComponentLocation();
        auto* Exposed = SpawnNPC(Origin+FVector(-240,200,0));
        auto* Covered = SpawnNPC(Origin+FVector(-240,-180,0));
        auto* Far = SpawnNPC(Origin+FVector(0,1200,0));
        auto* Cover = World->SpawnActor<AStaticMeshActor>(Origin+FVector(0,-90,0), FRotator::ZeroRotator);
        if (Cover) Owned.Add(Cover);
        if (!Test->TestNotNull(TEXT("spawn exposed neighbor"), Exposed) || !Test->TestNotNull(TEXT("spawn covered neighbor"), Covered)
            || !Test->TestNotNull(TEXT("spawn distant control"), Far) || !Test->TestNotNull(TEXT("spawn actual solid divider"), Cover)) return true;
        Cover->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
        Cover->GetStaticMeshComponent()->SetStaticMesh(Cube);
        Cover->GetStaticMeshComponent()->SetWorldScale3D(FVector(6,0.3f,7));
        Cover->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Cover->GetStaticMeshComponent()->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
        First->CurrentHP=Exposed->CurrentHP=Covered->CurrentHP=Far->CurrentHP=1000;
        AimAt(Shooter, Target);
        FHitResult Control;
        Query.AddIgnoredActor(First);
        Test->TestTrue(TEXT("independent divider blocks impact-to-covered-neighbor lane"),
            World->LineTraceSingleByChannel(Control,Target,Covered->GetActorLocation(),ECC_Visibility,Query) && Control.GetActor()==Cover);
        const auto* Phasma = Mode->GetOffer(1);
        if (!Test->TestNotNull(TEXT("normal Phasma offer"),Phasma)) return true;
        Shooter->Destroy();
        // Fresh actor prevents a previous Gale cadence window from masking this shot.
        auto* PlasmaShooter = World->SpawnActor<AShooterCharacter>(PlayerClass, Origin-FVector(3000,0,0), FRotator::ZeroRotator);
        if (!Test->TestNotNull(TEXT("spawn normal Phasma shooter"), PlasmaShooter)) return true;
        Owned.Add(PlasmaShooter);PlasmaShooter->SetRidgefireExactRayFixture(true); AimAt(PlasmaShooter,Target);
        PlasmaShooter->EquipRidgefireWeapon(*Phasma);
        PlasmaShooter->DoStartFiring(); PlasmaShooter->DoStopFiring();
        Test->AddInfo(FString::Printf(TEXT("Normal Phasma damage recipients: aimed=%.3f exposed=%.3f covered=%.3f far=%.3f"),
            1000-First->CurrentHP,1000-Exposed->CurrentHP,1000-Covered->CurrentHP,1000-Far->CurrentHP));
        Test->TestTrue(TEXT("normal Phasma actually damages aimed visible part"),First->CurrentHP<1000);
        Test->TestTrue(TEXT("normal Phasma has a real exposed-neighbor splash positive control"),Exposed->CurrentHP<1000);
        Test->TestEqual(TEXT("solid divider prevents Phasma damage to hidden neighbor"),Covered->CurrentHP,1000.0f);
        Test->TestEqual(TEXT("distant control remains unharmed"),Far->CurrentHP,1000.0f);
        Test->TestEqual(TEXT("blast feedback counts exposed splash recipients only"),PlasmaShooter->GetRidgefireBlastHits(),1);
        Test->TestEqual(TEXT("surviving blast recipients are not labeled down"),PlasmaShooter->GetRidgefireBlastKills(),0);

        // Actual impact on world cover: both panels share one owner, as arena
        // dressing does. Ignoring the struck owner must not bypass either panel.
        const FVector WallOrigin=Origin+FVector(0,30000,0);
        auto* Wall=World->SpawnActor<AStaticMeshActor>(WallOrigin,FRotator::ZeroRotator);
        if (!Test->TestNotNull(TEXT("spawn wall-impact cover owner"),Wall)) return true;
        Owned.Add(Wall);
        auto ConfigurePanel = [&](UStaticMeshComponent* Panel,const FVector& Position)
        {
            Panel->SetMobility(EComponentMobility::Movable);
            Panel->SetStaticMesh(Cube);
            Panel->SetWorldLocation(Position);
            Panel->SetWorldScale3D(FVector(0.3f,6,7));
            Panel->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            Panel->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
        };
        ConfigurePanel(Wall->GetStaticMeshComponent(),WallOrigin);
        auto* BackPanel=NewObject<UStaticMeshComponent>(Wall,TEXT("SharedOwnerBackPanel"));
        Wall->AddInstanceComponent(BackPanel);
        BackPanel->SetupAttachment(Wall->GetRootComponent());
        BackPanel->RegisterComponent();
        ConfigurePanel(BackPanel,WallOrigin+FVector(80,0,0));
        auto* WallProtected=SpawnNPC(WallOrigin+FVector(200,0,0));
        auto* WallExposed=SpawnNPC(WallOrigin+FVector(-180,180,0));
        auto* WallShooter=World->SpawnActor<AShooterCharacter>(PlayerClass,WallOrigin-FVector(3000,0,0),FRotator::ZeroRotator);
        if (!Test->TestNotNull(TEXT("spawn wall-protected sentry"),WallProtected)
            || !Test->TestNotNull(TEXT("spawn wall-side exposed control"),WallExposed)
            || !Test->TestNotNull(TEXT("spawn actual wall-impact shooter"),WallShooter)) return true;
        Owned.Add(WallShooter);WallShooter->SetRidgefireExactRayFixture(true);
        WallProtected->CurrentHP=WallExposed->CurrentHP=1000;
        AimAt(WallShooter,WallOrigin);
        WallShooter->EquipRidgefireWeapon(*Phasma);
        const FVector WallCameraStart=WallShooter->GetFirstPersonCameraComponent()->GetComponentLocation();
        const FVector WallAimEnd=WallCameraStart+WallShooter->GetFirstPersonCameraComponent()->GetForwardVector()*10000.0f;
        FCollisionQueryParams WallQuery(SCENE_QUERY_STAT(RidgefireWallImpactProbe),true,WallShooter);
        FHitResult WallCameraHit,WallMuzzleHit;
        const bool bWallCameraHit=World->LineTraceSingleByChannel(WallCameraHit,WallCameraStart,WallAimEnd,ECC_Visibility,WallQuery);
        const FVector WallMuzzleStart=WallShooter->GetRidgefireMuzzleWorldLocation();
        const FVector WallDirection=((bWallCameraHit?WallCameraHit.ImpactPoint:WallAimEnd)-WallMuzzleStart).GetSafeNormal();
        const bool bWallMuzzleHit=World->LineTraceSingleByChannel(WallMuzzleHit,WallMuzzleStart,WallMuzzleStart+WallDirection*10000.0f,ECC_Visibility,WallQuery);
        Test->TestTrue(TEXT("camera aims at cover itself, not a sentry"),bWallCameraHit && WallCameraHit.GetActor()==Wall);
        Test->TestTrue(TEXT("actual muzzle trace impacts the cover owner"),bWallMuzzleHit && WallMuzzleHit.GetActor()==Wall);
        const float ProtectedDistance=FVector::Distance(WallMuzzleHit.ImpactPoint,WallProtected->GetActorLocation());
        Test->TestTrue(TEXT("protected sentry is inside normal Phasma splash radius"),bWallMuzzleHit && ProtectedDistance<460.f);
        FHitResult BackPanelHit;
        Test->TestTrue(TEXT("another component of the struck actor also blocks the protected lane"),
            bWallMuzzleHit && BackPanel->LineTraceComponent(BackPanelHit,WallMuzzleHit.ImpactPoint,WallProtected->GetActorLocation(),WallQuery));
        WallShooter->DoStartFiring(); WallShooter->DoStopFiring();
        Test->AddInfo(FString::Printf(TEXT("Actual Phasma wall impact: owner=%s protected_distance=%.1f protected_damage=%.3f exposed_damage=%.3f"),
            *GetNameSafe(WallMuzzleHit.GetActor()),ProtectedDistance,1000-WallProtected->CurrentHP,1000-WallExposed->CurrentHP));
        Test->TestEqual(TEXT("impacting cover preserves all shared-owner panels as splash blockers"),WallProtected->CurrentHP,1000.f);
        Test->TestTrue(TEXT("wall-impact splash still damages an exposed same-side neighbor"),WallExposed->CurrentHP<1000);
        Test->TestEqual(TEXT("wall-impact feedback counts only the exposed recipient"),WallShooter->GetRidgefireBlastHits(),1);

        // Reproduce the owner's clustered-death report using one real Phasma shot
        // and three full-health production sentries inside ordinary arena spacing.
        const FVector ClusterOrigin=Origin+FVector(0,10000,0);
        auto* ClusterTarget=SpawnNPC(ClusterOrigin);
        auto* ClusterLeft=SpawnNPC(ClusterOrigin+FVector(-260,180,0));
        auto* ClusterEdge=SpawnNPC(ClusterOrigin+FVector(300,250,0));
        auto* ClusterShooter=World->SpawnActor<AShooterCharacter>(PlayerClass,ClusterOrigin-FVector(3000,0,0),FRotator::ZeroRotator);
        if (!Test->TestNotNull(TEXT("spawn clustered direct target"),ClusterTarget)
            || !Test->TestNotNull(TEXT("spawn clustered exposed left sentry"),ClusterLeft)
            || !Test->TestNotNull(TEXT("spawn farther exposed splash sentry"),ClusterEdge)
            || !Test->TestNotNull(TEXT("spawn clustered-shot player"),ClusterShooter)) return true;
        Owned.Add(ClusterShooter);
        const float ClusterTargetHP=ClusterTarget->CurrentHP;
        const float ClusterLeftHP=ClusterLeft->CurrentHP;
        const float ClusterEdgeHP=ClusterEdge->CurrentHP;
        UStaticMeshComponent* ClusterShoulder=VisibleShoulder(ClusterTarget);
        if (!Test->TestNotNull(TEXT("cluster target has an actual visible aim point"),ClusterShoulder)) return true;

        const FVector GaleOrigin=Origin+FVector(0,20000,0);
        auto* GaleTarget=SpawnNPC(GaleOrigin);
        auto* GaleNeighbor=SpawnNPC(GaleOrigin+FVector(-260,180,0));
        auto* GaleShooter=World->SpawnActor<AShooterCharacter>(PlayerClass,GaleOrigin-FVector(3000,0,0),FRotator::ZeroRotator);
        if (!Test->TestNotNull(TEXT("spawn Gale single-target control"),GaleTarget)
            || !Test->TestNotNull(TEXT("spawn same-spacing Gale neighbor"),GaleNeighbor)
            || !Test->TestNotNull(TEXT("spawn Gale control shooter"),GaleShooter)) return true;
        Owned.Add(GaleShooter);
        const float GaleTargetHP=GaleTarget->CurrentHP;
        const float GaleNeighborHP=GaleNeighbor->CurrentHP;
        UStaticMeshComponent* GaleShoulder=VisibleShoulder(GaleTarget);
        if (!Test->TestNotNull(TEXT("Gale control target has an actual visible aim point"),GaleShoulder)) return true;
        AimAt(GaleShooter,GaleShoulder->GetComponentLocation());
        const auto* Gale=Mode->GetOffer(0);
        if (!Test->TestNotNull(TEXT("normal single-target Gale offer"),Gale)) return true;
        GaleShooter->EquipRidgefireWeapon(*Gale);
        const FVector CameraStart=GaleShooter->GetFirstPersonCameraComponent()->GetComponentLocation();
        const FVector AimEnd=CameraStart+GaleShooter->GetFirstPersonCameraComponent()->GetForwardVector()*10000.0f;
        FCollisionQueryParams GaleTraceParams(SCENE_QUERY_STAT(RidgefireSingleTargetControl),true,GaleShooter);
        GaleTraceParams.AddIgnoredActor(GaleShooter);
        FHitResult CameraHit;
        const bool bCameraHit=World->LineTraceSingleByChannel(CameraHit,CameraStart,AimEnd,ECC_Visibility,GaleTraceParams);
        const FVector MuzzleStart=GaleShooter->GetRidgefireMuzzleWorldLocation();
        const FVector GaleDirection=((bCameraHit?CameraHit.ImpactPoint:AimEnd)-MuzzleStart).GetSafeNormal();
        FHitResult MuzzleHit;
        const bool bMuzzleHit=World->LineTraceSingleByChannel(MuzzleHit,MuzzleStart,MuzzleStart+GaleDirection*10000.0f,ECC_Visibility,GaleTraceParams);
        Test->AddInfo(FString::Printf(TEXT("Same-spacing Gale control trace: camera_actor=%s muzzle_actor=%s component=%s"),
            *GetNameSafe(bCameraHit?CameraHit.GetActor():nullptr),*GetNameSafe(bMuzzleHit?MuzzleHit.GetActor():nullptr),*GetNameSafe(bMuzzleHit?MuzzleHit.GetComponent():nullptr)));
        Test->TestTrue(TEXT("actual muzzle trace selects the visible directly aimed Gale target"),bMuzzleHit && MuzzleHit.GetActor()==GaleTarget);
        GaleShooter->DoStartFiring();
        GaleShooter->DoStopFiring();
        Test->AddInfo(FString::Printf(TEXT("One normal Gale shot at the same spacing: target_damage=%.1f neighbor_damage=%.1f"),
            GaleTargetHP-GaleTarget->CurrentHP,GaleNeighborHP-GaleNeighbor->CurrentHP));
        Test->TestTrue(TEXT("same-spacing Gale shot damages its direct target"),GaleTarget->CurrentHP<GaleTargetHP);
        Test->TestEqual(TEXT("same-spacing non-splash Gale shot leaves exposed neighbor untouched"),GaleNeighbor->CurrentHP,GaleNeighborHP);

        AimAt(ClusterShooter,ClusterShoulder->GetComponentLocation());
        const FVector ClusterCameraStart=ClusterShooter->GetFirstPersonCameraComponent()->GetComponentLocation();
        const FVector ClusterAimEnd=ClusterCameraStart+ClusterShooter->GetFirstPersonCameraComponent()->GetForwardVector()*10000.0f;
        FCollisionQueryParams ClusterTraceParams(SCENE_QUERY_STAT(RidgefirePhasmaClusterProbe),true,ClusterShooter);
        ClusterTraceParams.AddIgnoredActor(ClusterShooter);
        FHitResult ClusterCameraHit,ClusterMuzzleHit;
        const bool bClusterCameraHit=World->LineTraceSingleByChannel(ClusterCameraHit,ClusterCameraStart,ClusterAimEnd,ECC_Visibility,ClusterTraceParams);
        const FVector ClusterMuzzleStart=ClusterShooter->GetRidgefireMuzzleWorldLocation();
        const FVector ClusterShotDirection=((bClusterCameraHit?ClusterCameraHit.ImpactPoint:ClusterAimEnd)-ClusterMuzzleStart).GetSafeNormal();
        const bool bClusterMuzzleHit=World->LineTraceSingleByChannel(ClusterMuzzleHit,ClusterMuzzleStart,ClusterMuzzleStart+ClusterShotDirection*10000.0f,ECC_Visibility,ClusterTraceParams);
        const float LeftBlastDistance=bClusterMuzzleHit?FVector::Distance(ClusterMuzzleHit.ImpactPoint,ClusterLeft->GetActorLocation()):0.0f;
        const float EdgeBlastDistance=bClusterMuzzleHit?FVector::Distance(ClusterMuzzleHit.ImpactPoint,ClusterEdge->GetActorLocation()):0.0f;
        Test->AddInfo(FString::Printf(TEXT("Phasma cluster trace: camera=%s muzzle=%s near_distance=%.1f edge_distance=%.1f"),
            *GetNameSafe(bClusterCameraHit?ClusterCameraHit.GetActor():nullptr),*GetNameSafe(bClusterMuzzleHit?ClusterMuzzleHit.GetActor():nullptr),LeftBlastDistance,EdgeBlastDistance));
        Test->TestTrue(TEXT("Phasma muzzle trace selects the visible directly aimed target"),bClusterMuzzleHit && ClusterMuzzleHit.GetActor()==ClusterTarget);
        Test->TestTrue(TEXT("Phasma distance controls are separated"),LeftBlastDistance<EdgeBlastDistance);
        ClusterShooter->EquipRidgefireWeapon(*Phasma);
        ClusterShooter->DoStartFiring();
        ClusterShooter->DoStopFiring();
        const int32 ClusterDeaths=(ClusterTarget->CurrentHP<=0?1:0)+(ClusterLeft->CurrentHP<=0?1:0)+(ClusterEdge->CurrentHP<=0?1:0);
        Test->AddInfo(FString::Printf(TEXT("One normal Phasma shot at a full-health cluster: start_hp=%.1f/%.1f/%.1f end_hp=%.1f/%.1f/%.1f deaths=%d"),
            ClusterTargetHP,ClusterLeftHP,ClusterEdgeHP,ClusterTarget->CurrentHP,ClusterLeft->CurrentHP,ClusterEdge->CurrentHP,ClusterDeaths));
        Test->TestTrue(TEXT("one normal Phasma shot reaches its directly aimed sentry"),ClusterTarget->CurrentHP<ClusterTargetHP);
        Test->TestEqual(TEXT("one normal Phasma shot kills only its directly aimed full-health sentry"),ClusterDeaths,1);
        Test->TestTrue(TEXT("Phasma falloff still damages but does not kill a full-health neighbor"),ClusterLeft->CurrentHP>0 && ClusterLeft->CurrentHP<ClusterLeftHP);
        Test->TestTrue(TEXT("Phasma splash damage fades for a farther exposed neighbor"),ClusterLeftHP-ClusterLeft->CurrentHP>ClusterEdgeHP-ClusterEdge->CurrentHP);
        Test->TestEqual(TEXT("directly hit sentry is excluded from splash kill feedback"),ClusterShooter->GetRidgefireBlastKills(),0);

        TInlineComponentArray<UStaticMeshComponent*> BlastParts(PlasmaShooter);int32 PulseParts=0;
        for(auto* Part:BlastParts) if(Part->GetName().StartsWith(TEXT("RidgefireBlast_"))) {
            ++PulseParts;Test->TestTrue(TEXT("runtime blast is visible, world anchored and noncolliding"),!Part->bHiddenInGame && Part->IsUsingAbsoluteLocation() && Part->GetCollisionEnabled()==ECollisionEnabled::NoCollision);
        }
        Test->TestEqual(TEXT("bounded reusable pulse has 24 segments"),PulseParts,24);
        PlasmaShooter->Destroy();
        Cover->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        First->CurrentHP=Exposed->CurrentHP=Covered->CurrentHP=Far->CurrentHP=1000;
        auto* UncoveredShooter=World->SpawnActor<AShooterCharacter>(PlayerClass,Origin-FVector(3000,0,0),FRotator::ZeroRotator);
        if (!Test->TestNotNull(TEXT("spawn no-divider Phasma positive control"),UncoveredShooter)) return true;
        Owned.Add(UncoveredShooter);AimAt(UncoveredShooter,Target);UncoveredShooter->EquipRidgefireWeapon(*Phasma);
        UncoveredShooter->DoStartFiring();UncoveredShooter->DoStopFiring();
        Test->AddInfo(FString::Printf(TEXT("Phasma same hidden-neighbor lane with divider disabled: damage=%.3f"),1000-Covered->CurrentHP));
        Test->TestTrue(TEXT("removing only the divider permits damage to the same neighbor"),Covered->CurrentHP<1000);
        Test->TestEqual(TEXT("feedback follows changed exposure instead of overlap count"),UncoveredShooter->GetRidgefireBlastHits(),2);
        Cover->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        for (int32 Role=0;Role<4;++Role)
        {
            AShooterNPC* Sample=SpawnNPC(Origin+FVector(0,5000+Role*2000,0));
            if (!Test->TestNotNull(TEXT("spawn real role silhouette control"),Sample)) return true;
            if (Role==1) Sample->EnableRidgefireSprinter();
            else if (Role==2) Sample->EnableRidgefireBrute();
            else if (Role==3) Sample->EnableRidgefireChampion(false,false);
            const auto MovementCollision=Sample->GetCapsuleComponent()->GetCollisionEnabled();
            const auto PawnResponse=Sample->GetCapsuleComponent()->GetCollisionResponseToChannel(ECC_Pawn);
            for (int32 Pose=0;Pose<3;++Pose)
            {
                static_cast<AActor*>(Sample)->Tick(0.1f);
                TInlineComponentArray<UStaticMeshComponent*> RoleParts(Sample);
                const FVector Center=Sample->GetCapsuleComponent()->GetComponentLocation();
                const float R=Sample->GetCapsuleComponent()->GetScaledCapsuleRadius();
                const float H=Sample->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
                int32 Visible=0,Ghost=0;
                for (float Y=-R*0.85f;Y<=R*0.85f;Y+=12)
                    for (float Z=-H*0.85f;Z<=H*0.85f;Z+=12)
                    {
                        const FVector Start=Center+FVector(-600,Y,Z),End=Center+FVector(600,Y,Z);
                        FHitResult Hit;
                        if (!World->LineTraceSingleByChannel(Hit,Start,End,ECC_Visibility,Query) || Hit.GetActor()!=Sample) continue;
                        bool bVisible=false;
                        for (UStaticMeshComponent* Part:RoleParts)
                        {
                            FHitResult PartHit;
                            if (Part->IsRegistered() && Part->IsVisible() && !Part->bHiddenInGame && Part->LineTraceComponent(PartHit,Start,End,Query)) { bVisible=true;break; }
                        }
                        if (bVisible) ++Visible;else ++Ghost;
                    }
                Test->AddInfo(FString::Printf(TEXT("Role silhouette: role=%d pose=%d visible=%d invisible=%d"),Role,Pose,Visible,Ghost));
                Test->TestTrue(TEXT("posed role retains hittable visible armor"),Visible>0);
                Test->TestEqual(TEXT("posed role has no empty-space hits"),Ghost,0);
                Test->TestTrue(TEXT("pose retains movement collision and Pawn response"),
                    Sample->GetCapsuleComponent()->GetCollisionEnabled()==MovementCollision && Sample->GetCapsuleComponent()->GetCollisionResponseToChannel(ECC_Pawn)==PawnResponse);
            }
        }
        UE_LOG(LogTemp,Display,TEXT("RIDGEFIRE RELIABILITY GEOMETRY CHECK COMPLETE"));
        return true;
    }
};

class FInputSelection final : public IAutomationLatentCommand
{
    FAutomationTestBase* Test;
    double Started=FPlatformTime::Seconds(),NextEvent=0;
    int32 Stage=0,Action=0,InitialAmmo=0;
    FKey HeldKey;
    FString ExpectedName;
    TArray<FKey> Actions;
    bool bSprintOnly;
    float InitialWalkSpeed=0;
    bool bSprintObserved=false;
public:
    explicit FInputSelection(FAutomationTestBase* InTest,bool bInSprintOnly=false):Test(InTest),bSprintOnly(bInSprintOnly)
    {
        if (bSprintOnly) Actions.Add(EKeys::LeftShift);
        else
        {
            for (int32 Shot=0;Shot<6;++Shot) Actions.Add(EKeys::LeftMouseButton);
            Actions.Append({EKeys::R,EKeys::C,EKeys::LeftShift,EKeys::E,EKeys::F,EKeys::RightMouseButton,EKeys::W,EKeys::A,EKeys::S,EKeys::D});
        }
    }
    bool Update() override
    {
        if (FPlatformTime::Seconds()-Started>80) { Test->AddError(TEXT("normal input processing test timed out")); return true; }
        UWorld* World=GameWorld();
        auto* Mode=World?World->GetAuthGameMode<ARidgefireGameMode>():nullptr;
        auto* PC=World?Cast<AShooterPlayerController>(World->GetFirstPlayerController()):nullptr;
        auto* Pawn=PC?Cast<AShooterCharacter>(PC->GetPawn()):nullptr;
        if (!Mode || !Pawn) return false;
        if (Mode->IsArmoryOpen() || Mode->GetWave()<1)
        {
            if (Mode->GetSetupStage()==ERidgefireSetupStage::Secondary) Mode->SelectSetupSecondary(PC,2);
            UGameplayStatics::SetGlobalTimeDilation(World,20.f);return false;
        }
        UGameplayStatics::SetGlobalTimeDilation(World,1.f);
        for (TActorIterator<AShooterNPC> It(World);It;++It) Freeze(*It);
        const double Now=FPlatformTime::Seconds();
        if (Stage==0)
        {
            FVector Station;
            bool bStation=false;
            for (TActorIterator<ARidgefireArenaDressing> It(World);It;++It)
                if (It->GetArsenalStationWorldLocation(Station)) { bStation=true;break; }
            if (!Test->TestTrue(TEXT("input fixture has actual arsenal"),bStation)) return true;
            Pawn->SetActorLocation(Station+FVector(-360,0,96));
            if (!Test->TestTrue(TEXT("input fixture can open actual arsenal"),Mode->SetPlayerArsenalOpen(PC,true))) return true;
            if(!PC->GetRidgefireRunState().PrimaryWeaponSlots.Contains(1)) {
                Mode->AwardSharedCash(600);
                if(!Test->TestTrue(TEXT("input fixture buys Phasma through production shop"),Mode->PurchaseWeapon(PC,1,0))) return true;
            }
            Mode->SelectWeaponOffer(PC,0);
            Mode->SetPlayerArsenalOpen(PC,false);
            ExpectedName=PC->GetRidgefireRunState().WeaponOffers[1].Name;
            PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::One,IE_Pressed,1));
            HeldKey=EKeys::One;Stage=1;NextEvent=Now+0.1;return false;
        }
        if (HeldKey.IsValid() && Now>=NextEvent)
        {
            if (HeldKey==EKeys::LeftShift)
            {
                bSprintObserved=true;
                Test->TestTrue(TEXT("processed sprint input still increases walk speed"),Pawn->GetCharacterMovement()->GetMaxSpeed()>InitialWalkSpeed);
            }
            PC->InputKey(FInputKeyEventArgs::CreateSimulated(HeldKey,IE_Released,0));
            HeldKey=FKey();NextEvent=Now+0.8;
        }
        if (Stage==1)
        {
            if (Now<NextEvent || HeldKey.IsValid()) return false;
            if (!Test->TestEqual(TEXT("processed primary input actually selects Phasma"),Pawn->GetActiveWeaponName(),ExpectedName)) return true;
            InitialAmmo=Pawn->GetRidgefireAmmo();InitialWalkSpeed=Pawn->GetCharacterMovement()->GetMaxSpeed();Stage=2;
        }
        if (Pawn->GetActiveWeaponName()!=ExpectedName || PC->GetRidgefireRunState().ActiveWeaponIndex!=1)
        {
            Test->AddError(FString::Printf(TEXT("Unrequested switch through input processing: action=%d held=%s expected=%s actual=%s offer=%d"),
                Action,*HeldKey.ToString(),*ExpectedName,*Pawn->GetActiveWeaponName(),PC->GetRidgefireRunState().ActiveWeaponIndex));return true;
        }
        if (Now<NextEvent || HeldKey.IsValid()) return false;
        if (Action>=Actions.Num())
        {
            if (!bSprintOnly) Test->TestTrue(TEXT("normal input firing and reload were processed"),Pawn->GetRidgefireReserveAmmo()<PC->GetRidgefireRunState().WeaponOffers[1].ReserveAmmo);
            Test->TestTrue(TEXT("input test started with a real loaded magazine"),InitialAmmo>0);
            Test->TestTrue(TEXT("actual sprint input was processed"),bSprintObserved);
            UE_LOG(LogTemp,Display,TEXT("RIDGEFIRE RELIABILITY INPUT PASS sprint_only=%d real PlayerController InputKey and normal per-frame processing; selected primary retained, actual sprint speed increased"),bSprintOnly);return true;
        }
        HeldKey=Actions[Action++];
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(HeldKey,IE_Pressed,1));
        NextEvent=Now+(HeldKey==EKeys::LeftShift ? 1.2 : 0.12);
        return false;
    }
};
}
bool FRidgefireSelectionBoundaryTest::RunTest(const FString&) { ADD_LATENT_AUTOMATION_COMMAND(FSelectionBoundary(this)); return true; }
bool FRidgefireVisibleBodyTest::RunTest(const FString&) { ADD_LATENT_AUTOMATION_COMMAND(FVisibleBodyAndCover(this)); return true; }
bool FRidgefireShotSelectionTest::RunTest(const FString&) { ADD_LATENT_AUTOMATION_COMMAND(FSelectionBoundary(this,true)); return true; }
bool FRidgefireInputSelectionTest::RunTest(const FString&) { ADD_LATENT_AUTOMATION_COMMAND(FInputSelection(this)); return true; }
bool FRidgefireSprintSelectionTest::RunTest(const FString&) { ADD_LATENT_AUTOMATION_COMMAND(FInputSelection(this,true)); return true; }
#if WITH_EDITOR
bool FRidgefirePIESelectionTest::RunTest(const FString&)
{
    ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/Variant_Shooter/Lvl_Shooter")));
    ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
    ADD_LATENT_AUTOMATION_COMMAND(FSelectionBoundary(this,true));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
    return true;
}
bool FRidgefirePIEInputTest::RunTest(const FString&)
{
    ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/Variant_Shooter/Lvl_Shooter")));
    ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
    ADD_LATENT_AUTOMATION_COMMAND(FInputSelection(this));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
    return true;
}
#endif
#endif
