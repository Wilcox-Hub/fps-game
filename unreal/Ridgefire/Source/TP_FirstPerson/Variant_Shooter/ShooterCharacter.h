// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TP_FirstPersonCharacter.h"
#include "ShooterWeaponHolder.h"
#include "ShooterCharacter.generated.h"

class AShooterWeapon;
class AShooterNPC;
class UInputAction;
class UInputComponent;
class UPawnNoiseEmitterComponent;
class USoundAttenuation;
class UStaticMeshComponent;

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
	UPROPERTY(EditAnywhere, Category="Health")
	float MaxHP = 500.0f;

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
	TObjectPtr<class UStaticMeshComponent> RidgefireShotBeam;
	UPROPERTY(Transient)
	TObjectPtr<USoundAttenuation> PhasmaShotAttenuation;
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
	bool bRidgefireSecondaryEquipped = false;
	bool bRidgefireTriggerHeld = false;
	bool bRidgefireReloading = false;
	bool bBonusAfterReload = false;

	/** Team ID for this character*/
	UPROPERTY(EditAnywhere, Category="Team")
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
	AShooterCharacter();

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Gameplay cleanup */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

public:

	/** Handle incoming damage */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	void AddIonCharge(float Amount);
	void ActivateIonSurge();
	void UseFieldPatch();
	void RestoreHealth(float Amount);
	void EquipRidgefireWeapon(const FRidgefireWeaponRoll& WeaponRoll, int32 CurrentAmmo = -1, int32 ReserveAmmo = -1);
	void ToggleRidgefireSecondaryWeapon();
	void ReloadRidgefireWeapon();
	void ApplyRidgefireKillReward();
	float GetHealthRatio() const;
	float GetIonCharge() const;
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
	void AddRidgefireReserveAmmo(int32 Amount);
	void GrantFieldPatch();

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
	void ApplyRidgefireRadialDamage(float Damage, const FVector& Origin, float Radius, const TArray<AActor*>& IgnoredActors);
	void RecordRidgefireHitFeedback(AShooterNPC* Sentinel, float HealthBeforeHit);
	void BuildRidgefireWeaponModel();
	void FinishRidgefireReload();
	void PlayPhasmaShot(const FVector& Location);
	void ShowRidgefireBeam(const FVector& Start, const FVector& End, ERidgefireFireMode FireMode, bool bWildcard);
	void HideRidgefireBeam();
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
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRidgefireShotEffects(FVector Start, FVector End, ERidgefireFireMode FireMode, bool bWildcard);
	UFUNCTION()
	void OnRep_CurrentHP();
	UFUNCTION()
	void OnRep_RidgefireAmmo();
	UFUNCTION()
	void OnRep_RidgefireWeapon();

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
