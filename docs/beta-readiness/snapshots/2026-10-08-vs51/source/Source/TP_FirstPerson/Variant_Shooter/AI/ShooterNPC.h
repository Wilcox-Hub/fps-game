// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TP_FirstPersonCharacter.h"
#include "ShooterWeaponHolder.h"
#include "ShooterNPC.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPawnDeathDelegate, AActor*, DeadActor);

#if WITH_EDITOR
DECLARE_MULTICAST_DELEGATE_TwoParams(FSentryEditorEvent, FName, AActor*);
#endif

class AShooterWeapon;
class UStaticMesh;
class UStaticMeshComponent;
class USceneComponent;
class UMaterialInstanceDynamic;
class UPrimitiveComponent;
class USoundAttenuation;

/**
 *  A simple AI-controlled shooter game NPC
 *  Executes its behavior through a StateTree managed by its AI Controller
 *  Holds and manages a weapon
 */
UCLASS(abstract)
class TP_FIRSTPERSON_API AShooterNPC : public ATP_FirstPersonCharacter, public IShooterWeaponHolder
{
	GENERATED_BODY()

public:
	static constexpr float SentryMeleeTriggerRange = 235.0f;
	static constexpr float SentryVisualClearanceBuffer = 24.0f;
	AShooterNPC();
	static APawn* SelectRidgefireSentryTarget(const TArray<APawn*>& CandidatePawns, const FVector& SentryLocation, APawn* CurrentTarget, const AActor* PathFailedTarget, float PathFailedUntil, float CurrentTime);
	static float CalculateSentryVisualClearanceDistance(float SentryVisualExtent, float TargetCapsuleRadius, float ClearanceBuffer);
	static float GetRidgefireSentryApproachWeight(bool bBrute, bool bSprinter, bool bDodging, float Distance, float VisualClearanceDistance);
	static bool ShouldCancelSentryPathRecovery(bool bUsesRidgefireSentryMovement, bool bRecoveryActive, float Distance, float VisualClearanceDistance);
	static FVector GetSentryMovementWithClearance(const FVector& DesiredDirection, const FVector& TowardTarget, float Distance, float VisualClearanceDistance);
	static float GetSentryContactEscapeSpeed(float BaseSpeed, const FVector& TargetVelocity, const FVector& TowardTarget);
	float GetSentryVisualClearanceDistance(const AActor* Target, const FVector& TowardTarget) const;

public:

	/** Current HP for this character. It dies if it reaches zero through damage */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category="Damage")
	float CurrentHP = 100.0f;

protected:

	/** Name of the collision profile to use during ragdoll death */
	UPROPERTY(EditAnywhere, Category="Damage")
	FName RagdollCollisionProfile = FName("Ragdoll");

	/** Time to wait after death before destroying this actor */
	UPROPERTY(EditAnywhere, Category="Damage")
	float DeferredDestructionTime = 5.0f;

	/** Team byte for this character */
	UPROPERTY(EditAnywhere, Category="Team")
	uint8 TeamByte = 1;

	/** Actor tag to grant this character when it dies */
	UPROPERTY(EditAnywhere, Category="Team")
	FName DeathTag = FName("Dead");

	/** Pointer to the equipped weapon */
	TObjectPtr<AShooterWeapon> Weapon;

	/** Type of weapon to spawn for this character */
	UPROPERTY(EditAnywhere, Category="Weapon")
	TSubclassOf<AShooterWeapon> WeaponClass;

	/** Name of the first person mesh weapon socket */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="Weapons")
	FName FirstPersonWeaponSocket = FName("HandGrip_R");

	/** Name of the third person mesh weapon socket */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="Weapons")
	FName ThirdPersonWeaponSocket = FName("HandGrip_R");

	/** Max range for aiming calculations */
	UPROPERTY(EditAnywhere, Category="Aim")
	float AimRange = 10000.0f;

	/** Cone variance to apply while aiming */
	UPROPERTY(EditAnywhere, Category="Aim")
	float AimVarianceHalfAngle = 10.0f;

	/** Minimum vertical offset from the target center to apply when aiming */
	UPROPERTY(EditAnywhere, Category="Aim")
	float MinAimOffsetZ = -35.0f;

	/** Maximum vertical offset from the target center to apply when aiming */
	UPROPERTY(EditAnywhere, Category="Aim")
	float MaxAimOffsetZ = -60.0f;

	/** Actor currently being targeted */
	TObjectPtr<AActor> CurrentAimTarget;

	/** If true, this character is currently shooting its weapon */
	bool bIsShooting = false;

	/** If true, this character has already died */
	UPROPERTY(ReplicatedUsing=OnRep_Dead)
	bool bIsDead = false;

	/** Deferred destruction on death timer */
	FTimerHandle DeathTimer;
	FTimerHandle RidgefireSlowTimer;
	float RidgefireOriginalSpeed = 0.0f;
	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> SentinelVisualRoot;
	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> SentinelAuthoredModel;
	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> SentinelVisualParts;
	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> SentinelHumanoidParts;
	bool bHumanoidVisualsCreated = false;
	bool bHumanoidVisualsHidden = false;
	bool bFirstArenaSentinelVisualSelectionResolved = false;
	float SentinelHoverPhase = 0.0f;
	float SentinelStrafeDirection = 1.0f;
	float SentinelNextTurnTime = 0.0f;
	float SentryStuckDuration = 0.0f;
	float SentryMovementSampleTime = 0.0f;
	float SentryPathRecoveryRetryTime = 0.0f;
	float SentryPathRecoveryDeadline = 0.0f;
	FVector SentryMovementSampleLocation = FVector::ZeroVector;
	bool bSentryPathRecoveryActive = false;
	TWeakObjectPtr<AActor> SentryPathFailedTarget;
	float SentryPathFailedTargetUntil = 0.0f;
	FVector SentinelSpawnLocation = FVector::ZeroVector;
	UPROPERTY(ReplicatedUsing=OnRep_RidgefireSentryEnabled)
	bool bRidgefireSentry = false;
	UPROPERTY(ReplicatedUsing=OnRep_RidgefireSprinterEnabled)
	bool bRidgefireSprinter = false;
	UPROPERTY(ReplicatedUsing=OnRep_RidgefireBruteEnabled)
	bool bRidgefireBrute = false;
    UPROPERTY(Replicated) bool bRidgefireChampion = false;
    UPROPERTY(Replicated) bool bChampionGuarded = false;
    bool bChampionSecondPhase = false;
    float ChampionMaxHP = 0;
	bool bRidgefireSprinterVisualsCreated = false;
	bool bRidgefireBruteVisualsCreated = false;
	float SprinterNextDodgeTime = 0.0f;
	float SprinterDodgeEndsAt = 0.0f;
	float SprinterDodgeDirection = 1.0f;
	UPROPERTY(ReplicatedUsing=OnRep_SentryAttackState)
	bool bBruteRangedCharging = false;
	UPROPERTY(ReplicatedUsing=OnRep_SentryAttackState)
	bool bBruteBurstActive = false;
	int32 BruteBurstShotsRemaining = 0;
	float BruteRangedNextAttackTime = 0.0f;
	float BruteChargeEndsAt = 0.0f;
	float BruteNextBurstShotAt = 0.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, ReplicatedUsing=OnRep_FoundryTurretEnabled, Category="Ridgefire|Sentry")
	bool bFoundryTurret = false;
	UPROPERTY(ReplicatedUsing=OnRep_SentryAttackState)
	bool bSentryTelegraphActive = false;
	UPROPERTY(ReplicatedUsing=OnRep_SentryAttackState)
	bool bSentryMeleeTelegraphActive = false;
	UPROPERTY(ReplicatedUsing=OnRep_SentryAttackState)
	bool bFoundryTurretCharging = false;
	UPROPERTY(ReplicatedUsing=OnRep_SentryAttackState)
	bool bFoundryTurretBurstActive = false;
	UPROPERTY(Replicated)
	TObjectPtr<AActor> CurrentSentryTarget;
	float NextSentryShotTime = 0.0f;
	float NextSentryMeleeTime = 0.0f;
	float SentryFirstLineOfSightAt = -1.0f;
	bool bSentryOpeningAttackStarted = false;
	int32 FoundryTurretShotsRemaining = 0;
	float FoundryTurretChargeStartedAt = 0.0f;
	float FoundryTurretChargeEndsAt = 0.0f;
	float FoundryTurretClientChargeStartedAt = 0.0f;
	bool bFoundryTurretClientWasCharging = false;
	float FoundryTurretNextBurstShotAt = 0.0f;
	float FoundryTurretSpinAngle = 0.0f;
	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> FoundryTurretRotor;
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> FoundryTurretMaterials;
	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> FoundryTurretWeakPoint;
	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> SentryShotBeam;
	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> SentryTelegraphBeam;
	UPROPERTY(Transient)
	TObjectPtr<USoundAttenuation> SentryWarningAttenuation;
	#if WITH_EDITOR
	bool bForceSentryTelegraphFallbackForEditorSmokeTest = false;
	#endif
	FTimerHandle SentryBeamTimer;
	bool ShowSentryTelegraph(AActor* Target, bool bHeavyWarning = false);
	void DispatchSentryTelegraphCue(AActor* Target);
	void PlaySentryTelegraphCue(const FVector& Location);

#if WITH_EDITOR
	FSentryEditorEvent OnSentryEditorEvent;
	TArray<TWeakObjectPtr<UStaticMeshComponent>> SentryHitSmokeArmorParts;
	bool bSentryContactSmokeActor = false;
	bool bSentryContactSmokeStarted = false;
	bool bSentryContactSmokeFinished = false;
	uint8 SentryContactSmokeRole = 0;
	float SentryContactSmokeElapsed = 0.0f;
	float SentryContactSmokeInitialDistance = 0.0f;
	float SentryContactSmokeMinimumGap = 0.0f;
	FVector SentryContactSmokePlayerStart = FVector::ZeroVector;
	FVector SentryContactSmokePlayerWalkStart = FVector::ZeroVector;
	TWeakObjectPtr<ACharacter> SentryContactSmokePlayer;
	void TickSentryContactEditorSmokeTest(float DeltaTime);
#endif

public:

	/** Delegate called when this NPC dies */
	FPawnDeathDelegate OnPawnDeath;

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Gameplay cleanup */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void CreateSentinelVisuals();
	void CreateHumanoidSentinelVisuals();
	void CreateFirstArenaArmoredSentinelVisuals();
	void HideHumanoidSentinelVisuals();
	void UpdateHumanoidSentinelCollision();
	void CreateSprinterVisuals();
	void CreateBruteVisuals();
	void CreateFoundryTurretCannons();
	void TickFoundryTurret(AActor* Target, bool bCanShoot, float CurrentTime);
	void TickRidgefireBrute(AActor* Target, bool bHasLineOfSight, float Distance, float VerticalDistance, float CurrentTime);
	void UpdateFoundryTurretVisuals(float DeltaTime);
	void UpdateSentryTelegraphVisual();
	void ApplyDeathPresentation();
	UFUNCTION()
	void OnRep_Dead();
	UFUNCTION()
	void OnRep_RidgefireSentryEnabled();
	UFUNCTION()
	void OnRep_FoundryTurretEnabled();
	UFUNCTION()
	void OnRep_RidgefireSprinterEnabled();
	UFUNCTION()
	void OnRep_RidgefireBruteEnabled();
	UFUNCTION()
	void OnRep_SentryAttackState();
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_ShowSentryShotBeam(FVector Start, FVector ImpactPoint);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySentryTelegraphCue(FVector Location);
	void ShowSentryShotBeam(const FVector& Start, const FVector& ImpactPoint);
	void AddSentinelPart(UStaticMesh* StaticMesh, const FVector& Location, const FVector& Scale, const FRotator& Rotation, const FLinearColor& Color);

public:

	/** Handle incoming damage */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	void ApplyRidgefireSlow(float Duration, float SpeedMultiplier);
	bool CanRidgefireFinish() const;
	UPROPERTY(Replicated) float FinisherUntil=-1;
	float FinisherMaxHP=0;
	float GetTravelDistance() const;
	void FireSentryShot(AActor* Target, float Damage = 5.0f);
public:
 void ConfigurePressureTactics(int32 Seed,int32 Arena,int32 Encounter,int32 Ordinal,int32 Party);
 bool IsStrategicSentry() const {return bPressureStrategic;}
 bool IsPressureFlanker() const {return bPressureFlanker;}
 void LockPressureAim(AActor* Target);
 UFUNCTION(NetMulticast,Unreliable) void MulticastPressureHit(float Strength);
private:
 friend class FPressurePassChecks;
 bool bPressureStrategic=false,bPressureFlanker=false;
 int32 PressurePattern=0,PressureShotsRemaining=0;
 float PressureNextShot=0,PressureAimAt=0,PressureInterruptUntil=0,PressureInterruptReadyAt=0,PressureHitAt=-100;
 float PressureHitStrength=0;
 float PressureStepAt=-100;
 FVector PressureStepLocation=FVector::ZeroVector;
 void PlayPressureFootstep();
 float PressureDeathAt=-100;
 FRotator PressureDeathStart=FRotator::ZeroRotator;
 FVector PressureLockedAim=FVector::ZeroVector;
 TWeakObjectPtr<AActor> PressureAimTarget;
public:
	void EnableFoundryTurret();
	bool IsFoundryTurretWeakPoint(const UPrimitiveComponent* HitComponent) const;
	bool IsFoundryTurretRole() const { return bFoundryTurret; }
	bool IsRidgefireSprinterRole() const { return bRidgefireSprinter; }
	bool IsRidgefireBruteRole() const { return bRidgefireBrute; }
#if WITH_EDITOR
	bool RunSentryTelegraphCueEditorSmokeTest(AActor* Target);
	void RunSentryHitTraceEditorSmokeTest();
	bool bSentryHitSmokeActor = false;
	FSentryEditorEvent& GetSentryEditorEventForSmokeTest() { return OnSentryEditorEvent; }
#endif
	UStaticMeshComponent* GetFoundryTurretWeakPoint() const { return FoundryTurretWeakPoint.Get(); }
	void EnableRidgefireSprinter();
	void EnableRidgefireBrute();
    void EnableRidgefireChampion(bool bGuarded, bool bFoundryRoute);
    void SetRidgefireChampionGuard(bool bGuarded);
    bool IsRidgefireChampionGuarded() const { return bChampionGuarded; }
    bool IsRidgefireChampion() const { return bRidgefireChampion; }
    void SetRidgefireDamageScale(float Scale);
    float GetRidgefireDamageScale() const { return RidgefireDamageScale; }
private:
    float RidgefireDamageScale=1.0f;

public:

	//~Begin IShooterWeaponHolder interface

	/** Attaches a weapon's meshes to the owner */
	virtual void AttachWeaponMeshes(AShooterWeapon* Weapon) override;

	/** Plays the firing montage for the weapon */
	virtual void PlayFiringMontage(UAnimMontage* Montage) override;

	/** Applies weapon recoil to the owner */
	virtual void AddWeaponRecoil(float Recoil) override;

	/** Updates the weapon's HUD with the current ammo count */
	virtual void UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize) override;

	/** Calculates and returns the aim location for the weapon */
	virtual FVector GetWeaponTargetLocation() override;

	/** Gives a weapon of this class to the owner */
	virtual void AddWeaponClass(const TSubclassOf<AShooterWeapon>& WeaponClass) override;

	/** Activates the passed weapon */
	virtual void OnWeaponActivated(AShooterWeapon* Weapon) override;

	/** Deactivates the passed weapon */
	virtual void OnWeaponDeactivated(AShooterWeapon* Weapon) override;

	/** Notifies the owner that the weapon cooldown has expired and it's ready to shoot again */
	virtual void OnSemiWeaponRefire() override;

	//~End IShooterWeaponHolder interface

protected:

	/** Called when HP is depleted and the character should die */
	void Die();

	/** Called after death to destroy the actor */
	void DeferredDestruction();
	void ClearRidgefireSlow();
	void HideSentryBeam();
	void HideSentryTelegraph();
	void PerformSentryMeleeAttack(AActor* Target, float Damage = 18.0f);

public:

	/** Signals this character to start shooting at the passed actor */
	void StartShooting(AActor* ActorToShoot);

	/** Signals this character to stop shooting */
	void StopShooting();
};
