#pragma once

#include "CoreMinimal.h"
#include "ShooterCharacter.h"
#include "Variant_Shooter/ShooterGameMode.h"
#include "RidgefireGameMode.generated.h"

class AShooterNPC;
class AShooterCharacter;
class AShooterPlayerController;
class APlayerController;
class ARidgefirePickup;
class ARidgefireArenaDressing;

struct FRidgefirePlayerWeaponLoadout
{
	TArray<int32> PrimaryWeaponSlots;
	TArray<int32> WeaponAmmo;
	TArray<int32> WeaponReserveAmmo;
	int32 ActiveWeaponIndex = INDEX_NONE;
	int32 SecondaryWeaponOfferIndex = INDEX_NONE;
	bool bPersonalArsenalOpen = false;
};

struct FRidgefireObjectiveState
{
	FName Id;
	FString Title;
	int32 Progress = 0;
	int32 Goal = 0;
	bool bOptional = false;
	bool bCompleted = false;
	bool bFailed = false;
};

UCLASS()
class TP_FIRSTPERSON_API ARidgefireGameMode : public AShooterGameMode
{
	GENERATED_BODY()

public:
	ARidgefireGameMode();

	int32 GetWave() const;
	int32 GetEnemiesRemaining() const;
	int32 GetScore() const;
	int32 GetStreak() const;
	FString GetObjectiveDisplayText() const;
	FString GetArenaDisplayName() const;
	void RegisterOptionalObjective(FName Id, const FString& Title, int32 Goal);
	void AdvanceOptionalObjective(FName Id, int32 Amount = 1);
	void FailOptionalObjective(FName Id);
	bool IsArmoryOpen() const;
	int32 GetArmorySecondsRemaining() const;
	int32 GetOfferCount() const;
	const FRidgefireWeaponRoll* GetOffer(int32 Index) const;
	void SelectWeaponOffer(int32 Index);
	void SelectWeaponOffer(int32 Index, int32 ReplacementSlot);
	void SelectWeaponOffer(APlayerController* PlayerController, int32 Index, int32 ReplacementSlot = INDEX_NONE);
	void CycleWeaponOffer();
	void CycleWeaponOffer(APlayerController* PlayerController);
	void SelectSecondaryWeaponOffer(APlayerController* PlayerController, int32 Index);
	bool SetPlayerArsenalOpen(AShooterPlayerController* PlayerController, bool bOpen);
	bool IsPlayerNearArsenal(const AShooterPlayerController* PlayerController) const;
	bool SelectSecondaryWeaponOfferFromArsenal(AShooterPlayerController* PlayerController, int32 Index);
	int32 GetActiveWeaponIndex() const;
	int32 GetPrimarySlotOffer(int32 Slot) const;
	int32 GetSecondarySlotOffer() const;
	int32 GetSecondarySlotOffer(APlayerController* PlayerController) const;
	int32 GetWildcardRerollTokens() const;
	int32 GetWildcardRerollCount() const;
	bool IsWildcardClaimed() const;
	bool TryRerollWildcardOffer();
	bool ActivateFoundryFurnaceAnchor(AShooterPlayerController* PlayerController);
	void HandlePlayerDefeated();
	void RestartRun();
	bool IsRunOver() const;

protected:
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

private:
	UPROPERTY()
	TSubclassOf<AShooterNPC> SentinelClass;
	UPROPERTY(Transient)
	TObjectPtr<ARidgefireArenaDressing> ActiveArenaDressing;

	TArray<FTransform> SentinelEntrances;
	TSubclassOf<ARidgefirePickup> PickupClass;
	int32 CurrentWave = 0;
	int32 EnemiesRemaining = 0;
	int32 TotalScore = 0;
	int32 CurrentStreak = 0;
	int32 ConsecutiveWaveSpawnFailures = 0;
	FVector ArenaCenter = FVector::ZeroVector;
	float ArenaGroundZ = 0.0f;
	FString ArenaDisplayName = TEXT("IRON SUN ARENA");
	bool bFoundryArenaActive = false;
	bool bFoundryAnchorCleared = false;
	bool bBrassfallEntryAmmoRefillGranted = false;
	bool bArmoryOpen = true;
	bool bRunOver = false;
	bool bSmokeDropSpawned = false;
	int32 SmokeInitialReserveAmmo = 0;
	TArray<FRidgefireWeaponRoll> WeaponOffers;
	TArray<int32> WeaponAmmo;
	TArray<int32> WeaponReserveAmmo;
	TArray<int32> PrimaryWeaponSlots;
	TMap<TWeakObjectPtr<AShooterPlayerController>, FRidgefirePlayerWeaponLoadout> PlayerWeaponLoadouts;
	int32 WildcardRerollTokens = 0;
	int32 WildcardRerollCount = 0;
	double LastWildcardRerollTime = -1.0;
	int32 ActiveWeaponIndex = INDEX_NONE;
	FRidgefireObjectiveState WaveObjective;
	TArray<FRidgefireObjectiveState> OptionalObjectives;
	TSet<TWeakObjectPtr<AActor>> ProcessedSentinelDeaths;
	FTimerHandle WaveTimer;
	FTimerHandle StreakTimer;
	FTimerHandle ArmoryTimeoutTimer;
	FTimerHandle ArmoryStateTimer;
#if WITH_EDITOR
	FTimerHandle SmokeTestTimer;
	FTimerHandle SentryBehaviorSmokeTimer;
	FTimerHandle CoopCombatSmokeTimer;
	bool bRunSmokeTest = false;
	bool bRunCoopCombatSmokeTest = false;
	bool bSentryBehaviorSmokeStarted = false;
	bool bSentryBehaviorSmokeFinished = false;
	bool bSentryBehaviorSmokeTelegraphed = false;
	bool bSentryBehaviorSmokeAttackDelivered = false;
	bool bSentryBehaviorSmokeDamagedPlayer = false;
	float SentryBehaviorSmokeStartedAt = 0.0f;
	float SentryBehaviorSmokeTelegraphPlayerHealth = 0.0f;
	FVector SentryBehaviorSmokeStartLocation = FVector::ZeroVector;
	FVector SentryBehaviorSmokeTargetLocation = FVector::ZeroVector;
	FRotator SentryBehaviorSmokeOriginalControlRotation = FRotator::ZeroRotator;
	FDelegateHandle SentryBehaviorSmokeEventHandle;
	TWeakObjectPtr<AShooterNPC> SentryBehaviorSmokeSentry;
	TWeakObjectPtr<AShooterCharacter> SentryBehaviorSmokePlayer;
	bool bCoopSmokeArmoryTestComplete = false;
	bool bCoopSmokeScenarioStarted = false;
	bool bCoopSmokeTargetKilled = false;
	int32 CoopSmokeInitialScore = 0;
	int32 CoopSmokeInitialEnemies = 0;
	int32 CoopSmokeInitialAmmo = 0;
	int32 CoopSmokeInitialHostAmmo = 0;
	float CoopSmokeInitialRewardCharge = 0.0f;
	TWeakObjectPtr<AShooterNPC> CoopSmokeTarget;
	TWeakObjectPtr<AShooterCharacter> CoopSmokeHostCharacter;
	TWeakObjectPtr<AShooterCharacter> CoopSmokeRemoteCharacter;
	TWeakObjectPtr<AShooterCharacter> SmokeDefeatedGladiator;
#endif

	void BeginNextWave();
	void AutoSelectStartingWeapon();
	void StartWave();
	void AdvanceObjective(FRidgefireObjectiveState& Objective, int32 Amount);
	UFUNCTION()
	void HandleSentinelDeath(AActor* DeadSentinel);
#if WITH_EDITOR
	bool StartSentryBehaviorSmokeTest(AShooterCharacter* Player, AShooterPlayerController* Controller);
	void VerifySentryBehaviorSmokeTest();
	void RunGameplaySmokeTest();
	void VerifySecondWaveSmokeTest();
	void VerifyPlayerNoRespawnSmokeTest();
	void VerifyRestartSmokeTest();
	void RunCoopCombatSmokeTest();
	void VerifyCoopCombatSmokeTest();
#endif
	void ResetStreak();
	void RefreshRidgefireRunStates();
	void SendRidgefireRunState(AShooterPlayerController* PlayerController);
	FRidgefirePlayerWeaponLoadout& GetOrCreatePlayerWeaponLoadout(AShooterPlayerController* PlayerController);
	bool AreCurrentPlayersReadyForWave() const;
	void MirrorFirstPlayerWeaponLoadout(AShooterPlayerController* PlayerController, const FRidgefirePlayerWeaponLoadout& Loadout);
	void CreateWeaponOffers(FRandomStream* TestRandomStream = nullptr);
	void AwardWildcardRerollToken();
	void TransitionToBrassfallFoundry();
	void GrantBrassfallEntryAmmoRefill();
	AShooterCharacter* GetGladiator() const;
};
