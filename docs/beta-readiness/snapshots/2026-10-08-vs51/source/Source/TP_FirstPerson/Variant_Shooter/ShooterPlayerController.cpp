// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/ShooterPlayerController.h"
#include "RidgefireGameMode.h"
#include "RidgefireHealingTube.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameInstance.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "RidgefireCharacterMovement.h"
#include "Components/CapsuleComponent.h"
#include "InputCoreTypes.h"
#include "InputKeyEventArgs.h"
#include "ShooterCharacter.h"
#include "ShooterBulletCounterUI.h"
#include "TP_FirstPerson.h"
#include "Variant_Shooter/AI/ShooterNPC.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "EngineUtils.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

#if WITH_EDITOR
namespace
{
	bool bCoopRestartSmokeAwaitingFreshRun = false;
	int32 CoopRestartExpectedPersonalXP = 0;
}
#endif

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();
	LoadPersonalProgression();
    UpdateSessionInputMode();
#if WITH_EDITOR
	if (GetNetMode() == NM_Client && IsLocalController() && FParse::Param(FCommandLine::Get(), TEXT("RidgefireCoopCombatSmoke")))
	{
		if (bCoopRestartSmokeAwaitingFreshRun)
		{
			bCoopRestartSmokeAwaitingFreshRun = false;
			UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP CLIENT RESTART handoff consumed by fresh controller"));
			GetWorldTimerManager().SetTimer(CoopCombatSmokeTimer, this, &AShooterPlayerController::TickCoopRestartSmoke, 0.05f, true);
		}
		else
		{
			GetWorldTimerManager().SetTimer(CoopCombatSmokeTimer, this, &AShooterPlayerController::TickCoopCombatSmoke, 0.1f, true);
		}
	}
	if (HasAuthority() && IsLocalController() && FParse::Param(FCommandLine::Get(), TEXT("RidgefireSmokeTest")))
	{
		GetWorldTimerManager().SetTimer(RunOverPauseSmokeTimer, this, &AShooterPlayerController::TickRunOverPauseSmoke, 0.1f, true);
	}
#endif
}

#if WITH_EDITOR
void AShooterPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
    if(IsLocalController() && FApp::IsUnattended() && FParse::Param(FCommandLine::Get(),TEXT("RidgefireEconomyNetworkFixture"))) TickEconomyNetworkFixture();
    if (IsLocalController() && FApp::IsUnattended() && FParse::Param(FCommandLine::Get(), TEXT("RidgefireSetupNetworkFixture"))) TickTimedSetupNetworkFixture();
    if (IsLocalController() && FApp::IsUnattended() && FParse::Param(FCommandLine::Get(), TEXT("RidgefireRunNetworkFixture"))) TickRunNetworkFixture();
    if (IsLocalController() && FApp::IsUnattended() && FParse::Param(FCommandLine::Get(),TEXT("RidgefireLobbyFixture"))) TickLobbyNetworkFixture();
	if (bCoopRestartFreshArmoryAccepted && CoopRestartSmokeStage == 2 && IsLocalController())
	{
		if (AShooterCharacter* ShooterPawn = Cast<AShooterCharacter>(GetPawn()); ShooterPawn && !ShooterPawn->IsDead())
		{
			ShooterPawn->AddMovementInput(CoopRestartMovementDirection, 1.0f);
		}
	}
}

void AShooterPlayerController::TickTimedSetupNetworkFixture()
{
    if (bSetupFixtureDone)
    {
        if (FParse::Param(FCommandLine::Get(), TEXT("RidgefireSetupRecoveryFixture"))) TickSetupRecoveryNetworkFixture();
        return;
    }
    AShooterCharacter* Gladiator = Cast<AShooterCharacter>(GetPawn());
    if (!Gladiator) return;
    const auto& State = RidgefireRunState;
    if (State.SetupStage == ERidgefireSetupStage::PersonalPerks && State.SetupSeconds <= 0) return;
    const uint8 StageBit = 1 << static_cast<uint8>(State.SetupStage);
    if (!(SetupFixtureSeenStages & StageBit))
    {
        SetupFixtureSeenStages |= StageBit;
        UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SETUP NETWORK stage=%d seconds=%d vote_seconds=%d secondary=%d"), static_cast<int32>(State.SetupStage), State.SetupSeconds, State.StartingTeamVoteSeconds, State.SecondaryWeaponOfferIndex);
    }
    auto Press = [this](FKey Key) { FInputKeyEventArgs Args; Args.Key=Key; Args.Event=IE_Pressed; InputKey(Args); };
    if (State.SetupStage == ERidgefireSetupStage::Secondary && !bSetupFixtureSecondarySent)
    {
        bSetupFixtureSecondarySent = true;
        if (!HasAuthority()) Press(EKeys::Three);
        // Host abstains here to exercise the deterministic Phasma fallback.
    }
    if (State.SetupStage == ERidgefireSetupStage::TeamVote && State.StartingTeamVoteSeconds <= 3 && !bSetupFixtureVoteSent)
    {
        bSetupFixtureVoteSent = true;
        const bool bSkip = FParse::Param(FCommandLine::Get(), TEXT("RidgefireSetupNetworkSkip"));
        const int32 Choice = bSkip ? 2 : HasAuthority() ? 0 : 1;
        Press(Choice == 2 ? EKeys::F3 : Choice == 0 ? EKeys::F1 : EKeys::F2);
        ServerCastStartingTeamPerkVote(Choice);
        ServerCastStartingTeamPerkVote(99);
    }
    if (State.SetupStage == ERidgefireSetupStage::Weapons && !bSetupFixtureLateRequestsSent)
    {
        bSetupFixtureLateRequestsSent = true;
        // Exercise the authoritative stage lock and reserved-offer rejection via real RPCs.
        ServerSelectRidgefireSecondaryOffer(0);
        if (!HasAuthority()) ServerSelectRidgefireOffer(State.SecondaryWeaponOfferIndex, INDEX_NONE);
        // Let all peers deploy through the real timeout rather than skipping its timer.
    }
    if (State.SetupStage != ERidgefireSetupStage::Complete || State.Wave < 1) return;
    auto Fail = [this](const TCHAR* Reason) { UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SETUP NETWORK FAIL %s"), Reason); bSetupFixtureDone=true; };
    const bool bSkip = FParse::Param(FCommandLine::Get(), TEXT("RidgefireSetupNetworkSkip"));
    if (SetupFixtureSeenStages != 31 || State.bArmoryOpen || State.bStartingTeamVoteOpen || IsPersonalPerksOpen()) { Fail(TEXT("stage sequence or panels")); return; }
    if (State.SecondaryWeaponOfferIndex != (HasAuthority() ? 1 : 2) || State.ActiveWeaponIndex != 0) { Fail(TEXT("owned loadout or stage lock")); return; }
    if (PersonalProgression.FirstPerk != 0 || PersonalProgression.SecondPerk != 2) { Fail(TEXT("saved perk defaults")); return; }
    if (bSkip ? State.TeamArenaPerks != 0 : !(State.TeamArenaPerks & 12)) { Fail(TEXT("team vote result")); return; }
    if (SetupFixtureDrawStage == 0) { Gladiator->ToggleRidgefireSecondaryWeapon(); SetupFixtureDrawStage=1; return; }
    if (SetupFixtureDrawStage == 1)
    {
        if (!Gladiator->IsRidgefireSecondaryEquipped()) return;
        if (Gladiator->GetActiveWeaponName() != State.WeaponOffers[State.SecondaryWeaponOfferIndex].Name) { Fail(TEXT("actual secondary draw")); return; }
        Gladiator->ToggleRidgefireSecondaryWeapon(); SetupFixtureDrawStage=2; return;
    }
    if (Gladiator->IsRidgefireSecondaryEquipped()) return;
    if (Gladiator->GetActiveWeaponName() != TEXT("Gale Repeater")) { Fail(TEXT("actual primary return")); return; }
    bSetupFixtureDone=true;
    UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SETUP NETWORK PASS all_five_stages=true primary=0 secondary=%d perk_defaults=true mask=%d actual_secondary_draw=true"), State.SecondaryWeaponOfferIndex, State.TeamArenaPerks);
}

void AShooterPlayerController::TickSetupRecoveryNetworkFixture()
{
    if (bSetupRecoveryDone) return;
    ARidgefireHealingTube* Tube = nullptr;
    for (TActorIterator<ARidgefireHealingTube> It(GetWorld()); It; ++It)
        if (!It->IsActorBeingDestroyed()) { Tube=*It; break; }
    if (!Tube) return;
    if (!HasAuthority())
    {
        if (AShooterCharacter* Occupant = Tube->GetOccupant())
        {
            SetupRecoveryPatient=Occupant;
            bSetupRecoverySawOccupant=true;
            bSetupRecoverySawDowned |= Occupant->IsRidgefireDowned();
        }
        AShooterCharacter* Patient = SetupRecoveryPatient.Get();
        if (bSetupRecoverySawOccupant && bSetupRecoverySawDowned && !Tube->GetOccupant() && Patient && !Patient->IsRidgefireDowned() && !Patient->IsDead() && Patient->GetHealthRatio() > 0)
        {
            bSetupRecoveryDone=true;
            UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE FOUR RECOVERY CLIENT PASS observed_down=true observed_tube=true recovered_same_pawn=true owner_xp=%d"), PersonalProgression.XP);
        }
        return;
    }
    auto Fail = [this](const TCHAR* Reason) { UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE FOUR RECOVERY FAIL %s"), Reason); bSetupRecoveryDone=true; };
    ARidgefireGameMode* Mode=GetWorld()->GetAuthGameMode<ARidgefireGameMode>();
    if (!Mode || Mode->IsRunOver()) { Fail(TEXT("run not active")); return; }
    AShooterCharacter* Carrier=Cast<AShooterCharacter>(GetPawn());
    if (SetupRecoveryStage == 0)
    {
        // Give the owner-replicated secondary-draw checks time to finish before the server controls this scenario.
        if (GetWorld()->GetTimeSeconds() < 61) return;
        TArray<AShooterPlayerController*> Players;
        for (FConstPlayerControllerIterator It=GetWorld()->GetPlayerControllerIterator(); It; ++It)
            if (auto* Peer=Cast<AShooterPlayerController>(It->Get()); Peer && Cast<AShooterCharacter>(Peer->GetPawn())) Players.Add(Peer);
        if (Players.Num()!=4) { Fail(TEXT("requires four actual possessed peers")); return; }
        for (TActorIterator<AShooterNPC> Enemy(GetWorld()); Enemy; ++Enemy)
        {
            Enemy->SetActorTickEnabled(false);
            Enemy->GetCharacterMovement()->DisableMovement();
            if (Enemy->GetController()) Enemy->GetController()->SetActorTickEnabled(false);
        }
        AShooterCharacter* Patient=nullptr;
        for (AShooterPlayerController* Peer:Players)
            if (Peer!=this) { Patient=Cast<AShooterCharacter>(Peer->GetPawn()); break; }
        SetupRecoveryPatient=Patient;
        if (!Patient || !Carrier) { Fail(TEXT("patient or carrier missing")); return; }
        for (AShooterPlayerController* Peer:Players)
            if (Peer!=this && Peer->GetPawn()!=Patient)
                Peer->GetPawn()->SetActorLocation(Tube->GetActorLocation()+FVector(-800,600+(Players.IndexOfByKey(Peer)*180),100));
        UGameplayStatics::ApplyDamage(Patient,100000,this,Carrier,UDamageType::StaticClass());
        if (!Patient->IsRidgefireDowned() || Mode->IsRunOver()) { Fail(TEXT("single down must retain shared run")); return; }
        bool bDeposited=false;
        for (int32 Index=0; Index<8 && !bDeposited; ++Index)
        {
            const FVector Direction=FRotator(0,Index*45,0).Vector();
            Carrier->SetActorLocation(Tube->GetActorLocation()+Direction*175+FVector(0,0,80));
            Patient->SetActorLocation(Carrier->GetActorLocation()+Direction*120);
            if (Carrier->TryCarryRidgefirePlayer(Patient)) bDeposited=Tube->TryDeposit(Carrier);
            if (!bDeposited) Carrier->ReleaseRidgefireCarriedPlayer();
        }
        if (!bDeposited || Tube->GetOccupant()!=Patient) { Fail(TEXT("actual owned teammate carry/deposit")); return; }
        Carrier->SetActorLocation(Tube->GetActorLocation()+FVector(600,-600,100));
        // Actual death rewards must include the downed peer and cannot repeat on a dead actor.
        TArray<int32> XP;
        for (auto* Peer:Players) XP.Add(Peer->GetPersonalProgression().XP);
        AShooterNPC* Target=nullptr;
        for (TActorIterator<AShooterNPC> Enemy(GetWorld()); Enemy; ++Enemy) if (Enemy->CurrentHP>0) { Target=*Enemy;break; }
        if (!Target) { Fail(TEXT("wave target missing"));return; }
        UGameplayStatics::ApplyDamage(Target,100000,this,Carrier,UDamageType::StaticClass());
        UGameplayStatics::ApplyDamage(Target,100000,this,Carrier,UDamageType::StaticClass());
        for(int32 Index=0;Index<Players.Num();++Index)
            if(Players[Index]->GetPersonalProgression().XP != XP[Index]+10) { Fail(TEXT("four-Peer XP/dead-hit duplicate"));return; }
        SetupRecoveryDepositorXP=PersonalProgression.XP;
        SetupRecoveryStartedAt=GetWorld()->GetTimeSeconds();
        SetupRecoveryStage=1;
        UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE FOUR RECOVERY deposited actual remote Peer; team continues; four owned profiles gained exactly 10 XP including patient"));
        return;
    }
    AShooterCharacter* Patient=SetupRecoveryPatient.Get();
    if (!Patient) { Fail(TEXT("patient vanished"));return; }
    if (Tube->GetOccupant()) return;
    if (GetWorld()->GetTimeSeconds()-SetupRecoveryStartedAt<59.9f || Patient->IsDead() || Patient->IsRidgefireDowned() || Patient->GetHealthRatio()<=0
        || PersonalProgression.XP!=SetupRecoveryDepositorXP+12 || Mode->IsRunOver()) { Fail(TEXT("earned recovery/XP/run state"));return; }
    bSetupRecoveryDone=true;
    UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE FOUR RECOVERY HOST PASS four_possessed_peers=true real_60_seconds=true same_patient=true depositor_xp=12 team_active=true"));
}

void AShooterPlayerController::TickCoopCombatSmoke()
{
	CoopCombatSmokeElapsed += 0.1f;
	AShooterCharacter* ShooterPawn = Cast<AShooterCharacter>(GetPawn());
	if (!bCoopStartingVoteSent && ShooterPawn && RidgefireRunState.bStartingTeamVoteOpen && RidgefireRunState.bArmoryOpen)
	{
		bCoopStartingVoteSent = true;
		FInputKeyEventArgs VoteInput; VoteInput.Key = EKeys::F1; VoteInput.Event = IE_Pressed;
		InputKey(VoteInput);
		ServerCastStartingTeamPerkVote(0);
		ServerCastStartingTeamPerkVote(99);
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP STARTING VOTE sent through controller input; repeated and invalid ballot requests follow"));
	}

	if (bCoopTeamWipeSmokeRequestSent && !bCoopRestartSmokeEarlyRequestSent && ShooterPawn && !ShooterPawn->IsDead()
		&& !RidgefireRunState.bRunOver && FParse::Param(FCommandLine::Get(), TEXT("RidgefireRestartSmoke")))
	{
		for (TActorIterator<AShooterCharacter> It(GetWorld()); It; ++It)
		{
			if (*It != ShooterPawn && It->IsDead())
			{
				bCoopRestartSmokeEarlyRequestSent = true;
				UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP CLIENT RESTART_EARLY_REQUEST while surviving teammate remains active"));
				RestartRun();
				break;
			}
		}
	}
	if (bCoopTeamWipeSmokeRequestSent && ShooterPawn && ShooterPawn->IsDead() && RidgefireRunState.bRunOver)
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP CLIENT TEAM_WIPE PASS own_defeat=true replicated_run_over=true"));
		GetWorldTimerManager().ClearTimer(CoopCombatSmokeTimer);
		if (FParse::Param(FCommandLine::Get(), TEXT("RidgefireRestartSmoke")))
		{
			bCoopRestartSmokeAwaitingFreshRun = true;
			CoopRestartExpectedPersonalXP = PersonalProgression.XP;
			UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP CLIENT RESTART_REQUEST after replicated team wipe; repeated_requests=3"));
			for (int32 RequestIndex = 0; RequestIndex < 3; ++RequestIndex)
			{
				RestartRun();
			}
		}
		return;
	}
	if (!bCoopCombatSmokeStarted)
	{
		if (!ShooterPawn || RidgefireRunState.bArmoryOpen || RidgefireRunState.Wave < 1 || ShooterPawn->GetRidgefireAmmo() <= 3)
		{
			if (CoopCombatSmokeElapsed >= 45.0f)
			{
				UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE COOP CLIENT SMOKE FAIL timed out waiting for wave, remote pawn, and equipped ammo"));
				GetWorldTimerManager().ClearTimer(CoopCombatSmokeTimer);
			}
			return;
		}
		float ClosestDistance = TNumericLimits<float>::Max();
		for (TActorIterator<AShooterNPC> It(GetWorld()); It; ++It)
		{
			AShooterNPC* Candidate = *It;
			if (!IsValid(Candidate) || Candidate->CurrentHP <= 0.0f || Candidate->CurrentHP > 1.1f)
			{
				continue;
			}
			const float Distance = FVector::DistSquared(Candidate->GetActorLocation(), ShooterPawn->GetActorLocation());
			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				CoopCombatSmokeTarget = Candidate;
			}
		}
		AShooterNPC* Target = CoopCombatSmokeTarget.Get();
		if (!Target)
		{
			return;
		}
		CoopCombatSmokeTargetLocation = Target->GetActorLocation() + FVector(0.0f, 0.0f, 35.0f);
		CoopCombatSmokeInitialAmmo = ShooterPawn->GetRidgefireAmmo();
		CoopCombatSmokeInitialScore = RidgefireRunState.Score;
		bCoopCombatSmokeStarted = true;
		const FRotator AimRotation = (CoopCombatSmokeTargetLocation - ShooterPawn->GetPawnViewLocation()).Rotation();
		SetControlRotation(AimRotation);
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP CLIENT SMOKE START target=%s hp=%.0f ammo=%d score=%d"),
			*GetNameSafe(Target), Target->CurrentHP, CoopCombatSmokeInitialAmmo, CoopCombatSmokeInitialScore);
		ServerSetCoopCombatSmokeAim(AimRotation);
		return;
	}

	if (bCoopCombatSmokeTriggerHeld)
	{
		if (ShooterPawn)
		{
			ShooterPawn->DoStopFiring();
		}
		bCoopCombatSmokeTriggerHeld = false;
		++CoopCombatSmokeShotsSent;
		CoopCombatSmokeDelayTicks = 2;
	}
	else if (CoopCombatSmokeShotsSent < 3)
	{
		if (CoopCombatSmokeDelayTicks > 0)
		{
			--CoopCombatSmokeDelayTicks;
		}
		else if (ShooterPawn)
		{
			SetControlRotation((CoopCombatSmokeTargetLocation - ShooterPawn->GetPawnViewLocation()).Rotation());
			ShooterPawn->DoStartFiring();
			bCoopCombatSmokeTriggerHeld = true;
		}
	}

	ShooterPawn = Cast<AShooterCharacter>(GetPawn());
	AShooterNPC* Target = CoopCombatSmokeTarget.Get();
	const float TargetHealth = Target ? Target->CurrentHP : 0.0f;
	const int32 CurrentAmmo = ShooterPawn ? ShooterPawn->GetRidgefireAmmo() : CoopCombatSmokeInitialAmmo;
	const bool bCombatChecksPassed = CoopCombatSmokeShotsSent >= 3 && ShooterPawn
		&& CurrentAmmo < CoopCombatSmokeInitialAmmo && TargetHealth <= 0.0f
		&& RidgefireRunState.Score == CoopCombatSmokeInitialScore + 110;
	const int32 CurrentReserveAmmo = ShooterPawn ? ShooterPawn->GetRidgefireReserveAmmo() : CoopCombatSmokeReloadStartReserve;
	if (CoopCombatSmokeReloadStage == 0 && bCombatChecksPassed && bCoopCombatSmokeHostCompleted
		&& CurrentAmmo == CoopCombatSmokeAuthoritativeAmmo && CurrentReserveAmmo == CoopCombatSmokeAuthoritativeReserve)
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP CLIENT SMOKE PASS replicated ammo=%d target_hp=%.0f score=%d after host completion"),
			CurrentAmmo, TargetHealth, RidgefireRunState.Score);
		if (PersonalProgression.XP < 10 || PersonalProgression.XP > 13 || (RidgefireRunState.TeamArenaPerks & 1) == 0) return;
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP PROGRESSION OWNER PASS xp=%d replicated to owning client; team_momentum=true"), PersonalProgression.XP);
		CoopCombatSmokeReloadStartAmmo = ShooterPawn->GetRidgefireAmmo();
		CoopCombatSmokeReloadStartReserve = CurrentReserveAmmo;
		if (CoopCombatSmokeReloadStartReserve <= 0)
		{
			UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE COOP CLIENT SMOKE FAIL reload requires positive reserve ammo; magazine=%d reserve=%d"),
				CoopCombatSmokeReloadStartAmmo, CoopCombatSmokeReloadStartReserve);
			GetWorldTimerManager().ClearTimer(CoopCombatSmokeTimer);
			return;
		}
		CoopCombatSmokeReloadStage = 1;
		ReloadRidgefireWeapon();
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP CLIENT SMOKE RELOAD_REQUEST magazine=%d reserve=%d"),
			CoopCombatSmokeReloadStartAmmo, CoopCombatSmokeReloadStartReserve);
		return;
	}

	const int32 MagazineAmmoGained = CurrentAmmo - CoopCombatSmokeReloadStartAmmo;
	const int32 ReserveAmmoSpent = CoopCombatSmokeReloadStartReserve - CurrentReserveAmmo;
	if (CoopCombatSmokeReloadStage == 1 && ShooterPawn && ShooterPawn->IsRidgefireReloading())
	{
		CoopCombatSmokeReloadStage = 2;
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP CLIENT SMOKE RELOAD_STARTED observed_replicated=true"));
	}
	else if (CoopCombatSmokeReloadStage == 2 && !bCoopTeamWipeSmokeRequestSent && ShooterPawn && !ShooterPawn->IsRidgefireReloading()
		&& MagazineAmmoGained > 0 && MagazineAmmoGained == ReserveAmmoSpent)
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP CLIENT SMOKE RELOAD PASS replicated_reload=true->false magazine=%d->%d reserve=%d->%d transferred=%d"),
			CoopCombatSmokeReloadStartAmmo, CurrentAmmo, CoopCombatSmokeReloadStartReserve, CurrentReserveAmmo,
			MagazineAmmoGained);
		if (FParse::Param(FCommandLine::Get(), TEXT("RidgefireTeamWipeSmoke")))
		{
			bCoopTeamWipeSmokeRequestSent = true;
			ServerRequestCoopTeamWipeSmoke();
		}
		else
		{
			GetWorldTimerManager().ClearTimer(CoopCombatSmokeTimer);
		}
		return;
	}
	if (CoopCombatSmokeElapsed >= 45.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE COOP CLIENT SMOKE FAIL replicated checks incomplete shots=%d ammo=%d/%d target_hp=%.0f score=%d/%d host_completed=%s baseline_ammo=%d baseline_reserve=%d reload_stage=%d reloading=%s magazine=%d->%d reserve=%d->%d"),
			CoopCombatSmokeShotsSent, CurrentAmmo, CoopCombatSmokeInitialAmmo, TargetHealth,
			RidgefireRunState.Score, CoopCombatSmokeInitialScore + 110,
			bCoopCombatSmokeHostCompleted ? TEXT("true") : TEXT("false"), CoopCombatSmokeAuthoritativeAmmo, CoopCombatSmokeAuthoritativeReserve,
			CoopCombatSmokeReloadStage,
			ShooterPawn && ShooterPawn->IsRidgefireReloading() ? TEXT("true") : TEXT("false"),
			CoopCombatSmokeReloadStartAmmo, CurrentAmmo, CoopCombatSmokeReloadStartReserve,
			ShooterPawn ? ShooterPawn->GetRidgefireReserveAmmo() : -1);
		GetWorldTimerManager().ClearTimer(CoopCombatSmokeTimer);
	}
}

void AShooterPlayerController::TickCoopRestartSmoke()
{
	CoopCombatSmokeElapsed += 0.05f;
	AShooterCharacter* ShooterPawn = Cast<AShooterCharacter>(GetPawn());
	const bool bFreshRun = GetNetMode() == NM_Client && IsLocalController() && ShooterPawn && !ShooterPawn->IsDead()
		&& !ShooterPawn->ActorHasTag(TEXT("Dead")) && !RidgefireRunState.bRunOver
		&& RidgefireRunState.bArmoryOpen && RidgefireRunState.Wave == 0
		&& RidgefireRunState.Score == 0 && RidgefireRunState.WeaponOffers.Num() == 3 && RidgefireRunState.TeamArenaPerks == 0;
	if (!bCoopRestartFreshArmoryAccepted && bFreshRun && PersonalProgression.XP == CoopRestartExpectedPersonalXP)
	{
		bCoopRestartFreshArmoryAccepted = true;
		CoopRestartSmokeStage = 1;
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP CLIENT RESTART PASS remained remote with living pawn and replicated fresh armory wave=0 score=0 offers=3"));
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP PROGRESSION RESTART PASS xp=%d retained on owning remote across shared restart; team_perks_reset=true"), PersonalProgression.XP);
		ChooseOfferOne();
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP CLIENT RESTART selected opening offer through normal input path"));
		return;
	}
	if (CoopRestartSmokeStage == 2)
	{
		if (!ShooterPawn || ShooterPawn->IsDead())
		{
			UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE COOP CLIENT SMOKE FAIL post-restart movement lost its living pawn"));
			GetWorldTimerManager().ClearTimer(CoopCombatSmokeTimer);
			return;
		}
		CoopRestartMovementElapsed += 0.05f;
		if (CoopRestartMovementElapsed >= 2.0f)
		{
			const FVector Displacement = ShooterPawn->GetActorLocation() - CoopRestartMovementStartLocation;
			CoopRestartLocalMovementProjection = FVector::DotProduct(Displacement, CoopRestartMovementDirection);
			if (CoopRestartLocalMovementProjection < 75.0f)
			{
				UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE COOP CLIENT SMOKE FAIL post-restart movement input displaced only %.1f cm"), CoopRestartLocalMovementProjection);
				GetWorldTimerManager().ClearTimer(CoopCombatSmokeTimer);
				return;
			}
			CoopRestartSmokeStage = 3;
			UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP CLIENT RESTART PASS real movement input projected displacement %.1f cm"),
				CoopRestartLocalMovementProjection);
			if (bCoopRestartFireStartPending)
			{
				bCoopRestartFireStartPending = false;
				BeginCoopRestartFireSmoke(CoopRestartPendingFireAmmo);
			}
		}
		return;
	}
	if (CoopRestartSmokeStage == 4)
	{
		if (!ShooterPawn || ShooterPawn->IsDead())
		{
			UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE COOP CLIENT SMOKE FAIL post-restart firing lost its living pawn"));
			GetWorldTimerManager().ClearTimer(CoopCombatSmokeTimer);
			return;
		}
		if (ShooterPawn->GetRidgefireAmmo() < CoopRestartAmmoBeforeFire)
		{
			ShooterPawn->DoStopFiring();
			CoopRestartSmokeStage = 5;
			UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP CLIENT RESTART PASS replicated magazine consumed after movement %.1f cm; ammo %d->%d"),
				CoopRestartLocalMovementProjection, CoopRestartAmmoBeforeFire, ShooterPawn->GetRidgefireAmmo());
			GetWorldTimerManager().ClearTimer(CoopCombatSmokeTimer);
			return;
		}
		CoopRestartFireElapsed += 0.05f;
		if (CoopRestartFireElapsed >= 6.0f)
		{
			ShooterPawn->DoStopFiring();
			UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE COOP CLIENT SMOKE FAIL post-restart fire did not replicate magazine consumption; ammo=%d"),
				ShooterPawn->GetRidgefireAmmo());
			GetWorldTimerManager().ClearTimer(CoopCombatSmokeTimer);
			return;
		}
	}
	if (CoopCombatSmokeElapsed >= 45.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE COOP CLIENT SMOKE FAIL restart gameplay checks timed out (stage=%d wave=%d armory=%d)"),
			CoopRestartSmokeStage, RidgefireRunState.Wave, RidgefireRunState.bArmoryOpen);
		GetWorldTimerManager().ClearTimer(CoopCombatSmokeTimer);
	}
}

void AShooterPlayerController::BeginCoopRestartMovementSmoke()
{
	if (!bCoopRestartFreshArmoryAccepted || !IsLocalController() || !FParse::Param(FCommandLine::Get(), TEXT("RidgefireRestartSmoke")))
	{
		return;
	}
	AShooterCharacter* ShooterPawn = Cast<AShooterCharacter>(GetPawn());
	if (!ShooterPawn || ShooterPawn->IsDead())
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE COOP CLIENT SMOKE FAIL post-restart movement could not start without a living pawn"));
		return;
	}
	CoopRestartMovementStartLocation = ShooterPawn->GetActorLocation();
	CoopRestartMovementDirection = ShooterPawn->GetActorForwardVector();
	CoopRestartMovementDirection.Z = 0.0f;
	CoopRestartMovementDirection = CoopRestartMovementDirection.GetSafeNormal();
	if (CoopRestartMovementDirection.IsNearlyZero())
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE COOP CLIENT SMOKE FAIL post-restart movement has no horizontal direction"));
		return;
	}
	CoopRestartMovementElapsed = 0.0f;
	CoopRestartSmokeStage = 2;
	FHitResult LaneHit;
	const UCapsuleComponent* Capsule = ShooterPawn->GetCapsuleComponent();
	const bool bLaneBlocked = GetWorld()->SweepSingleByChannel(LaneHit,
		ShooterPawn->GetActorLocation() + FVector(0.0f, 0.0f, 2.0f),
		ShooterPawn->GetActorLocation() + FVector(0.0f, 0.0f, 2.0f) + CoopRestartMovementDirection * 900.0f,
		FQuat::Identity, ECC_Pawn,
		FCollisionShape::MakeCapsule(Capsule->GetScaledCapsuleRadius(), Capsule->GetScaledCapsuleHalfHeight()),
		FCollisionQueryParams(SCENE_QUERY_STAT(RidgefireRestartMovementDiagnostic), false, ShooterPawn));
	UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP CLIENT RESTART movement diagnostic mode=%d ignored=%d speed=%.1f blocked=%d penetration=%d blocker=%s component=%s distance=%.1f"),
		static_cast<int32>(ShooterPawn->GetCharacterMovement()->MovementMode), IsMoveInputIgnored(),
		ShooterPawn->GetCharacterMovement()->MaxWalkSpeed, bLaneBlocked, LaneHit.bStartPenetrating,
		*GetNameSafe(LaneHit.GetActor()), *GetNameSafe(LaneHit.GetComponent()), LaneHit.Distance);
	UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP CLIENT RESTART movement input started through CharacterMovement"));
}

void AShooterPlayerController::BeginCoopRestartFireSmoke(int32 AuthoritativeAmmoBeforeFire)
{
	if (CoopRestartSmokeStage != 3 || !IsLocalController()
		|| !FParse::Param(FCommandLine::Get(), TEXT("RidgefireRestartSmoke")))
	{
		return;
	}
	AShooterCharacter* ShooterPawn = Cast<AShooterCharacter>(GetPawn());
	if (!ShooterPawn || ShooterPawn->IsDead()
		|| CoopRestartLocalMovementProjection < 75.0f
		|| ShooterPawn->GetRidgefireAmmo() != AuthoritativeAmmoBeforeFire)
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE COOP CLIENT SMOKE FAIL post-restart fire precondition invalid (movement=%.1f ammo=%d/%d)"),
			CoopRestartLocalMovementProjection, ShooterPawn ? ShooterPawn->GetRidgefireAmmo() : -1, AuthoritativeAmmoBeforeFire);
		GetWorldTimerManager().ClearTimer(CoopCombatSmokeTimer);
		return;
	}
	CoopRestartAmmoBeforeFire = AuthoritativeAmmoBeforeFire;
	CoopRestartFireElapsed = 0.0f;
	CoopRestartSmokeStage = 4;
	ShooterPawn->DoStartFiring();
	UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP CLIENT RESTART firing through normal weapon input and authoritative RPC"));
}

void AShooterPlayerController::BeginCoopCombatSmokeShot()
{
	if (AShooterCharacter* ShooterPawn = Cast<AShooterCharacter>(GetPawn()))
	{
		ShooterPawn->DoStartFiring();
		bCoopCombatSmokeTriggerHeld = true;
	}
}
#endif

void AShooterPlayerController::ServerSetCoopCombatSmokeAim_Implementation(FRotator AimRotation)
{
	if (!FParse::Param(FCommandLine::Get(), TEXT("RidgefireCoopCombatSmoke"))
		|| !FMath::IsFinite(AimRotation.Pitch) || !FMath::IsFinite(AimRotation.Yaw))
	{
		return;
	}
	AShooterCharacter* ShooterPawn = Cast<AShooterCharacter>(GetPawn());
	if (!ShooterPawn)
	{
		return;
	}
	AimRotation.Normalize();
	SetControlRotation(AimRotation);
	ShooterPawn->SetActorRotation(FRotator(0.0f, AimRotation.Yaw, 0.0f));
	if (UCameraComponent* Camera = ShooterPawn->GetFirstPersonCameraComponent())
	{
		Camera->SetWorldRotation(AimRotation);
	}
	UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP SERVER AIM_SYNC pawn=%s rotation=%s"),
		*GetNameSafe(ShooterPawn), *AimRotation.ToString());
	ClientConfirmCoopCombatSmokeAim();
}

void AShooterPlayerController::ServerRequestCoopTeamWipeSmoke_Implementation()
{
#if WITH_EDITOR
	if (!FParse::Param(FCommandLine::Get(), TEXT("RidgefireCoopCombatSmoke"))
		|| !FParse::Param(FCommandLine::Get(), TEXT("RidgefireTeamWipeSmoke")))
	{
		return;
	}
	ARidgefireGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ARidgefireGameMode>() : nullptr;
	if (!GameMode || !GameMode->RunCoopTeamWipeSmokeTest(this))
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE COOP SMOKE FAIL team-wipe smoke request rejected by the server"));
	}
#endif
}

void AShooterPlayerController::ClientConfirmCoopCombatSmokeAim_Implementation()
{
#if WITH_EDITOR
	if (FParse::Param(FCommandLine::Get(), TEXT("RidgefireCoopCombatSmoke")) && bCoopCombatSmokeStarted && !bCoopCombatSmokeTriggerHeld)
	{
		BeginCoopCombatSmokeShot();
	}
#endif
}

void AShooterPlayerController::ClientAuthorizeCoopCombatSmokeReload_Implementation(int32 AuthoritativeAmmo, int32 AuthoritativeReserve)
{
#if WITH_EDITOR
	if (FParse::Param(FCommandLine::Get(), TEXT("RidgefireRestartSmoke")) && bCoopRestartFreshArmoryAccepted)
	{
		if (CoopRestartSmokeStage == 1)
		{
			BeginCoopRestartMovementSmoke();
		}
		else if (CoopRestartSmokeStage == 2)
		{
			bCoopRestartFireStartPending = true;
			CoopRestartPendingFireAmmo = AuthoritativeAmmo;
		}
		else if (CoopRestartSmokeStage == 3)
		{
			BeginCoopRestartFireSmoke(AuthoritativeAmmo);
		}
		return;
	}
	if (!FParse::Param(FCommandLine::Get(), TEXT("RidgefireCoopCombatSmoke")) || !bCoopCombatSmokeStarted)
	{
		return;
	}
	CoopCombatSmokeAuthoritativeAmmo = AuthoritativeAmmo;
	CoopCombatSmokeAuthoritativeReserve = AuthoritativeReserve;
	bCoopCombatSmokeHostCompleted = true;
	UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP CLIENT SMOKE HOST_COMPLETE authoritative_ammo=%d reserve=%d"),
		AuthoritativeAmmo, AuthoritativeReserve);
#endif
}

void AShooterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShooterPlayerController, RidgefireRunState);
	DOREPLIFETIME_CONDITION(AShooterPlayerController, PersonalProgression, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AShooterPlayerController, bProgressionPersistent, COND_OwnerOnly);
}

void AShooterPlayerController::SetRidgefireRunState(const FRidgefireClientRunState& NewState)
{
	if (HasAuthority())
	{
		RidgefireRunState = NewState;
		OnRep_RidgefireRunState();
		ForceNetUpdate();
	}
}

const FRidgefireClientRunState& AShooterPlayerController::GetRidgefireRunState() const
{
	return RidgefireRunState;
}

int32 AShooterPlayerController::GetPendingPrimaryReplacementSlot() const
{
	return PendingPrimaryReplacementSlot;
}

void AShooterPlayerController::OnRep_RidgefireRunState()
{
	// Seamless party restart retains this controller. A new loadout stage must
	// discard the prior run's menus and queued choices, while keeping its profile.
	if (bSetupStageObserved && ObservedSetupStage == ERidgefireSetupStage::Complete
		&& RidgefireRunState.SetupStage == ERidgefireSetupStage::PersonalPerks
		&& RidgefireRunState.bArmoryOpen && RidgefireRunState.Wave == 0)
	{
		bSessionMenuOpen = false;
		PendingWeaponOfferIndex = INDEX_NONE;
		PendingPrimaryReplacementSlot = INDEX_NONE;
		GetWorldTimerManager().ClearTimer(PendingWeaponOfferTimer);
	}
	if (!bSetupStageObserved || ObservedSetupStage != RidgefireRunState.SetupStage)
	{
		bPersonalPerksOpen = RidgefireRunState.SetupStage == ERidgefireSetupStage::PersonalPerks && RidgefireRunState.bArmoryOpen;
		ObservedSetupStage = RidgefireRunState.SetupStage;
		bSetupStageObserved = true;
	}
	if (!RidgefireRunState.bArmoryOpen && !RidgefireRunState.bRunOver) bPersonalPerksOpen = false;
    if (RidgefireRunState.bLobbyOpen) bSessionMenuOpen=false;
    UpdateSessionInputMode();
}

bool AShooterPlayerController::InputKey(const FInputKeyEventArgs& Params)
{
	if (Params.Event != IE_Released && (Params.Event != IE_Axis || FMath::Abs(Params.AmountDepressed) > 0.2f || Params.AmountDepressed2D.SizeSquared() > 0.04f))
	{
		bUsingGamepad = Params.IsGamepad();
	}
    if (HandleSessionInput(Params)) return true;
    if(Params.Event==IE_Pressed && Params.Key==EKeys::Gamepad_LeftThumbstick && RidgefireRunState.bPersonalArsenalOpen) {ShopToggleDetails();return true;}
	if (Params.Event == IE_Pressed && (Params.Key == EKeys::K || (Params.Key == EKeys::Gamepad_LeftThumbstick && (RidgefireRunState.bArmoryOpen || RidgefireRunState.bRunOver))))
	{
		TogglePersonalPerks();
		return true;
	}
    if (RidgefireRunState.bRouteVoteOpen && Params.Event==IE_Pressed)
    {
        int32 Choice=Params.Key==EKeys::One || Params.Key==EKeys::Gamepad_FaceButton_Bottom ? 0
            : Params.Key==EKeys::Two || Params.Key==EKeys::Gamepad_FaceButton_Left ? 1 : -1;
        if (Choice>=0) { ServerCastRouteVote(Choice,RidgefireRunState.RouteEpoch); return true; }
    }
	if (IsPersonalPerksOpen())
	{
		if (Params.Event == IE_Pressed)
		{
			if (Params.Key == EKeys::Escape || Params.Key == EKeys::Gamepad_Special_Right) bPersonalPerksOpen = false;
			else if (Params.Key == EKeys::Up || Params.Key == EKeys::Gamepad_DPad_Up) PersonalPerkCursor = (PersonalPerkCursor + 9) % 10;
			else if (Params.Key == EKeys::Down || Params.Key == EKeys::Gamepad_DPad_Down) PersonalPerkCursor = (PersonalPerkCursor + 1) % 10;
			else if (Params.Key == EKeys::One || Params.Key == EKeys::Gamepad_FaceButton_Bottom) { if (HasAuthority()) EquipPersonalPerk(0, PersonalPerkCursor); else ServerEquipPersonalPerk(0, PersonalPerkCursor); }
			else if (Params.Key == EKeys::Two || Params.Key == EKeys::Gamepad_FaceButton_Left) { if (HasAuthority()) EquipPersonalPerk(1, PersonalPerkCursor); else ServerEquipPersonalPerk(1, PersonalPerkCursor); }
		}
		return true;
	}
	if (RidgefireRunState.bArmoryOpen && RidgefireRunState.SetupStage == ERidgefireSetupStage::Secondary)
	{
		if (Params.Event == IE_Pressed)
		{
			const int32 Choice = Params.Key == EKeys::One || Params.Key == EKeys::Gamepad_FaceButton_Bottom ? 0
				: Params.Key == EKeys::Two || Params.Key == EKeys::Gamepad_FaceButton_Right ? 1
				: Params.Key == EKeys::Three || Params.Key == EKeys::Gamepad_FaceButton_Left ? 2 : -1;
			if (Choice >= 0) RequestSecondaryWeaponOffer(Choice);
		}
		return true;
	}
	if (RidgefireRunState.bStartingTeamVoteOpen && RidgefireRunState.bArmoryOpen && Params.Event == IE_Pressed)
	{
		const int32 Choice = Params.Key == EKeys::F1 || Params.Key == EKeys::Gamepad_FaceButton_Bottom ? 0
			: Params.Key == EKeys::F2 || Params.Key == EKeys::Gamepad_FaceButton_Left ? 1
			: Params.Key == EKeys::F3 || Params.Key == EKeys::Gamepad_FaceButton_Right ? 2 : -1;
		if (Choice >= 0) { ServerCastStartingTeamPerkVote(Choice); return true; }
	}
 if(Params.Key==EKeys::RightMouseButton || Params.Key==EKeys::Gamepad_LeftTrigger)
 {
  if(auto* P=Cast<AShooterCharacter>(GetPawn()))
   P->SetRidgefireAiming(Params.Event==IE_Pressed || Params.Event==IE_Repeat || Params.Event==IE_DoubleClick || (Params.Event==IE_Axis && Params.AmountDepressed>0.5f));
  return true;
 }
 if (Params.Key == EKeys::C || Params.Key == EKeys::LeftControl || Params.Key == EKeys::Gamepad_RightThumbstick)
 {
  if (auto* Gladiator = Cast<AShooterCharacter>(GetPawn()))
  {
   if (Params.Event == IE_Pressed || (Params.Event == IE_Axis && Params.AmountDepressed > 0.5f)) Gladiator->StartRidgefireCrouch();
   else if (Params.Event == IE_Released || Params.Event == IE_Axis) Gladiator->StopRidgefireCrouch();
  }
  return true;
 }
	return Super::InputKey(Params);
}

void AShooterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
    InputComponent->BindKey(EKeys::Tab,IE_Pressed,this,&AShooterPlayerController::ShopToggleDetails);
    InputComponent->BindKey(EKeys::Left,IE_Pressed,this,&AShooterPlayerController::ShopPrevious);
    InputComponent->BindKey(EKeys::Right,IE_Pressed,this,&AShooterPlayerController::ShopNext);
    InputComponent->BindKey(EKeys::Enter,IE_Pressed,this,&AShooterPlayerController::ShopConfirm);
    InputComponent->BindKey(EKeys::U,IE_Pressed,this,&AShooterPlayerController::ShopUpgradeEcho);
    InputComponent->BindKey(EKeys::I,IE_Pressed,this,&AShooterPlayerController::ShopUpgradeThrough);
    InputComponent->BindKey(EKeys::J,IE_Pressed,this,&AShooterPlayerController::ShopSecondary);
    InputComponent->BindKey(EKeys::N,IE_Pressed,this,&AShooterPlayerController::ReadyForEncounter);
    InputComponent->BindKey(EKeys::MiddleMouseButton,IE_Pressed,this,&AShooterPlayerController::MeleeFinisher);
    InputComponent->BindKey(EKeys::M,IE_Pressed,this,&AShooterPlayerController::MeleeFinisher);
	InputComponent->BindKey(EKeys::E, IE_Pressed, this, &AShooterPlayerController::ActivateIonSurge);
	InputComponent->BindKey(EKeys::F, IE_Pressed, this, &AShooterPlayerController::UseFieldPatch);
	InputComponent->BindKey(EKeys::LeftShift, IE_Pressed, this, &AShooterPlayerController::StartSprint);
	InputComponent->BindKey(EKeys::LeftShift, IE_Released, this, &AShooterPlayerController::StopSprint);
	InputComponent->BindKey(EKeys::One, IE_Pressed, this, &AShooterPlayerController::ChooseOfferOne);
	InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &AShooterPlayerController::ChooseOfferTwo);
	InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &AShooterPlayerController::ChooseOfferThree);
	InputComponent->BindKey(EKeys::G, IE_Pressed, this, &AShooterPlayerController::RequestWildcardReroll);
	InputComponent->BindKey(EKeys::H, IE_Pressed, this, &AShooterPlayerController::UseFoundryFurnaceAnchor);
	InputComponent->BindKey(EKeys::B, IE_Pressed, this, &AShooterPlayerController::ToggleRidgefireSecondaryWeapon);
	InputComponent->BindKey(EKeys::T, IE_Pressed, this, &AShooterPlayerController::InteractWithArsenal);
	InputComponent->BindKey(EKeys::V, IE_Pressed, this, &AShooterPlayerController::InteractWithTeammate);
	InputComponent->BindKey(EKeys::Gamepad_DPad_Down, IE_Pressed, this, &AShooterPlayerController::ChoosePrimaryReplacementTwo);
	InputComponent->BindKey(EKeys::Gamepad_DPad_Right, IE_Pressed, this, &AShooterPlayerController::RequestWildcardReroll);
	InputComponent->BindKey(EKeys::Gamepad_DPad_Left, IE_Pressed, this, &AShooterPlayerController::UseFoundryFurnaceAnchor);
	InputComponent->BindKey(EKeys::Gamepad_FaceButton_Bottom, IE_Pressed, this, &AShooterPlayerController::ChooseOfferOne);
	InputComponent->BindKey(EKeys::Gamepad_FaceButton_Right, IE_Pressed, this, &AShooterPlayerController::ChooseOfferTwo);
	InputComponent->BindKey(EKeys::Gamepad_FaceButton_Left, IE_Pressed, this, &AShooterPlayerController::ChooseOfferThree);
	InputComponent->BindKey(EKeys::Gamepad_Special_Left, IE_Pressed, this, &AShooterPlayerController::InteractWithArsenal);
	InputComponent->BindKey(EKeys::R, IE_Pressed, this, &AShooterPlayerController::ReloadRidgefireWeapon);
	InputComponent->BindKey(EKeys::Gamepad_FaceButton_Top, IE_Pressed, this, &AShooterPlayerController::ToggleRidgefireSecondaryWeapon);
	// Right stick is hold-crouch/slide; reload remains R (keyboard), D-pad up (gamepad).
 InputComponent->BindKey(EKeys::Gamepad_DPad_Up, IE_Pressed, this, &AShooterPlayerController::ReloadRidgefireWeapon);
	InputComponent->BindKey(EKeys::Gamepad_LeftShoulder, IE_Pressed, this, &AShooterPlayerController::UseFieldPatch);
	InputComponent->BindKey(EKeys::Gamepad_RightShoulder, IE_Pressed, this, &AShooterPlayerController::ActivateIonSurge);
	InputComponent->BindKey(EKeys::Gamepad_LeftThumbstick, IE_Pressed, this, &AShooterPlayerController::StartSprint);
	InputComponent->BindKey(EKeys::Gamepad_LeftThumbstick, IE_Released, this, &AShooterPlayerController::StopSprint);
	FInputKeyBinding& EscapeBinding = InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AShooterPlayerController::TogglePause);
	EscapeBinding.bExecuteWhenPaused = true;
	FInputKeyBinding& PauseBinding = InputComponent->BindKey(EKeys::P, IE_Pressed, this, &AShooterPlayerController::TogglePause);
	PauseBinding.bExecuteWhenPaused = true;
	FInputKeyBinding& GamepadPauseBinding = InputComponent->BindKey(EKeys::Gamepad_Special_Right, IE_Pressed, this, &AShooterPlayerController::TogglePause);
	GamepadPauseBinding.bExecuteWhenPaused = true;

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// add the input mapping contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}

		if (ShouldUseTouchControls())
		{
			// spawn the mobile controls widget
			MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

			if (MobileControlsWidget)
			{
				// add the controls to the player screen
				MobileControlsWidget->AddToPlayerScreen(0);

			} else {

				UE_LOG(LogTP_FirstPerson, Error, TEXT("Could not spawn mobile controls widget."));

			}
		}

		if (!GetWorld()->GetAuthGameMode<ARidgefireGameMode>())
		{
			BulletCounterUI = CreateWidget<UShooterBulletCounterUI>(this, BulletCounterUIClass);

			if (BulletCounterUI)
			{
				BulletCounterUI->AddToPlayerScreen(0);
			}
			else
			{
				UE_LOG(LogTP_FirstPerson, Error, TEXT("Could not spawn bullet counter widget."));
			}
		}
	}
}

void AShooterPlayerController::ActivateIonSurge()
{
    if(bUsingGamepad && RidgefireRunState.bPersonalArsenalOpen && RidgefireRunState.bEconomyEnabled) {ShopUpgradeThrough();return;}
	if (AShooterCharacter* Gladiator = Cast<AShooterCharacter>(GetPawn()))
	{
		Gladiator->ActivateIonSurge();
	}
}

void AShooterPlayerController::UseFieldPatch()
{
    if(bUsingGamepad && RidgefireRunState.bPersonalArsenalOpen && RidgefireRunState.bEconomyEnabled) {ShopUpgradeEcho();return;}
	if (AShooterCharacter* Gladiator = Cast<AShooterCharacter>(GetPawn()))
	{
		Gladiator->UseFieldPatch();
	}
}

void AShooterPlayerController::StartSprint()
{
	if (AShooterCharacter* Gladiator = Cast<AShooterCharacter>(GetPawn()))
	{
		if (Gladiator->IsDead() || Gladiator->IsCarryingRidgefirePlayer())
		{
			return;
		}
		UCharacterMovementComponent* Movement = Gladiator->GetCharacterMovement();
		if (!Movement)
		{
			return;
		}
		if (!bIsSprinting || SprintingCharacter.Get() != Gladiator)
		{
			SprintingCharacter = Gladiator;
			SprintBaseSpeed = Movement->MaxWalkSpeed;
			bIsSprinting = true;
		}
		Gladiator->StopRidgefireCrouch();
		CastChecked<URidgefireCharacterMovement>(Movement)->RequestSprint(true);
	}
}

void AShooterPlayerController::StopSprint()
{
	if (AShooterCharacter* Gladiator = SprintingCharacter.Get())
	{
		if (UCharacterMovementComponent* Movement = Gladiator->GetCharacterMovement())
		{
			if (bIsSprinting && Gladiator->GetVelocity().SizeSquared2D() > 100.0f) Gladiator->NotifyRidgefireHandlingRecovery();
			CastChecked<URidgefireCharacterMovement>(Movement)->RequestSprint(false);
		}
	}
	SprintingCharacter.Reset();
	SprintBaseSpeed = 0.0f;
	bIsSprinting = false;
}

void AShooterPlayerController::CancelRidgefireSprint()
{
	StopSprint();
}

void AShooterPlayerController::ChooseOfferOne()
{
    if(bUsingGamepad && RidgefireRunState.bPersonalArsenalOpen && RidgefireRunState.bEconomyEnabled) {ShopConfirm();return;}
    if(!bUsingGamepad && !RidgefireRunState.bArmoryOpen) {SelectCarriedPrimarySlot(0);return;}
    SelectWeaponOffer(0);
}

void AShooterPlayerController::ChooseOfferTwo()
{
    if(bUsingGamepad && RidgefireRunState.bPersonalArsenalOpen && RidgefireRunState.bEconomyEnabled) {ShopNext();return;}
    if(bUsingGamepad && RidgefireRunState.bRegroupOpen && !RidgefireRunState.bPersonalArsenalOpen) {ReadyForEncounter();return;}
    if(!bUsingGamepad && !RidgefireRunState.bArmoryOpen) {SelectCarriedSecondary();return;}
    SelectWeaponOffer(1);
}

void AShooterPlayerController::ChooseOfferThree()
{
    if(bUsingGamepad && RidgefireRunState.bPersonalArsenalOpen && RidgefireRunState.bEconomyEnabled) {ShopPrevious();return;}
    if(!bUsingGamepad && !RidgefireRunState.bArmoryOpen) return; // Two carried weapons; 3 remains an opening offer key.
    SelectWeaponOffer(2);
}

void AShooterPlayerController::SelectCarriedSecondary()
{
    if (!HasAuthority()) { ServerEquipCarriedSecondary(); return; }
    if (AShooterCharacter* ShooterPawn = Cast<AShooterCharacter>(GetPawn()); ShooterPawn && !ShooterPawn->IsRidgefireSecondaryEquipped())
        ShooterPawn->ToggleRidgefireSecondaryWeapon();
}

void AShooterPlayerController::ServerEquipCarriedSecondary_Implementation()
{
    SelectCarriedSecondary();
}

void AShooterPlayerController::SelectCarriedPrimarySlot(int32 Slot)
{
	if (!RidgefireRunState.PrimaryWeaponSlots.IsValidIndex(Slot)) return;
	const int32 Offer = RidgefireRunState.PrimaryWeaponSlots[Slot];
	if (!RidgefireRunState.WeaponOffers.IsValidIndex(Offer)) return;
	if (!HasAuthority())
	{
		ServerSelectRidgefireOffer(Offer, INDEX_NONE);
		return;
	}
	if (ARidgefireGameMode* GameMode = GetWorld()->GetAuthGameMode<ARidgefireGameMode>())
	{
		GameMode->SelectWeaponOffer(this, Offer, INDEX_NONE);
	}
}

void AShooterPlayerController::ChoosePrimaryReplacementOne()
{
	PendingPrimaryReplacementSlot = 0;
}

void AShooterPlayerController::ChoosePrimaryReplacementTwo()
{
    if(bUsingGamepad && !RidgefireRunState.bArmoryOpen && !RidgefireRunState.bPersonalArsenalOpen) {MeleeFinisher();return;}
	// No additional primary destination. D-pad down retains its combat finisher above.
}

void AShooterPlayerController::SelectWeaponOffer(int32 Index)
{
	if (RidgefireRunState.bPersonalArsenalOpen && RidgefireRunState.bEconomyEnabled) {
        if(RidgefireRunState.WeaponOffers.IsValidIndex(Index)) {ShopCursor=Index;ShopUpgradePreview=INDEX_NONE;}
        return;
    }
	if (RidgefireRunState.bPersonalArsenalOpen)
	{
		RequestSecondaryWeaponOffer(Index);
		PendingPrimaryReplacementSlot = INDEX_NONE;
		return;
	}
	const int32 ReplacementSlot = PendingPrimaryReplacementSlot;
	if (!HasAuthority())
	{
		const bool bKnownOffer = RidgefireRunState.WeaponOffers.IsValidIndex(Index);
		const bool bStartingOffer = RidgefireRunState.bArmoryOpen && Index >= 0 && Index < 3;
		if (!RidgefireRunState.bRunOver && (bKnownOffer || bStartingOffer))
		{
			ServerSelectRidgefireOffer(Index, ReplacementSlot);
			PendingPrimaryReplacementSlot = INDEX_NONE;
		}
		return;
	}
	if (ARidgefireGameMode* GameMode = GetWorld()->GetAuthGameMode<ARidgefireGameMode>(); GameMode && !GameMode->IsRunOver())
	{
		if (GameMode->IsArmoryOpen())
		{
			PendingWeaponOfferIndex = Index;
			TrySelectPendingWeaponOffer();
		}
		else
		{
			GameMode->SelectWeaponOffer(this, Index, ReplacementSlot);
			PendingPrimaryReplacementSlot = INDEX_NONE;
		}
	}
}

void AShooterPlayerController::ServerSelectRidgefireOffer_Implementation(int32 Index, int32 ReplacementSlot)
{
	ARidgefireGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ARidgefireGameMode>() : nullptr;
	if (!GameMode || GameMode->IsRunOver() || !GameMode->GetOffer(Index) || (ReplacementSlot != INDEX_NONE && ReplacementSlot != 0))
	{
		return;
	}
	if (RidgefireRunState.bPersonalArsenalOpen && !RidgefireRunState.bEconomyEnabled)
	{
		GameMode->SelectSecondaryWeaponOfferFromArsenal(this, Index);
		return;
	}
	if (GameMode->IsArmoryOpen())
	{
		if (!GameMode->IsStartingWeaponSelectionOpen() || GameMode->GetSecondarySlotOffer(this) == Index) return;
		PendingWeaponOfferIndex = Index;
		TrySelectPendingWeaponOffer();
		return;
	}
	GameMode->SelectWeaponOffer(this, Index, ReplacementSlot);
}

void AShooterPlayerController::RequestSecondaryWeaponOffer(int32 Index)
{
	if (RidgefireRunState.bRunOver || (!RidgefireRunState.bPersonalArsenalOpen && RidgefireRunState.SetupStage != ERidgefireSetupStage::Secondary))
	{
		return;
	}
	if (!HasAuthority())
	{
		if (RidgefireRunState.WeaponOffers.IsValidIndex(Index))
		{
			ServerSelectRidgefireSecondaryOffer(Index);
		}
		return;
	}
	if (ARidgefireGameMode* GameMode = GetWorld()->GetAuthGameMode<ARidgefireGameMode>())
	{
		if (GameMode->GetSetupStage() == ERidgefireSetupStage::Secondary) GameMode->SelectSetupSecondary(this, Index);
		else GameMode->SelectSecondaryWeaponOfferFromArsenal(this, Index);
	}
}

void AShooterPlayerController::ToggleRidgefireSecondaryWeapon()
{
    if(bUsingGamepad && RidgefireRunState.bPersonalArsenalOpen && RidgefireRunState.bEconomyEnabled) {ShopSecondary();return;}
	if (AShooterCharacter* ShooterPawn = Cast<AShooterCharacter>(GetPawn()))
	{
		ShooterPawn->ToggleRidgefireSecondaryWeapon();
	}
}

void AShooterPlayerController::ServerSelectRidgefireSecondaryOffer_Implementation(int32 Index)
{
	ARidgefireGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ARidgefireGameMode>() : nullptr;
	if (!GameMode || GameMode->IsRunOver() || !GameMode->GetOffer(Index))
	{
		return;
	}
	if (GameMode->GetSetupStage() == ERidgefireSetupStage::Secondary) GameMode->SelectSetupSecondary(this, Index);
	else if (RidgefireRunState.bPersonalArsenalOpen) GameMode->SelectSecondaryWeaponOfferFromArsenal(this, Index);
}

void AShooterPlayerController::InteractWithArsenal()
{
	if (AShooterCharacter* Gladiator = Cast<AShooterCharacter>(GetPawn()); Gladiator
		&& (Gladiator->IsCarryingRidgefirePlayer() || Gladiator->FindRidgefireCarryTarget()))
	{
		InteractWithTeammate();
		return;
	}
	const bool bOpen = !RidgefireRunState.bPersonalArsenalOpen;
    if(bOpen) {ShopCursor=FMath::Max(0,RidgefireRunState.ActiveWeaponIndex);ShopUpgradePreview=INDEX_NONE;bShopSecondary=false;bShopDetails=false;}
	if (!HasAuthority())
	{
		ServerSetPersonalArsenalOpen(bOpen);
		return;
	}
	if (ARidgefireGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ARidgefireGameMode>() : nullptr)
	{
		GameMode->SetPlayerArsenalOpen(this, bOpen);
	}
}

void AShooterPlayerController::InteractWithTeammate()
{
	if (!IsPaused())
	{
		if (AShooterCharacter* Gladiator = Cast<AShooterCharacter>(GetPawn()))
		{
			Gladiator->InteractWithRidgefireTeammate();
		}
	}
}

void AShooterPlayerController::ServerSetPersonalArsenalOpen_Implementation(bool bOpen)
{
	if (ARidgefireGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ARidgefireGameMode>() : nullptr)
	{
		GameMode->SetPlayerArsenalOpen(this, bOpen);
	}
}

void AShooterPlayerController::ServerRestartRidgefireRun_Implementation()
{
	if (ARidgefireGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ARidgefireGameMode>() : nullptr)
	{
		if (GameMode->IsRunOver())
		{
			GameMode->RestartRun();
		}
#if WITH_EDITOR
		else if (FParse::Param(FCommandLine::Get(), TEXT("RidgefireRestartSmoke")))
		{
			UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP SMOKE PASS early remote restart request rejected while run remains active"));
		}
#endif
	}
}

void AShooterPlayerController::TrySelectPendingWeaponOffer()
{
	if (!HasAuthority() || PendingWeaponOfferIndex == INDEX_NONE)
	{
		return;
	}

	ARidgefireGameMode* GameMode = GetWorld()->GetAuthGameMode<ARidgefireGameMode>();
	if (!GameMode || !GameMode->IsStartingWeaponSelectionOpen() || GameMode->GetSecondarySlotOffer(this) == PendingWeaponOfferIndex)
	{
		PendingWeaponOfferIndex = INDEX_NONE;
		GetWorldTimerManager().ClearTimer(PendingWeaponOfferTimer);
		return;
	}

	if (!Cast<AShooterCharacter>(GetPawn()))
	{
		GetWorldTimerManager().SetTimer(PendingWeaponOfferTimer, this, &AShooterPlayerController::TrySelectPendingWeaponOffer, 0.1f, false);
		return;
	}

	GameMode->SelectWeaponOffer(this, PendingWeaponOfferIndex);
	if (!GameMode->IsArmoryOpen())
	{
		PendingWeaponOfferIndex = INDEX_NONE;
		GetWorldTimerManager().ClearTimer(PendingWeaponOfferTimer);
	}
	else
	{
		GetWorldTimerManager().SetTimer(PendingWeaponOfferTimer, this, &AShooterPlayerController::TrySelectPendingWeaponOffer, 0.1f, false);
	}
}

void AShooterPlayerController::ReloadRidgefireWeapon()
{
	if (!HasAuthority() && RidgefireRunState.bRunOver)
	{
		ServerRestartRidgefireRun();
		return;
	}
	if (ARidgefireGameMode* GameMode = GetWorld()->GetAuthGameMode<ARidgefireGameMode>(); GameMode && GameMode->IsRunOver())
	{
		GameMode->RestartRun();
		return;
	}
	if (AShooterCharacter* Gladiator = Cast<AShooterCharacter>(GetPawn()))
	{
		Gladiator->ReloadRidgefireWeapon();
	}
}

void AShooterPlayerController::TogglePause()
{
    if (GetNetMode()!=NM_Standalone) { bSessionMenuOpen=!bSessionMenuOpen;UpdateSessionInputMode();return; }
	const ARidgefireGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ARidgefireGameMode>() : nullptr;
	if (RidgefireRunState.bRunOver || (GameMode && GameMode->IsRunOver()))
	{
		StopSprint();
		if (AShooterCharacter* Gladiator = Cast<AShooterCharacter>(GetPawn()))
		{
			Gladiator->DoStopFiring();
		}
		if (IsPaused())
		{
			SetPause(false);
		}
		SetShowMouseCursor(true);
		return;
	}
	if (!IsPaused())
	{
		StopSprint();
		if (AShooterCharacter* Gladiator = Cast<AShooterCharacter>(GetPawn()))
		{
			Gladiator->DoStopFiring();
		}
	}
	SetPause(!IsPaused());
	SetShowMouseCursor(IsPaused());
}

#if WITH_EDITOR
void AShooterPlayerController::TickRunOverPauseSmoke()
{
	if (bRunOverPauseSmokeCompleted)
	{
		GetWorldTimerManager().ClearTimer(RunOverPauseSmokeTimer);
		return;
	}
	const ARidgefireGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ARidgefireGameMode>() : nullptr;
	if (!RidgefireRunState.bRunOver && !(GameMode && GameMode->IsRunOver()))
	{
		return;
	}

	bRunOverPauseSmokeCompleted = true;
	GetWorldTimerManager().ClearTimer(RunOverPauseSmokeTimer);
	SetPause(true);
	SetShowMouseCursor(false);
	TogglePause();
	const bool bRunOverInputLeavesSafeScreen = !IsPaused() && bShowMouseCursor && IsValid(this);
	if (bRunOverInputLeavesSafeScreen)
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: pause/back on host game-over keeps the game running, unpaused, and cursor-accessible."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: pause/back on host game-over trapped controls or left the safe screen."));
		if (IsPaused())
		{
			SetPause(false);
		}
		SetShowMouseCursor(true);
	}
}
#endif

bool AShooterPlayerController::IsUsingGamepad() const
{
	return bUsingGamepad;
}

void AShooterPlayerController::RequestWildcardReroll()
{
	if (!HasAuthority())
	{
		if (RidgefireRunState.WildcardRerollTokens > 0 && !RidgefireRunState.bWildcardClaimed)
		{
			ServerRequestWildcardReroll();
		}
		return;
	}
	if (ARidgefireGameMode* GameMode = GetWorld()->GetAuthGameMode<ARidgefireGameMode>())
	{
		GameMode->TryRerollWildcardOffer();
	}
}

void AShooterPlayerController::ServerRequestWildcardReroll_Implementation()
{
	if (ARidgefireGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ARidgefireGameMode>() : nullptr)
	{
		GameMode->TryRerollWildcardOffer();
	}
}

void AShooterPlayerController::UseFoundryFurnaceAnchor()
{
	if (!HasAuthority())
	{
		if (!RidgefireRunState.bRunOver && (RidgefireRunState.RelayCount > 0 || RidgefireRunState.ArenaName == TEXT("BRASSFALL FOUNDRY")))
		{
			ServerUseFoundryFurnaceAnchor();
		}
		return;
	}
	if (ARidgefireGameMode* GameMode = GetWorld()->GetAuthGameMode<ARidgefireGameMode>())
	{
		GameMode->InteractWithArenaObjective(this);
	}
}

void AShooterPlayerController::ServerUseFoundryFurnaceAnchor_Implementation()
{
	if (ARidgefireGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ARidgefireGameMode>() : nullptr)
	{
		GameMode->InteractWithArenaObjective(this);
	}
}

void AShooterPlayerController::RestartRun()
{
	if (!HasAuthority())
	{
		ServerRestartRidgefireRun();
		return;
	}
	if (ARidgefireGameMode* GameMode = GetWorld()->GetAuthGameMode<ARidgefireGameMode>())
	{
		GameMode->RestartRun();
	}
}

void AShooterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	LoadPersonalProgression();
	StopSprint();

	// subscribe to the pawn's OnDestroyed delegate
	InPawn->OnDestroyed.AddUniqueDynamic(this, &AShooterPlayerController::OnPawnDestroyed);

	// is this a shooter character?
	if (AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(InPawn))
	{
		// add the player tag
		ShooterCharacter->Tags.Add(PlayerPawnTag);

		// set the team
		ShooterCharacter->SetTeam(TeamByte);

		// subscribe to the pawn's delegates
		ShooterCharacter->OnBulletCountUpdated.AddUniqueDynamic(this, &AShooterPlayerController::OnBulletCountUpdated);
		ShooterCharacter->OnDamaged.AddUniqueDynamic(this, &AShooterPlayerController::OnPawnDamaged);

		// force update the life bar
		ShooterCharacter->OnDamaged.Broadcast(1.0f);
	}

	TrySelectPendingWeaponOffer();
    if(auto* Mode=GetWorld()->GetAuthGameMode<ARidgefireGameMode>();Mode && Mode->GetSetupStage()==ERidgefireSetupStage::Complete) {
        const auto& Slots=RidgefireRunState.PrimaryWeaponSlots;
        if(Slots.IsValidIndex(0) && Slots[0]==INDEX_NONE) Mode->SelectWeaponOffer(this,0);
    }
}

void AShooterPlayerController::OnPawnDestroyed(AActor* DestroyedActor)
{
	if (const ARidgefireGameMode* GameMode = GetWorld()->GetAuthGameMode<ARidgefireGameMode>(); GameMode && GameMode->IsRunOver())
	{
		return;
	}
	// reset the bullet counter HUD
	if (IsValid(BulletCounterUI))
	{
		BulletCounterUI->BP_UpdateBulletCounter(0, 0);
	}

	if (TeamByte < TeamTags.Num())
	{
		// find the player start
		TArray<AActor*> ActorList;
		UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), APlayerStart::StaticClass(), TeamTags[TeamByte], ActorList);

		if (ActorList.Num() > 0)
		{
			// select a random player start
			AActor* RandomPlayerStart = ActorList[FMath::RandRange(0, ActorList.Num() - 1)];

			// spawn a character at the player start
			const FTransform SpawnTransform = RandomPlayerStart->GetActorTransform();

			if (AShooterCharacter* RespawnedCharacter = GetWorld()->SpawnActor<AShooterCharacter>(CharacterClass, SpawnTransform))
			{
				// possess the character
				Possess(RespawnedCharacter);
			}
		}
	}
}

void AShooterPlayerController::OnBulletCountUpdated(int32 MagazineSize, int32 Bullets)
{
	// update the UI
	if (BulletCounterUI)
	{
		BulletCounterUI->BP_UpdateBulletCounter(MagazineSize, Bullets);
	}
}

void AShooterPlayerController::OnPawnDamaged(float LifePercent)
{
	if (IsValid(BulletCounterUI))
	{
		BulletCounterUI->BP_Damaged(LifePercent);
	}
}

bool AShooterPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

void AShooterPlayerController::SetTeam(uint8 Team)
{
	TeamByte = Team;

	// if we already have a pawn, set its team
	if (AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(GetPawn()))
	{
		ShooterCharacter->SetTeam(Team);
	}
}

void AShooterPlayerController::LoadPersonalProgression()
{
    if (!HasAuthority() || !GetGameInstance() || (!PlayerState && !GetLocalPlayer())) return;
    if (URidgefireProgressionStore* Store = GetGameInstance()->GetSubsystem<URidgefireProgressionStore>())
    {
        bool bNewPersistent = false;
        const FString NewIdentity = Store->GetIdentity(this, bNewPersistent);
        const bool bTemporaryIdentity = ProgressionIdentity.StartsWith(TEXT("Session:Guest_")) || ProgressionIdentity.StartsWith(TEXT("Session:Controller_"));
        const bool bNewTemporary = NewIdentity.StartsWith(TEXT("Session:Guest_")) || NewIdentity.StartsWith(TEXT("Session:Controller_"));
        if (ProgressionIdentity.IsEmpty() || (bTemporaryIdentity && !bNewTemporary && !NewIdentity.IsEmpty()))
        {
            const int32 PendingXP = bTemporaryIdentity ? PersonalProgression.XP : 0;
            ProgressionIdentity = NewIdentity;
            bProgressionPersistent = bNewPersistent;
            PersonalProgression = Store->GetProfile(ProgressionIdentity, bProgressionPersistent);
            if (PendingXP > 0)
            {
                PersonalProgression.XP = static_cast<int32>(FMath::Min<int64>(1000000000, static_cast<int64>(PersonalProgression.XP) + PendingXP));
                Store->UpdateProfile(ProgressionIdentity, bProgressionPersistent, PersonalProgression);
            }
            ForceNetUpdate();
        }
        OnRep_PersonalProgression();
    }
}

void AShooterPlayerController::OnRep_PersonalProgression()
{
    if (AShooterCharacter* ShooterPawn = Cast<AShooterCharacter>(GetPawn())) ShooterPawn->ApplyPersonalProgressionStats();
}

void AShooterPlayerController::GrantPersonalXP(int32 Amount)
{
    if (!HasAuthority() || Amount <= 0 || !GetGameInstance()) return;
    LoadPersonalProgression();
    if (ProgressionIdentity.IsEmpty()) return;
    PersonalProgression.XP = static_cast<int32>(FMath::Min<int64>(1000000000, static_cast<int64>(PersonalProgression.XP) + Amount));
    if (URidgefireProgressionStore* Store = GetGameInstance()->GetSubsystem<URidgefireProgressionStore>()) Store->UpdateProfile(ProgressionIdentity, bProgressionPersistent, PersonalProgression);
    ForceNetUpdate();
}

bool AShooterPlayerController::EquipPersonalPerk(int32 Slot, int32 Node)
{
    const ARidgefireGameMode* Mode = GetWorld() ? GetWorld()->GetAuthGameMode<ARidgefireGameMode>() : nullptr;
    if (!HasAuthority() || !Mode || !Mode->IsPersonalPerkSelectionOpen() || !GetGameInstance()) return false;
    LoadPersonalProgression();
    if (ProgressionIdentity.IsEmpty() || !PersonalProgression.Equip(Slot, Node)) return false;
    StopSprint();
    OnRep_PersonalProgression();
    if (URidgefireProgressionStore* Store = GetGameInstance()->GetSubsystem<URidgefireProgressionStore>())
    {
        Store->UpdateProfile(ProgressionIdentity, bProgressionPersistent, PersonalProgression);
        Store->Flush();
    }
    ForceNetUpdate();
    return true;
}

void AShooterPlayerController::ServerEquipPersonalPerk_Implementation(int32 Slot, int32 Node)
{
    EquipPersonalPerk(Slot, Node);
}

void AShooterPlayerController::TogglePersonalPerks()
{
    bool bCanSelect = RidgefireRunState.bRunOver || (RidgefireRunState.bArmoryOpen && RidgefireRunState.SetupStage == ERidgefireSetupStage::PersonalPerks);
    if (const auto* Mode = GetWorld()->GetAuthGameMode<ARidgefireGameMode>()) bCanSelect = Mode->IsPersonalPerkSelectionOpen();
    if (!bCanSelect) { bPersonalPerksOpen = false; return; }
    if (IsPaused()) return;
    bPersonalPerksOpen = !bPersonalPerksOpen;
    StopSprint();
    if (AShooterCharacter* ShooterPawn = Cast<AShooterCharacter>(GetPawn())) ShooterPawn->DoStopFiring();
}

void AShooterPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();
	OnRep_PersonalProgression();
}

void AShooterPlayerController::ServerCastStartingTeamPerkVote_Implementation(int32 Choice)
{
	if (ARidgefireGameMode* Mode = GetWorld()->GetAuthGameMode<ARidgefireGameMode>()) Mode->CastStartingTeamPerkVote(this, Choice);
}



void AShooterPlayerController::ServerCastRouteVote_Implementation(int32 Choice, uint32 Epoch)
{
    if (auto* Mode=GetWorld() ? GetWorld()->GetAuthGameMode<ARidgefireGameMode>() : nullptr) Mode->CastRouteVote(this,Choice,Epoch);
}

