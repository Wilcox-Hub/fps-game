// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShooterCharacter.h"
#include "RidgefireWeaponProgression.h"
#include "ShooterWeapon.h"
#include "EnhancedInputComponent.h"
#include "Components/InputComponent.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "DrawDebugHelpers.h"
#include "InputCoreTypes.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "ShooterGameMode.h"
#include "RidgefireGameMode.h"
#include "ShooterPlayerController.h"
#include "RidgefireSynthWave.h"
#include "Variant_Shooter/AI/ShooterNPC.h"
#include "Engine/EngineTypes.h"
#include "Engine/OverlapResult.h"
#include "Math/RotationMatrix.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Sound/SoundWaveProcedural.h"
#include "Sound/SoundAttenuation.h"
#include "Net/UnrealNetwork.h"
#include "EngineUtils.h"
#include "RidgefireCharacterMovement.h"
#include "RidgefireHealingTube.h"

TArray<FHitResult> RidgefirePiercingTrace::Trace(UWorld* World, const FVector& Start, const FVector& End, const FCollisionQueryParams& QueryParams)
{
	TArray<FHitResult> Hits;
	if (!World || Start.ContainsNaN() || End.ContainsNaN() || Start.Equals(End))
	{
		return Hits;
	}
	FCollisionQueryParams PiercingQueryParams(QueryParams);
	constexpr int32 MaximumPiercingHits = 32;
	for (int32 HitIndex = 0; HitIndex < MaximumPiercingHits; ++HitIndex)
	{
		FHitResult Hit;
		if (!World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, PiercingQueryParams))
		{
			break;
		}
		Hits.Add(Hit);
		AShooterNPC* Sentry = Cast<AShooterNPC>(Hit.GetActor());
		if (!Sentry)
		{
			break;
		}
		PiercingQueryParams.AddIgnoredActor(Sentry);
	}
	return Hits;
}

AShooterCharacter::AShooterCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<URidgefireCharacterMovement>(ACharacter::CharacterMovementComponentName))
{
	bReplicates = true;
 PrimaryActorTick.bCanEverTick=true;
	SetReplicateMovement(true);
	// create the noise emitter component
	PawnNoiseEmitter = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("Pawn Noise Emitter"));

	// configure movement
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 600.0f, 0.0f);
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
}

void AShooterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	RidgefireStandingMeshTransform = GetMesh()->GetRelativeTransform();
 RidgefireNormalFOV=GetFirstPersonCameraComponent()->FieldOfView;
	RidgefireStandingCollision = GetCapsuleComponent()->GetCollisionEnabled();
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// reset HP to max
	if (HasAuthority())
	{
		CurrentHP = MaxHP;
	}

	// update the HUD
	OnDamaged.Broadcast(GetHealthRatio());
	BuildRidgefireAppearance();
	ApplyRidgefireRescuePresentation();
}

void AShooterCharacter::OnRep_CurrentHP()
{
	OnDamaged.Broadcast(GetHealthRatio());
	ApplyRidgefireRescuePresentation();
}

void AShooterCharacter::OnRep_RidgefireAmmo()
{
	OnBulletCountUpdated.Broadcast(RidgefireWeapon.MagazineSize, RidgefireAmmo);
}

void AShooterCharacter::OnRep_RidgefireWeapon()
{
	if (bRidgefireWeaponActive)
	{
		BuildRidgefireWeaponModel();
		OnBulletCountUpdated.Broadcast(RidgefireWeapon.MagazineSize, RidgefireAmmo);
		ApplyRidgefireRescuePresentation();
	}
}

void AShooterCharacter::OnRep_RidgefireAppearance()
{
	BuildRidgefireAppearance();
}

void AShooterCharacter::BuildRidgefireAppearance()
{
	for (UStaticMeshComponent* Part : RidgefireAppearanceParts)
	{
		if (IsValid(Part))
		{
			Part->DestroyComponent();
		}
	}
	RidgefireAppearanceParts.Empty();
	if (!bEnablePlayerModelPrototype)
	{
		return;
	}
	if (!GetMesh() || !GetMesh()->GetSkeletalMeshAsset())
	{
		return;
	}

	static const TCHAR* SpeciesNames[] = { TEXT("Veteran"), TEXT("Veyran"), TEXT("Defender") };
	const int32 SpeciesIndex = FMath::Clamp<int32>(RidgefireAppearance, 0, 2);
	const FString Species = SpeciesNames[SpeciesIndex];
	struct FPartPlacement
	{
		const TCHAR* Name;
		const TCHAR* Bone;
		const TCHAR* EndBone;
	};
	static const FPartPlacement Placements[] = {
		{ TEXT("Head"), TEXT("head"), nullptr },
		{ TEXT("Torso"), TEXT("spine_03"), nullptr },
		{ TEXT("Pelvis"), TEXT("pelvis"), nullptr },
		{ TEXT("UpperArm"), TEXT("upperarm_l"), TEXT("lowerarm_l") },
		{ TEXT("UpperArm"), TEXT("upperarm_r"), TEXT("lowerarm_r") },
		{ TEXT("Forearm"), TEXT("lowerarm_l"), TEXT("hand_l") },
		{ TEXT("Forearm"), TEXT("lowerarm_r"), TEXT("hand_r") },
		{ TEXT("Hand"), TEXT("hand_l"), nullptr },
		{ TEXT("Hand"), TEXT("hand_r"), nullptr },
		{ TEXT("Thigh"), TEXT("thigh_l"), TEXT("calf_l") },
		{ TEXT("Thigh"), TEXT("thigh_r"), TEXT("calf_r") },
		{ TEXT("Calf"), TEXT("calf_l"), TEXT("foot_l") },
		{ TEXT("Calf"), TEXT("calf_r"), TEXT("foot_r") },
		{ TEXT("Foot"), TEXT("foot_l"), nullptr },
		{ TEXT("Foot"), TEXT("foot_r"), nullptr },
	};

	for (const FPartPlacement& Placement : Placements)
	{
		const FName BoneName(Placement.Bone);
		if (GetMesh()->GetBoneIndex(BoneName) == INDEX_NONE)
		{
			continue;
		}
		const FString AssetPath = FString::Printf(TEXT("/Game/Variant_Shooter/Meshes/Players/SM_IS_%s_%s.SM_IS_%s_%s"),
			*Species, Placement.Name, *Species, Placement.Name);
		UStaticMesh* Model = LoadObject<UStaticMesh>(nullptr, *AssetPath);
		if (!Model)
		{
			UE_LOG(LogTemp, Warning, TEXT("Iron Sun player appearance missing %s"), *AssetPath);
			continue;
		}
		const FVector BoneLocation = GetMesh()->GetBoneLocation(BoneName);
		FVector Location = BoneLocation;
		FRotator Rotation = GetActorRotation();
		if (Placement.EndBone && GetMesh()->GetBoneIndex(FName(Placement.EndBone)) != INDEX_NONE)
		{
			const FVector EndLocation = GetMesh()->GetBoneLocation(FName(Placement.EndBone));
			Location = (BoneLocation + EndLocation) * 0.5f;
			const FVector UpDirection = (BoneLocation - EndLocation).GetSafeNormal();
			Rotation = FRotationMatrix::MakeFromZX(UpDirection, GetActorForwardVector()).Rotator();
		}
		UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(this);
		Part->SetStaticMesh(Model);
		Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Part->SetOwnerNoSee(true);
		Part->RegisterComponent();
		Part->SetWorldLocationAndRotation(Location, Rotation);
		Part->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform, BoneName);
		RidgefireAppearanceParts.Add(Part);
	}

	if (SpeciesIndex == 2 && bRidgefireDefenderHelmet)
	{
		const FString HelmetPath = TEXT("/Game/Variant_Shooter/Meshes/Players/SM_IS_Defender_Helmet.SM_IS_Defender_Helmet");
		if (UStaticMesh* Helmet = LoadObject<UStaticMesh>(nullptr, *HelmetPath))
		{
			UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(this);
			Part->SetStaticMesh(Helmet);
			Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Part->SetOwnerNoSee(true);
			Part->RegisterComponent();
			Part->SetWorldLocationAndRotation(GetMesh()->GetBoneLocation(FName(TEXT("head"))), GetActorRotation());
			Part->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform, FName(TEXT("head")));
			RidgefireAppearanceParts.Add(Part);
		}
	}
}

void AShooterCharacter::CycleRidgefireAppearance()
{
	if (bEnablePlayerModelPrototype)
	{
		ServerSetRidgefireAppearance((RidgefireAppearance + 1) % 3, bRidgefireDefenderHelmet);
	}
}

void AShooterCharacter::ToggleRidgefireDefenderHelmet()
{
	if (bEnablePlayerModelPrototype && RidgefireAppearance == 2)
	{
		ServerSetRidgefireAppearance(RidgefireAppearance, !bRidgefireDefenderHelmet);
	}
}

void AShooterCharacter::ServerSetRidgefireAppearance_Implementation(uint8 Appearance, bool bHelmet)
{
	RidgefireAppearance = FMath::Min<uint8>(Appearance, 2);
	bRidgefireDefenderHelmet = bHelmet;
	BuildRidgefireAppearance();
}

void AShooterCharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	ClearRidgefireCarryOwnership();
	Super::EndPlay(EndPlayReason);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RespawnTimer);
		World->GetTimerManager().ClearTimer(RidgefireFireTimer);
		World->GetTimerManager().ClearTimer(RidgefireReloadTimer);
		World->GetTimerManager().ClearTimer(RidgefireBeamTimer);
        World->GetTimerManager().ClearTimer(RidgefireBlastTimer);
	}
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// base class handles move, aim and jump inputs
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Switch weapon
		EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Triggered, this, &AShooterCharacter::DoSwitchWeapon);
	}

	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AShooterCharacter::DoStartFiring);
	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Released, this, &AShooterCharacter::DoStopFiring);
	PlayerInputComponent->BindKey(EKeys::C, IE_Pressed, this, &AShooterCharacter::CycleRidgefireAppearance);
	PlayerInputComponent->BindKey(EKeys::M, IE_Pressed, this, &AShooterCharacter::ToggleRidgefireDefenderHelmet);

}

float AShooterCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority() || CurrentHP <= 0.0f || !FMath::IsFinite(Damage) || Damage <= 0.0f)
	{
		return 0.0f;
	}

	const float AppliedDamage = FMath::Min(Damage * GetPersonalPerkModifiers().IncomingDamage, CurrentHP);
	CurrentHP -= AppliedDamage;
	if (AppliedDamage > 0.0f && GetWorld())
	{
		RidgefireLastDamageTime = GetWorld()->GetTimeSeconds();
		RidgefireLastDamageYaw = 0.0f;
		if (IsValid(DamageCauser))
		{
			const FVector ToDamageSource = DamageCauser->GetActorLocation() - GetActorLocation();
			const float ViewYaw = GetController() ? GetController()->GetControlRotation().Yaw : GetActorRotation().Yaw;
			RidgefireLastDamageYaw = FMath::FindDeltaAngleDegrees(ViewYaw, ToDamageSource.Rotation().Yaw);
		}
	}

	// Have we depleted HP?
	if (CurrentHP <= 0.0f)
	{
		Die();
	}

	// update the HUD
	OnDamaged.Broadcast(FMath::Max(0.0f, CurrentHP / MaxHP));

	return AppliedDamage;
}

void AShooterCharacter::UnPossessed()
{
	ClearRidgefireCarryOwnership();
	Super::UnPossessed();
}

bool AShooterCharacter::CanCarryRidgefirePlayer(const AShooterCharacter* Target) const
{
	const AShooterPlayerController* PlayerController = Cast<AShooterPlayerController>(GetController());
	const AShooterPlayerController* TargetController = IsValid(Target) ? Cast<AShooterPlayerController>(Target->GetController()) : nullptr;
	if (!IsValid(Target) || !PlayerController || PlayerController->GetPawn() != this
		|| (HasAuthority() && (!TargetController || TargetController->GetPawn() != Target))
		|| PlayerController->IsPaused() || Target == this || !GetWorld() || Target->GetWorld() != GetWorld()
		|| IsDead() || IsCarryingRidgefirePlayer() || !Target->bRidgefireDowned || IsValid(Target->RidgefireCarrier) || IsValid(Target->RidgefireHealingTube)
		|| TeamByte != Target->TeamByte || !bRidgefireWeaponActive
		|| FVector::DistSquared(GetActorLocation(), Target->GetActorLocation()) > FMath::Square(200.0f))
	{
		return false;
	}
	if (const ARidgefireGameMode* GameMode = GetWorld()->GetAuthGameMode<ARidgefireGameMode>(); GameMode
		&& (GameMode->IsRunOver() || GameMode->IsArmoryOpen()))
	{
		return false;
	}
	const FRidgefireClientRunState& RunState = PlayerController->GetRidgefireRunState();
	if (RunState.bRunOver || RunState.bArmoryOpen || RunState.bPersonalArsenalOpen || RunState.Wave < 1)
	{
		return false;
	}
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(RidgefireCarryVisibility), false, this);
	QueryParams.AddIgnoredActor(Target);
	FHitResult Obstruction;
	return !GetWorld()->LineTraceSingleByChannel(Obstruction, GetActorLocation(), Target->GetActorLocation(), ECC_Visibility, QueryParams);
}

AShooterCharacter* AShooterCharacter::FindRidgefireCarryTarget() const
{
	AShooterCharacter* Closest = nullptr;
	float ClosestDistance = FMath::Square(200.0f);
	if (GetWorld())
	{
		for (TActorIterator<AShooterCharacter> Player(GetWorld()); Player; ++Player)
		{
			const float Distance = FVector::DistSquared(GetActorLocation(), Player->GetActorLocation());
			if (Distance <= ClosestDistance && CanCarryRidgefirePlayer(*Player))
			{
				Closest = *Player;
				ClosestDistance = Distance;
			}
		}
	}
	return Closest;
}

bool AShooterCharacter::TryCarryRidgefirePlayer(AShooterCharacter* Target)
{
	if (!HasAuthority() || !CanCarryRidgefirePlayer(Target))
	{
		return false;
	}
	AShooterPlayerController* PlayerController = Cast<AShooterPlayerController>(GetController());
	UnCrouch();
	GetCharacterMovement()->UnCrouch(false);
	if (bIsCrouched) return false; // A low ceiling must not trap a standing carry.
	PlayerController->CancelRidgefireSprint();
	if (!bRidgefireSecondaryEquipped && !CanUsePrimaryWhileCarrying())
	{
		ToggleRidgefireSecondaryWeapon();
	}
	if (!bRidgefireSecondaryEquipped && !CanUsePrimaryWhileCarrying())
	{
		return false;
	}
	DoStopFiring();
	RidgefireCarrySpeed = GetCharacterMovement()->GetMaxSpeed() * GetPersonalPerkModifiers().CarrySpeedFraction;
	if (!Target->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale))
	{
		return false;
	}
	RidgefireCarriedPlayer = Target;
	Target->RidgefireCarrier = this;
	Target->SetActorRelativeLocation(FVector(-25.0f, 0.0f, 55.0f));
	Target->ApplyRidgefireRescuePresentation();
	ForceNetUpdate();
	Target->ForceNetUpdate();
	return true;
}

void AShooterCharacter::ReleaseRidgefireCarriedPlayer()
{
	if (!HasAuthority())
	{
		return;
	}
	AShooterCharacter* Target = RidgefireCarriedPlayer;
	RidgefireCarriedPlayer = nullptr;
	if (IsValid(Target) && Target->RidgefireCarrier == this)
	{
		Target->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		FVector DropLocation = GetActorLocation();
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(RidgefireCarryDrop), false, this);
		QueryParams.AddIgnoredActor(Target);
		FHitResult FloorHit;
		const FCollisionShape DropShape = FCollisionShape::MakeCapsule(Target->GetCapsuleComponent()->GetScaledCapsuleRadius(),
			Target->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		if (GetWorld()->SweepSingleByChannel(FloorHit, DropLocation, DropLocation - FVector(0.0f, 0.0f, 2500.0f),
			FQuat::Identity, ECC_Pawn, DropShape, QueryParams) && !FloorHit.bStartPenetrating)
		{
			DropLocation = FloorHit.Location;
		}
		Target->SetActorLocation(DropLocation);
		Target->RidgefireCarrier = nullptr;
		Target->ApplyRidgefireRescuePresentation();
		Target->ForceNetUpdate();
	}
	ForceNetUpdate();
}

ARidgefireHealingTube* AShooterCharacter::FindNearbyRidgefireHealingTube() const
{
	ARidgefireHealingTube* Closest = nullptr;
	float ClosestDistance = FMath::Square(220.0f);
	if (GetWorld() && IsCarryingRidgefirePlayer())
	{
		for (TActorIterator<ARidgefireHealingTube> Tube(GetWorld()); Tube; ++Tube)
		{
			const float Distance = FVector::DistSquared(GetActorLocation(), Tube->GetActorLocation() + FVector(0.0f, 0.0f, 96.0f));
			if (Distance <= ClosestDistance)
			{
				Closest = *Tube;
				ClosestDistance = Distance;
			}
		}
	}
	return Closest;
}

bool AShooterCharacter::TransferRidgefireCarriedPlayerToTube(ARidgefireHealingTube* Tube)
{
	ARidgefireGameMode* Mode = GetWorld() ? GetWorld()->GetAuthGameMode<ARidgefireGameMode>() : nullptr;
	AShooterCharacter* Target = RidgefireCarriedPlayer;
	AShooterPlayerController* PlayerController = Cast<AShooterPlayerController>(GetController());
	AShooterPlayerController* TargetController = IsValid(Target) ? Cast<AShooterPlayerController>(Target->GetController()) : nullptr;
	if (!HasAuthority() || !IsValid(Tube) || !Mode || !Mode->IsRidgefireHealingTubeActive(Tube)
		|| Mode->IsRunOver() || Mode->IsArmoryOpen() || Mode->GetWave() < 1 || Tube->GetOccupant()
		|| !PlayerController || PlayerController->GetPawn() != this || PlayerController->IsPaused() || IsDead()
		|| !TargetController || TargetController->GetPawn() != Target || !Target->bRidgefireDowned
		|| Target->RidgefireCarrier != this || IsValid(Target->RidgefireHealingTube))
	{
		return false;
	}
	if (!Target->AttachToComponent(Tube->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale))
	{
		return false;
	}
	RidgefireCarriedPlayer = nullptr;
	Target->RidgefireCarrier = nullptr;
	Target->RidgefireHealingTube = Tube;
	Target->ApplyRidgefireRescuePresentation();
	Target->ForceNetUpdate();
	ForceNetUpdate();
	return true;
}

void AShooterCharacter::ExitRidgefireHealingTube(ARidgefireHealingTube* Tube)
{
	if (!HasAuthority() || !Tube || RidgefireHealingTube != Tube)
	{
		return;
	}
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	RidgefireHealingTube = nullptr;
	const FVector FloorOrigin = Tube->GetActorLocation() + FVector(140.0f, 0.0f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 5.0f);
	SetActorLocation(FloorOrigin);
	ApplyRidgefireRescuePresentation();
	ForceNetUpdate();
}

bool AShooterCharacter::TryRestoreRidgefireFromTube(ARidgefireHealingTube* Tube, float HealthFraction)
{
	ARidgefireGameMode* Mode = GetWorld() ? GetWorld()->GetAuthGameMode<ARidgefireGameMode>() : nullptr;
	AShooterPlayerController* PlayerController = Cast<AShooterPlayerController>(GetController());
	if (!HasAuthority() || !IsValid(Tube) || RidgefireHealingTube != Tube || Tube->GetOccupant() != this
		|| !bRidgefireDowned || !Mode || Mode->IsRunOver() || !Mode->IsRidgefireHealingTubeActive(Tube)
		|| Tube->GetRecoverySeconds() < 60.0f || Tube->GetRecoveryPauseRemaining() > 0.0f
		|| !PlayerController || PlayerController->GetPawn() != this || !FMath::IsFinite(HealthFraction)
		|| HealthFraction <= 0.0f || MaxHP <= 0.0f)
	{
		return false;
	}
	FCollisionQueryParams Query(SCENE_QUERY_STAT(RidgefireReviveExit), false, this);
	Query.AddIgnoredActor(Tube);
	const float HalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FCollisionShape Capsule = FCollisionShape::MakeCapsule(GetCapsuleComponent()->GetScaledCapsuleRadius(), HalfHeight);
	FVector ExitLocation;
	bool bExitClear = false;
	for (const FVector Offset : {FVector(140.0f, 0.0f, 0.0f), FVector(-140.0f, 0.0f, 0.0f),
		FVector(0.0f, 140.0f, 0.0f), FVector(0.0f, -140.0f, 0.0f)})
	{
		const FVector Candidate = Tube->GetActorLocation() + Offset;
		FHitResult Floor;
		if (GetWorld()->LineTraceSingleByChannel(Floor, Candidate + FVector(0.0f, 0.0f, 200.0f),
			Candidate - FVector(0.0f, 0.0f, 300.0f), ECC_Visibility, Query) && Floor.ImpactNormal.Z >= 0.7f
			&& FMath::Abs(Floor.ImpactPoint.Z - Tube->GetActorLocation().Z) <= GetCharacterMovement()->MaxStepHeight + 5.0f)
		{
			ExitLocation = Floor.ImpactPoint + FVector(0.0f, 0.0f, HalfHeight + 5.0f);
			FHitResult PathHit;
			if (!GetWorld()->OverlapBlockingTestByChannel(ExitLocation, FQuat::Identity, ECC_Pawn, Capsule, Query)
				&& !GetWorld()->SweepSingleByChannel(PathHit, Tube->GetActorLocation() + FVector(0.0f, 0.0f, HalfHeight + 5.0f),
					ExitLocation, FQuat::Identity, ECC_Pawn, Capsule, Query))
			{
				bExitClear = true;
				break;
			}
		}
	}
	if (!bExitClear) return false;
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	RidgefireHealingTube = nullptr;
	SetActorLocation(ExitLocation);
	CurrentHP = MaxHP * FMath::Clamp(HealthFraction, 0.01f, 1.0f);
	bRidgefireDowned = false;
	ApplyRidgefireRescuePresentation();
	OnDamaged.Broadcast(GetHealthRatio());
	ForceNetUpdate();
	return true;
}

void AShooterCharacter::ClearRidgefireCarryOwnership()
{
	if (HasAuthority())
	{
		if (IsValid(RidgefireCarrier))
		{
			RidgefireCarrier->ReleaseRidgefireCarriedPlayer();
		}
		ReleaseRidgefireCarriedPlayer();
		if (IsValid(RidgefireHealingTube))
		{
			RidgefireHealingTube->ReleaseOccupant(this);
		}
	}
}

void AShooterCharacter::InteractWithRidgefireTeammate()
{
	if (IsCarryingRidgefirePlayer())
	{
		if (ARidgefireHealingTube* Tube = FindNearbyRidgefireHealingTube())
		{
			ServerRequestRidgefireDeposit(Tube);
		}
		else
		{
			ServerRequestRidgefireRelease();
		}
	}
	else if (AShooterCharacter* Target = FindRidgefireCarryTarget())
	{
		ServerRequestRidgefireCarry(Target);
	}
}

void AShooterCharacter::ServerRequestRidgefireCarry_Implementation(AShooterCharacter* Target)
{
	TryCarryRidgefirePlayer(Target);
}

void AShooterCharacter::ServerRequestRidgefireRelease_Implementation()
{
	ReleaseRidgefireCarriedPlayer();
}

void AShooterCharacter::ServerRequestRidgefireDeposit_Implementation(ARidgefireHealingTube* Tube)
{
	if (IsValid(Tube))
	{
		Tube->TryDeposit(this);
	}
}

void AShooterCharacter::OnRep_RidgefireRescueState()
{
	ApplyRidgefireRescuePresentation();
	if (IsCarryingRidgefirePlayer())
	{
		if (AShooterPlayerController* PlayerController = Cast<AShooterPlayerController>(GetController()))
		{
			PlayerController->CancelRidgefireSprint();
		}
	}
}

void AShooterCharacter::ApplyRidgefireRescuePresentation()
{
	if (bRidgefireDowned)
	{
		bRidgefireRescuePresentationApplied = true;
		Tags.AddUnique(DeathTag);
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->DisableMovement();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetMesh()->SetSimulatePhysics(false);
		GetMesh()->bPauseAnims = true;
		if (IsValid(RidgefireHealingTube))
		{
			AttachToComponent(RidgefireHealingTube->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			SetActorRelativeLocation(FVector(0.0f, 0.0f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 12.0f));
			GetMesh()->SetRelativeTransform(RidgefireStandingMeshTransform);
			const float FootHeight = FMath::Min(GetMesh()->GetBoneLocation(FName(TEXT("foot_l"))).Z,
				GetMesh()->GetBoneLocation(FName(TEXT("foot_r"))).Z);
			const float DesiredFootHeight = RidgefireHealingTube->GetActorLocation().Z + 12.0f
				+ RidgefireTubeFootClearance * GetMesh()->GetRelativeScale3D().Z;
			GetMesh()->SetRelativeLocation(GetMesh()->GetRelativeLocation() + FVector(0.0f, 0.0f, DesiredFootHeight - FootHeight));
			CacheInitialMeshOffset(GetMesh()->GetRelativeLocation(), GetMesh()->GetRelativeRotation());
		}
		else
		{
			const FRotator DownedRotation(0.0f, 0.0f, 90.0f);
			const FVector TorsoLocation = GetMesh()->GetSocketTransform(FName(TEXT("spine_03")), RTS_Component).GetLocation();
			const FVector TorsoOffset = DownedRotation.RotateVector(TorsoLocation * GetMesh()->GetRelativeScale3D());
			const FVector TorsoAnchor(0.0f, 0.0f, IsValid(RidgefireCarrier) ? 0.0f : 25.0f - GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
			const FVector MeshLocation = TorsoAnchor - TorsoOffset;
			GetMesh()->SetRelativeLocationAndRotation(MeshLocation, DownedRotation);
			CacheInitialMeshOffset(MeshLocation, DownedRotation);
			if (IsValid(RidgefireCarrier))
			{
				AttachToComponent(RidgefireCarrier->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				SetActorRelativeLocation(FVector(-25.0f, 0.0f, 55.0f));
			}
			else if (Cast<AShooterCharacter>(GetAttachParentActor()) || Cast<ARidgefireHealingTube>(GetAttachParentActor()))
			{
				DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			}
		}
	}
	else if (bRidgefireRescuePresentationApplied && !IsDead())
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		Tags.Remove(DeathTag);
		GetMesh()->SetSimulatePhysics(false);
		GetMesh()->bPauseAnims = false;
		GetMesh()->SetRelativeTransform(RidgefireStandingMeshTransform);
		CacheInitialMeshOffset(GetMesh()->GetRelativeLocation(), GetMesh()->GetRelativeRotation());
		GetCapsuleComponent()->SetCollisionEnabled(RidgefireStandingCollision);
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		if (IsValid(CurrentWeapon) && !bRidgefireWeaponActive) CurrentWeapon->ActivateWeapon(PlayerTag);
		bRidgefireRescuePresentationApplied = false;
	}
	GetFirstPersonMesh()->SetVisibility(!bRidgefireDowned && !IsDead() && !bRidgefireWeaponActive, true);
	for (UStaticMeshComponent* Part : RidgefireWeaponParts)
	{
		if (IsValid(Part))
		{
			Part->SetVisibility(!bRidgefireDowned && !IsDead());
		}
	}
}

void AShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShooterCharacter, CurrentHP);
	DOREPLIFETIME(AShooterCharacter, MaxHP);
	DOREPLIFETIME(AShooterCharacter, IonCharge);
	DOREPLIFETIME(AShooterCharacter, FieldPatches);
	DOREPLIFETIME(AShooterCharacter, RidgefireWeapon);
	DOREPLIFETIME(AShooterCharacter, RidgefireAmmo);
	DOREPLIFETIME(AShooterCharacter, RidgefireReserveAmmo);
	DOREPLIFETIME(AShooterCharacter, bRidgefireReloading);
	DOREPLIFETIME(AShooterCharacter, bRidgefireWeaponActive);
	DOREPLIFETIME(AShooterCharacter, RidgefireAppearance);
	DOREPLIFETIME(AShooterCharacter, bRidgefireDefenderHelmet);
	DOREPLIFETIME(AShooterCharacter, bRidgefireSecondaryEquipped);
	DOREPLIFETIME(AShooterCharacter, bRidgefireDowned);
	DOREPLIFETIME(AShooterCharacter, RidgefireCarrier);
	DOREPLIFETIME(AShooterCharacter, RidgefireCarriedPlayer);
	DOREPLIFETIME(AShooterCharacter, RidgefireCarrySpeed);
	DOREPLIFETIME(AShooterCharacter, RidgefireHealingTube);
	DOREPLIFETIME(AShooterCharacter, TeamByte);
}

void AShooterCharacter::AddIonCharge(float Amount)
{
	if (HasAuthority() && FMath::IsFinite(Amount))
	{
		IonCharge = FMath::Clamp(IonCharge + Amount * (Amount > 0.0f ? GetPersonalPerkModifiers().IonCharge : 1.0f), 0.0f, 100.0f);
	}
}

void AShooterCharacter::ActivateIonSurge()
{
	if (!HasAuthority())
	{
		ServerRequestIonSurge();
		return;
	}
	UWorld* World = GetWorld();
	UCameraComponent* Camera = GetFirstPersonCameraComponent();
	if (IonCharge < 100.0f || IsDead() || !World || !Camera)
	{
		return;
	}

	IonCharge = 0.0f;
	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectQuery;
	ObjectQuery.AddObjectTypesToQuery(ECC_Pawn);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	const FVector Origin = Camera->GetComponentLocation();
	World->OverlapMultiByObjectType(Overlaps, Origin, FQuat::Identity, ObjectQuery, FCollisionShape::MakeSphere(1800.0f), QueryParams);

	TSet<AActor*> HitSentinels;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AShooterNPC* Sentinel = Cast<AShooterNPC>(Overlap.GetActor());
		if (!Sentinel || HitSentinels.Contains(Sentinel))
		{
			continue;
		}

		FHitResult VisibilityHit;
		FCollisionQueryParams VisibilityParams;
		VisibilityParams.AddIgnoredActor(this);
		const bool bBlocked = World->LineTraceSingleByChannel(VisibilityHit, Origin, Sentinel->GetActorLocation(), ECC_Visibility, VisibilityParams)
			&& VisibilityHit.GetActor() != Sentinel;
		if (!bBlocked)
		{
			HitSentinels.Add(Sentinel);
			UGameplayStatics::ApplyDamage(Sentinel, 320.0f, GetController(), this, UDamageType::StaticClass());
		}
	}
}

void AShooterCharacter::UseFieldPatch()
{
	if (!HasAuthority())
	{
		ServerRequestFieldPatch();
		return;
	}
	if (FieldPatches <= 0 || IsDead() || CurrentHP >= MaxHP)
	{
		return;
	}

	--FieldPatches;
	RestoreHealth(190.0f * GetPersonalPerkModifiers().PatchHealing);
}

void AShooterCharacter::RestoreHealth(float Amount)
{
	if (HasAuthority() && !IsDead() && FMath::IsFinite(Amount) && Amount > 0.0f && MaxHP > 0.0f)
	{
		CurrentHP = FMath::Min(MaxHP, CurrentHP + Amount);
		OnDamaged.Broadcast(CurrentHP / MaxHP);
	}
}

float AShooterCharacter::GetHealthRatio() const
{
	return MaxHP > 0.0f ? FMath::Clamp(CurrentHP / MaxHP, 0.0f, 1.0f) : 0.0f;
}

float AShooterCharacter::GetIonCharge() const
{
	return IonCharge;
}

float AShooterCharacter::GetRidgefireShotFeedbackAlpha() const
{
	if (!GetWorld() || RidgefireLastShotTime < 0.0f)
	{
		return 0.0f;
	}
	return FMath::Clamp(1.0f - (GetWorld()->GetTimeSeconds() - RidgefireLastShotTime) / 0.18f, 0.0f, 1.0f);
}

float AShooterCharacter::GetRidgefireHitFeedbackAlpha() const
{
	if (!GetWorld() || RidgefireLastHitTime < 0.0f)
	{
		return 0.0f;
	}
	return FMath::Clamp(1.0f - (GetWorld()->GetTimeSeconds() - RidgefireLastHitTime) / 0.24f, 0.0f, 1.0f);
}

float AShooterCharacter::GetRidgefireKillFeedbackAlpha() const
{
	if (!GetWorld() || RidgefireLastKillTime < 0.0f)
	{
		return 0.0f;
	}
	return FMath::Clamp(1.0f - (GetWorld()->GetTimeSeconds() - RidgefireLastKillTime) / 0.48f, 0.0f, 1.0f);
}

float AShooterCharacter::GetRidgefireDamageFeedbackAlpha() const
{
	if (!GetWorld() || RidgefireLastDamageTime < 0.0f)
	{
		return 0.0f;
	}
	return FMath::Clamp(1.0f - (GetWorld()->GetTimeSeconds() - RidgefireLastDamageTime) / 0.6f, 0.0f, 1.0f);
}

float AShooterCharacter::GetRidgefireDamageFeedbackYaw() const
{
	return RidgefireLastDamageYaw;
}

int32 AShooterCharacter::GetFieldPatches() const
{
	return FieldPatches;
}

FString AShooterCharacter::GetActiveWeaponName() const
{
	return RidgefireWeapon.Name;
}

int32 AShooterCharacter::GetRidgefireAmmo() const
{
	return RidgefireAmmo;
}

int32 AShooterCharacter::GetRidgefireReserveAmmo() const
{
	return RidgefireReserveAmmo;
}

void AShooterCharacter::AddRidgefireReserveAmmo(int32 Amount)
{
	if (!HasAuthority())
	{
		return;
	}
	RidgefireReserveAmmo = static_cast<int32>(FMath::Clamp<int64>(static_cast<int64>(RidgefireReserveAmmo) + Amount, 0, MAX_int32));
	OnBulletCountUpdated.Broadcast(RidgefireWeapon.MagazineSize, RidgefireAmmo);
}

void AShooterCharacter::GrantFieldPatch()
{
	FieldPatches = FMath::Min(FieldPatches + 1, 5);
}

void AShooterCharacter::EquipRidgefireWeapon(const FRidgefireWeaponRoll& WeaponRoll, int32 CurrentAmmo, int32 ReserveAmmo)
{
	if (!HasAuthority() || (IsCarryingRidgefirePlayer() && !CanUsePrimaryWhileCarrying()) || bRidgefireDowned)
	{
		return;
	}
	GetWorldTimerManager().ClearTimer(RidgefireBurstTimer);RidgefireBurstRemaining=0;
	RidgefireWeapon = WeaponRoll;
	RidgefireWeapon.MagazineSize = FMath::Clamp(RidgefireWeapon.MagazineSize, 1, 100);
	RidgefireWeapon.ReserveAmmo = FMath::Clamp(RidgefireWeapon.ReserveAmmo, 0, 10000);
	RidgefireWeapon.Pellets = FMath::Clamp(RidgefireWeapon.Pellets, 1, 16);
	RidgefireWeapon.RefireDelay = FMath::IsFinite(RidgefireWeapon.RefireDelay)
		? FMath::Clamp(RidgefireWeapon.RefireDelay, 0.055f, 3.0f)
		: 0.16f;
	if (!FMath::IsFinite(RidgefireWeapon.DamageScale))
	{
		RidgefireWeapon.DamageScale = 1.0f;
	}
	RidgefireWeapon.DamageScale = FMath::Clamp(RidgefireWeapon.DamageScale, 0.05f, 40.0f);
	RidgefireAmmo = FMath::Clamp(CurrentAmmo >= 0 ? CurrentAmmo : RidgefireWeapon.MagazineSize, 0, RidgefireWeapon.MagazineSize);
	RidgefireReserveAmmo = FMath::Clamp(ReserveAmmo >= 0 ? ReserveAmmo : RidgefireWeapon.ReserveAmmo, 0, 10000);
	bRidgefireWeaponActive = true;
	bRidgefireTriggerHeld = false;
	bRidgefireReloading = false;
	bBonusAfterReload = false;
	RidgefireNextFireTime = 0.0f;
	GetWorldTimerManager().ClearTimer(RidgefireFireTimer);
	GetWorldTimerManager().ClearTimer(RidgefireReloadTimer);
	HideRidgefireBeam();

	if (CurrentWeapon)
	{
		CurrentWeapon->DeactivateWeapon();
		CurrentWeapon->SetActorHiddenInGame(true);
	}

	BuildRidgefireWeaponModel();
	OnBulletCountUpdated.Broadcast(RidgefireWeapon.MagazineSize, RidgefireAmmo);
}

void AShooterCharacter::ToggleRidgefireSecondaryWeapon()
{
	if (!HasAuthority())
	{
		ServerRequestToggleSecondaryWeapon();
		return;
	}
	if (!bRidgefireWeaponActive || IsDead() || (IsCarryingRidgefirePlayer() && !CanUsePrimaryWhileCarrying()))
	{
		return;
	}

	AShooterPlayerController* PlayerController = Cast<AShooterPlayerController>(GetController());
	if (!PlayerController)
	{
		return;
	}
	const FRidgefireClientRunState& RunState = PlayerController->GetRidgefireRunState();
	const int32 OfferIndex = RunState.SecondaryWeaponOfferIndex;
	if (RunState.bRunOver || RunState.Wave < 1)
	{
		return;
	}

	if (bRidgefireSecondaryEquipped)
	{
		RidgefireSecondaryAmmo = RidgefireAmmo;
		RidgefireSecondaryReserveAmmo = RidgefireReserveAmmo;
		bRidgefireSecondaryEquipped = false;
		EquipRidgefireWeapon(RidgefireStoredPrimaryWeapon, RidgefireStoredPrimaryAmmo, RidgefireStoredPrimaryReserveAmmo);
		return;
	}
	if (RunState.bArmoryOpen || !RunState.WeaponOffers.IsValidIndex(OfferIndex))
	{
		return;
	}
	if (RidgefireSecondaryOfferIndex != OfferIndex)
	{
		RidgefireSecondaryOfferIndex = OfferIndex;
		const FRidgefireWeaponRoll& Offer = RunState.WeaponOffers[OfferIndex];
		RidgefireSecondaryAmmo = Offer.MagazineSize;
		RidgefireSecondaryReserveAmmo = Offer.ReserveAmmo;
	}

	RidgefireStoredPrimaryWeapon = RidgefireWeapon;
	RidgefireStoredPrimaryAmmo = RidgefireAmmo;
	RidgefireStoredPrimaryReserveAmmo = RidgefireReserveAmmo;
	bRidgefireSecondaryEquipped = true;
	EquipRidgefireWeapon(RunState.WeaponOffers[OfferIndex], RidgefireSecondaryAmmo, RidgefireSecondaryReserveAmmo);
}

UStaticMeshComponent* AShooterCharacter::GetOrCreateRidgefireWeaponPart(FName PartName)
{
	UStaticMeshComponent* Part = nullptr;
	for (UStaticMeshComponent* Candidate : RidgefireWeaponPartPool)
	{
		if (IsValid(Candidate) && Candidate->GetFName() == PartName)
		{
			Part = Candidate;
			break;
		}
	}
	if (!Part)
	{
		Part = NewObject<UStaticMeshComponent>(this, PartName);
		Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Part->SetGenerateOverlapEvents(false);
		Part->SetCastShadow(false);
		AddInstanceComponent(Part);
		RidgefireWeaponPartPool.Add(Part);
	}
	Part->SetRelativeRotation(FRotator::ZeroRotator);
	Part->SetVisibility(true);
	Part->SetHiddenInGame(false);
	RidgefireWeaponParts.AddUnique(Part);
	return Part;
}

void AShooterCharacter::BuildRidgefireWeaponModel()
{
	for (UStaticMeshComponent* Part : RidgefireWeaponPartPool)
	{
		if (IsValid(Part))
		{
			Part->SetVisibility(false);
			Part->SetHiddenInGame(true);
		}
	}
	RidgefireWeaponParts.Reset();
 RidgefireWeaponRestLocations.Reset();
	auto GetPartMaterial = [this](UStaticMeshComponent* Part, int32 Slot, UMaterialInterface* Parent)
	{
		UMaterialInstanceDynamic* Material = Cast<UMaterialInstanceDynamic>(Part->GetMaterial(Slot));
		if (!Material || Material->Parent != Parent)
		{
			Material = UMaterialInstanceDynamic::Create(Parent, this);
			if (Material) Part->SetMaterial(Slot, Material);
		}
		return Material;
	};

	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	UMaterialInterface* RidgefireMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Variant_Shooter/Materials/M_Ridgefire_Weapon.M_Ridgefire_Weapon"));
	if (RidgefireMaterial)
	{
		BaseMaterial = RidgefireMaterial;
	}
	if (!GetFirstPersonCameraComponent())
	{
		return;
	}
	if (USkeletalMeshComponent* FirstPersonArms = GetFirstPersonMesh())
	{
		FirstPersonArms->SetVisibility(false, true);
	}

	const FLinearColor MainColor = RidgefireWeapon.Mode == ERidgefireFireMode::Phasma
		? FLinearColor(0.24f, 0.29f, 0.36f)
		: RidgefireWeapon.bWildcard ? FLinearColor(0.16f, 0.045f, 0.015f) : FLinearColor(0.025f, 0.10f, 0.105f);
	const FLinearColor AccentColor = RidgefireWeapon.Mode == ERidgefireFireMode::Phasma
		? FLinearColor(0.10f, 0.68f, 0.98f)
		: RidgefireWeapon.bWildcard ? FLinearColor(0.98f, 0.67f, 0.16f) : FLinearColor(0.45f, 0.90f, 0.80f);
	const FLinearColor TrimColor = RidgefireWeapon.Mode == ERidgefireFireMode::Phasma
		? FLinearColor(0.72f, 0.80f, 0.90f)
		: FMath::Lerp(MainColor, AccentColor, 0.48f);

	UStaticMesh* ViewModel = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Variant_Shooter/Meshes/SM_Ridgefire_ViewModel.SM_Ridgefire_ViewModel"));
	if (ViewModel && RidgefireMaterial)
	{
		const FVector ViewModelLocation(67.0f, 28.0f, -24.0f);
		const float ViewModelScale = 0.9f;
		RidgefireMuzzleOffset = ViewModelLocation + FVector(52.6f, 0.0f, 1.8f) * ViewModelScale;
		UStaticMeshComponent* WeaponModel = GetOrCreateRidgefireWeaponPart(TEXT("RidgefireWeaponViewModel"));
		WeaponModel->SetStaticMesh(ViewModel);
		WeaponModel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponModel->SetGenerateOverlapEvents(false);
		WeaponModel->SetCastShadow(false);
		WeaponModel->SetRelativeLocation(ViewModelLocation);
		WeaponModel->SetRelativeScale3D(FVector(ViewModelScale));
		WeaponModel->AttachToComponent(GetFirstPersonCameraComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		if (!WeaponModel->IsRegistered()) WeaponModel->RegisterComponent();

		const int32 MaterialCount = ViewModel->GetStaticMaterials().Num();
		for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
		{
			const FString SlotName = ViewModel->GetStaticMaterials()[MaterialIndex].MaterialSlotName.ToString();
			const bool bIonSlot = SlotName.Contains(TEXT("Ion"));
			const bool bTrimSlot = SlotName.Contains(TEXT("Trim"));
			const FLinearColor SlotColor = bIonSlot ? AccentColor : bTrimSlot ? TrimColor : MainColor;
			UMaterialInstanceDynamic* Material = GetPartMaterial(WeaponModel, MaterialIndex, RidgefireMaterial);
			if (!Material)
			{
				continue;
			}
			Material->SetVectorParameterValue(TEXT("BaseColor"), SlotColor);
			Material->SetVectorParameterValue(TEXT("EmissiveColor"), bIonSlot ? AccentColor * (RidgefireWeapon.Mode == ERidgefireFireMode::Phasma ? 0.75f : 1.25f) : SlotColor * 0.035f);
			Material->SetScalarParameterValue(TEXT("Metallic"), bIonSlot ? 0.36f : RidgefireWeapon.Mode == ERidgefireFireMode::Phasma ? 0.92f : 0.72f);
			Material->SetScalarParameterValue(TEXT("Roughness"), bIonSlot ? 0.2f : RidgefireWeapon.Mode == ERidgefireFireMode::Phasma ? 0.16f : 0.34f);
			WeaponModel->SetMaterial(MaterialIndex, Material);
		}
		if (RidgefireWeapon.Mode == ERidgefireFireMode::Phasma && Cube)
		{
			for (int32 Side = 0; Side < 2; ++Side)
			{
				UStaticMeshComponent* EmitterProng = GetOrCreateRidgefireWeaponPart(Side == 0 ? TEXT("PhasmaEmitterProngLeft") : TEXT("PhasmaEmitterProngRight"));
				EmitterProng->SetStaticMesh(Cube);
				EmitterProng->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				EmitterProng->SetGenerateOverlapEvents(false);
				EmitterProng->SetCastShadow(false);
				const FVector EmitterWorldLocation(98.0f, 26.0f + (Side == 0 ? -12.0f : 12.0f), -27.0f);
				EmitterProng->SetRelativeLocation((EmitterWorldLocation - ViewModelLocation) / ViewModelScale);
				EmitterProng->SetRelativeScale3D(FVector(0.05f, 0.009f, 0.027f) / ViewModelScale);
				EmitterProng->AttachToComponent(WeaponModel, FAttachmentTransformRules::KeepRelativeTransform);
				if (!EmitterProng->IsRegistered()) EmitterProng->RegisterComponent();
				if (RidgefireMaterial)
				{
					UMaterialInstanceDynamic* ProngMaterial = GetPartMaterial(EmitterProng, 0, RidgefireMaterial);
					if (ProngMaterial)
					{
						ProngMaterial->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.78f, 0.84f, 0.92f));
						ProngMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(0.12f, 0.52f, 0.82f));
						ProngMaterial->SetScalarParameterValue(TEXT("Metallic"), 0.94f);
						ProngMaterial->SetScalarParameterValue(TEXT("Roughness"), 0.14f);
						EmitterProng->SetMaterial(0, ProngMaterial);
					}
				}
			}
		}
		else if (RidgefireWeapon.Mode != ERidgefireFireMode::Phasma && Cylinder)
		{
			UStaticMeshComponent* Foregrip = GetOrCreateRidgefireWeaponPart(TEXT("GaleRepeaterForegrip"));
			Foregrip->SetStaticMesh(Cylinder);
			Foregrip->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Foregrip->SetGenerateOverlapEvents(false);
			Foregrip->SetCastShadow(false);
			Foregrip->SetRelativeLocation(FVector(18.0f, 0.0f, -18.0f));
			Foregrip->SetRelativeScale3D(FVector(0.022f, 0.022f, 0.065f));
			Foregrip->AttachToComponent(WeaponModel, FAttachmentTransformRules::KeepRelativeTransform);
			if (!Foregrip->IsRegistered()) Foregrip->RegisterComponent();
			if (UMaterialInstanceDynamic* ForegripMaterial = GetPartMaterial(Foregrip, 0, RidgefireMaterial))
			{
				ForegripMaterial->SetVectorParameterValue(TEXT("BaseColor"), MainColor * 0.72f);
				ForegripMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), AccentColor * 0.035f);
				ForegripMaterial->SetScalarParameterValue(TEXT("Metallic"), 0.68f);
				ForegripMaterial->SetScalarParameterValue(TEXT("Roughness"), 0.36f);
				Foregrip->SetMaterial(0, ForegripMaterial);
			}
		}
		return;
	}

	if (!Cube)
	{
		UE_LOG(LogTemp, Warning, TEXT("RIDGEFIRE: Authored weapon viewmodel unavailable; cannot build primitive fallback."));
		return;
	}
	const FVector FallbackPivot(72.0f, 33.0f, -30.0f);
	const float FallbackScale = 1.3f;
	RidgefireMuzzleOffset = FallbackPivot + (FVector(97.0f, 33.0f, -28.0f) - FallbackPivot) * FallbackScale;

	auto AddPart = [this, Cube, Cylinder, Sphere, BaseMaterial, FallbackPivot, FallbackScale, GetPartMaterial](const TCHAR* PartName, const FVector& Location, const FVector& Scale, const FLinearColor& Color, UStaticMesh* Shape = nullptr, const FRotator& Rotation = FRotator::ZeroRotator)
	{
		UStaticMeshComponent* Part = GetOrCreateRidgefireWeaponPart(PartName);
		Part->SetStaticMesh(Shape ? Shape : Cube);
		Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Part->SetCastShadow(false);
		Part->SetRelativeLocation(FallbackPivot + (Location - FallbackPivot) * FallbackScale);
		Part->SetRelativeScale3D(Scale * FallbackScale);
		Part->SetRelativeRotation(Rotation);
		Part->AttachToComponent(GetFirstPersonCameraComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		if (!Part->IsRegistered()) Part->RegisterComponent();
			if (BaseMaterial)
			{
				UMaterialInstanceDynamic* Material = GetPartMaterial(Part, 0, BaseMaterial);
				Material->SetVectorParameterValue(TEXT("Color"), Color);
				Material->SetVectorParameterValue(TEXT("BaseColor"), Color);
				Material->SetVectorParameterValue(TEXT("EmissiveColor"), Color * (RidgefireWeapon.Mode == ERidgefireFireMode::Phasma ? 0.10f : 0.18f));
				Material->SetScalarParameterValue(TEXT("Metallic"), RidgefireWeapon.Mode == ERidgefireFireMode::Phasma ? 0.92f : 0.28f);
				Material->SetScalarParameterValue(TEXT("Roughness"), RidgefireWeapon.Mode == ERidgefireFireMode::Phasma ? 0.16f : 0.46f);
			Part->SetMaterial(0, Material);
		}
	};

	if (RidgefireWeapon.Mode == ERidgefireFireMode::Phasma)
	{
		AddPart(TEXT("PhasmaEmitterProngLeft"), FVector(98.0f, 21.0f, -27.0f), FVector(0.05f, 0.009f, 0.027f), TrimColor, Cube);
		AddPart(TEXT("PhasmaEmitterProngRight"), FVector(98.0f, 45.0f, -27.0f), FVector(0.05f, 0.009f, 0.027f), TrimColor, Cube);
	}
	else if (Cylinder)
	{
		AddPart(TEXT("GaleRepeaterForegrip"), FVector(82.0f, 33.0f, -42.0f), FVector(0.022f, 0.022f, 0.065f), MainColor * 0.72f, Cylinder);
	}

	AddPart(TEXT("RidgefireWeaponReceiver"), FVector(72.0f, 33.0f, -30.0f), FVector(0.095f, 0.035f, 0.04f), MainColor);
	AddPart(TEXT("RidgefireWeaponBarrel"), FVector(88.0f, 33.0f, -28.0f), FVector(0.019f, 0.019f, 0.075f), AccentColor, Cylinder, FRotator(90.0f, 0.0f, 0.0f));
	AddPart(TEXT("RidgefireWeaponGrip"), FVector(66.0f, 33.0f, -38.0f), FVector(0.026f, 0.03f, 0.045f), MainColor * 0.55f);
	AddPart(TEXT("RidgefireWeaponSight"), FVector(73.0f, 33.0f, -22.0f), FVector(0.024f, 0.022f, 0.014f), AccentColor);
	AddPart(TEXT("RidgefireWeaponStock"), FVector(57.0f, 33.0f, -32.0f), FVector(0.036f, 0.03f, 0.028f), MainColor * 0.72f);
	AddPart(TEXT("RidgefireWeaponMuzzle"), FVector(97.0f, 33.0f, -28.0f), FVector(0.03f, 0.03f, 0.014f), AccentColor, Cylinder, FRotator(90.0f, 0.0f, 0.0f));
	AddPart(TEXT("RidgefireWeaponCore"), FVector(79.0f, 33.0f, -28.0f), FVector(0.018f), AccentColor, Sphere);
}

void AShooterCharacter::ReloadRidgefireWeapon()
{
	if (!HasAuthority())
	{
		ServerRequestReload();
		return;
	}
	if (!bRidgefireWeaponActive || bRidgefireReloading || IsDead() || RidgefireAmmo >= RidgefireWeapon.MagazineSize || RidgefireReserveAmmo <= 0)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(RidgefireBurstTimer);RidgefireBurstRemaining=0;
	bRidgefireReloading = true;
	bRidgefireTriggerHeld = false;
	GetWorldTimerManager().ClearTimer(RidgefireFireTimer);
	GetWorldTimerManager().SetTimer(RidgefireReloadTimer, this, &AShooterCharacter::FinishRidgefireReload, RidgefireWeaponProgression::ReloadSeconds(RidgefireWeapon) * GetPersonalPerkModifiers().ReloadTime, false);
}

void AShooterCharacter::FinishRidgefireReload()
{
	if (!bRidgefireWeaponActive || IsDead())
	{
		bRidgefireReloading = false;
		return;
	}

	const int32 Reloaded = FMath::Min(RidgefireWeapon.MagazineSize - RidgefireAmmo, RidgefireReserveAmmo);
	RidgefireAmmo += Reloaded;
	RidgefireReserveAmmo -= Reloaded;
	bRidgefireReloading = false;
	bBonusAfterReload = true;
	OnBulletCountUpdated.Broadcast(RidgefireWeapon.MagazineSize, RidgefireAmmo);
}

void AShooterCharacter::ApplyRidgefireKillReward()
{
	if (!HasAuthority())
	{
		return;
	}
	if (RidgefireWeapon.Payload == ERidgefirePayload::Salvage)
	{
		const int32 AmmoFound = FMath::RandRange(2, 30);
		AddRidgefireReserveAmmo(AmmoFound);
	}
	else if (RidgefireWeapon.Payload == ERidgefirePayload::Siphon)
	{
		const TArray<float> HealRolls = {2.0f, 8.0f, 20.0f, 45.0f};
		RestoreHealth(HealRolls[FMath::RandRange(0, HealRolls.Num() - 1)]);
	}
}

FVector AShooterCharacter::GetRidgefireMuzzleWorldLocation() const
{
    return GetFirstPersonCameraComponent()->GetComponentTransform().TransformPosition(RidgefireMuzzleOffset+(IsRidgefireAiming() ? FVector(0,-28,12) : FVector::ZeroVector));
}

void AShooterCharacter::FireRidgefireWeapon()
{
	if (!HasAuthority() || !bRidgefireTriggerHeld || !bRidgefireWeaponActive || bRidgefireReloading || IsDead()
		|| (IsCarryingRidgefirePlayer() && !bRidgefireSecondaryEquipped && !CanUsePrimaryWhileCarrying()))
	{
		return;
	}
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	const ARidgefireGameMode* GameMode = World->GetAuthGameMode<ARidgefireGameMode>();
	if (!GameMode || GameMode->IsArmoryOpen() || GameMode->IsRunOver())
	{
		return;
	}
	if (const AShooterPlayerController* PlayerController = Cast<AShooterPlayerController>(GetController()); PlayerController
		&& PlayerController->GetRidgefireRunState().bPersonalArsenalOpen)
	{
		DoStopFiring();
		return;
	}
	const float CurrentTime = World->GetTimeSeconds();
	if (RidgefireNextFireTime > 0.0f && CurrentTime < RidgefireNextFireTime)
	{
		World->GetTimerManager().SetTimer(RidgefireFireTimer, this, &AShooterCharacter::FireRidgefireWeapon, RidgefireNextFireTime - CurrentTime, false);
		return;
	}
	if (RidgefireAmmo <= 0)
	{
		ReloadRidgefireWeapon();
		return;
	}

	--RidgefireAmmo;
	RidgefireLastShotTime = CurrentTime;
	OnBulletCountUpdated.Broadcast(RidgefireWeapon.MagazineSize, RidgefireAmmo);
	const float RefireDelay = FMath::Max(0.055f, RidgefireWeapon.RefireDelay);
	RidgefireNextFireTime = CurrentTime + RefireDelay;
	const UCameraComponent* Camera = GetFirstPersonCameraComponent();
	const FVector AimStart = Camera->GetComponentLocation();
	const FVector AimEnd = AimStart + Camera->GetForwardVector() * MaxAimDistance;
	const FVector Start = GetRidgefireMuzzleWorldLocation();
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(RidgefireWeapon), true, this);
	QueryParams.AddIgnoredActor(this);
	FHitResult AimHit;
	const bool bAimHit = GetWorld()->LineTraceSingleByChannel(AimHit, AimStart, AimEnd, ECC_Visibility, QueryParams);
	const FVector BaseShotDirection = ((bAimHit ? AimHit.ImpactPoint : AimEnd) - Start).GetSafeNormal();
 const FVector ShotDirection = (RidgefireWeapon.Mode==ERidgefireFireMode::Scatter || RidgefireWeapon.Mode==ERidgefireFireMode::Roulette) ? BaseShotDirection
  : FMath::VRandCone(BaseShotDirection,FMath::DegreesToRadians(GetRidgefireShotSpread()));
	const FVector End = Start + ShotDirection * MaxAimDistance;
	FHitResult MainHit;
	const bool bHasMainHit = GetWorld()->LineTraceSingleByChannel(MainHit, Start, End, ECC_Visibility, QueryParams);
	const TArray<FHitResult> PiercingHits = RidgefireWeapon.Mode == ERidgefireFireMode::Pierce
		? RidgefirePiercingTrace::Trace(GetWorld(), Start, End, QueryParams) : TArray<FHitResult>();
	const TArray<FHitResult> UpgradeHits=RidgefireWeapon.UpgradeBehavior==ERidgefireUpgradeBehavior::ThroughShot
        ? RidgefirePiercingTrace::Trace(GetWorld(),Start,End,QueryParams) : TArray<FHitResult>();

	const FVector ImpactPoint = PiercingHits.Num() > 0
		? PiercingHits.Last().ImpactPoint : bHasMainHit ? MainHit.ImpactPoint : End;
	const float Distance = FVector::Distance(Start, ImpactPoint);
#if WITH_EDITOR
	if (FParse::Param(FCommandLine::Get(), TEXT("RidgefireCoopCombatSmoke")))
	{
		const AShooterNPC* HitSentinel = bHasMainHit ? Cast<AShooterNPC>(MainHit.GetActor()) : nullptr;
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP SERVER TRACE pawn=%s camera=%s forward=%s aimActor=%s mainActor=%s hitHP=%.1f"),
			*GetNameSafe(this), *AimStart.ToCompactString(), *Camera->GetForwardVector().ToCompactString(),
			*GetNameSafe(bAimHit ? AimHit.GetActor() : nullptr), *GetNameSafe(bHasMainHit ? MainHit.GetActor() : nullptr),
			HitSentinel ? HitSentinel->CurrentHP : -1.0f);
	}
#endif
	const FColor TracerColor = RidgefireWeapon.Mode == ERidgefireFireMode::Phasma ? FColor(220, 242, 255) : RidgefireWeapon.bWildcard ? FColor(255, 104, 30) : FColor(65, 245, 214);
	if (RidgefireWeapon.Mode != ERidgefireFireMode::Roulette && RidgefireWeapon.Mode != ERidgefireFireMode::Scatter)
	{
		MulticastRidgefireShotEffects(Start, ImpactPoint, RidgefireWeapon.Mode, RidgefireWeapon.bWildcard);
		if (bHasMainHit)
		{
			DrawDebugPoint(GetWorld(), ImpactPoint, 14.0f, TracerColor, false, 0.12f);
		}
	}

	if (RidgefireWeapon.Mode == ERidgefireFireMode::Scatter)
	{
		TArray<FVector> PelletEnds;
		PelletEnds.Reserve(RidgefireWeapon.Pellets);
		for (int32 Pellet = 0; Pellet < RidgefireWeapon.Pellets; ++Pellet)
		{
			const FVector PelletDirection = FMath::VRandCone(ShotDirection, FMath::DegreesToRadians(GetRidgefireShotSpread()));
			const FVector PelletEnd = Start + PelletDirection * MaxAimDistance;
			FHitResult PelletHit;
			const bool bPelletHit = GetWorld()->LineTraceSingleByChannel(PelletHit, Start, PelletEnd, ECC_Visibility, QueryParams);
			PelletEnds.Add(bPelletHit ? PelletHit.ImpactPoint : PelletEnd);
			if (bPelletHit)
			{
				ApplyRidgefireHit(PelletHit.GetActor(), PelletHit.ImpactPoint, FVector::Distance(Start, PelletHit.ImpactPoint), 1.0f / FMath::Sqrt(static_cast<float>(RidgefireWeapon.Pellets)), &PelletHit, PelletDirection);
			}
		}
		MulticastRidgefireScatterEffects(Start, PelletEnds, RidgefireWeapon.bWildcard);
	}
	else if (RidgefireWeapon.Mode == ERidgefireFireMode::Pierce)
	{
		for (const FHitResult& Hit : PiercingHits)
		{
			ApplyRidgefireHit(Hit.GetActor(), Hit.ImpactPoint, FVector::Distance(Start, Hit.ImpactPoint), 1.0f, &Hit, ShotDirection);
		}
	}
	else if (RidgefireWeapon.Mode == ERidgefireFireMode::Roulette)
	{
		const FVector WildDirection = FMath::VRandCone(ShotDirection, FMath::DegreesToRadians(GetRidgefireShotSpread()));
		FHitResult WildHit;
		const FVector WildEnd = Start + WildDirection * MaxAimDistance;
		const bool bHasWildHit = GetWorld()->LineTraceSingleByChannel(WildHit, Start, WildEnd, ECC_Visibility, QueryParams);
		MulticastRidgefireShotEffects(Start, bHasWildHit ? WildHit.ImpactPoint : WildEnd, RidgefireWeapon.Mode, RidgefireWeapon.bWildcard);
		if (bHasWildHit)
		{
			DrawDebugPoint(GetWorld(), WildHit.ImpactPoint, 14.0f, TracerColor, false, 0.12f);
			const TArray<float> Chaos = {0.1f, 0.4f, 1.0f, 2.0f, 5.0f, 12.0f};
			ApplyRidgefireHit(WildHit.GetActor(), WildHit.ImpactPoint, FVector::Distance(Start, WildHit.ImpactPoint), Chaos[FMath::RandRange(0, Chaos.Num() - 1)], &WildHit, WildDirection);
		}
	}
	else if (bHasMainHit)
	{
		if (RidgefireWeapon.Mode == ERidgefireFireMode::Mortar || RidgefireWeapon.Mode == ERidgefireFireMode::Phasma)
		{
			const bool bPhasmaBlast = RidgefireWeapon.Mode == ERidgefireFireMode::Phasma;
			const float Radius = bPhasmaBlast ? 460.0f : 280.0f;
			const float Damage = bPhasmaBlast ? FMath::Min(75.0f, 95.0f * RidgefireWeapon.DamageScale) : 95.0f * RidgefireWeapon.DamageScale;
			TArray<AActor*> IgnoredActors = {this};
			if (bPhasmaBlast)
			{
				// Exclude the direct recipient from splash, but preserve world cover
				// (including other components owned by the struck arena actor).
				if (AShooterNPC* DirectSentinel = Cast<AShooterNPC>(MainHit.GetActor()))
				{
					IgnoredActors.Add(DirectSentinel);
				}
			}
			ApplyRidgefireRadialDamage(Damage, ImpactPoint, Radius, IgnoredActors, MainHit.ImpactNormal, bPhasmaBlast);
			ApplyRidgefireHit(MainHit.GetActor(), ImpactPoint, Distance, 1.0f, &MainHit, ShotDirection);
		}
		else if (RidgefireWeapon.Mode == ERidgefireFireMode::Burst)
		{
			ApplyRidgefireHit(MainHit.GetActor(), ImpactPoint, Distance, 0.72f, &MainHit, ShotDirection);
		}
		else
		{
			ApplyRidgefireHit(MainHit.GetActor(), ImpactPoint, Distance, 1.0f, &MainHit, ShotDirection);
			if (RidgefireWeapon.Mode == ERidgefireFireMode::Echo)
			{
				ApplyRidgefireHit(MainHit.GetActor(), ImpactPoint, Distance, 0.72f, &MainHit, ShotDirection);
			}
			else if (RidgefireWeapon.Mode == ERidgefireFireMode::Arc)
			{
				TArray<FOverlapResult> Nearby;
				FCollisionObjectQueryParams ObjectQuery;
				ObjectQuery.AddObjectTypesToQuery(ECC_Pawn);
				GetWorld()->OverlapMultiByObjectType(Nearby, ImpactPoint, FQuat::Identity, ObjectQuery, FCollisionShape::MakeSphere(620.0f), QueryParams);
				int32 Chained = 0;
				for (const FOverlapResult& NearbyHit : Nearby)
				{
					if (NearbyHit.GetActor() != MainHit.GetActor() && Cast<AShooterNPC>(NearbyHit.GetActor()) && Chained < 4)
					{
						ApplyRidgefireHit(NearbyHit.GetActor(), NearbyHit.GetActor()->GetActorLocation(), FVector::Distance(Start, NearbyHit.GetActor()->GetActorLocation()), FMath::Pow(0.72f, ++Chained));
					}
				}
			}
		}
	}

    if(UpgradeHits.Num()>1) {
        const FHitResult& Second=UpgradeHits[1];
        ApplyRidgefireHit(Second.GetActor(),Second.ImpactPoint,FVector::Distance(Start,Second.ImpactPoint),0.5f,&Second,ShotDirection);
    }
 if(RidgefireWeapon.Mode==ERidgefireFireMode::Burst)
 {
  RidgefireBurstRemaining=2;
  bRidgefireBurstAfterReload=bBonusAfterReload;
  GetWorldTimerManager().SetTimer(RidgefireBurstTimer,this,&AShooterCharacter::FireRidgefireBurstFollowup,FMath::Min(0.065f,RefireDelay/3),true);
 }
 // Owner-only camera recoil: server damage/traces remain authoritative.
 ClientRidgefireRecoil(RidgefireWeaponProgression::RecoilDegrees(RidgefireWeapon));
	if (bBonusAfterReload && RidgefireWeapon.Condition == ERidgefireCondition::AfterReload)
	{
		bBonusAfterReload = false;
	}

	if (RidgefireAmmo == 0 && RidgefireReserveAmmo > 0 && RidgefireBurstRemaining==0)
	{
		ReloadRidgefireWeapon();
	}
	else if (bRidgefireTriggerHeld)
	{
		GetWorldTimerManager().SetTimer(RidgefireFireTimer, this, &AShooterCharacter::FireRidgefireWeapon, RefireDelay, false);
	}
}

UStaticMeshComponent* AShooterCharacter::UpdateRidgefireBeam(UStaticMeshComponent* Beam, FName PartName, const FVector& Start, const FVector& End, ERidgefireFireMode FireMode, bool bWildcard)
{
	if (!Beam)
	{
		UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
		UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Variant_Shooter/Materials/M_Ridgefire_Weapon.M_Ridgefire_Weapon"));
		if (!Cylinder || !Material)
		{
			return Beam;
		}
		Beam = NewObject<UStaticMeshComponent>(this, PartName);
		Beam->SetStaticMesh(Cylinder);
		Beam->SetMaterial(0, Material);
		Beam->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Beam->SetCastShadow(false);
		Beam->SetupAttachment(GetRootComponent());
		Beam->SetAbsolute(true, true, true);
		AddInstanceComponent(Beam);
		Beam->RegisterComponent();
	}

	const FVector BeamVector = End - Start;
	const float BeamLength = BeamVector.Size();
	if (BeamLength < 1.0f)
	{
		return Beam;
	}
	const FLinearColor BeamColor = FireMode == ERidgefireFireMode::Phasma
		? FLinearColor(0.45f, 0.85f, 1.0f)
		: bWildcard ? FLinearColor(1.0f, 0.25f, 0.035f) : FLinearColor(0.12f, 0.9f, 0.72f);
	UMaterialInstanceDynamic* Material = Cast<UMaterialInstanceDynamic>(Beam->GetMaterial(0));
	if (!Material)
	{
		Material = Beam->CreateDynamicMaterialInstance(0);
	}
	if (Material)
	{
		Material->SetVectorParameterValue(TEXT("BaseColor"), BeamColor);
		Material->SetVectorParameterValue(TEXT("EmissiveColor"), BeamColor * 3.0f);
		Material->SetScalarParameterValue(TEXT("Metallic"), 0.0f);
		Material->SetScalarParameterValue(TEXT("Roughness"), 0.5f);
	}
	Beam->SetWorldLocation((Start + End) * 0.5f);
	Beam->SetWorldRotation(FRotationMatrix::MakeFromZ(BeamVector).Rotator());
	const float BeamRadius = FireMode == ERidgefireFireMode::Phasma ? 0.035f : 0.012f;
	Beam->SetWorldScale3D(FVector(BeamRadius, BeamRadius, BeamLength / 100.0f));
	Beam->SetHiddenInGame(false);
	return Beam;
}

void AShooterCharacter::ShowRidgefireBeam(const FVector& Start, const FVector& End, ERidgefireFireMode FireMode, bool bWildcard)
{
	for (UStaticMeshComponent* Part : RidgefireScatterBeams)
	{
		if (IsValid(Part)) Part->SetHiddenInGame(true);
	}
	RidgefireShotBeam = UpdateRidgefireBeam(RidgefireShotBeam, TEXT("RidgefireShotBeam"), Start, End, FireMode, bWildcard);
	if ((End - Start).Size() < 1.0f)
	{
		return;
	}
	if (RidgefireShotBeam && !RidgefireShotBeam->bHiddenInGame)
	{
		const float BeamLifetime = FireMode == ERidgefireFireMode::Phasma ? 0.22f : 0.11f;
		GetWorldTimerManager().SetTimer(RidgefireBeamTimer, this, &AShooterCharacter::HideRidgefireBeam, BeamLifetime, false);
	}
}

void AShooterCharacter::MulticastRidgefireShotEffects_Implementation(FVector Start, FVector End, ERidgefireFireMode FireMode, bool bWildcard)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}
	ShowRidgefireBeam(Start, End, FireMode, bWildcard);
 if(FireMode!=ERidgefireFireMode::Phasma) PlayRidgefireMechanicalSound(Start,FireMode,false);
	if (FireMode == ERidgefireFireMode::Phasma)
	{
		PlayPhasmaShot(Start);
	}
}

void AShooterCharacter::MulticastRidgefireScatterEffects_Implementation(FVector Start, const TArray<FVector>& Ends, bool bWildcard)
{
	if (GetNetMode() == NM_DedicatedServer || Start.ContainsNaN() || Ends.IsEmpty() || Ends.Num() > 16)
	{
		return;
	}
	for (const FVector& End : Ends)
	{
		if (End.ContainsNaN()) return;
	}
	ShowRidgefireBeam(Start, Ends[0], ERidgefireFireMode::Scatter, bWildcard);
 PlayRidgefireMechanicalSound(Start,ERidgefireFireMode::Scatter,false);
	for (int32 PelletIndex = 1; PelletIndex < Ends.Num(); ++PelletIndex)
	{
		if (!RidgefireScatterBeams.IsValidIndex(PelletIndex - 1)) RidgefireScatterBeams.Add(nullptr);
		const FName PartName(*FString::Printf(TEXT("RidgefireScatterBeam_%d"), PelletIndex));
		RidgefireScatterBeams[PelletIndex - 1] = UpdateRidgefireBeam(RidgefireScatterBeams[PelletIndex - 1], PartName,
			Start, Ends[PelletIndex], ERidgefireFireMode::Scatter, bWildcard);
	}
	GetWorldTimerManager().SetTimer(RidgefireBeamTimer, this, &AShooterCharacter::HideRidgefireBeam, 0.11f, false);
}

void AShooterCharacter::HideRidgefireBeam()
{
	GetWorldTimerManager().ClearTimer(RidgefireBeamTimer);
	if (RidgefireShotBeam)
	{
		RidgefireShotBeam->SetHiddenInGame(true);
	}
	for (UStaticMeshComponent* Part : RidgefireScatterBeams)
	{
		if (IsValid(Part)) Part->SetHiddenInGame(true);
	}
}

#if WITH_EDITOR
bool AShooterCharacter::IsRidgefireBeamVisibleForSmokeTest() const
{
	return RidgefireShotBeam && !RidgefireShotBeam->bHiddenInGame;
}
#endif

void AShooterCharacter::ApplyRidgefireHit(AActor* HitActor, const FVector& HitLocation, float Distance, float DamageMultiplier, const FHitResult* HitResult, FVector ShotDirection)
{
	if (!HasAuthority() || !FMath::IsFinite(DamageMultiplier) || DamageMultiplier <= 0.0f)
	{
		return;
	}
	AShooterNPC* Sentinel = Cast<AShooterNPC>(HitActor);
	if (!Sentinel || Sentinel->CurrentHP <= 0.0f)
	{
		return;
	}

	float Damage = 42.0f * RidgefireWeapon.DamageScale * DamageMultiplier * GetPersonalPerkModifiers().OutgoingDamage;
    const ARidgefireGameMode* Mode=GetWorld()->GetAuthGameMode<ARidgefireGameMode>();
    Damage*=RidgefireWeaponProgression::ConditionMultiplier(RidgefireWeapon,Distance,CastChecked<URidgefireCharacterMovement>(GetCharacterMovement())->bSprintRequested,
        CurrentHP/FMath::Max(1.0f,MaxHP),RidgefireAmmo,Mode ? Mode->GetStreak() : 0,bBonusAfterReload);

	if (RidgefireWeapon.Payload == ERidgefirePayload::Glass)
	{
		const TArray<float> FragileRolls = {0.12f, 0.3f, 0.65f, 1.4f, 3.0f, 8.0f};
		Damage *= FragileRolls[FMath::RandRange(0, FragileRolls.Num() - 1)];
	}
	else if (RidgefireWeapon.Payload == ERidgefirePayload::Leaden)
	{
		Damage *= 0.38f;
		if (FMath::FRand() < 0.15f)
		{
			UGameplayStatics::ApplyDamage(this, 5.0f, nullptr, this, UDamageType::StaticClass());
		}
	}

	if (HasPersonalPerk(4) && HitResult && (HitResult->BoneName == TEXT("head")
  || (HitResult->Component.IsValid() && HitResult->Component->ComponentHasTag(TEXT("RidgefireHead"))))) Damage *= 1.08f;
	const float HealthBeforeHit = Sentinel->CurrentHP;
 MulticastRidgefireImpact(HitLocation,RidgefireWeapon.Mode);
    if(RidgefireWeapon.UpgradeBehavior==ERidgefireUpgradeBehavior::EchoPulse) {
        const float PulseDamage=Damage*0.35f*(RidgefireWeapon.Payload==ERidgefirePayload::Burn ? 1.28f : 1);
        const TWeakObjectPtr<AShooterNPC> Target=Sentinel;const TWeakObjectPtr<AShooterCharacter> Shooter=this;
        FTimerHandle Pulse;GetWorldTimerManager().SetTimer(Pulse,FTimerDelegate::CreateWeakLambda(this,[Target,Shooter,PulseDamage]() {
            if(Target.IsValid() && Shooter.IsValid() && Target->CurrentHP>0)
                UGameplayStatics::ApplyDamage(Target.Get(),PulseDamage,Shooter->GetController(),Shooter.Get(),UDamageType::StaticClass());
        }),0.09f,false);
    }
	if (HitResult)
	{
		UGameplayStatics::ApplyPointDamage(Sentinel, Damage, ShotDirection, *HitResult, GetController(), this, UDamageType::StaticClass());
	}
	else
	{
		UGameplayStatics::ApplyDamage(Sentinel, Damage, GetController(), this, UDamageType::StaticClass());
	}
	if (RidgefireWeapon.Payload == ERidgefirePayload::Burn)
	{
		UGameplayStatics::ApplyDamage(Sentinel, Damage * 0.28f, GetController(), this, UDamageType::StaticClass());
	}
	else if (RidgefireWeapon.Payload == ERidgefirePayload::Frost)
	{
		Sentinel->ApplyRidgefireSlow(2.5f, 0.4f);
	}
	else if (RidgefireWeapon.Payload == ERidgefirePayload::Siphon)
	{
		RestoreHealth(8.0f);
	}
	RecordRidgefireHitFeedback(Sentinel, HealthBeforeHit);

	if (RidgefireWeapon.Payload == ERidgefirePayload::Volatile && HealthBeforeHit > 0.0f && Sentinel->CurrentHP <= 0.0f)
	{
		const float BlastDamage = Damage * FMath::RandRange(0.1f, 0.75f);
		TArray<AActor*> IgnoredActors = {this, Sentinel};
		ApplyRidgefireRadialDamage(BlastDamage, HitLocation, 430.0f, IgnoredActors);
	}

	if (RidgefireWeapon.Payload == ERidgefirePayload::Ricochet || RidgefireWeapon.Payload == ERidgefirePayload::Split)
	{
		TArray<FOverlapResult> Nearby;
		FCollisionObjectQueryParams ObjectQuery;
		ObjectQuery.AddObjectTypesToQuery(ECC_Pawn);
		const float Radius = RidgefireWeapon.Payload == ERidgefirePayload::Split ? 330.0f : 220.0f;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		GetWorld()->OverlapMultiByObjectType(Nearby, HitLocation, FQuat::Identity, ObjectQuery, FCollisionShape::MakeSphere(Radius), QueryParams);
		AShooterNPC* Nearest = nullptr;
		float NearestDistance = TNumericLimits<float>::Max();
		for (const FOverlapResult& Candidate : Nearby)
		{
			AShooterNPC* OtherSentinel = Cast<AShooterNPC>(Candidate.GetActor());
			if (OtherSentinel && OtherSentinel != Sentinel && OtherSentinel->CurrentHP > 0.0f)
			{
				const float CandidateDistance = FVector::DistSquared(HitLocation, OtherSentinel->GetActorLocation());
				if (CandidateDistance < NearestDistance)
				{
					Nearest = OtherSentinel;
					NearestDistance = CandidateDistance;
				}
			}
		}
		if (Nearest)
		{
			ApplyRidgefireDamageToSentinel(Nearest, Damage * 0.62f);
		}
	}
}

void AShooterCharacter::ApplyRidgefireDamageToSentinel(AShooterNPC* Sentinel, float Damage)
{
	if (!HasAuthority() || !IsValid(Sentinel) || Sentinel->CurrentHP <= 0.0f || !FMath::IsFinite(Damage) || Damage <= 0.0f)
	{
		return;
	}

	const float HealthBeforeHit = Sentinel->CurrentHP;
	UGameplayStatics::ApplyDamage(Sentinel, Damage, GetController(), this, UDamageType::StaticClass());
	RecordRidgefireHitFeedback(Sentinel, HealthBeforeHit);
}

void AShooterCharacter::ApplyRidgefireRadialDamage(float Damage, const FVector& Origin, float Radius, const TArray<AActor*>& IgnoredActors, FVector Normal, bool bUseFalloff)
{
	if (!HasAuthority() || !FMath::IsFinite(Damage) || Damage <= 0.0f || !FMath::IsFinite(Radius) || Radius <= 0.0f)
	{
		return;
	}
	TArray<AActor*> SentinelActors;
	UGameplayStatics::GetAllActorsOfClass(this, AShooterNPC::StaticClass(), SentinelActors);
	TArray<TPair<TWeakObjectPtr<AShooterNPC>, float>> HealthBeforeHits;
	for (AActor* Actor : SentinelActors)
	{
		if (AShooterNPC* Sentinel = Cast<AShooterNPC>(Actor); Sentinel && Sentinel->CurrentHP > 0.0f)
		{
			HealthBeforeHits.Emplace(Sentinel, Sentinel->CurrentHP);
		}
	}

    MulticastRidgefireBlast(Origin,Radius,Normal);
	if (bUseFalloff)
	{
		for (const TPair<TWeakObjectPtr<AShooterNPC>, float>& HealthBeforeHit : HealthBeforeHits)
		{
			AShooterNPC* Sentinel = HealthBeforeHit.Key.Get();
			if (!Sentinel || Sentinel->CurrentHP <= 0.0f || IgnoredActors.Contains(Sentinel))
			{
				continue;
			}
			const float Distance = FVector::Distance(Origin, Sentinel->GetActorLocation());
			const float DistanceScale = FMath::Clamp(1.0f - Distance / Radius, 0.0f, 1.0f);
			const float DamageAtDistance = Damage * DistanceScale;
			if (DamageAtDistance <= KINDA_SMALL_NUMBER)
			{
				continue;
			}

			FCollisionQueryParams VisibilityParams(SCENE_QUERY_STAT(RidgefirePhasmaBlast), false, this);
			for (AActor* IgnoredActor : IgnoredActors)
			{
				if (IsValid(IgnoredActor))
				{
					VisibilityParams.AddIgnoredActor(IgnoredActor);
				}
			}
			VisibilityParams.AddIgnoredActor(Sentinel);
			FHitResult Blocker;
			if (GetWorld()->LineTraceSingleByChannel(Blocker, Origin, Sentinel->GetActorLocation(), ECC_Visibility, VisibilityParams))
			{
				continue;
			}
			UGameplayStatics::ApplyDamage(Sentinel, DamageAtDistance, GetController(), this, UDamageType::StaticClass());
		}
	}
	else
	{
		UGameplayStatics::ApplyRadialDamage(this, Damage, Origin, Radius, UDamageType::StaticClass(), IgnoredActors, this, GetController(), false);
	}
    int32 Hits=0,Kills=0;
	for (const TPair<TWeakObjectPtr<AShooterNPC>, float>& HealthBeforeHit : HealthBeforeHits)
	{
		if (AShooterNPC* Sentinel = HealthBeforeHit.Key.Get())
		{
            if(Sentinel->CurrentHP<HealthBeforeHit.Value) {++Hits;if(Sentinel->CurrentHP<=0) ++Kills;}
			RecordRidgefireHitFeedback(Sentinel, HealthBeforeHit.Value);
		}
	}
    ClientRidgefireBlastResult(Hits,Kills);
}

void AShooterCharacter::RecordRidgefireHitFeedback(AShooterNPC* Sentinel, float HealthBeforeHit)
{
	if (!IsValid(Sentinel) || Sentinel->CurrentHP >= HealthBeforeHit || !GetWorld())
	{
		return;
	}

	RidgefireLastHitTime = GetWorld()->GetTimeSeconds();
	if (HealthBeforeHit > 0.0f && Sentinel->CurrentHP <= 0.0f)
	{
		RidgefireLastKillTime = RidgefireLastHitTime;
	}
}

#if WITH_EDITOR
void AShooterCharacter::RunRidgefireHitFeedbackSmokeTest(AShooterNPC* Sentinel, float StartingHealth)
{
	if (!IsValid(Sentinel))
	{
		return;
	}

	RidgefireLastHitTime = -1.0f;
	RidgefireLastKillTime = -1.0f;
	const FRidgefireWeaponRoll OriginalWeapon = RidgefireWeapon;
	RidgefireWeapon = FRidgefireWeaponRoll();
	RidgefireWeapon.Mode = ERidgefireFireMode::Auto;
	RidgefireWeapon.Payload = ERidgefirePayload::Plain;
	RidgefireWeapon.Condition = ERidgefireCondition::Steady;
	RidgefireWeapon.DamageScale = 1.0f;
	Sentinel->CurrentHP = StartingHealth;
	const FVector HitLocation = Sentinel->GetActorLocation();
	ApplyRidgefireHit(Sentinel, HitLocation, FVector::Distance(GetPawnViewLocation(), HitLocation));
	RidgefireWeapon = OriginalWeapon;
}

void AShooterCharacter::RunRidgefireSecondaryDamageSmokeTest(AShooterNPC* Sentinel)
{
	if (!IsValid(Sentinel))
	{
		return;
	}

	RidgefireLastHitTime = -1.0f;
	RidgefireLastKillTime = -1.0f;
	Sentinel->CurrentHP = 1.0f;
	ApplyRidgefireDamageToSentinel(Sentinel, 42.0f);
}

void AShooterCharacter::RunRidgefireSecondaryWeaponSmokeTest()
{
	AShooterPlayerController* PlayerController = Cast<AShooterPlayerController>(GetController());
	if (!HasAuthority() || !PlayerController || !bRidgefireWeaponActive || bRidgefireSecondaryEquipped || IsDead()
		|| bRidgefireTriggerHeld || bRidgefireReloading || GetWorldTimerManager().IsTimerActive(RidgefireFireTimer)
		|| GetWorldTimerManager().IsTimerActive(RidgefireReloadTimer) || GetWorldTimerManager().IsTimerActive(RidgefireBeamTimer))
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SECONDARY WEAPON SMOKE SKIP: requires an idle authoritative primary weapon."));
		return;
	}

	const FRidgefireClientRunState& RunState = PlayerController->GetRidgefireRunState();
	const int32 OfferIndex = RunState.SecondaryWeaponOfferIndex;
	if (RunState.bRunOver || RunState.bArmoryOpen || RunState.Wave < 1 || !RunState.WeaponOffers.IsValidIndex(OfferIndex))
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SECONDARY WEAPON SMOKE SKIP: no active selected secondary offer."));
		return;
	}

	const FRidgefireWeaponRoll SelectedOffer = RunState.WeaponOffers[OfferIndex];
	if (SelectedOffer.MagazineSize < 2)
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SECONDARY WEAPON SMOKE SKIP: selected offer cannot provide a distinct magazine count."));
		return;
	}

	const FRidgefireWeaponRoll OriginalWeapon = RidgefireWeapon;
	const int32 OriginalAmmo = RidgefireAmmo;
	const int32 OriginalReserveAmmo = RidgefireReserveAmmo;
	const FRidgefireWeaponRoll OriginalStoredPrimaryWeapon = RidgefireStoredPrimaryWeapon;
	const int32 OriginalStoredPrimaryAmmo = RidgefireStoredPrimaryAmmo;
	const int32 OriginalStoredPrimaryReserveAmmo = RidgefireStoredPrimaryReserveAmmo;
	const int32 OriginalSecondaryOfferIndex = RidgefireSecondaryOfferIndex;
	const int32 OriginalSecondaryAmmo = RidgefireSecondaryAmmo;
	const int32 OriginalSecondaryReserveAmmo = RidgefireSecondaryReserveAmmo;
	const bool OriginalSecondaryEquipped = bRidgefireSecondaryEquipped;
	const float OriginalNextFireTime = RidgefireNextFireTime;
	const float OriginalLastShotTime = RidgefireLastShotTime;
	const float OriginalLastHitTime = RidgefireLastHitTime;
	const float OriginalLastKillTime = RidgefireLastKillTime;
	const bool OriginalBonusAfterReload = bBonusAfterReload;
	const int32 ExpectedSecondaryAmmo = OriginalAmmo == SelectedOffer.MagazineSize
		? SelectedOffer.MagazineSize - 1
		: SelectedOffer.MagazineSize;
	const int32 ExpectedSecondaryReserveAmmo = FMath::Max(0, SelectedOffer.ReserveAmmo - 1);

	RidgefireSecondaryOfferIndex = OfferIndex;
	RidgefireSecondaryAmmo = ExpectedSecondaryAmmo;
	RidgefireSecondaryReserveAmmo = ExpectedSecondaryReserveAmmo;
	ToggleRidgefireSecondaryWeapon();

	const bool bSecondaryWeaponEquipped = bRidgefireSecondaryEquipped
		&& RidgefireWeapon.Name == SelectedOffer.Name
		&& RidgefireWeapon.Mode == SelectedOffer.Mode
		&& RidgefireWeapon.Payload == SelectedOffer.Payload
		&& RidgefireWeapon.Condition == SelectedOffer.Condition
		&& RidgefireAmmo == ExpectedSecondaryAmmo
		&& RidgefireAmmo != OriginalAmmo
		&& RidgefireReserveAmmo == ExpectedSecondaryReserveAmmo;

	bool bShotVerified = false;
	bool bShotSkipped = false;
	bool bShotAttempted = false;
	FString ShotSkipReason;
	int32 ShotAmmoBefore = INDEX_NONE;
	int32 ShotAmmoAfter = INDEX_NONE;
	float ShotTargetHealthBefore = -1.0f;
	float ShotTargetHealthAfter = -1.0f;
	APlayerController* AimController = Cast<APlayerController>(GetController());
	UCameraComponent* Camera = GetFirstPersonCameraComponent();
	const ARidgefireGameMode* ActiveGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ARidgefireGameMode>() : nullptr;
	const FString SecondaryWeaponNameAfterEquip = RidgefireWeapon.Name;
	const int32 SecondaryAmmoAfterEquip = RidgefireAmmo;
	const int32 SecondaryReserveAfterEquip = RidgefireReserveAmmo;
	if (bSecondaryWeaponEquipped && RidgefireWeapon.Mode == ERidgefireFireMode::Auto
		&& RidgefireWeapon.Payload == ERidgefirePayload::Plain && AimController && Camera && RidgefireAmmo > 0
		&& ActiveGameMode && !ActiveGameMode->IsArmoryOpen() && !ActiveGameMode->IsRunOver())
	{
		TArray<AActor*> NPCActors;
		UGameplayStatics::GetAllActorsOfClass(this, AShooterNPC::StaticClass(), NPCActors);
		AShooterNPC* Target = nullptr;
		float ClosestDistanceSquared = FMath::Square(FMath::Min(MaxAimDistance, 1800.0f));
		for (AActor* Actor : NPCActors)
		{
			AShooterNPC* Candidate = Cast<AShooterNPC>(Actor);
			if (!Candidate || !Candidate->HasAuthority() || Candidate->CurrentHP <= 0.0f || Candidate->ActorHasTag(TEXT("Dead")))
			{
				continue;
			}
			const float DistanceSquared = FVector::DistSquared(GetActorLocation(), Candidate->GetActorLocation());
			if (DistanceSquared < ClosestDistanceSquared)
			{
				Target = Candidate;
				ClosestDistanceSquared = DistanceSquared;
			}
		}

		if (Target)
		{
			const FTransform OriginalTargetTransform = Target->GetActorTransform();
			const float OriginalTargetHealth = Target->CurrentHP;
			const FVector AimStart = Camera->GetComponentLocation();
			const FVector TemporaryTargetLocation = AimStart + Camera->GetForwardVector() * 650.0f;
			const bool bTargetRepositioned = Target->SetActorLocation(
				TemporaryTargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
			if (bTargetRepositioned)
			{
				const FVector AimEnd = AimStart + Camera->GetForwardVector() * MaxAimDistance;
				const FVector MuzzleStart = GetRidgefireMuzzleWorldLocation();
				FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(RidgefireSecondarySmoke), true, this);
				FHitResult AimHit;
				const bool bAimTargetHit = GetWorld()->LineTraceSingleByChannel(AimHit, AimStart, AimEnd, ECC_Visibility, TraceParams)
					&& AimHit.GetActor() == Target;
				const FVector ShotDirection = ((bAimTargetHit ? AimHit.ImpactPoint : AimEnd) - MuzzleStart).GetSafeNormal();
				FHitResult MuzzleHit;
				const bool bMuzzleTargetHit = GetWorld()->LineTraceSingleByChannel(
					MuzzleHit, MuzzleStart, MuzzleStart + ShotDirection * MaxAimDistance, ECC_Visibility, TraceParams)
					&& MuzzleHit.GetActor() == Target;

				if (bAimTargetHit && bMuzzleTargetHit && Target->HasAuthority())
				{
					ShotAmmoBefore = RidgefireAmmo;
					ShotTargetHealthBefore = 50000.0f;
					Target->CurrentHP = 50000.0f;
					bShotAttempted = true;
					DoStartFiring();
					DoStopFiring();
					ShotAmmoAfter = RidgefireAmmo;
					ShotTargetHealthAfter = Target->CurrentHP;
					bShotVerified = RidgefireAmmo == ShotAmmoBefore - 1 && Target->CurrentHP > 0.0f && Target->CurrentHP < 50000.0f;
				}
				else
				{
					bShotSkipped = true;
					ShotSkipReason = TEXT("repositioned target was not reached by both camera and muzzle traces");
				}
			}
			else
			{
				bShotSkipped = true;
				ShotSkipReason = TEXT("nearby living NPC could not be temporarily repositioned for the fixture");
			}

			Target->CurrentHP = OriginalTargetHealth;
			Target->SetActorTransform(OriginalTargetTransform, false, nullptr, ETeleportType::TeleportPhysics);
		}
	else
	{
		bShotSkipped = true;
		ShotSkipReason = bSecondaryWeaponEquipped
			? TEXT("no nearby living target/camera or the aim/muzzle trace was unsafe")
			: TEXT("secondary offer did not equip, so no shot was attempted");
	}
	}
	else
	{
		bShotSkipped = true;
		ShotSkipReason = TEXT("selected offer is not safe direct fire or the game is not in active play");
	}

	const FString SecondaryWeaponAfterShot = RidgefireWeapon.Name;
	const int32 SecondaryAmmoAfterShot = bRidgefireSecondaryEquipped ? RidgefireAmmo : INDEX_NONE;
	const int32 SecondaryReserveAfterShot = bRidgefireSecondaryEquipped ? RidgefireReserveAmmo : INDEX_NONE;
	bool bPrimaryRestored = false;
	FString PrimaryWeaponAfterRestore;
	int32 PrimaryAmmoAfterRestore = INDEX_NONE;
	int32 PrimaryReserveAfterRestore = INDEX_NONE;
	bool bSecondaryAmmoRetained = false;
	FString SecondaryWeaponAfterReturn;
	int32 SecondaryAmmoAfterReturn = INDEX_NONE;
	int32 SecondaryReserveAfterReturn = INDEX_NONE;
	if (bRidgefireSecondaryEquipped)
	{
		ToggleRidgefireSecondaryWeapon();
		bPrimaryRestored = !bRidgefireSecondaryEquipped
			&& RidgefireWeapon.Name == OriginalWeapon.Name
			&& RidgefireWeapon.Mode == OriginalWeapon.Mode
			&& RidgefireWeapon.Payload == OriginalWeapon.Payload
			&& RidgefireWeapon.Condition == OriginalWeapon.Condition
			&& RidgefireAmmo == OriginalAmmo
			&& RidgefireReserveAmmo == OriginalReserveAmmo;
		PrimaryWeaponAfterRestore = RidgefireWeapon.Name;
		PrimaryAmmoAfterRestore = RidgefireAmmo;
		PrimaryReserveAfterRestore = RidgefireReserveAmmo;
		if (bPrimaryRestored)
		{
			ToggleRidgefireSecondaryWeapon();
			SecondaryWeaponAfterReturn = RidgefireWeapon.Name;
			SecondaryAmmoAfterReturn = RidgefireAmmo;
			SecondaryReserveAfterReturn = RidgefireReserveAmmo;
			bSecondaryAmmoRetained = bRidgefireSecondaryEquipped
				&& RidgefireWeapon.Name == SecondaryWeaponAfterShot
				&& RidgefireAmmo == SecondaryAmmoAfterShot
				&& RidgefireReserveAmmo == SecondaryReserveAfterShot
				&& (bShotSkipped ? RidgefireAmmo == ExpectedSecondaryAmmo : true);
		}
	}
	EquipRidgefireWeapon(OriginalWeapon, OriginalAmmo, OriginalReserveAmmo);
	RidgefireStoredPrimaryWeapon = OriginalStoredPrimaryWeapon;
	RidgefireStoredPrimaryAmmo = OriginalStoredPrimaryAmmo;
	RidgefireStoredPrimaryReserveAmmo = OriginalStoredPrimaryReserveAmmo;
	RidgefireSecondaryOfferIndex = OriginalSecondaryOfferIndex;
	RidgefireSecondaryAmmo = OriginalSecondaryAmmo;
	RidgefireSecondaryReserveAmmo = OriginalSecondaryReserveAmmo;
	bRidgefireSecondaryEquipped = OriginalSecondaryEquipped;
	RidgefireNextFireTime = OriginalNextFireTime;
	RidgefireLastShotTime = OriginalLastShotTime;
	RidgefireLastHitTime = OriginalLastHitTime;
	RidgefireLastKillTime = OriginalLastKillTime;
	bBonusAfterReload = OriginalBonusAfterReload;

	if (bSecondaryWeaponEquipped && bPrimaryRestored && bSecondaryAmmoRetained && bShotVerified)
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP SMOKE PASS: secondary shot consumed ammo and damaged a living sentry; primary and spent secondary ammo/reserve both restored."));
	}
	else if (bShotSkipped && bSecondaryWeaponEquipped && bPrimaryRestored && bSecondaryAmmoRetained)
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP SMOKE SKIP: secondary ammo/reserve round-trip passed without decrement; real-shot check skipped because %s."), *ShotSkipReason);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE COOP SMOKE FAIL: secondary equip, real shot/damage, or primary ammo round-trip assertion failed."));
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE COOP SMOKE SECONDARY DIAGNOSTIC: secondaryEquipped=%s primaryRestored=%s secondaryAmmoRetained=%s shotVerified=%s shotSkipped=%s shotAttempted=%s reason=\"%s\" prePrimary=\"%s\" preAmmo=%d preReserve=%d secondary=\"%s\" secondaryAmmo=%d secondaryReserve=%d shotAmmo=%d->%d targetHealth=%.1f->%.1f restoredPrimary=\"%s\" restoredAmmo=%d restoredReserve=%d returnedSecondary=\"%s\" returnedAmmo=%d returnedReserve=%d"),
			bSecondaryWeaponEquipped ? TEXT("true") : TEXT("false"),
			bPrimaryRestored ? TEXT("true") : TEXT("false"),
			bSecondaryAmmoRetained ? TEXT("true") : TEXT("false"),
			bShotVerified ? TEXT("true") : TEXT("false"),
			bShotSkipped ? TEXT("true") : TEXT("false"),
			bShotAttempted ? TEXT("true") : TEXT("false"),
			ShotSkipReason.IsEmpty() ? TEXT("none") : *ShotSkipReason,
			*OriginalWeapon.Name, OriginalAmmo, OriginalReserveAmmo,
			*SecondaryWeaponNameAfterEquip, SecondaryAmmoAfterEquip, SecondaryReserveAfterEquip,
			ShotAmmoBefore, ShotAmmoAfter, ShotTargetHealthBefore, ShotTargetHealthAfter,
			*PrimaryWeaponAfterRestore, PrimaryAmmoAfterRestore, PrimaryReserveAfterRestore,
			*SecondaryWeaponAfterReturn, SecondaryAmmoAfterReturn, SecondaryReserveAfterReturn);
	}
}
#endif

void AShooterCharacter::PlayPhasmaShot(const FVector& Location)
{
	constexpr int32 SampleRate = 44100;
	constexpr float Duration = 0.98f;
	const int32 SampleCount = static_cast<int32>(SampleRate * Duration);
	TArray<int16> Samples;
	Samples.SetNumUninitialized(SampleCount);

	for (int32 Index = 0; Index < SampleCount; ++Index)
	{
		const float Time = static_cast<float>(Index) / SampleRate;
		float Value = 0.0f;
		if (Time < 0.52f)
		{
			const float Progress = Time / 0.52f;
			const float Frequency = FMath::Lerp(390.0f, 58.0f, Progress);
			Value += FMath::Sin(2.0f * PI * Frequency * Time) * 0.16f * FMath::Exp(-Time * 4.0f);
		}
		if (Time > 0.22f)
		{
			const float ImpactTime = Time - 0.22f;
			const float ImpactFrequency = FMath::Lerp(102.0f, 31.0f, FMath::Clamp(ImpactTime / 0.72f, 0.0f, 1.0f));
			Value += FMath::Sin(2.0f * PI * ImpactFrequency * ImpactTime) * 0.36f * FMath::Exp(-ImpactTime * 3.8f);
		}
		if (Time > 0.29f)
		{
			const float RingTime = Time - 0.29f;
			const float RingFrequency = FMath::Lerp(720.0f, 190.0f, FMath::Clamp(RingTime / 0.55f, 0.0f, 1.0f));
			const float Ring = FMath::Sin(2.0f * PI * RingFrequency * RingTime);
			Value += Ring * 0.075f * FMath::Exp(-RingTime * 3.1f);
			if (RingTime > 0.12f)
			{
				Value += Ring * 0.028f * FMath::Exp(-(RingTime - 0.12f) * 4.0f);
			}
		}
		const float NoiseEnvelope = Time > 0.21f && Time < 0.48f ? FMath::Exp(-(Time - 0.21f) * 14.0f) : 0.0f;
		Value += FMath::FRandRange(-1.0f, 1.0f) * NoiseEnvelope * 0.035f;
		Samples[Index] = static_cast<int16>(FMath::Clamp(Value, -1.0f, 1.0f) * 32767.0f);
	}

	URidgefireSynthWave* Sound = NewObject<URidgefireSynthWave>(GetTransientPackage());
	Sound->Duration = Duration;
	Sound->QueueAudio(reinterpret_cast<const uint8*>(Samples.GetData()), Samples.Num() * sizeof(int16));
	if (!PhasmaShotAttenuation)
	{
		PhasmaShotAttenuation = NewObject<USoundAttenuation>(this);
		if (PhasmaShotAttenuation)
		{
			FSoundAttenuationSettings& Settings = PhasmaShotAttenuation->Attenuation;
			Settings.bAttenuate = true;
			Settings.bSpatialize = true;
			Settings.AttenuationShape = EAttenuationShape::Sphere;
			Settings.AttenuationShapeExtents = FVector::ZeroVector;
			Settings.DistanceAlgorithm = EAttenuationDistanceModel::Logarithmic;
			Settings.FalloffDistance = 2200.0f;
			Settings.NonSpatializedRadiusStart = 0.0f;
			Settings.NonSpatializedRadiusEnd = 0.0f;
		}
	}
	if (PhasmaShotAttenuation)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Sound, Location, 0.9f, 1.0f, 0.0f, PhasmaShotAttenuation);
	}
}

void AShooterCharacter::DoAim(float Yaw, float Pitch)
{
	// only route inputs if the character is not dead
	if (!IsDead() && FMath::IsFinite(Yaw) && FMath::IsFinite(Pitch))
	{
		Super::DoAim(Yaw, Pitch);
	}
}

void AShooterCharacter::DoMove(float Right, float Forward)
{
	// only route inputs if the character is not dead
	if (!IsDead() && FMath::IsFinite(Right) && FMath::IsFinite(Forward))
	{
		Super::DoMove(Right, Forward);
	}
}

void AShooterCharacter::DoJumpStart()
{
	// only route inputs if the character is not dead
	if (!IsDead())
	{
		Super::DoJumpStart();
	}
}

void AShooterCharacter::DoJumpEnd()
{
	// only route inputs if the character is not dead
	if (!IsDead())
	{
		Super::DoJumpEnd();
	}
}

void AShooterCharacter::DoStartFiring()
{
	if (bRidgefireWeaponActive && !IsDead())
	{
		if (!HasAuthority())
		{
			ServerRequestFire();
			return;
		}
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE: Fire input received for %s (%d rounds in magazine)."), *RidgefireWeapon.Name, RidgefireAmmo);
		bRidgefireTriggerHeld = true;
		FireRidgefireWeapon();
		return;
	}

	// fire the current weapon
	if (CurrentWeapon && !IsDead())
	{
		CurrentWeapon->StartFiring();
	}
}

void AShooterCharacter::DoStopFiring()
{
	if (bRidgefireWeaponActive)
	{
		if (!HasAuthority())
		{
			ServerRequestStopFiring();
			return;
		}
		bRidgefireTriggerHeld = false;
		GetWorldTimerManager().ClearTimer(RidgefireFireTimer);
		return;
	}

	// stop firing the current weapon
	if (CurrentWeapon && !IsDead())
	{
		CurrentWeapon->StopFiring();
	}
}

void AShooterCharacter::DoSwitchWeapon()
{
	if (IsDead() || IsCarryingRidgefirePlayer())
	{
		return;
	}
	if (bRidgefireWeaponActive)
	{
		// The template swap action shares Shift with sprint. IRON SUN selects its
		// arsenal through the numbered controls and secondary toggle instead.
		return;
	}
	if (OwnedWeapons.Num() > 1 && !IsDead())
	{
		if (IsValid(CurrentWeapon))
		{
			CurrentWeapon->DeactivateWeapon();
		}

		const int32 CurrentIndex = OwnedWeapons.IndexOfByKey(CurrentWeapon);
		for (int32 Offset = 1; Offset <= OwnedWeapons.Num(); ++Offset)
		{
			const int32 WeaponIndex = (CurrentIndex + Offset) % OwnedWeapons.Num();
			if (IsValid(OwnedWeapons[WeaponIndex]))
			{
				CurrentWeapon = OwnedWeapons[WeaponIndex];
				CurrentWeapon->ActivateWeapon(PlayerTag);
				return;
			}
		}
		CurrentWeapon = nullptr;
	}
}

void AShooterCharacter::ServerRequestFire_Implementation()
{
	if (!bRidgefireWeaponActive || IsDead())
	{
		return;
	}
#if WITH_EDITOR
	if (FParse::Param(FCommandLine::Get(), TEXT("RidgefireCoopCombatSmoke")))
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP SERVER FIRE_RPC pawn=%s ammo=%d"), *GetNameSafe(this), RidgefireAmmo);
	}
#endif
	DoStartFiring();
}

void AShooterCharacter::ServerRequestStopFiring_Implementation()
{
	DoStopFiring();
}

void AShooterCharacter::ServerRequestReload_Implementation()
{
	ReloadRidgefireWeapon();
}

void AShooterCharacter::ServerRequestCycleWeapon_Implementation()
{
	if (bRidgefireSecondaryEquipped || IsDead() || IsCarryingRidgefirePlayer())
	{
		return;
	}
	if (ARidgefireGameMode* GameMode = GetWorld()->GetAuthGameMode<ARidgefireGameMode>())
	{
		GameMode->CycleWeaponOffer(Cast<APlayerController>(GetController()));
	}
}

void AShooterCharacter::ServerRequestToggleSecondaryWeapon_Implementation()
{
	ToggleRidgefireSecondaryWeapon();
}

void AShooterCharacter::ServerRequestIonSurge_Implementation()
{
	ActivateIonSurge();
}

void AShooterCharacter::ServerRequestFieldPatch_Implementation()
{
	UseFieldPatch();
}

void AShooterCharacter::AttachWeaponMeshes(AShooterWeapon* Weapon)
{
	if (!IsValid(Weapon) || !IsValid(Weapon->GetFirstPersonMesh()) || !IsValid(Weapon->GetThirdPersonMesh()) || !IsValid(GetFirstPersonMesh()) || !IsValid(GetMesh()))
	{
		return;
	}

	const FAttachmentTransformRules AttachmentRule(EAttachmentRule::SnapToTarget, false);

	// attach the weapon actor
	Weapon->AttachToActor(this, AttachmentRule);

	// attach the weapon meshes
	Weapon->GetFirstPersonMesh()->AttachToComponent(GetFirstPersonMesh(), AttachmentRule, FirstPersonWeaponSocket);
	Weapon->GetThirdPersonMesh()->AttachToComponent(GetMesh(), AttachmentRule, FirstPersonWeaponSocket);
	
}

void AShooterCharacter::PlayFiringMontage(UAnimMontage* Montage)
{
	// stub
}

void AShooterCharacter::AddWeaponRecoil(float Recoil)
{
	auto* OwnerController = Cast<APlayerController>(GetController());
	if (!FMath::IsFinite(Recoil) || IsDead() || !IsLocallyControlled() || !OwnerController || !OwnerController->IsLocalController() || OwnerController->IsPaused() || OwnerController->IsLookInputIgnored()) return;
	// Physical kick always raises aim; mouse/controller inversion applies only to look input.
	// UpdateRotation still applies the normal camera limits.
	OwnerController->RotationInput.Pitch += FMath::Abs(Recoil) * GetRidgefireRecoilScale();
}

void AShooterCharacter::UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize)
{
	OnBulletCountUpdated.Broadcast(MagazineSize, CurrentAmmo);
}

FVector AShooterCharacter::GetWeaponTargetLocation()
{
	UCameraComponent* Camera = GetFirstPersonCameraComponent();
	UWorld* World = GetWorld();
	if (!Camera || !World)
	{
		return GetActorLocation() + GetActorForwardVector() * MaxAimDistance;
	}

	// trace ahead from the camera viewpoint
	FHitResult OutHit;

	const FVector Start = Camera->GetComponentLocation();
	const FVector End = Start + (Camera->GetForwardVector() * MaxAimDistance);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	World->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, QueryParams);

	// return either the impact point or the trace end
	return OutHit.bBlockingHit ? OutHit.ImpactPoint : OutHit.TraceEnd;
}

void AShooterCharacter::AddWeaponClass(const TSubclassOf<AShooterWeapon>& WeaponClass)
{
	if (!WeaponClass || !GetWorld() || bRidgefireDowned || IsCarryingRidgefirePlayer())
	{
		return;
	}

	// do we already own this weapon?
	AShooterWeapon* OwnedWeapon = FindWeaponOfType(WeaponClass);

	if (!OwnedWeapon)
	{
		// spawn the new weapon
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;

		AShooterWeapon* AddedWeapon = GetWorld()->SpawnActor<AShooterWeapon>(WeaponClass, GetActorTransform(), SpawnParams);

		if (AddedWeapon)
		{
			// add the weapon to the owned list
			OwnedWeapons.Add(AddedWeapon);

			// if we have an existing weapon, deactivate it
			if (CurrentWeapon)
			{
				CurrentWeapon->DeactivateWeapon();
			}

			// switch to the new weapon
			CurrentWeapon = AddedWeapon;
			CurrentWeapon->ActivateWeapon(PlayerTag);
		}
	}
}

void AShooterCharacter::OnWeaponActivated(AShooterWeapon* Weapon)
{
	if (!IsValid(Weapon) || bRidgefireDowned || IsCarryingRidgefirePlayer())
	{
		return;
	}

	// update the bullet counter
	OnBulletCountUpdated.Broadcast(Weapon->GetMagazineSize(), Weapon->GetBulletCount());

	// set the character mesh AnimInstances
	GetFirstPersonMesh()->SetAnimInstanceClass(Weapon->GetFirstPersonAnimInstanceClass());
	GetMesh()->SetAnimInstanceClass(Weapon->GetThirdPersonAnimInstanceClass());
}

void AShooterCharacter::OnWeaponDeactivated(AShooterWeapon* Weapon)
{
	// unused
}

void AShooterCharacter::OnSemiWeaponRefire()
{
	// unused
}

AShooterWeapon* AShooterCharacter::FindWeaponOfType(TSubclassOf<AShooterWeapon> WeaponClass) const
{
	// check each owned weapon
	for (AShooterWeapon* Weapon : OwnedWeapons)
	{
		if (IsValid(Weapon) && Weapon->IsA(WeaponClass))
		{
			return Weapon;
		}
	}

	// weapon not found
	return nullptr;

}

void AShooterCharacter::Die()
{
	if (!HasAuthority())
	{
		return;
	}
	bRidgefireTriggerHeld = false;
	bRidgefireReloading = false;
	GetWorldTimerManager().ClearTimer(RidgefireFireTimer);
	GetWorldTimerManager().ClearTimer(RidgefireReloadTimer);

	if (ARidgefireGameMode* RidgefireMode = GetWorld()->GetAuthGameMode<ARidgefireGameMode>())
	{
		ClearRidgefireCarryOwnership();
		bRidgefireDowned = true;
		HideRidgefireBeam();
		if (IsValid(CurrentWeapon))
		{
			CurrentWeapon->DeactivateWeapon();
		}
		ApplyRidgefireRescuePresentation();
		ForceNetUpdate();
		RidgefireMode->HandlePlayerDefeated();
		return;
	}

	// deactivate the weapon
	if (IsValid(CurrentWeapon))
	{
		CurrentWeapon->DeactivateWeapon();
	}

	// increment the team score
	if (AShooterGameMode* GM = Cast<AShooterGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->IncrementTeamScore(TeamByte);
	}

	// grant the death tag to the character
	Tags.Add(DeathTag);
		
	// stop character movement
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	// disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// disable controls
	DisableInput(nullptr);

	// reset the bullet counter UI
	OnBulletCountUpdated.Broadcast(0, 0);

	// call the BP handler
	BP_OnDeath();

	if (!GetWorld()->GetAuthGameMode<ARidgefireGameMode>())
	{
		GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &AShooterCharacter::OnRespawn, RespawnTime, false);
	}
}

void AShooterCharacter::OnRespawn()
{
	// destroy the character to force the PC to respawn
	Destroy();
}

bool AShooterCharacter::IsDead() const
{
	// the character is dead if their current HP drops to zero
	return CurrentHP <= 0.0f;
}

void AShooterCharacter::SetTeam(uint8 Team)
{
	TeamByte = Team;
}

FRidgefirePerkModifiers AShooterCharacter::GetPersonalPerkModifiers() const
{
    const AShooterPlayerController* Player = Cast<AShooterPlayerController>(GetController());
    FRidgefirePerkModifiers Modifiers = Player ? RidgefireProgression::GetModifiers(Player->GetPersonalProgression()) : FRidgefirePerkModifiers();
    if (Player)
    {
        const uint8 TeamPerks = Player->GetRidgefireRunState().TeamArenaPerks;
        if (TeamPerks & 1) Modifiers.MoveSpeed *= 1.05f;
        if (TeamPerks & 2) Modifiers.OutgoingDamage *= 1.10f;
        if (TeamPerks & 4) Modifiers.IncomingDamage *= 0.95f;
        if (TeamPerks & 8) Modifiers.ReloadTime *= 0.90f;
    }
    return Modifiers;
}

void AShooterCharacter::ApplyPersonalProgressionStats()
{
    if (!PersonalBaseMaxHP) PersonalBaseMaxHP = MaxHP;
    if (!PersonalBaseJumpSpeed) PersonalBaseJumpSpeed = GetCharacterMovement()->JumpZVelocity;
    const FRidgefirePerkModifiers Modifiers = GetPersonalPerkModifiers();
    if (HasAuthority())
    {
        const float HealthFraction = GetHealthRatio();
        MaxHP = PersonalBaseMaxHP * Modifiers.MaxHealth;
        CurrentHP = HealthFraction * MaxHP;
        OnDamaged.Broadcast(GetHealthRatio());
    }
    GetCharacterMovement()->JumpZVelocity = PersonalBaseJumpSpeed * Modifiers.JumpSpeed;
}

bool AShooterCharacter::HasPersonalPerk(int32 Node) const
{
 const auto* Player = Cast<AShooterPlayerController>(GetController());
 if (!Player) return false;
 const auto& Profile = Player->GetPersonalProgression();
 return Profile.FirstPerk == Node || Profile.SecondPerk == Node;
}

void AShooterCharacter::StartRidgefireCrouch()
{
 auto* M=CastChecked<URidgefireCharacterMovement>(GetCharacterMovement());
 if(!M->CanUseCombatMovement()) return;
 // Capture moving intent before cancelling sprint; velocity itself is never discarded.
 M->RequestCrouch(true);
 if(auto* PC=Cast<AShooterPlayerController>(GetController())) PC->CancelRidgefireSprint();
 Crouch();
}
void AShooterCharacter::StopRidgefireCrouch()
{
 CastChecked<URidgefireCharacterMovement>(GetCharacterMovement())->RequestCrouch(false);
 UnCrouch();
}
void AShooterCharacter::SetRidgefireAiming(bool Held){CastChecked<URidgefireCharacterMovement>(GetCharacterMovement())->RequestAim(Held);}
bool AShooterCharacter::IsRidgefireSliding() const {return CastChecked<URidgefireCharacterMovement>(GetCharacterMovement())->bSliding;}
bool AShooterCharacter::IsRidgefireAiming() const {return CastChecked<URidgefireCharacterMovement>(GetCharacterMovement())->bAimRequested;}
float AShooterCharacter::GetRidgefireShotSpread() const
{
#if WITH_DEV_AUTOMATION_TESTS
 if(bRidgefireExactRayFixture) return 0;
#endif
 const float Base=RidgefireWeaponProgression::SpreadDegrees(RidgefireWeapon);
 if(IsRidgefireSliding()) return Base+(IsRidgefireAiming() ? 3.8f : 2.6f);
 return Base*(IsRidgefireAiming() ? 0.45f : 1.0f)*(bIsCrouched ? 0.8f : 1.0f);
}
void AShooterCharacter::Tick(float DT)
{
 Super::Tick(DT);
 if(IsLocallyControlled())
 {
  auto* Camera=GetFirstPersonCameraComponent();
  if(RidgefireNormalFOV<=0) RidgefireNormalFOV=Camera->FieldOfView;
  const float Desired=IsRidgefireAiming() ? RidgefireNormalFOV-12.0f : RidgefireNormalFOV;
  Camera->SetFieldOfView(FMath::FInterpTo(Camera->FieldOfView,Desired,DT,12.0f));
  RidgefireVisualAim=FMath::FInterpTo(RidgefireVisualAim,IsRidgefireAiming() ? 1.0f : 0.0f,DT,14.0f);
  RidgefireVisualKick=FMath::FInterpTo(RidgefireVisualKick,0.0f,DT,18.0f);
  for(UStaticMeshComponent* Part:RidgefireWeaponParts)
  {
   if(!IsValid(Part) || Part->GetAttachParent()!=Camera) continue;
   const TWeakObjectPtr<UStaticMeshComponent> Key(Part);
   if(!RidgefireWeaponRestLocations.Contains(Key)) RidgefireWeaponRestLocations.Add(Key,Part->GetRelativeLocation());
   Part->SetRelativeLocation(RidgefireWeaponRestLocations[Key]+FVector(0,-28,12)*RidgefireVisualAim+FVector(-RidgefireVisualKick,0,0));
  }
 }
}
void AShooterCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
 if (!bRidgefireCrouchCameraAdjusted) RidgefireStandingCameraLocation=GetFirstPersonCameraComponent()->GetRelativeLocation();
 Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
 GetMesh()->UpdateComponentToWorld();
 GetFirstPersonMesh()->UpdateComponentToWorld();
 if (!bRidgefireCrouchCameraAdjusted && ScaledHalfHeightAdjust > 0)
 {
  UCameraComponent* Camera=GetFirstPersonCameraComponent();
  // The FP animation copies its parent's pose. Offset the camera socket child,
  // rather than its mesh, so pose copying cannot cancel or double the view shift.
  const FTransform Socket=Camera->GetAttachParent()->GetSocketTransform(Camera->GetAttachSocketName(),RTS_World);
  Camera->SetRelativeLocation(RidgefireStandingCameraLocation+Socket.InverseTransformVector(FVector(0,0,-ScaledHalfHeightAdjust)));
  bRidgefireCrouchCameraAdjusted=true;
 }
}
void AShooterCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
 Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
 // Native crouch adjusts the skeletal mesh's relative offset before its normal
 // component update. Refresh the attachment transforms before restoring view.
 GetMesh()->UpdateComponentToWorld();
 GetFirstPersonMesh()->UpdateComponentToWorld();
 if (bRidgefireCrouchCameraAdjusted) {
  GetFirstPersonCameraComponent()->SetRelativeLocation(RidgefireStandingCameraLocation);
  bRidgefireCrouchCameraAdjusted=false;
 }
}
void AShooterCharacter::NotifyRidgefireHandlingRecovery()
{
 if (!IsDead() && GetWorld()) RidgefireHandlingRecoveryAt = GetWorld()->GetTimeSeconds();
}
void AShooterCharacter::Landed(const FHitResult& Hit)
{
 const bool bHardLanding = GetVelocity().Z < -200.0f;
 Super::Landed(Hit);
 if (bHardLanding) NotifyRidgefireHandlingRecovery();
}
float AShooterCharacter::GetRidgefireHandlingRecovery() const
{
 if (!GetWorld()) return 0;
 const float Duration = HasPersonalPerk(1) ? 0.65f : 1.0f;
 return FMath::Clamp(1.0f - (GetWorld()->GetTimeSeconds()-RidgefireHandlingRecoveryAt)/Duration, 0.0f, 1.0f);
}
float AShooterCharacter::GetRidgefireRecoilScale() const
{
 // Crouch bracing is universal. Stable grip shortens post-movement settling only.
 return (IsRidgefireSliding() ? 1.25f : bIsCrouched ? 0.65f : 1.0f) * (1.0f + 0.50f * GetRidgefireHandlingRecovery());
}
void AShooterCharacter::ClientRidgefireRecoil_Implementation(float Amount)
{
 if(!IsLocallyControlled() || !FMath::IsFinite(Amount)) return;
 const float Now=GetWorld()->GetTimeSeconds();
 if(Now-RidgefireLocalShotAt>0.45f) RidgefireRecoilSequence=0;
 RidgefireLocalShotAt=Now;
 const float Build=1.0f+FMath::Min(RidgefireRecoilSequence,12)*0.035f;
 AddWeaponRecoil(FMath::Clamp(Amount,0.0f,2.0f)*Build);
 if(auto* PC=Cast<APlayerController>(GetController());PC && !IsDead() && !PC->IsLookInputIgnored() && !PC->IsPaused())
 {
  const float Family=RidgefireWeapon.Mode==ERidgefireFireMode::Burst ? 0.7f : RidgefireWeapon.Mode==ERidgefireFireMode::Auto ? 1.0f : 0.45f;
  const float Side=FMath::Sin((RidgefireRecoilSequence+1)*1.7f)*0.10f+FMath::FRandRange(-0.025f,0.025f);
  PC->RotationInput.Yaw+=Side*Amount*Family*GetRidgefireRecoilScale();
 }
 RidgefireVisualKick=FMath::Min(5.0f,RidgefireVisualKick+Amount*3.0f);
 ++RidgefireRecoilSequence;
}

void AShooterCharacter::MulticastRidgefireImpact_Implementation(FVector Location,ERidgefireFireMode Mode)
{
 if(GetNetMode()!=NM_DedicatedServer && !Location.ContainsNaN()) PlayRidgefireMechanicalSound(Location,Mode,true);
}
void AShooterCharacter::PlayRidgefireMechanicalSound(const FVector& Location,ERidgefireFireMode Mode,bool Impact)
{
 if(GetNetMode()==NM_DedicatedServer) return;
 // Original short mechanical transients. No source recording or copied audio.
 if(!PhasmaShotAttenuation)
 {
  PhasmaShotAttenuation=NewObject<USoundAttenuation>(this);
  auto& A=PhasmaShotAttenuation->Attenuation;
  A.bAttenuate=true;A.bSpatialize=true;A.AttenuationShape=EAttenuationShape::Sphere;
  A.AttenuationShapeExtents=FVector::ZeroVector;A.DistanceAlgorithm=EAttenuationDistanceModel::Logarithmic;A.FalloffDistance=2200;
 }
 constexpr int32 Rate=22050;
 const bool Heavy=Mode==ERidgefireFireMode::Pierce || Mode==ERidgefireFireMode::Mortar || Mode==ERidgefireFireMode::Scatter;
 const float Duration=Impact ? 0.10f : Heavy ? 0.28f : 0.13f;
 TArray<int16> Samples;Samples.SetNumUninitialized(FMath::RoundToInt(Rate*Duration));
 const float Tone=Impact ? 1250.0f : Mode==ERidgefireFireMode::Burst ? 260.0f : Heavy ? 95.0f : 410.0f;
 for(int32 I=0;I<Samples.Num();++I)
 {
  const float T=static_cast<float>(I)/Rate;
  const float Envelope=FMath::Exp(-T*(Impact ? 45.0f : Heavy ? 17.0f : 32.0f));
  const float V=(FMath::Sin(2*PI*Tone*T)*(Impact ? 0.11f : 0.25f)+FMath::FRandRange(-0.2f,0.2f))*Envelope;
  Samples[I]=static_cast<int16>(FMath::Clamp(V,-1.0f,1.0f)*32767);
 }
 auto* Wave=NewObject<URidgefireSynthWave>(GetTransientPackage());Wave->SetSampleRate(Rate);Wave->Duration=Duration;
 Wave->QueueAudio(reinterpret_cast<const uint8*>(Samples.GetData()),Samples.Num()*sizeof(int16));
 UGameplayStatics::PlaySoundAtLocation(this,Wave,Location,Impact ? 0.55f : 0.8f,1,0,PhasmaShotAttenuation);
}

void AShooterCharacter::FireRidgefireBurstFollowup()
{
 auto* M=CastChecked<URidgefireCharacterMovement>(GetCharacterMovement());
 const auto* Mode=GetWorld()->GetAuthGameMode<ARidgefireGameMode>();
 const auto* PC=Cast<AShooterPlayerController>(GetController());
 if(!HasAuthority() || RidgefireBurstRemaining<=0 || IsDead() || bRidgefireReloading || !Mode || Mode->IsArmoryOpen() || Mode->IsRunOver()
    || !PC || PC->GetRidgefireRunState().bPersonalArsenalOpen || RidgefireWeapon.Mode!=ERidgefireFireMode::Burst
    || (IsCarryingRidgefirePlayer() && !bRidgefireSecondaryEquipped && !CanUsePrimaryWhileCarrying()))
 {RidgefireBurstRemaining=0;GetWorldTimerManager().ClearTimer(RidgefireBurstTimer);return;}
 const FVector AimStart=GetFirstPersonCameraComponent()->GetComponentLocation();
 const FVector Start=GetRidgefireMuzzleWorldLocation();
 FCollisionQueryParams Q(SCENE_QUERY_STAT(RidgefireBurst),true,this);FHitResult Crosshair;
 const FVector Far=AimStart+GetFirstPersonCameraComponent()->GetForwardVector()*MaxAimDistance;
 const bool Aimed=GetWorld()->LineTraceSingleByChannel(Crosshair,AimStart,Far,ECC_Visibility,Q);
 const FVector Direction=FMath::VRandCone(((Aimed ? Crosshair.ImpactPoint : Far)-Start).GetSafeNormal(),FMath::DegreesToRadians(GetRidgefireShotSpread()));
 FHitResult Hit;const FVector End=Start+Direction*MaxAimDistance;
 const bool Blocked=GetWorld()->LineTraceSingleByChannel(Hit,Start,End,ECC_Visibility,Q);
 MulticastRidgefireShotEffects(Start,Blocked ? Hit.ImpactPoint : End,ERidgefireFireMode::Burst,RidgefireWeapon.bWildcard);
 const bool PriorReloadBonus=bBonusAfterReload;bBonusAfterReload=bRidgefireBurstAfterReload;
 if(Blocked) ApplyRidgefireHit(Hit.GetActor(),Hit.ImpactPoint,FVector::Distance(Start,Hit.ImpactPoint),0.72f,&Hit,Direction);
 bBonusAfterReload=PriorReloadBonus;
 ClientRidgefireRecoil(RidgefireWeaponProgression::RecoilDegrees(RidgefireWeapon)*0.72f);
 if(--RidgefireBurstRemaining==0)
 {
  GetWorldTimerManager().ClearTimer(RidgefireBurstTimer);
  if(RidgefireAmmo==0 && RidgefireReserveAmmo>0) ReloadRidgefireWeapon();
 }
}

