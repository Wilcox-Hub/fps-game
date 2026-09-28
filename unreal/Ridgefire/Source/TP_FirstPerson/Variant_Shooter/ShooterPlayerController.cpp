// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/ShooterPlayerController.h"
#include "RidgefireGameMode.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
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

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();
#if WITH_EDITOR
	if (GetNetMode() == NM_Client && IsLocalController() && FParse::Param(FCommandLine::Get(), TEXT("RidgefireCoopCombatSmoke")))
	{
		GetWorldTimerManager().SetTimer(CoopCombatSmokeTimer, this, &AShooterPlayerController::TickCoopCombatSmoke, 0.1f, true);
	}
	if (HasAuthority() && IsLocalController() && FParse::Param(FCommandLine::Get(), TEXT("RidgefireSmokeTest")))
	{
		GetWorldTimerManager().SetTimer(RunOverPauseSmokeTimer, this, &AShooterPlayerController::TickRunOverPauseSmoke, 0.1f, true);
	}
#endif
}

#if WITH_EDITOR
void AShooterPlayerController::TickCoopCombatSmoke()
{
	CoopCombatSmokeElapsed += 0.1f;
	AShooterCharacter* ShooterPawn = Cast<AShooterCharacter>(GetPawn());
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
	if (CoopCombatSmokeShotsSent >= 3 && ShooterPawn && CurrentAmmo < CoopCombatSmokeInitialAmmo && TargetHealth <= 0.0f
		&& RidgefireRunState.Score == CoopCombatSmokeInitialScore + 110)
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP CLIENT SMOKE PASS replicated ammo=%d target_hp=%.0f score=%d after 3 fire inputs"),
			CurrentAmmo, TargetHealth, RidgefireRunState.Score);
		GetWorldTimerManager().ClearTimer(CoopCombatSmokeTimer);
		return;
	}
	if (CoopCombatSmokeElapsed >= 45.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE COOP CLIENT SMOKE FAIL replicated checks incomplete shots=%d ammo=%d/%d target_hp=%.0f score=%d/%d"),
			CoopCombatSmokeShotsSent, CurrentAmmo, CoopCombatSmokeInitialAmmo, TargetHealth,
			RidgefireRunState.Score, CoopCombatSmokeInitialScore + 110);
		GetWorldTimerManager().ClearTimer(CoopCombatSmokeTimer);
	}
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

void AShooterPlayerController::ClientConfirmCoopCombatSmokeAim_Implementation()
{
#if WITH_EDITOR
	if (FParse::Param(FCommandLine::Get(), TEXT("RidgefireCoopCombatSmoke")) && bCoopCombatSmokeStarted && !bCoopCombatSmokeTriggerHeld)
	{
		BeginCoopCombatSmokeShot();
	}
#endif
}

void AShooterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShooterPlayerController, RidgefireRunState);
}

void AShooterPlayerController::SetRidgefireRunState(const FRidgefireClientRunState& NewState)
{
	if (HasAuthority())
	{
		RidgefireRunState = NewState;
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
}

bool AShooterPlayerController::InputKey(const FInputKeyEventArgs& Params)
{
	if (Params.Event != IE_Released && (Params.Event != IE_Axis || FMath::Abs(Params.AmountDepressed) > 0.2f || Params.AmountDepressed2D.SizeSquared() > 0.04f))
	{
		bUsingGamepad = Params.IsGamepad();
	}
	return Super::InputKey(Params);
}

void AShooterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindKey(EKeys::E, IE_Pressed, this, &AShooterPlayerController::ActivateIonSurge);
	InputComponent->BindKey(EKeys::F, IE_Pressed, this, &AShooterPlayerController::UseFieldPatch);
	InputComponent->BindKey(EKeys::LeftShift, IE_Pressed, this, &AShooterPlayerController::StartSprint);
	InputComponent->BindKey(EKeys::LeftShift, IE_Released, this, &AShooterPlayerController::StopSprint);
	InputComponent->BindKey(EKeys::One, IE_Pressed, this, &AShooterPlayerController::ChooseOfferOne);
	InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &AShooterPlayerController::ChooseOfferTwo);
	InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &AShooterPlayerController::ChooseOfferThree);
	InputComponent->BindKey(EKeys::Four, IE_Pressed, this, &AShooterPlayerController::ChoosePrimaryReplacementOne);
	InputComponent->BindKey(EKeys::Five, IE_Pressed, this, &AShooterPlayerController::ChoosePrimaryReplacementTwo);
	InputComponent->BindKey(EKeys::G, IE_Pressed, this, &AShooterPlayerController::RequestWildcardReroll);
	InputComponent->BindKey(EKeys::H, IE_Pressed, this, &AShooterPlayerController::UseFoundryFurnaceAnchor);
	InputComponent->BindKey(EKeys::B, IE_Pressed, this, &AShooterPlayerController::ToggleRidgefireSecondaryWeapon);
	InputComponent->BindKey(EKeys::T, IE_Pressed, this, &AShooterPlayerController::InteractWithArsenal);
	InputComponent->BindKey(EKeys::Gamepad_DPad_Up, IE_Pressed, this, &AShooterPlayerController::ChoosePrimaryReplacementOne);
	InputComponent->BindKey(EKeys::Gamepad_DPad_Down, IE_Pressed, this, &AShooterPlayerController::ChoosePrimaryReplacementTwo);
	InputComponent->BindKey(EKeys::Gamepad_DPad_Right, IE_Pressed, this, &AShooterPlayerController::RequestWildcardReroll);
	InputComponent->BindKey(EKeys::Gamepad_DPad_Left, IE_Pressed, this, &AShooterPlayerController::UseFoundryFurnaceAnchor);
	InputComponent->BindKey(EKeys::Gamepad_FaceButton_Bottom, IE_Pressed, this, &AShooterPlayerController::ChooseOfferOne);
	InputComponent->BindKey(EKeys::Gamepad_FaceButton_Right, IE_Pressed, this, &AShooterPlayerController::ChooseOfferTwo);
	InputComponent->BindKey(EKeys::Gamepad_FaceButton_Left, IE_Pressed, this, &AShooterPlayerController::ChooseOfferThree);
	InputComponent->BindKey(EKeys::Gamepad_Special_Left, IE_Pressed, this, &AShooterPlayerController::InteractWithArsenal);
	InputComponent->BindKey(EKeys::R, IE_Pressed, this, &AShooterPlayerController::ReloadRidgefireWeapon);
	InputComponent->BindKey(EKeys::Gamepad_FaceButton_Top, IE_Pressed, this, &AShooterPlayerController::ToggleRidgefireSecondaryWeapon);
	InputComponent->BindKey(EKeys::Gamepad_RightThumbstick, IE_Pressed, this, &AShooterPlayerController::ReloadRidgefireWeapon);
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
	if (AShooterCharacter* Gladiator = Cast<AShooterCharacter>(GetPawn()))
	{
		Gladiator->ActivateIonSurge();
	}
}

void AShooterPlayerController::UseFieldPatch()
{
	if (AShooterCharacter* Gladiator = Cast<AShooterCharacter>(GetPawn()))
	{
		Gladiator->UseFieldPatch();
	}
}

void AShooterPlayerController::StartSprint()
{
	if (AShooterCharacter* Gladiator = Cast<AShooterCharacter>(GetPawn()))
	{
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
		Movement->MaxWalkSpeed = SprintBaseSpeed * 1.5f;
	}
}

void AShooterPlayerController::StopSprint()
{
	if (AShooterCharacter* Gladiator = SprintingCharacter.Get())
	{
		if (UCharacterMovementComponent* Movement = Gladiator->GetCharacterMovement())
		{
			Movement->MaxWalkSpeed = SprintBaseSpeed;
		}
	}
	SprintingCharacter.Reset();
	SprintBaseSpeed = 0.0f;
	bIsSprinting = false;
}

void AShooterPlayerController::ChooseOfferOne()
{
	SelectWeaponOffer(0);
}

void AShooterPlayerController::ChooseOfferTwo()
{
	SelectWeaponOffer(1);
}

void AShooterPlayerController::ChooseOfferThree()
{
	SelectWeaponOffer(2);
}

void AShooterPlayerController::ChoosePrimaryReplacementOne()
{
	PendingPrimaryReplacementSlot = 0;
}

void AShooterPlayerController::ChoosePrimaryReplacementTwo()
{
	PendingPrimaryReplacementSlot = 1;
}

void AShooterPlayerController::SelectWeaponOffer(int32 Index)
{
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
	if (!GameMode || GameMode->IsRunOver() || !GameMode->GetOffer(Index) || (ReplacementSlot != INDEX_NONE && ReplacementSlot != 0 && ReplacementSlot != 1))
	{
		return;
	}
	if (RidgefireRunState.bPersonalArsenalOpen)
	{
		GameMode->SelectSecondaryWeaponOfferFromArsenal(this, Index);
		return;
	}
	if (GameMode->IsArmoryOpen())
	{
		PendingWeaponOfferIndex = Index;
		TrySelectPendingWeaponOffer();
		return;
	}
	GameMode->SelectWeaponOffer(this, Index, ReplacementSlot);
}

void AShooterPlayerController::RequestSecondaryWeaponOffer(int32 Index)
{
	if (RidgefireRunState.bRunOver || !RidgefireRunState.bPersonalArsenalOpen)
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
		GameMode->SelectSecondaryWeaponOfferFromArsenal(this, Index);
	}
}

void AShooterPlayerController::ToggleRidgefireSecondaryWeapon()
{
	if (AShooterCharacter* ShooterPawn = Cast<AShooterCharacter>(GetPawn()))
	{
		ShooterPawn->ToggleRidgefireSecondaryWeapon();
	}
}

void AShooterPlayerController::ServerSelectRidgefireSecondaryOffer_Implementation(int32 Index)
{
	ARidgefireGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ARidgefireGameMode>() : nullptr;
	if (!GameMode || GameMode->IsRunOver() || !RidgefireRunState.bPersonalArsenalOpen || !GameMode->GetOffer(Index))
	{
		return;
	}
	GameMode->SelectSecondaryWeaponOfferFromArsenal(this, Index);
}

void AShooterPlayerController::InteractWithArsenal()
{
	const bool bOpen = !RidgefireRunState.bPersonalArsenalOpen;
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

void AShooterPlayerController::ServerSetPersonalArsenalOpen_Implementation(bool bOpen)
{
	if (ARidgefireGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ARidgefireGameMode>() : nullptr)
	{
		GameMode->SetPlayerArsenalOpen(this, bOpen);
	}
}

void AShooterPlayerController::ServerRestartRidgefireRun_Implementation()
{
	if (ARidgefireGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ARidgefireGameMode>() : nullptr; GameMode && GameMode->IsRunOver())
	{
		GameMode->RestartRun();
	}
}

void AShooterPlayerController::TrySelectPendingWeaponOffer()
{
	if (!HasAuthority() || PendingWeaponOfferIndex == INDEX_NONE)
	{
		return;
	}

	ARidgefireGameMode* GameMode = GetWorld()->GetAuthGameMode<ARidgefireGameMode>();
	if (!GameMode || !GameMode->IsArmoryOpen())
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
		if (RidgefireRunState.ArenaName == TEXT("BRASSFALL FOUNDRY") && RidgefireRunState.ObjectiveText.Contains(TEXT("OPTIONAL: STABILIZE THE FURNACE ANCHOR")))
		{
			ServerUseFoundryFurnaceAnchor();
		}
		return;
	}
	if (ARidgefireGameMode* GameMode = GetWorld()->GetAuthGameMode<ARidgefireGameMode>())
	{
		GameMode->ActivateFoundryFurnaceAnchor(this);
	}
}

void AShooterPlayerController::ServerUseFoundryFurnaceAnchor_Implementation()
{
	if (ARidgefireGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ARidgefireGameMode>() : nullptr)
	{
		GameMode->ActivateFoundryFurnaceAnchor(this);
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
	StopSprint();

	// subscribe to the pawn's OnDestroyed delegate
	InPawn->OnDestroyed.AddDynamic(this, &AShooterPlayerController::OnPawnDestroyed);

	// is this a shooter character?
	if (AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(InPawn))
	{
		// add the player tag
		ShooterCharacter->Tags.Add(PlayerPawnTag);

		// set the team
		ShooterCharacter->SetTeam(TeamByte);

		// subscribe to the pawn's delegates
		ShooterCharacter->OnBulletCountUpdated.AddDynamic(this, &AShooterPlayerController::OnBulletCountUpdated);
		ShooterCharacter->OnDamaged.AddDynamic(this, &AShooterPlayerController::OnPawnDamaged);

		// force update the life bar
		ShooterCharacter->OnDamaged.Broadcast(1.0f);
	}

	TrySelectPendingWeaponOffer();
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
