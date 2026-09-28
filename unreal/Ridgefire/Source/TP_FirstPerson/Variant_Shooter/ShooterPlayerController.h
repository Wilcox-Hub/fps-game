// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterCharacter.h"
#include "ShooterPlayerController.generated.h"

class UInputMappingContext;
class AShooterCharacter;
class UShooterBulletCounterUI;

USTRUCT()
struct FRidgefireClientRunState
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Wave = 0;
	UPROPERTY()
	int32 EnemiesRemaining = 0;
	UPROPERTY()
	int32 Score = 0;
	UPROPERTY()
	int32 Streak = 0;
	UPROPERTY()
	FString ArenaName;
	UPROPERTY()
	int32 ArmorySecondsRemaining = 0;
	UPROPERTY()
	int32 ActiveWeaponIndex = INDEX_NONE;
	UPROPERTY()
	bool bArmoryOpen = true;
	UPROPERTY()
	bool bRunOver = false;
	UPROPERTY()
	FString ObjectiveText;
	UPROPERTY()
	TArray<FRidgefireWeaponRoll> WeaponOffers;
	UPROPERTY()
	TArray<int32> PrimaryWeaponSlots;
	UPROPERTY()
	int32 SecondaryWeaponOfferIndex = INDEX_NONE;
	UPROPERTY()
	bool bPersonalArsenalOpen = false;
	UPROPERTY()
	int32 WildcardRerollTokens = 0;
	UPROPERTY()
	int32 WildcardRerollCount = 0;
	UPROPERTY()
	bool bWildcardClaimed = false;
};

/**
 *  Simple PlayerController for a first person shooter game
 *  Manages input mappings
 *  Respawns the player pawn when it's destroyed
 */
UCLASS(abstract, config="Game")
class TP_FIRSTPERSON_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Input mapping contexts for this player */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** Character class to respawn when the possessed pawn is destroyed */
	UPROPERTY(EditAnywhere, Category="Shooter|Respawn")
	TSubclassOf<AShooterCharacter> CharacterClass;

	/** Type of bullet counter UI widget to spawn */
	UPROPERTY(EditAnywhere, Category="Shooter|UI")
	TSubclassOf<UShooterBulletCounterUI> BulletCounterUIClass;

	TWeakObjectPtr<AShooterCharacter> SprintingCharacter;
	float SprintBaseSpeed = 0.0f;
	bool bIsSprinting = false;

	/** Tag to grant the possessed pawn to flag it as the player */
	UPROPERTY(EditAnywhere, Category="Shooter|Player")
	FName PlayerPawnTag = FName("Player");

	/** Pointer to the bullet counter UI widget */
	UPROPERTY()
	TObjectPtr<UShooterBulletCounterUI> BulletCounterUI;

	/** Team ID for this player */
	uint8 TeamByte = 0;

	/** Tags that identify each team for PlayerStart selection upon respawning */
	UPROPERTY(EditAnywhere, Category="Shooter|Team")
	TArray<FName> TeamTags;

protected:

	/** Gameplay Initialization */
	virtual void BeginPlay() override;

	/** Initialize input bindings */
	virtual void SetupInputComponent() override;
	void ActivateIonSurge();
	void UseFieldPatch();
	void StartSprint();
	void StopSprint();
	void ChooseOfferOne();
	void ChooseOfferTwo();
	void ChooseOfferThree();
	void ChoosePrimaryReplacementOne();
	void ChoosePrimaryReplacementTwo();
	void SelectWeaponOffer(int32 Index);
	void TrySelectPendingWeaponOffer();
	void ReloadRidgefireWeapon();
	void RestartRun();
	void RequestWildcardReroll();
	void UseFoundryFurnaceAnchor();
	void InteractWithArsenal();
	int32 PendingWeaponOfferIndex = INDEX_NONE;
	int32 PendingPrimaryReplacementSlot = INDEX_NONE;
	FTimerHandle PendingWeaponOfferTimer;
	bool bUsingGamepad = false;
#if WITH_EDITOR
	FTimerHandle CoopCombatSmokeTimer;
	FTimerHandle RunOverPauseSmokeTimer;
	TWeakObjectPtr<AShooterNPC> CoopCombatSmokeTarget;
	FVector CoopCombatSmokeTargetLocation = FVector::ZeroVector;
	int32 CoopCombatSmokeInitialAmmo = 0;
	int32 CoopCombatSmokeInitialScore = 0;
	int32 CoopCombatSmokeShotsSent = 0;
	int32 CoopCombatSmokeDelayTicks = 0;
	float CoopCombatSmokeElapsed = 0.0f;
	bool bCoopCombatSmokeStarted = false;
	bool bCoopCombatSmokeTriggerHeld = false;
	bool bRunOverPauseSmokeCompleted = false;
	void TickCoopCombatSmoke();
	void TickRunOverPauseSmoke();
	void BeginCoopCombatSmokeShot();
#endif
	UPROPERTY(ReplicatedUsing=OnRep_RidgefireRunState)
	FRidgefireClientRunState RidgefireRunState;

	UFUNCTION()
	void OnRep_RidgefireRunState();
	UFUNCTION(Server, Reliable)
	void ServerSelectRidgefireOffer(int32 Index, int32 ReplacementSlot);
	UFUNCTION(Server, Reliable)
	void ServerSelectRidgefireSecondaryOffer(int32 Index);
	UFUNCTION(Server, Reliable)
	void ServerSetPersonalArsenalOpen(bool bOpen);
	UFUNCTION(Server, Reliable)
	void ServerRestartRidgefireRun();
	UFUNCTION(Server, Reliable)
	void ServerRequestWildcardReroll();
	UFUNCTION(Server, Reliable)
	void ServerUseFoundryFurnaceAnchor();
	UFUNCTION(Server, Reliable)
	void ServerSetCoopCombatSmokeAim(FRotator AimRotation);
	UFUNCTION(Client, Reliable)
	void ClientConfirmCoopCombatSmokeAim();

	/** Pawn initialization */
	virtual void OnPossess(APawn* InPawn) override;

	/** Called if the possessed pawn is destroyed */
	UFUNCTION()
	void OnPawnDestroyed(AActor* DestroyedActor);

	/** Called when the bullet count on the possessed pawn is updated */
	UFUNCTION()
	void OnBulletCountUpdated(int32 MagazineSize, int32 Bullets);

	/** Called when the possessed pawn is damaged */
	UFUNCTION()
	void OnPawnDamaged(float LifePercent);

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Assigns a team ID to this player */
	void SetTeam(uint8 Team);
	void SetRidgefireRunState(const FRidgefireClientRunState& NewState);
	const FRidgefireClientRunState& GetRidgefireRunState() const;
	void RequestSecondaryWeaponOffer(int32 Index);
	void ToggleRidgefireSecondaryWeapon();
	int32 GetPendingPrimaryReplacementSlot() const;
	virtual bool InputKey(const FInputKeyEventArgs& Params) override;
	void TogglePause();
	bool IsUsingGamepad() const;
};
