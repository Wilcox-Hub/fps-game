// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TP_FirstPersonCharacter.h"
#include "ShooterWeaponHolder.h"
#include "RidgefireProgression.h"
#include "ShooterCharacter.generated.h"

class AShooterWeapon;
class AShooterNPC;
class ARidgefireHealingTube;
class UInputAction;
class UInputComponent;
class UPawnNoiseEmitterComponent;
class USoundAttenuation;
class UStaticMeshComponent;

namespace RidgefirePiercingTrace
{
	TArray<FHitResult> Trace(UWorld* World, const FVector& Start, const FVector& End, const FCollisionQueryParams& QueryParams);
}

UENUM()
enum class ERidgefireFireMode : uint8
{
	Auto,
	Scatter,
	Pierce,
	Arc,
	Burst,
	Mortar,
	Echo,
	Roulette,
	Phasma
};

UENUM()
enum class ERidgefirePayload : uint8
{
	Plain,
	Burn,
	Frost,
	Siphon,
	Salvage,
	Volatile,
	Ricochet,
	Split,
	Glass,
	Leaden
};

UENUM()
enum class ERidgefireCondition : uint8
{
	Steady,
	Close,
	Longshot,
	Sprint,
	LowHealth,
	FullHealth,
	LastShot,
	Streak,
	AfterReload,
	Desperate
};

UENUM()
enum class ERidgefireUpgradeBehavior : uint8 { None, EchoPulse, ThroughShot };

USTRUCT()
struct FRidgefireWeaponRoll
{
	GENERATED_BODY()

	UPROPERTY()
	FString Name;
	UPROPERTY()
	FString Description;
	UPROPERTY()
	FString ModeName;
	UPROPERTY()
	FString PayloadName;
	UPROPERTY()
	FString ConditionName;
	UPROPERTY()
	ERidgefireFireMode Mode = ERidgefireFireMode::Auto;
	UPROPERTY()
	ERidgefirePayload Payload = ERidgefirePayload::Plain;
	UPROPERTY()
	ERidgefireCondition Condition = ERidgefireCondition::Steady;
	UPROPERTY()
	float DamageScale = 1.0f;
	UPROPERTY()
	float RefireDelay = 0.16f;
	UPROPERTY()
	int32 MagazineSize = 24;
	UPROPERTY()
	int32 ReserveAmmo = 144;
	UPROPERTY()
	int32 Pellets = 1;
	UPROPERTY()
	bool bWildcard = false;
	UPROPERTY() int32 UpgradeLevel = 0;
	UPROPERTY() ERidgefireUpgradeBehavior UpgradeBehavior = ERidgefireUpgradeBehavior::None;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBulletCountUpdatedDelegate, int32, MagazineSize, int32, Bullets);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDamagedDelegate, float, LifePercent);

/**
 *  A player controllable first person shooter character
 *  Manages a weapon inventory through the IShooterWeaponHolder interface
 *  Manages health and death
 */
UCLASS(abstract)
class TP_FIRSTPERSON_API AShooterCharacter : public ATP_FirstPersonCharacter, public IShooterWeaponHolder
{
	GENERATED_BODY()
	
	/** AI Noise emitter component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UPawnNoiseEmitterComponent* PawnNoiseEmitter;

protected:

	/** Fire weapon input action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* FireAction;

	/** Switch weapon input action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* SwitchWeaponAction;

	/** Name of the first person mesh weapon socket */
	UPROPERTY(EditAnywhere, Category ="Weapons")
	FName FirstPersonWeaponSocket = FName("HandGrip_R");

	/** Name of the third person mesh weapon socket */
	UPROPERTY(EditAnywhere, Category ="Weapons")
	FName ThirdPersonWeaponSocket = FName("HandGrip_R");

	/** Max distance to use for aim traces */
	UPROPERTY(EditAnywhere, Category ="Aim", meta = (ClampMin = 0, ClampMax = 100000, Units = "cm"))
	float MaxAimDistance = 10000.0f;

	/** Max HP this character can have */
	UPROPERTY(EditAnywhere, Replicated, Category="Health")
	float MaxHP = 500.0f;
	float PersonalBaseMaxHP = 0.0f;
	float PersonalBaseJumpSpeed = 0.0f;

	/** Current HP remaining to this character */
	UPROPERTY(ReplicatedUsing=OnRep_CurrentHP)
	float CurrentHP = 0.0f;
	UPROPERTY(Replicated)
	float IonCharge = 0.0f;
	UPROPERTY(Replicated)
	int32 FieldPatches = 2;
	FTimerHandle RidgefireFireTimer;
	FTimerHandle RidgefireReloadTimer;
	float RidgefireNextFireTime = 0.0f;
	float RidgefireLastShotTime = -1.0f;
	float RidgefireLastHitTime = -1.0f;
	float RidgefireLastKillTime = -1.0f;
	float RidgefireLastDamageTime = -1.0f;
	float RidgefireLastDamageYaw = 0.0f;
	UPROPERTY(ReplicatedUsing=OnRep_RidgefireWeapon)
	FRidgefireWeaponRoll RidgefireWeapon;
	TArray<TObjectPtr<class UStaticMeshComponent>> RidgefireWeaponParts;
	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> RidgefireWeaponPartPool;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_RidgefireAppearance, Category="Iron Sun|Appearance")
	uint8 RidgefireAppearance = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_RidgefireAppearance, Category="Iron Sun|Appearance")
	bool bRidgefireDefenderHelmet = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Iron Sun|Appearance")
	bool bEnablePlayerModelPrototype = false;
	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> RidgefireAppearanceParts;
	UPROPERTY(Transient)
	TObjectPtr<class UStaticMeshComponent> RidgefireShotBeam;
	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> RidgefireScatterBeams;
	UPROPERTY(Transient)
	TObjectPtr<USoundAttenuation> PhasmaShotAttenuation;
    UPROPERTY(Transient) TArray<TObjectPtr<UStaticMeshComponent>> RidgefireBlastParts;
    FVector RidgefireBlastOrigin=FVector::ZeroVector,RidgefireBlastNormal=FVector::UpVector;
    float RidgefireBlastRadius=0,RidgefireBlastStarted=0;
    FTimerHandle RidgefireBlastTimer;
    int32 RidgefireBlastHits=0,RidgefireBlastKills=0;
    float RidgefireBlastFeedbackUntil=0;
	FVector RidgefireMuzzleOffset = FVector(97.0f, 33.0f, -28.0f);
	FTimerHandle RidgefireBeamTimer;
	UPROPERTY(ReplicatedUsing=OnRep_RidgefireAmmo)
	int32 RidgefireAmmo = 0;
	UPROPERTY(ReplicatedUsing=OnRep_RidgefireAmmo)
	int32 RidgefireReserveAmmo = 0;
	UPROPERTY(ReplicatedUsing=OnRep_RidgefireWeapon)
	bool bRidgefireWeaponActive = false;
	FRidgefireWeaponRoll RidgefireStoredPrimaryWeapon;
	int32 RidgefireStoredPrimaryAmmo = 0;
	int32 RidgefireStoredPrimaryReserveAmmo = 0;
	int32 RidgefireSecondaryOfferIndex = INDEX_NONE;
	int32 RidgefireSecondaryAmmo = 0;
	int32 RidgefireSecondaryReserveAmmo = 0;
	UPROPERTY(Replicated)
	bool bRidgefireSecondaryEquipped = false;
	bool bRidgefireTriggerHeld = false;
	UPROPERTY(Replicated)
	bool bRidgefireReloading = false;
	bool bBonusAfterReload = false;
	UPROPERTY(ReplicatedUsing=OnRep_RidgefireRescueState)
	bool bRidgefireDowned = false;
	UPROPERTY(ReplicatedUsing=OnRep_RidgefireRescueState)
	TObjectPtr<AShooterCharacter> RidgefireCarrier;
	UPROPERTY(ReplicatedUsing=OnRep_RidgefireRescueState)
	TObjectPtr<AShooterCharacter> RidgefireCarriedPlayer;
	UPROPERTY(ReplicatedUsing=OnRep_RidgefireRescueState)
	TObjectPtr<ARidgefireHealingTube> RidgefireHealingTube;
	FTransform RidgefireStandingMeshTransform;
	ECollisionEnabled::Type RidgefireStandingCollision = ECollisionEnabled::QueryAndPhysics;
	bool bRidgefireRescuePresentationApplied = false;
	UPROPERTY(EditDefaultsOnly, Category="Rescue", meta=(ClampMin="0.0"))
	float RidgefireTubeFootClearance = 6.0f;
	UPROPERTY(Replicated)
	float RidgefireCarrySpeed = 390.0f;

	/** Team ID for this character*/
	UPROPERTY(EditAnywhere, Replicated, Category="Team")
	uint8 TeamByte = 0;

	/** Actor tag to grant this character when it dies */
	UPROPERTY(EditAnywhere, Category="Team")
	FName DeathTag = FName("Dead");

	/** Tag to pass to weapons and projectiles to identify their AI perception noise as player-generated */
	UPROPERTY(EditAnywhere, Category="Tags")
	FName PlayerTag = FName("Player");

	/** List of weapons picked up by the character */
	TArray<AShooterWeapon*> OwnedWeapons;

	/** Weapon currently equipped and ready to shoot with */
	TObjectPtr<AShooterWeapon> CurrentWeapon;

	UPROPERTY(EditAnywhere, Category ="Destruction", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float RespawnTime = 5.0f;

	FTimerHandle RespawnTimer;

public:

	/** Bullet count updated delegate */
	FBulletCountUpdatedDelegate OnBulletCountUpdated;

	/** Damaged delegate */
	FDamagedDelegate OnDamaged;

public:

	/** Constructor */
	AShooterCharacter(const FObjectInitializer& ObjectInitializer);

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void PostInitializeComponents() override;

	/** Gameplay cleanup */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	virtual void UnPossessed() override;

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

public:

	/** Handle incoming damage */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	void AddIonCharge(float Amount);
	void ActivateIonSurge();
	void UseFieldPatch();
	FRidgefirePerkModifiers GetPersonalPerkModifiers() const;
	bool HasPersonalPerk(int32 Node) const;
 bool CanUsePrimaryWhileCarrying() const { return HasPersonalPerk(8); }
 void StartRidgefireCrouch();
 void SetRidgefireAiming(bool bHeld);
 bool IsRidgefireSliding() const;
 bool IsRidgefireAiming() const;
 float GetRidgefireShotSpread() const;
#if WITH_DEV_AUTOMATION_TESTS
 // Exact geometry fixtures isolate collision/cover from random accuracy.
 // This seam and its state do not exist in Shipping; ordinary accuracy tests never enable it.
 void SetRidgefireExactRayFixture(bool Enabled){bRidgefireExactRayFixture=Enabled;}
private:
 bool bRidgefireExactRayFixture=false;
public:
#endif
 virtual void Tick(float DeltaSeconds) override;
 void StopRidgefireCrouch();
 void NotifyRidgefireHandlingRecovery();
 float GetRidgefireHandlingRecovery() const;
 float GetRidgefireRecoilScale() const;
private:
 float RidgefireHandlingRecoveryAt = -100.0f;
 float RidgefireNormalFOV=90;
 float RidgefireVisualAim=0, RidgefireVisualKick=0;
 TMap<TWeakObjectPtr<UStaticMeshComponent>,FVector> RidgefireWeaponRestLocations;
 int32 RidgefireRecoilSequence=0;
 float RidgefireLocalShotAt=-100;
 FVector RidgefireStandingCameraLocation = FVector::ZeroVector;
 bool bRidgefireCrouchCameraAdjusted = false;
public:
 UFUNCTION(Client, Unreliable) void ClientRidgefireRecoil(float Amount);
	friend class FGroundedPerkInteractions;
 friend class FPressurePassChecks;
 FTimerHandle RidgefireBurstTimer;
 int32 RidgefireBurstRemaining=0;
 bool bRidgefireBurstAfterReload=false;
 void FireRidgefireBurstFollowup();
	void ApplyPersonalProgressionStats();
	void RestoreHealth(float Amount);
	void EquipRidgefireWeapon(const FRidgefireWeaponRoll& WeaponRoll, int32 CurrentAmmo = -1, int32 ReserveAmmo = -1);
	void UpdateOwnedRidgefireWeapon(int32 Offer,const FRidgefireWeaponRoll& Weapon);
	void RequestRidgefireFinisher();
	UFUNCTION(Server,Reliable) void ServerRidgefireFinisher();
	const FRidgefireWeaponRoll& GetRidgefireWeaponStatsSource() const { return RidgefireWeapon; }
    FVector GetRidgefireMuzzleWorldLocation() const;
	float FinisherNextTime=0;
	void ToggleRidgefireSecondaryWeapon();
	void ReloadRidgefireWeapon();
	void ApplyRidgefireKillReward();
	float GetHealthRatio() const;
	float GetIonCharge() const;
	FString GetRidgefireBlastFeedback() const;
    int32 GetRidgefireBlastHits() const { return RidgefireBlastHits; }
    int32 GetRidgefireBlastKills() const { return RidgefireBlastKills; }
    float GetRidgefireShotFeedbackAlpha() const;
	float GetRidgefireHitFeedbackAlpha() const;
	float GetRidgefireKillFeedbackAlpha() const;
	float GetRidgefireDamageFeedbackAlpha() const;
	float GetRidgefireDamageFeedbackYaw() const;
#if WITH_EDITOR
	bool IsRidgefireBeamVisibleForSmokeTest() const;
	void RunRidgefireHitFeedbackSmokeTest(AShooterNPC* Sentinel, float StartingHealth);
	void RunRidgefireSecondaryDamageSmokeTest(AShooterNPC* Sentinel);
	void RunRidgefireSecondaryWeaponSmokeTest();
#endif
	int32 GetFieldPatches() const;
	FString GetActiveWeaponName() const;
	int32 GetRidgefireAmmo() const;
	int32 GetRidgefireReserveAmmo() const;
	bool IsRidgefireReloading() const { return bRidgefireReloading; }
	void AddRidgefireReserveAmmo(int32 Amount);
	void GrantFieldPatch();
	bool IsRidgefireDowned() const { return bRidgefireDowned; }
	bool IsCarryingRidgefirePlayer() const { return IsValid(RidgefireCarriedPlayer); }
	bool IsRidgefireSecondaryEquipped() const { return bRidgefireSecondaryEquipped; }
	float GetRidgefireCarrySpeed() const { return RidgefireCarrySpeed; }
	AShooterCharacter* GetRidgefireCarrier() const { return RidgefireCarrier; }
	AShooterCharacter* GetRidgefireCarriedPlayer() const { return RidgefireCarriedPlayer; }
	AShooterCharacter* FindRidgefireCarryTarget() const;
	ARidgefireHealingTube* FindNearbyRidgefireHealingTube() const;
	ARidgefireHealingTube* GetRidgefireHealingTube() const { return RidgefireHealingTube; }
	bool TryCarryRidgefirePlayer(AShooterCharacter* Target);
	void ReleaseRidgefireCarriedPlayer();
	bool TransferRidgefireCarriedPlayerToTube(ARidgefireHealingTube* Tube);
	void ExitRidgefireHealingTube(ARidgefireHealingTube* Tube);
	bool TryRestoreRidgefireFromTube(ARidgefireHealingTube* Tube, float HealthFraction);
	void InteractWithRidgefireTeammate();

public:

	/** Handles aim inputs from either controls or UI interfaces */
	virtual void DoAim(float Yaw, float Pitch) override;

	/** Handles move inputs from either controls or UI interfaces */
	virtual void DoMove(float Right, float Forward)  override;

	/** Handles jump start inputs from either controls or UI interfaces */
	virtual void DoJumpStart()  override;

	/** Handles jump end inputs from either controls or UI interfaces */
	virtual void DoJumpEnd()  override;

	/** Handles start firing input */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoStartFiring();

	/** Handles stop firing input */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoStopFiring();

	/** Handles switch weapon input */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoSwitchWeapon();

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

	/** Returns true if the character already owns a weapon of the given class */
	AShooterWeapon* FindWeaponOfType(TSubclassOf<AShooterWeapon> WeaponClass) const;

	/** Called when this character's HP is depleted */
	void Die();
	void FireRidgefireWeapon();
	void ApplyRidgefireHit(AActor* HitActor, const FVector& HitLocation, float Distance, float DamageMultiplier = 1.0f, const FHitResult* HitResult = nullptr, FVector ShotDirection = FVector::ZeroVector);
	void ApplyRidgefireDamageToSentinel(AShooterNPC* Sentinel, float Damage);
	void ApplyRidgefireRadialDamage(float Damage, const FVector& Origin, float Radius, const TArray<AActor*>& IgnoredActors, FVector Normal=FVector::UpVector, bool bUseFalloff=false);
	void RecordRidgefireHitFeedback(AShooterNPC* Sentinel, float HealthBeforeHit);
	void BuildRidgefireWeaponModel();
	UStaticMeshComponent* GetOrCreateRidgefireWeaponPart(FName PartName);
	void BuildRidgefireAppearance();
	void CycleRidgefireAppearance();
	void ToggleRidgefireDefenderHelmet();
	void FinishRidgefireReload();
	void PlayPhasmaShot(const FVector& Location);
 void PlayRidgefireMechanicalSound(const FVector& Location, ERidgefireFireMode Mode, bool bImpact);
 UFUNCTION(NetMulticast, Unreliable) void MulticastRidgefireImpact(FVector Location, ERidgefireFireMode Mode);
	void ShowRidgefireBeam(const FVector& Start, const FVector& End, ERidgefireFireMode FireMode, bool bWildcard);
	UStaticMeshComponent* UpdateRidgefireBeam(UStaticMeshComponent* Beam, FName PartName, const FVector& Start, const FVector& End, ERidgefireFireMode FireMode, bool bWildcard);
	void HideRidgefireBeam();
    void AnimateRidgefireBlast();
    UFUNCTION(NetMulticast,Unreliable) void MulticastRidgefireBlast(FVector Origin,float Radius,FVector Normal);
    UFUNCTION(Client,Reliable) void ClientRidgefireBlastResult(int32 Hits,int32 Kills);
	bool CanCarryRidgefirePlayer(const AShooterCharacter* Target) const;
	void ApplyRidgefireRescuePresentation();
	void ClearRidgefireCarryOwnership();
	UFUNCTION(Server, Reliable)
	void ServerRequestRidgefireCarry(AShooterCharacter* Target);
	UFUNCTION(Server, Reliable)
	void ServerRequestRidgefireRelease();
	UFUNCTION(Server, Reliable)
	void ServerRequestRidgefireDeposit(ARidgefireHealingTube* Tube);
	UFUNCTION()
	void OnRep_RidgefireRescueState();
	UFUNCTION(Server, Reliable)
	void ServerRequestFire();
	UFUNCTION(Server, Reliable)
	void ServerRequestStopFiring();
	UFUNCTION(Server, Reliable)
	void ServerRequestReload();
	UFUNCTION(Server, Reliable)
	void ServerRequestCycleWeapon();
	UFUNCTION(Server, Reliable)
	void ServerRequestToggleSecondaryWeapon();
	UFUNCTION(Server, Reliable)
	void ServerRequestIonSurge();
	UFUNCTION(Server, Reliable)
	void ServerRequestFieldPatch();
	UFUNCTION(Server, Reliable)
	void ServerSetRidgefireAppearance(uint8 Appearance, bool bHelmet);
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRidgefireShotEffects(FVector Start, FVector End, ERidgefireFireMode FireMode, bool bWildcard);
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRidgefireScatterEffects(FVector Start, const TArray<FVector>& Ends, bool bWildcard);
	UFUNCTION()
	void OnRep_CurrentHP();
	UFUNCTION()
	void OnRep_RidgefireAmmo();
	UFUNCTION()
	void OnRep_RidgefireWeapon();
	UFUNCTION()
	void OnRep_RidgefireAppearance();

	/** Called to allow Blueprint code to react to this character's death */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta = (DisplayName = "On Death"))
	void BP_OnDeath();

	/** Called from the respawn timer to destroy this character and force the PC to respawn */
	void OnRespawn();

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Returns true if the character is dead */
	bool IsDead() const;

	/** Sets the team ID for this character */
	void SetTeam(uint8 Team);
};
