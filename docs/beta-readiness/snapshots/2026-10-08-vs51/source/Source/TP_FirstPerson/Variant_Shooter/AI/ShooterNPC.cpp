// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/AI/ShooterNPC.h"
#include "GameFramework/GameStateBase.h"
#include "ShooterWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "ShooterGameMode.h"
#include "RidgefireGameMode.h"
#include "RidgefireHealingTube.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Variant_Shooter/ShooterPlayerController.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
#include "Engine/DamageEvents.h"
#include "Math/RotationMatrix.h"
#include "RidgefireSynthWave.h"
#include "Sound/SoundAttenuation.h"
#include "Net/UnrealNetwork.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"

#if WITH_EDITOR
namespace
{
bool bRidgefireSentryHitSmokeDiagnosticRun = false;
bool bRidgefireSentryContactSmokeClaimed = false;
}
#endif

AShooterNPC::AShooterNPC()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
}

void AShooterNPC::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AShooterNPC,bRidgefireChampion);
    DOREPLIFETIME(AShooterNPC,bChampionGuarded);
	DOREPLIFETIME(AShooterNPC, CurrentHP);
	DOREPLIFETIME(AShooterNPC, FinisherUntil);
	DOREPLIFETIME(AShooterNPC, bIsDead);
	DOREPLIFETIME(AShooterNPC, bRidgefireSentry);
	DOREPLIFETIME(AShooterNPC, bRidgefireSprinter);
	DOREPLIFETIME(AShooterNPC, bRidgefireBrute);
	DOREPLIFETIME(AShooterNPC, bFoundryTurret);
	DOREPLIFETIME(AShooterNPC, bSentryTelegraphActive);
	DOREPLIFETIME(AShooterNPC, bSentryMeleeTelegraphActive);
	DOREPLIFETIME(AShooterNPC, bFoundryTurretCharging);
	DOREPLIFETIME(AShooterNPC, bFoundryTurretBurstActive);
	DOREPLIFETIME(AShooterNPC, bBruteRangedCharging);
	DOREPLIFETIME(AShooterNPC, bBruteBurstActive);
	DOREPLIFETIME(AShooterNPC, CurrentSentryTarget);
}

void AShooterNPC::BeginPlay()
{
	Super::BeginPlay();
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	bRidgefireSentry = GetWorld()->GetAuthGameMode<ARidgefireGameMode>() != nullptr;
	if (bFoundryTurret)
	{
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	}
	NextSentryShotTime = GetWorld()->GetTimeSeconds() + FMath::FRandRange(5.0f, 9.0f);
	NextSentryMeleeTime = GetWorld()->GetTimeSeconds() + FMath::FRandRange(5.0f, 8.0f);
	SentinelSpawnLocation = GetActorLocation();
	SentryMovementSampleLocation = GetActorLocation();
	SentryMovementSampleTime = GetWorld()->GetTimeSeconds();
	SentinelHoverPhase = FMath::FRandRange(0.0f, 2.0f * PI);
	SentinelStrafeDirection = FMath::RandBool() ? 1.0f : -1.0f;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	CreateSentinelVisuals();
	CreateHumanoidSentinelVisuals();
#if WITH_EDITOR
	if (HasAuthority() && bRidgefireSentry && FParse::Param(FCommandLine::Get(), TEXT("RidgefireSentryHitSmoke")))
	{
		RunSentryHitTraceEditorSmokeTest();
	}
#endif
	if (bRidgefireSprinter)
	{
		CreateSprinterVisuals();
	}
	if (bRidgefireBrute)
	{
		CreateBruteVisuals();
	}
	if (bFoundryTurret)
	{
		HideHumanoidSentinelVisuals();
		CreateFoundryTurretCannons();
	}
	if (GetMesh() && SentinelVisualParts.Num() > 0)
	{
		GetMesh()->SetVisibility(false, false);
	}

	// spawn the weapon
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (HasAuthority() && WeaponClass && !bRidgefireSentry)
	{
		Weapon = GetWorld()->SpawnActor<AShooterWeapon>(WeaponClass, GetActorTransform(), SpawnParams);
	}
}

void AShooterNPC::CreateSentinelVisuals()
{
	if (!GetCapsuleComponent())
	{
		return;
	}

	SentinelVisualRoot = NewObject<USceneComponent>(this, TEXT("SentinelVisualRoot"));
	if (!SentinelVisualRoot)
	{
		return;
	}
	SentinelVisualRoot->SetupAttachment(GetCapsuleComponent());
	SentinelVisualRoot->SetMobility(EComponentMobility::Movable);
	SentinelVisualRoot->SetRelativeLocation(FVector(0.0f, 0.0f, -70.0f));
	AddInstanceComponent(SentinelVisualRoot);
	SentinelVisualRoot->RegisterComponent();

	UStaticMesh* SentinelMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Variant_Shooter/Meshes/SM_Ridgefire_Sentinel.SM_Ridgefire_Sentinel"));
	UMaterialInterface* SentinelMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Variant_Shooter/Materials/M_Ridgefire_Sentinel.M_Ridgefire_Sentinel"));
	if (SentinelMesh)
	{
		UStaticMeshComponent* SentinelModel = NewObject<UStaticMeshComponent>(this, TEXT("RidgefireSentinelModel"));
		SentinelModel->SetStaticMesh(SentinelMesh);
		SentinelAuthoredModel = SentinelModel;
		SentinelModel->SetRelativeScale3D(FVector(1.15f));
		SentinelModel->SetupAttachment(SentinelVisualRoot);
		SentinelModel->SetMobility(EComponentMobility::Movable);
		SentinelModel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SentinelModel->SetGenerateOverlapEvents(false);
		SentinelModel->SetCastShadow(true);
		for (int32 MaterialIndex = 0; MaterialIndex < SentinelMesh->GetStaticMaterials().Num(); ++MaterialIndex)
		{
			UMaterialInterface* SourceMaterial = SentinelMaterial ? SentinelMaterial : SentinelMesh->GetMaterial(MaterialIndex);
			if (!SourceMaterial)
			{
				continue;
			}
			const FString SlotName = SentinelMesh->GetStaticMaterials()[MaterialIndex].MaterialSlotName.ToString();
			const bool bIonSlot = SlotName.Contains(TEXT("Ion"));
			const bool bEmberSlot = SlotName.Contains(TEXT("Ember"));
			const bool bTrimSlot = SlotName.Contains(TEXT("Trim"));
			const FLinearColor SlotColor = bIonSlot
				? FLinearColor(0.012f, 0.24f, 0.46f)
				: bEmberSlot ? FLinearColor(0.48f, 0.07f, 0.02f)
				: bTrimSlot ? FLinearColor(0.18f, 0.24f, 0.29f)
				: FLinearColor(0.015f, 0.03f, 0.045f);
			UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(SourceMaterial, this);
			if (!Material)
			{
				continue;
			}
			Material->SetVectorParameterValue(TEXT("BaseColor"), SlotColor);
			Material->SetVectorParameterValue(TEXT("EmissiveColor"), bIonSlot ? SlotColor * 0.45f : bEmberSlot ? SlotColor * 0.55f : SlotColor * 0.015f);
			Material->SetScalarParameterValue(TEXT("Metallic"), bIonSlot || bEmberSlot ? 0.15f : bTrimSlot ? 0.7f : 0.5f);
			Material->SetScalarParameterValue(TEXT("Roughness"), bIonSlot || bEmberSlot ? 0.38f : bTrimSlot ? 0.45f : 0.6f);
			SentinelModel->SetMaterial(MaterialIndex, Material);
		}
		AddInstanceComponent(SentinelModel);
		SentinelModel->RegisterComponent();
		SentinelVisualParts.Add(SentinelModel);
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("RIDGEFIRE: Authored sentinel mesh unavailable; using primitive fallback."));

	UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	UStaticMesh* CylinderMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (!SphereMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("Sentinel visuals for %s: engine sphere mesh unavailable; sphere parts skipped."), *GetName());
	}
	if (!CylinderMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("Sentinel visuals for %s: engine cylinder mesh unavailable; rings and struts skipped."), *GetName());
	}
	const FLinearColor CoreColor(0.035f, 0.055f, 0.075f, 1.0f);
	const FLinearColor IceColor(0.015f, 0.65f, 0.9f, 1.0f);
	const FLinearColor EmberColor(1.0f, 0.18f, 0.035f, 1.0f);

	AddSentinelPart(SphereMesh, FVector::ZeroVector, FVector(0.62f), FRotator::ZeroRotator, CoreColor);

	const FVector ArmDirections[] =
	{
		FVector(1.0f, 0.0f, 0.0f),
		FVector(-1.0f, 0.0f, 0.0f),
		FVector(0.0f, 1.0f, 0.0f),
		FVector(0.0f, -1.0f, 0.0f)
	};

	for (int32 ArmIndex = 0; ArmIndex < UE_ARRAY_COUNT(ArmDirections); ++ArmIndex)
	{
		const FVector& Direction = ArmDirections[ArmIndex];
		const FRotator ArmRotation = FRotationMatrix::MakeFromZ(Direction).Rotator();
		const FLinearColor PodColor = (ArmIndex % 2 == 0) ? IceColor : EmberColor;
		AddSentinelPart(CylinderMesh, Direction * 38.0f, FVector(0.085f, 0.085f, 0.55f), ArmRotation, EmberColor);
		AddSentinelPart(SphereMesh, Direction * 68.0f, FVector(0.24f), FRotator::ZeroRotator, PodColor);
	}
}

void AShooterNPC::CreateHumanoidSentinelVisuals()
{
	if (bHumanoidVisualsCreated || !SentinelVisualRoot)
	{
		return;
	}
	bHumanoidVisualsCreated = true;

	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (!Cube || !Sphere || !Cylinder)
	{
		UE_LOG(LogTemp, Warning, TEXT("RIDGEFIRE: Humanoid sentinel primitives unavailable for %s."), *GetName());
		return;
	}

	const FLinearColor Armor(0.025f, 0.055f, 0.075f, 1.0f);
	const FLinearColor Steel(0.2f, 0.29f, 0.34f, 1.0f);
	const FLinearColor Ember(0.95f, 0.16f, 0.035f, 1.0f);
	const FLinearColor Visor(1.0f, 0.28f, 0.035f, 1.0f);
	auto AddHumanoidPart = [this](UStaticMesh* PartMesh, const FVector& Position, const FVector& Scale, const FRotator& Rotation, const FLinearColor& Color)
	{
		const int32 PreviousPartCount = SentinelVisualParts.Num();
		AddSentinelPart(PartMesh, Position * 1.08f, Scale * 1.08f, Rotation, Color);
		if (SentinelVisualParts.Num() > PreviousPartCount)
		{
			SentinelHumanoidParts.Add(SentinelVisualParts.Last());
			return SentinelVisualParts.Last().Get();
		}
		return static_cast<UStaticMeshComponent*>(nullptr);
	};

	AddHumanoidPart(Cube, FVector(-3.0f, 0.0f, 69.0f), FVector(0.38f, 0.31f, 0.39f), FRotator::ZeroRotator, Armor);
	AddHumanoidPart(Cube, FVector(12.0f, 0.0f, 79.0f), FVector(0.29f, 0.32f, 0.3f), FRotator::ZeroRotator, Steel);
	AddHumanoidPart(Cube, FVector(-4.0f, 0.0f, 37.0f), FVector(0.28f, 0.27f, 0.2f), FRotator::ZeroRotator, Steel);
	AddHumanoidPart(Cylinder, FVector(0.0f, 0.0f, 99.0f), FVector(0.13f, 0.13f, 0.13f), FRotator::ZeroRotator, Ember);
	if (UStaticMeshComponent* Head = AddHumanoidPart(Sphere, FVector(1.0f, 0.0f, 122.0f), FVector(0.27f, 0.25f, 0.29f), FRotator::ZeroRotator, Armor)) Head->ComponentTags.AddUnique(TEXT("RidgefireHead"));
	if (UStaticMeshComponent* Head = AddHumanoidPart(Cube, FVector(15.0f, 0.0f, 123.0f), FVector(0.12f, 0.235f, 0.065f), FRotator::ZeroRotator, Visor)) Head->ComponentTags.AddUnique(TEXT("RidgefireHead"));
	AddHumanoidPart(Cube, FVector(1.0f, -33.0f, 91.0f), FVector(0.27f, 0.21f, 0.23f), FRotator(0.0f, 0.0f, -8.0f), Steel);
	AddHumanoidPart(Cube, FVector(1.0f, 33.0f, 91.0f), FVector(0.27f, 0.21f, 0.23f), FRotator(0.0f, 0.0f, 8.0f), Steel);
	AddHumanoidPart(Cylinder, FVector(3.0f, -39.0f, 63.0f), FVector(0.15f, 0.15f, 0.25f), FRotator(0.0f, 0.0f, -12.0f), Armor);
	AddHumanoidPart(Cylinder, FVector(3.0f, 39.0f, 63.0f), FVector(0.15f, 0.15f, 0.25f), FRotator(0.0f, 0.0f, 12.0f), Armor);
	AddHumanoidPart(Cube, FVector(10.0f, -41.0f, 34.0f), FVector(0.19f, 0.17f, 0.24f), FRotator::ZeroRotator, Steel);
	AddHumanoidPart(Cube, FVector(10.0f, 41.0f, 34.0f), FVector(0.19f, 0.17f, 0.24f), FRotator::ZeroRotator, Steel);
	AddHumanoidPart(Cube, FVector(-4.0f, -17.0f, 11.0f), FVector(0.2f, 0.16f, 0.31f), FRotator::ZeroRotator, Armor);
	AddHumanoidPart(Cube, FVector(-4.0f, 17.0f, 11.0f), FVector(0.2f, 0.16f, 0.31f), FRotator::ZeroRotator, Armor);
	AddHumanoidPart(Cylinder, FVector(-5.0f, -17.0f, -12.0f), FVector(0.15f, 0.15f, 0.2f), FRotator::ZeroRotator, Steel);
	AddHumanoidPart(Cylinder, FVector(-5.0f, 17.0f, -12.0f), FVector(0.15f, 0.15f, 0.2f), FRotator::ZeroRotator, Steel);
	AddHumanoidPart(Cube, FVector(5.0f, -18.0f, -27.0f), FVector(0.3f, 0.17f, 0.09f), FRotator::ZeroRotator, Armor);
	AddHumanoidPart(Cube, FVector(5.0f, 18.0f, -27.0f), FVector(0.3f, 0.17f, 0.09f), FRotator::ZeroRotator, Armor);

	CreateFirstArenaArmoredSentinelVisuals();

	for (UStaticMeshComponent* Part : SentinelVisualParts)
	{
		if (IsValid(Part))
		{
			Part->SetVisibility(false, true);
		}
	}
	for (UStaticMeshComponent* Part : SentinelHumanoidParts)
	{
		if (IsValid(Part))
		{
			Part->SetVisibility(true, true);
		}
	}
	if (SentinelAuthoredModel)
	{
		SentinelAuthoredModel->SetVisibility(false, true);
	}
	UpdateHumanoidSentinelCollision();
}

#if WITH_EDITOR
void AShooterNPC::RunSentryHitTraceEditorSmokeTest()
{
	if (bRidgefireSentryHitSmokeDiagnosticRun)
	{
		return;
	}
	bRidgefireSentryHitSmokeDiagnosticRun = true;

	const UCapsuleComponent* Capsule = GetCapsuleComponent();
	UWorld* World = GetWorld();
	UStaticMeshComponent* ArmorPart = nullptr;
	for (const TWeakObjectPtr<UStaticMeshComponent>& ArmorPartRef : SentryHitSmokeArmorParts)
	{
		UStaticMeshComponent* Part = ArmorPartRef.Get();
		if (!IsValid(Part) || Part->bHiddenInGame || !Part->IsVisible())
		{
			continue;
		}
		const FVector RelativeLocation = Part->GetRelativeLocation();
		if (FMath::Abs(RelativeLocation.Y) > 60.0f && RelativeLocation.Z > 95.0f && RelativeLocation.Z < 120.0f)
		{
			ArmorPart = Part;
			break;
		}
	}

	bool bAllFirstArenaArmorPartsPresent = SentryHitSmokeArmorParts.Num() == 12;
	for (const TWeakObjectPtr<UStaticMeshComponent>& ArmorPartRef : SentryHitSmokeArmorParts)
	{
		bAllFirstArenaArmorPartsPresent &= ArmorPartRef.IsValid();
	}
	if (!Capsule || !World || !bAllFirstArenaArmorPartsPresent || !IsValid(ArmorPart))
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE NPC SMOKE FAIL sentry visibility trace fixture invalid actor=%s capsule=%s first_arena_armor_parts=%d/12 selected_visible_shoulder=%s"),
			*GetName(), *GetNameSafe(Capsule), SentryHitSmokeArmorParts.Num(), *GetNameSafe(ArmorPart));
		return;
	}

	const FVector CapsuleCenter = Capsule->GetComponentLocation();
	const FVector Forward = GetActorForwardVector().GetSafeNormal2D();
	const FVector Right = GetActorRightVector().GetSafeNormal2D();
	const FVector ArmorPoint = ArmorPart->GetComponentLocation();
	const float CapsuleRadius = Capsule->GetScaledCapsuleRadius();
	const float ArmorPointOffset = FVector::Dist2D(ArmorPoint, CapsuleCenter);
	const float ArmorRayOffset = FMath::Abs(FVector::DotProduct(ArmorPoint - CapsuleCenter, Right));
	const bool bArmorPointOutsideCapsule = ArmorPointOffset > CapsuleRadius + 1.0f && ArmorRayOffset > CapsuleRadius + 1.0f;
	bSentryHitSmokeActor = bArmorPointOutsideCapsule;
	if (!bArmorPointOutsideCapsule)
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE NPC SMOKE FAIL sentry visibility trace fixture armor point is not outside runtime capsule actor=%s armor_part=%s capsule_radius=%.1f armor_point_offset=%.1f armor_ray_offset=%.1f"),
			*GetName(), *GetNameSafe(ArmorPart), CapsuleRadius, ArmorPointOffset, ArmorRayOffset);
		return;
	}

	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(RidgefireSentryHitSmoke), false);
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (*It != this && Cast<APawn>(*It))
		{
			TraceParams.AddIgnoredActor(*It);
		}
	}

	FHitResult CenterHit;
	const FVector CenterTraceExtent = Forward * (CapsuleRadius + 40.0f);
	const bool bCenterBlocked = World->LineTraceSingleByChannel(CenterHit, CapsuleCenter + CenterTraceExtent,
		CapsuleCenter - CenterTraceExtent, ECC_Visibility, TraceParams);
	const bool bCenterHitActor = bCenterBlocked && CenterHit.GetActor() == this;

	FHitResult ArmorHit;
	const FVector ArmorTraceExtent = Forward * 45.0f;
	const bool bArmorBlocked = bArmorPointOutsideCapsule && World->LineTraceSingleByChannel(ArmorHit,
		ArmorPoint + ArmorTraceExtent, ArmorPoint - ArmorTraceExtent, ECC_Visibility, TraceParams);
	UStaticMeshComponent* HitArmorPart = Cast<UStaticMeshComponent>(ArmorHit.GetComponent());
	const bool bArmorHitPart = bArmorBlocked && ArmorHit.GetActor() == this && IsValid(HitArmorPart)
		&& SentinelHumanoidParts.Contains(HitArmorPart) && HitArmorPart->IsVisible() && !HitArmorPart->bHiddenInGame
		&& FMath::Abs(FVector::DotProduct(ArmorHit.ImpactPoint - CapsuleCenter, Right)) > CapsuleRadius + 1.0f;

	float VisibleHalfWidth = 0.0f;
	const FVector AbsRight(FMath::Abs(Right.X), FMath::Abs(Right.Y), 0.0f);
	for (const UStaticMeshComponent* Part : SentinelHumanoidParts)
	{
		if (!IsValid(Part) || Part->bHiddenInGame || !Part->IsVisible())
		{
			continue;
		}
		const FBoxSphereBounds& Bounds = Part->Bounds;
		VisibleHalfWidth = FMath::Max(VisibleHalfWidth,
			FMath::Abs(FVector::DotProduct(Bounds.Origin - CapsuleCenter, Right))
			+ FVector::DotProduct(Bounds.BoxExtent, AbsRight));
	}
	const FVector EmptyPoint = CapsuleCenter + Right * (VisibleHalfWidth + 50.0f)
		+ FVector::UpVector * (ArmorPoint.Z - CapsuleCenter.Z);
	FHitResult EmptyHit;
	const FVector EmptyTraceExtent = Forward * 45.0f;
	const bool bEmptyBlocked = World->LineTraceSingleByChannel(EmptyHit, EmptyPoint + EmptyTraceExtent,
		EmptyPoint - EmptyTraceExtent, ECC_Visibility, TraceParams);
	const bool bEmptySpaceMissed = !bEmptyBlocked;

	UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE NPC SMOKE DIAGNOSTIC sentry visibility trace actor=%s center_actor=%s center_component=%s armor_part=%s armor_actor=%s armor_hit_component=%s capsule_radius=%.1f armor_point_offset=%.1f armor_ray_offset=%.1f armor_outside_capsule=%s empty_actor=%s empty_component=%s empty_blocked=%s"),
		*GetName(), *GetNameSafe(bCenterBlocked ? CenterHit.GetActor() : nullptr), *GetNameSafe(bCenterBlocked ? CenterHit.GetComponent() : nullptr),
		*GetNameSafe(ArmorPart), *GetNameSafe(bArmorBlocked ? ArmorHit.GetActor() : nullptr), *GetNameSafe(bArmorBlocked ? ArmorHit.GetComponent() : nullptr),
		CapsuleRadius, ArmorPointOffset, ArmorRayOffset, bArmorPointOutsideCapsule ? TEXT("true") : TEXT("false"),
		*GetNameSafe(bEmptyBlocked ? EmptyHit.GetActor() : nullptr), *GetNameSafe(bEmptyBlocked ? EmptyHit.GetComponent() : nullptr),
		bEmptyBlocked ? TEXT("true") : TEXT("false"));
	if (bCenterHitActor && bArmorPointOutsideCapsule && bArmorHitPart && bEmptySpaceMissed)
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE NPC SMOKE PASS sentry visible armor visibility trace hit visible armor outside capsule; center control hit actor and empty-space control missed"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE NPC SMOKE FAIL sentry visibility trace center_actor=%s armor_point_outside_capsule=%s armor_hit_visible_outside_capsule=%s empty_space_missed=%s"),
			bCenterHitActor ? TEXT("true") : TEXT("false"), bArmorPointOutsideCapsule ? TEXT("true") : TEXT("false"),
			bArmorHitPart ? TEXT("true") : TEXT("false"), bEmptySpaceMissed ? TEXT("true") : TEXT("false"));
	}
}
#endif

void AShooterNPC::CreateFirstArenaArmoredSentinelVisuals()
{
	if (bFirstArenaSentinelVisualSelectionResolved || !SentinelVisualRoot || !bRidgefireSentry)
	{
		return;
	}

	FString ArenaName;
	if (HasAuthority())
	{
		const ARidgefireGameMode* RidgefireGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ARidgefireGameMode>() : nullptr;
		ArenaName = RidgefireGameMode ? RidgefireGameMode->GetArenaDisplayName() : FString();
	}
	else if (GetWorld())
	{
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			const AShooterPlayerController* PlayerController = Cast<AShooterPlayerController>(It->Get());
			if (PlayerController && PlayerController->IsLocalController())
			{
				ArenaName = PlayerController->GetRidgefireRunState().ArenaName;
				break;
			}
		}
	}
	if (ArenaName.IsEmpty())
	{
		return;
	}

	bFirstArenaSentinelVisualSelectionResolved = true;
	if (!ArenaName.Equals(TEXT("IRON SUN ARENA"), ESearchCase::IgnoreCase))
	{
		return;
	}

	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (!Cube)
	{
		return;
	}

	const FLinearColor Chrome(0.31f, 0.43f, 0.49f, 1.0f);
	const FLinearColor DarkChrome(0.065f, 0.11f, 0.14f, 1.0f);
	const FLinearColor SignalAmber(1.0f, 0.32f, 0.045f, 1.0f);
	const int32 FirstArmorPartIndex = SentinelVisualParts.Num();
#if WITH_EDITOR
	SentryHitSmokeArmorParts.Reset();
#endif
	auto AddArmoredPart = [this, Cube](const FVector& Position, const FVector& Scale, const FRotator& Rotation, const FLinearColor& Color)
	{
		const int32 PreviousPartCount = SentinelVisualParts.Num();
		AddSentinelPart(Cube, Position * 1.08f, Scale * 1.08f, Rotation, Color);
		if (SentinelVisualParts.Num() > PreviousPartCount)
		{
			if (UStaticMeshComponent* Part = SentinelVisualParts.Last())
			{
				Part->SetVisibility(!bHumanoidVisualsHidden, true);
				SentinelHumanoidParts.Add(Part);
				#if WITH_EDITOR
				SentryHitSmokeArmorParts.Add(Part);
				#endif
			}
		}
	};
	AddArmoredPart(FVector(-4.0f, -52.0f, 98.0f), FVector(0.39f, 0.45f, 0.29f), FRotator(0.0f, 0.0f, -10.0f), Chrome);
	AddArmoredPart(FVector(-4.0f, 52.0f, 98.0f), FVector(0.39f, 0.45f, 0.29f), FRotator(0.0f, 0.0f, 10.0f), Chrome);
	AddArmoredPart(FVector(-8.0f, -67.0f, 99.0f), FVector(0.29f, 0.22f, 0.33f), FRotator(0.0f, 0.0f, -12.0f), DarkChrome);
	AddArmoredPart(FVector(-8.0f, 67.0f, 99.0f), FVector(0.29f, 0.22f, 0.33f), FRotator(0.0f, 0.0f, 12.0f), DarkChrome);
	AddArmoredPart(FVector(17.0f, 0.0f, 81.0f), FVector(0.13f, 0.27f, 0.3f), FRotator::ZeroRotator, DarkChrome);
	AddArmoredPart(FVector(24.0f, 0.0f, 81.0f), FVector(0.055f, 0.18f, 0.23f), FRotator::ZeroRotator, SignalAmber);
	AddArmoredPart(FVector(3.0f, -44.0f, 65.0f), FVector(0.21f, 0.16f, 0.28f), FRotator(0.0f, 0.0f, -8.0f), DarkChrome);
	AddArmoredPart(FVector(3.0f, 44.0f, 65.0f), FVector(0.21f, 0.16f, 0.28f), FRotator(0.0f, 0.0f, 8.0f), DarkChrome);
	AddArmoredPart(FVector(12.0f, -19.0f, 14.0f), FVector(0.15f, 0.17f, 0.2f), FRotator(0.0f, 0.0f, -5.0f), Chrome);
	AddArmoredPart(FVector(12.0f, 19.0f, 14.0f), FVector(0.15f, 0.17f, 0.2f), FRotator(0.0f, 0.0f, 5.0f), Chrome);
	AddArmoredPart(FVector(1.0f, 0.0f, 190.0f), FVector(0.13f, 0.16f, 0.58f), FRotator::ZeroRotator, Chrome);
	AddArmoredPart(FVector(1.0f, 0.0f, 222.0f), FVector(0.18f, 0.19f, 0.24f), FRotator::ZeroRotator, SignalAmber);
	if (SentinelVisualParts.Num() > FirstArmorPartIndex) SentinelVisualParts.Last()->ComponentTags.AddUnique(TEXT("RidgefireHead"));
	UpdateHumanoidSentinelCollision();
#if WITH_EDITOR
	bool bArmorPartsAreQueryOnly = SentinelVisualParts.Num() - FirstArmorPartIndex == 12;
	float ArmorTop = -TNumericLimits<float>::Max();
	float ArmorHalfWidth = 0.0f;
	for (int32 PartIndex = FirstArmorPartIndex; PartIndex < SentinelVisualParts.Num(); ++PartIndex)
	{
		const UStaticMeshComponent* Part = SentinelVisualParts[PartIndex];
		bArmorPartsAreQueryOnly = bArmorPartsAreQueryOnly && IsValid(Part)
			&& Part->GetAttachParent() == SentinelVisualRoot.Get()
			&& Part->GetCollisionEnabled() == (bIsDead || bHumanoidVisualsHidden ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryOnly)
			&& Part->GetCollisionResponseToChannel(ECC_Visibility) == ECR_Block
			&& Part->GetCollisionResponseToChannel(ECC_Pawn) == ECR_Ignore
			&& Part->GetCollisionResponseToChannel(ECC_WorldStatic) == ECR_Ignore
			&& Part->GetCollisionResponseToChannel(ECC_WorldDynamic) == ECR_Ignore
			&& !Part->CanEverAffectNavigation();
		if (IsValid(Part))
		{
			const FVector RelativeLocation = Part->GetRelativeLocation();
			const FVector RelativeScale = Part->GetRelativeScale3D();
			ArmorTop = FMath::Max(ArmorTop, RelativeLocation.Z + RelativeScale.Z * 50.0f);
			ArmorHalfWidth = FMath::Max(ArmorHalfWidth, FMath::Abs(RelativeLocation.Y) + RelativeScale.Y * 50.0f);
		}
	}
	bArmorPartsAreQueryOnly = bArmorPartsAreQueryOnly && ArmorTop >= 245.0f && ArmorHalfWidth >= 80.0f;
	if (bArmorPartsAreQueryOnly)
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE NPC SMOKE PASS first-arena armored sentry added 12 attached visibility-query armor parts without movement/navigation collision with top %.1f and half-width %.1f: %s"), ArmorTop, ArmorHalfWidth, *GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE NPC SMOKE FAIL first-arena armored sentry visuals count/attachment/collision/shape invalid: count=%d top=%.1f half-width=%.1f %s"),
			SentinelVisualParts.Num() - FirstArmorPartIndex, ArmorTop, ArmorHalfWidth, *GetName());
	}
#endif
}

void AShooterNPC::HideHumanoidSentinelVisuals()
{
	if (bHumanoidVisualsHidden)
	{
		return;
	}
	bHumanoidVisualsHidden = true;
	for (UStaticMeshComponent* Part : SentinelHumanoidParts)
	{
		if (IsValid(Part))
		{
			Part->SetVisibility(false, true);
		}
	}
	if (IsValid(SentinelAuthoredModel))
	{
		SentinelAuthoredModel->SetVisibility(true, true);
	}
	UpdateHumanoidSentinelCollision();
}

void AShooterNPC::UpdateHumanoidSentinelCollision()
{
	// The movement capsule extends through empty gaps between the visible armor.
	// Keep its physical collision, but let weapon traces use the visible query parts.
	if (GetCapsuleComponent())
	{
		const bool bUseVisibleArmor = !bHumanoidVisualsHidden && !SentinelHumanoidParts.IsEmpty();
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility,
			bFoundryTurret || bUseVisibleArmor ? ECR_Ignore : ECR_Block);
	}
	for (UStaticMeshComponent* Part : SentinelHumanoidParts)
	{
		if (!IsValid(Part))
		{
			continue;
		}
		Part->SetCanEverAffectNavigation(false);
		Part->SetCollisionResponseToAllChannels(ECR_Ignore);
		Part->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		const bool bArmorIsHittable = !bIsDead && !bHumanoidVisualsHidden && Part->IsVisible() && !Part->bHiddenInGame;
		Part->SetCollisionEnabled(bArmorIsHittable ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}
}

void AShooterNPC::CreateFoundryTurretCannons()
{
	if (!SentinelVisualRoot || FoundryTurretRotor)
	{
		return;
	}
	UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Variant_Shooter/Materials/M_Ridgefire_Sentinel.M_Ridgefire_Sentinel"));
	if (!BaseMaterial)
	{
		BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	}
	if (!Cylinder || !Sphere)
	{
		UE_LOG(LogTemp, Warning, TEXT("RIDGEFIRE: Foundry turret cannon meshes could not be loaded for %s."), *GetName());
		return;
	}
	if (!IsValid(SentinelAuthoredModel))
	{
		AddSentinelPart(Sphere, FVector::ZeroVector, FVector(0.5f, 0.5f, 0.32f), FRotator::ZeroRotator, FLinearColor(0.025f, 0.045f, 0.065f));
		AddSentinelPart(Cylinder, FVector(0.0f, 0.0f, -19.0f), FVector(0.42f, 0.42f, 0.11f), FRotator::ZeroRotator, FLinearColor(0.22f, 0.31f, 0.36f));
	}
	FoundryTurretRotor = NewObject<USceneComponent>(this, TEXT("FoundryTurretRotor"));
	if (!FoundryTurretRotor)
	{
		return;
	}
	FoundryTurretRotor->SetupAttachment(SentinelVisualRoot);
	FoundryTurretRotor->SetMobility(EComponentMobility::Movable);
	FoundryTurretRotor->SetRelativeLocation(FVector(0.0f, 0.0f, 87.0f));
	AddInstanceComponent(FoundryTurretRotor);
	FoundryTurretRotor->RegisterComponent();

	FoundryTurretWeakPoint = NewObject<UStaticMeshComponent>(this, TEXT("FoundryTurretWeakPoint"));
	if (FoundryTurretWeakPoint)
	{
		FoundryTurretWeakPoint->SetStaticMesh(Sphere);
		FoundryTurretWeakPoint->SetupAttachment(SentinelVisualRoot);
		FoundryTurretWeakPoint->SetMobility(EComponentMobility::Movable);
		FoundryTurretWeakPoint->SetRelativeLocation(FVector(38.0f, 0.0f, 87.0f));
		FoundryTurretWeakPoint->SetRelativeScale3D(FVector(0.42f));
		FoundryTurretWeakPoint->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		FoundryTurretWeakPoint->SetCollisionObjectType(ECC_WorldDynamic);
		FoundryTurretWeakPoint->SetCollisionResponseToAllChannels(ECR_Ignore);
		FoundryTurretWeakPoint->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		FoundryTurretWeakPoint->SetGenerateOverlapEvents(false);
		FoundryTurretWeakPoint->SetCastShadow(false);
		if (BaseMaterial)
		{
			if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, this))
			{
				Material->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.78f, 0.06f, 0.012f));
				Material->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(4.4f, 0.16f, 0.025f));
				Material->SetScalarParameterValue(TEXT("Metallic"), 0.35f);
				Material->SetScalarParameterValue(TEXT("Roughness"), 0.22f);
				FoundryTurretWeakPoint->SetMaterial(0, Material);
			}
		}
		AddInstanceComponent(FoundryTurretWeakPoint);
		FoundryTurretWeakPoint->RegisterComponent();
		SentinelVisualParts.Add(FoundryTurretWeakPoint);
	}

	const FVector CannonOffsets[] =
	{
		FVector(0.0f, -15.0f, -9.0f),
		FVector(0.0f, 15.0f, -9.0f),
		FVector(0.0f, 0.0f, 18.0f)
	};
	for (const FVector& Offset : CannonOffsets)
	{
		UStaticMeshComponent* Barrel = NewObject<UStaticMeshComponent>(this);
		if (Barrel)
		{
			Barrel->SetStaticMesh(Cylinder);
			Barrel->SetupAttachment(FoundryTurretRotor);
			Barrel->SetMobility(EComponentMobility::Movable);
			Barrel->SetRelativeLocation(Offset);
			Barrel->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
			Barrel->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.96f));
			Barrel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Barrel->SetGenerateOverlapEvents(false);
			Barrel->SetCastShadow(false);
			if (BaseMaterial)
			{
				if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, this))
				{
					Material->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.32f, 0.4f, 0.46f));
					Material->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(0.025f, 0.075f, 0.1f));
					Material->SetScalarParameterValue(TEXT("Metallic"), 0.88f);
					Material->SetScalarParameterValue(TEXT("Roughness"), 0.24f);
					Barrel->SetMaterial(0, Material);
					FoundryTurretMaterials.Add(Material);
				}
			}
			AddInstanceComponent(Barrel);
			Barrel->RegisterComponent();
			SentinelVisualParts.Add(Barrel);
		}

		UStaticMeshComponent* Muzzle = NewObject<UStaticMeshComponent>(this);
		if (Muzzle)
		{
			Muzzle->SetStaticMesh(Sphere);
			Muzzle->SetupAttachment(FoundryTurretRotor);
			Muzzle->SetMobility(EComponentMobility::Movable);
			Muzzle->SetRelativeLocation(Offset + FVector(47.0f, 0.0f, 0.0f));
			Muzzle->SetRelativeScale3D(FVector(0.12f));
			Muzzle->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Muzzle->SetGenerateOverlapEvents(false);
			Muzzle->SetCastShadow(false);
			if (BaseMaterial)
			{
				if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, this))
				{
					Material->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.03f, 0.3f, 0.48f));
					Material->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(0.08f, 0.55f, 0.85f));
					Material->SetScalarParameterValue(TEXT("Metallic"), 0.3f);
					Material->SetScalarParameterValue(TEXT("Roughness"), 0.2f);
					Muzzle->SetMaterial(0, Material);
					FoundryTurretMaterials.Add(Material);
				}
			}
			AddInstanceComponent(Muzzle);
			Muzzle->RegisterComponent();
			SentinelVisualParts.Add(Muzzle);
		}
	}
}

void AShooterNPC::EnableFoundryTurret()
{
	if (!HasAuthority() || bFoundryTurret)
	{
		return;
	}
	bFoundryTurret = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	HideHumanoidSentinelVisuals();
	CreateFoundryTurretCannons();
	NextSentryShotTime = FMath::Max(NextSentryShotTime, GetWorld()->GetTimeSeconds() + 1.0f);
	ForceNetUpdate();
}

bool AShooterNPC::IsFoundryTurretWeakPoint(const UPrimitiveComponent* HitComponent) const
{
	return HasAuthority() && bFoundryTurret && IsValid(FoundryTurretWeakPoint) && HitComponent == FoundryTurretWeakPoint.Get();
}

void AShooterNPC::EnableRidgefireSprinter()
{
	if (!HasAuthority() || bRidgefireSprinter || bFoundryTurret)
	{
		return;
	}
	bRidgefireSprinter = true;
	CurrentHP = 55.0f;
	GetCharacterMovement()->MaxWalkSpeed *= 1.25f;
	SprinterDodgeDirection = FMath::RandBool() ? 1.0f : -1.0f;
	SprinterNextDodgeTime = GetWorld()->GetTimeSeconds() + FMath::FRandRange(0.35f, 0.75f);
	NextSentryMeleeTime = GetWorld()->GetTimeSeconds() + FMath::FRandRange(5.0f, 6.5f);
	if (bIsShooting)
	{
		StopShooting();
	}
	if (Weapon)
	{
		Weapon->Destroy();
		Weapon = nullptr;
	}
	CreateSprinterVisuals();
	ForceNetUpdate();
}

void AShooterNPC::EnableRidgefireBrute()
{
	if (!HasAuthority() || bRidgefireBrute || bRidgefireSprinter || bFoundryTurret)
	{
		return;
	}
	bRidgefireBrute = true;
	CurrentHP = 320.0f;
	GetCharacterMovement()->MaxWalkSpeed *= 0.58f;
	SetActorScale3D(GetActorScale3D() * 1.28f);
	BruteRangedNextAttackTime = GetWorld()->GetTimeSeconds() + FMath::FRandRange(5.0f, 7.0f);
	NextSentryMeleeTime = GetWorld()->GetTimeSeconds() + FMath::FRandRange(5.0f, 6.5f);
	if (bIsShooting)
	{
		StopShooting();
	}
	if (Weapon)
	{
		Weapon->Destroy();
		Weapon = nullptr;
	}
	CreateBruteVisuals();
	ForceNetUpdate();
}

void AShooterNPC::CreateSprinterVisuals()
{
	if (bRidgefireSprinterVisualsCreated || !SentinelVisualRoot)
	{
		return;
	}
	UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (!Cylinder || !Sphere)
	{
		UE_LOG(LogTemp, Warning, TEXT("RIDGEFIRE: Sprinter visual meshes could not be loaded for %s."), *GetName());
		return;
	}
	const FLinearColor IonCyan(0.015f, 0.68f, 0.95f, 1.0f);
	const FLinearColor HotAmber(1.0f, 0.28f, 0.045f, 1.0f);
	AddSentinelPart(Cylinder, FVector(37.0f, -20.0f, 10.0f), FVector(0.075f, 0.075f, 0.62f), FRotator(90.0f, 0.0f, 0.0f), IonCyan);
	AddSentinelPart(Cylinder, FVector(37.0f, 20.0f, 10.0f), FVector(0.075f, 0.075f, 0.62f), FRotator(90.0f, 0.0f, 0.0f), IonCyan);
	AddSentinelPart(Cylinder, FVector(48.0f, 0.0f, 17.0f), FVector(0.09f, 0.09f, 0.52f), FRotator(90.0f, 0.0f, 0.0f), HotAmber);
	AddSentinelPart(Sphere, FVector(43.0f, 0.0f, 17.0f), FVector(0.16f), FRotator::ZeroRotator, HotAmber);
	bRidgefireSprinterVisualsCreated = true;
}

void AShooterNPC::CreateBruteVisuals()
{
	if (bRidgefireBruteVisualsCreated || !SentinelVisualRoot)
	{
		return;
	}
	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (!Cube || !Sphere)
	{
		UE_LOG(LogTemp, Warning, TEXT("RIDGEFIRE: Brute visual meshes could not be loaded for %s."), *GetName());
		return;
	}
	const FLinearColor Armor(0.055f, 0.045f, 0.095f, 1.0f);
	const FLinearColor WarningViolet(0.48f, 0.045f, 0.82f, 1.0f);
	AddSentinelPart(Cube, FVector(-2.0f, -39.0f, 12.0f), FVector(0.42f, 0.32f, 0.5f), FRotator(0.0f, -12.0f, -8.0f), Armor);
	AddSentinelPart(Cube, FVector(-2.0f, 39.0f, 12.0f), FVector(0.42f, 0.32f, 0.5f), FRotator(0.0f, 12.0f, 8.0f), Armor);
	AddSentinelPart(Cube, FVector(16.0f, 0.0f, -13.0f), FVector(0.32f, 0.4f, 0.38f), FRotator::ZeroRotator, Armor);
	AddSentinelPart(Sphere, FVector(38.0f, 0.0f, 12.0f), FVector(0.19f), FRotator::ZeroRotator, WarningViolet);
	bRidgefireBruteVisualsCreated = true;
}

void AShooterNPC::AddSentinelPart(UStaticMesh* StaticMesh, const FVector& Location, const FVector& Scale, const FRotator& Rotation, const FLinearColor& Color)
{
	if (!StaticMesh || !SentinelVisualRoot)
	{
		return;
	}

	UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(this);
	if (!Part)
	{
		return;
	}
	Part->SetStaticMesh(StaticMesh);
	Part->SetupAttachment(SentinelVisualRoot);
	Part->SetMobility(EComponentMobility::Movable);
	Part->SetRelativeLocation(Location);
	Part->SetRelativeScale3D(Scale);
	Part->SetRelativeRotation(Rotation);
	Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Part->SetGenerateOverlapEvents(false);
	Part->CastShadow = false;
	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Variant_Shooter/Materials/M_Ridgefire_Sentinel.M_Ridgefire_Sentinel"));
	if (!BaseMaterial)
	{
		BaseMaterial = StaticMesh->GetMaterial(0);
	}
	if (BaseMaterial)
	{
		if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, this))
		{
			Material->SetVectorParameterValue(TEXT("Color"), Color);
			Material->SetVectorParameterValue(TEXT("BaseColor"), Color);
			const bool bSignalPart = Color.R > 0.7f || Color.G > 0.5f || Color.B > 0.7f;
			Material->SetVectorParameterValue(TEXT("EmissiveColor"), Color * (bSignalPart ? 0.8f : 0.015f));
			Material->SetScalarParameterValue(TEXT("Emissive"), bSignalPart ? 0.8f : 0.015f);
			Material->SetScalarParameterValue(TEXT("Metallic"), bSignalPart ? 0.45f : 0.8f);
			Material->SetScalarParameterValue(TEXT("Roughness"), bSignalPart ? 0.25f : 0.42f);
			Part->SetMaterial(0, Material);
		}
	}
	AddInstanceComponent(Part);
	Part->RegisterComponent();
	SentinelVisualParts.Add(Part);
}

void AShooterNPC::UpdateFoundryTurretVisuals(float DeltaTime)
{
	if (!FoundryTurretRotor)
	{
		return;
	}
	if (!HasAuthority() && bFoundryTurretCharging && !bFoundryTurretClientWasCharging)
	{
		FoundryTurretClientChargeStartedAt = GetWorld()->GetTimeSeconds();
	}
	bFoundryTurretClientWasCharging = bFoundryTurretCharging;
	const float SpinRate = bFoundryTurretCharging ? 1260.0f : bFoundryTurretBurstActive ? 720.0f : 24.0f;
	FoundryTurretSpinAngle = FMath::Fmod(FoundryTurretSpinAngle + SpinRate * DeltaTime, 360.0f);
	FoundryTurretRotor->SetRelativeRotation(FRotator(0.0f, 0.0f, FoundryTurretSpinAngle));
	const float ChargeAlpha = HasAuthority()
		? bFoundryTurretCharging
			? FMath::Clamp((GetWorld()->GetTimeSeconds() - FoundryTurretChargeStartedAt) / FMath::Max(0.01f, FoundryTurretChargeEndsAt - FoundryTurretChargeStartedAt), 0.0f, 1.0f)
			: bFoundryTurretBurstActive ? 1.0f : 0.0f
		: bFoundryTurretCharging
			? FMath::Clamp((GetWorld()->GetTimeSeconds() - FoundryTurretClientChargeStartedAt) / 1.1f, 0.0f, 1.0f)
			: bFoundryTurretBurstActive ? 1.0f : 0.0f;
	for (int32 MaterialIndex = 1; MaterialIndex < FoundryTurretMaterials.Num(); MaterialIndex += 2)
	{
		if (FoundryTurretMaterials[MaterialIndex])
		{
			FoundryTurretMaterials[MaterialIndex]->SetVectorParameterValue(TEXT("EmissiveColor"), FMath::Lerp(FLinearColor(0.08f, 0.55f, 0.85f), FLinearColor(2.6f, 0.62f, 0.09f), ChargeAlpha));
		}
	}
}

void AShooterNPC::UpdateSentryTelegraphVisual()
{
	if (HasAuthority())
	{
		return;
	}
	if ((bSentryTelegraphActive || bSentryMeleeTelegraphActive || bFoundryTurretCharging || bBruteRangedCharging) && IsValid(CurrentSentryTarget))
	{
		ShowSentryTelegraph(CurrentSentryTarget, bRidgefireBrute);
	}
	else
	{
		HideSentryTelegraph();
	}
}

void AShooterNPC::OnRep_Dead()
{
	if (bIsDead)
	{
		ApplyDeathPresentation();
	}
}

void AShooterNPC::OnRep_RidgefireSentryEnabled()
{
	if (bIsDead && bRidgefireSentry)
	{
		ApplyDeathPresentation();
	}
}

void AShooterNPC::OnRep_FoundryTurretEnabled()
{
	if (bFoundryTurret)
	{
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
		HideHumanoidSentinelVisuals();
		CreateFoundryTurretCannons();
		if (bIsDead)
		{
			ApplyDeathPresentation();
		}
	}
}

void AShooterNPC::OnRep_RidgefireSprinterEnabled()
{
	if (bRidgefireSprinter)
	{
		CreateSprinterVisuals();
		if (bIsDead)
		{
			ApplyDeathPresentation();
		}
	}
}

void AShooterNPC::OnRep_RidgefireBruteEnabled()
{
	if (bRidgefireBrute)
	{
		CreateBruteVisuals();
		if (bIsDead)
		{
			ApplyDeathPresentation();
		}
	}
}

void AShooterNPC::OnRep_SentryAttackState()
{
	UpdateFoundryTurretVisuals(0.0f);
	UpdateSentryTelegraphVisual();
}

APawn* AShooterNPC::SelectRidgefireSentryTarget(const TArray<APawn*>& CandidatePawns, const FVector& SentryLocation, APawn* CurrentTarget, const AActor* PathFailedTarget, float PathFailedUntil, float CurrentTime)
{
	APawn* Target = nullptr;
	APawn* FallbackTarget = nullptr;
	float ClosestTargetDistance = TNumericLimits<float>::Max();
	float FallbackTargetDistance = TNumericLimits<float>::Max();
	float CurrentTargetDistance = TNumericLimits<float>::Max();
	bool bCurrentTargetEligible = false;
	for (APawn* Candidate : CandidatePawns)
	{
		if (!IsValid(Candidate) || Candidate->ActorHasTag(TEXT("Dead")))
		{
			continue;
		}
		const float CandidateDistance = FVector::Dist2D(SentryLocation, Candidate->GetActorLocation());
		if (CandidateDistance < FallbackTargetDistance)
		{
			FallbackTarget = Candidate;
			FallbackTargetDistance = CandidateDistance;
		}
		if (Candidate == PathFailedTarget && CurrentTime < PathFailedUntil)
		{
			continue;
		}
		if (Candidate == CurrentTarget)
		{
			CurrentTargetDistance = CandidateDistance;
			bCurrentTargetEligible = true;
		}
		if (CandidateDistance < ClosestTargetDistance)
		{
			Target = Candidate;
			ClosestTargetDistance = CandidateDistance;
		}
	}
	if (bCurrentTargetEligible && CurrentTargetDistance <= ClosestTargetDistance + 250.0f)
	{
		Target = CurrentTarget;
	}
	return Target ? Target : FallbackTarget;
}

float AShooterNPC::CalculateSentryVisualClearanceDistance(float SentryVisualExtent, float TargetCapsuleRadius, float ClearanceBuffer)
{
	return FMath::Max(0.0f, SentryVisualExtent) + FMath::Max(0.0f, TargetCapsuleRadius) + FMath::Max(0.0f, ClearanceBuffer);
}

float AShooterNPC::GetRidgefireSentryApproachWeight(bool bBrute, bool bSprinter, bool bDodging, float Distance, float VisualClearanceDistance)
{
	if (Distance <= VisualClearanceDistance)
	{
		return -0.15f;
	}
	if (bDodging)
	{
		return 0.18f;
	}
	if (Distance > 850.0f)
	{
		return 1.0f;
	}
	return bBrute ? 0.55f : bSprinter ? 0.45f : 0.15f;
}

bool AShooterNPC::ShouldCancelSentryPathRecovery(bool bUsesRidgefireSentryMovement, bool bRecoveryActive, float Distance, float VisualClearanceDistance)
{
	return bUsesRidgefireSentryMovement && bRecoveryActive && Distance <= VisualClearanceDistance;
}

FVector AShooterNPC::GetSentryMovementWithClearance(const FVector& DesiredDirection, const FVector& TowardTarget, float Distance, float VisualClearanceDistance)
{
	return Distance <= VisualClearanceDistance ? -TowardTarget.GetSafeNormal2D() : DesiredDirection;
}

float AShooterNPC::GetSentryContactEscapeSpeed(float BaseSpeed, const FVector& TargetVelocity, const FVector& TowardTarget)
{
	const float ClosingSpeed = -FVector::DotProduct(TargetVelocity, TowardTarget.GetSafeNormal2D());
	const float SafeClosingSpeed = FMath::IsFinite(ClosingSpeed) ? FMath::Max(0.0f, ClosingSpeed) : 0.0f;
	const float SafeBaseSpeed = FMath::IsFinite(BaseSpeed) ? FMath::Max(0.0f, BaseSpeed) : 0.0f;
	return FMath::Max(SafeBaseSpeed, SafeClosingSpeed + 160.0f);
}

float AShooterNPC::GetSentryVisualClearanceDistance(const AActor* Target, const FVector& TowardTarget) const
{
	float SentryVisualExtent = GetCapsuleComponent() ? GetCapsuleComponent()->GetScaledCapsuleRadius() : 0.0f;
	const FVector Direction = TowardTarget.GetSafeNormal2D();
	const FVector AbsoluteDirection(FMath::Abs(Direction.X), FMath::Abs(Direction.Y), 0.0f);
	for (const UStaticMeshComponent* VisualPart : SentinelVisualParts)
	{
		if (!IsValid(VisualPart) || VisualPart->bHiddenInGame || !VisualPart->IsVisible())
		{
			continue;
		}

		const FBoxSphereBounds& Bounds = VisualPart->Bounds;
		const FVector CenterOffset = Bounds.Origin - GetActorLocation();
		const float ExtentTowardTarget = FVector::DotProduct(CenterOffset, Direction)
			+ FVector::DotProduct(Bounds.BoxExtent, AbsoluteDirection);
		SentryVisualExtent = FMath::Max(SentryVisualExtent, ExtentTowardTarget);
	}

	float TargetCapsuleRadius = 0.0f;
	if (const ACharacter* TargetCharacter = Cast<ACharacter>(Target))
	{
		if (const UCapsuleComponent* TargetCapsule = TargetCharacter->GetCapsuleComponent())
		{
			TargetCapsuleRadius = TargetCapsule->GetScaledCapsuleRadius();
		}
	}
	return CalculateSentryVisualClearanceDistance(SentryVisualExtent, TargetCapsuleRadius, SentryVisualClearanceBuffer);
}

void AShooterNPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
 if(bIsDead && SentinelVisualRoot && PressureDeathAt>=0)
 {
  const float A=FMath::Clamp((GetWorld()->GetTimeSeconds()-PressureDeathAt)/0.38f,0.0f,1.0f);
  SentinelVisualRoot->SetRelativeRotation(FMath::Lerp(PressureDeathStart,FRotator(72,0,18),A*A*(3-2*A)));
 }
	CreateFirstArenaArmoredSentinelVisuals();
 if(!bIsDead && !bFoundryTurret && GetNetMode()!=NM_DedicatedServer)
 {
  if(PressureStepLocation.IsNearlyZero()) PressureStepLocation=GetActorLocation();
  if(FVector::Dist2D(PressureStepLocation,GetActorLocation())>110.0f && GetWorld()->GetTimeSeconds()-PressureStepAt>0.3f)
  {PressureStepLocation=GetActorLocation();PressureStepAt=GetWorld()->GetTimeSeconds();PlayPressureFootstep();}
 }
#if WITH_EDITOR
	if (HasAuthority() && !bIsDead && FParse::Param(FCommandLine::Get(), TEXT("RidgefireSentryContactSmoke")))
	{
		if (!bRidgefireSentryContactSmokeClaimed)
		{
			bRidgefireSentryContactSmokeClaimed = true;
			bSentryContactSmokeActor = true;
		}
		if (bSentryContactSmokeActor)
		{
			TickSentryContactEditorSmokeTest(DeltaTime);
		}
	}
#endif
	if (SentinelVisualRoot && !bIsDead)
	{
		SentinelHoverPhase += DeltaTime * 1.7f;
		SentinelVisualRoot->SetRelativeLocation(FVector(0.0f, 0.0f, -70.0f + FMath::Sin(SentinelHoverPhase) * 1.5f - PressureHitStrength*3.0f*FMath::Clamp(1.0f-(GetWorld()->GetTimeSeconds()-PressureHitAt)/0.18f,0.0f,1.0f)));
		SentinelVisualRoot->SetRelativeRotation(FRotator(0.0f, 0.0f, FMath::Cos(SentinelHoverPhase) * 1.5f + PressureHitStrength*5.0f*FMath::Clamp(1.0f-(GetWorld()->GetTimeSeconds()-PressureHitAt)/0.18f,0.0f,1.0f)));
		UpdateFoundryTurretVisuals(DeltaTime);
		if (!HasAuthority())
		{
			UpdateSentryTelegraphVisual();
			return;
		}

		const float CurrentTime = GetWorld()->GetTimeSeconds();
        if(CurrentTime<PressureInterruptUntil) return;
		TArray<APawn*> CandidatePawns;
		APawn* CurrentTarget = Cast<APawn>(CurrentSentryTarget.Get());
		for (FConstPlayerControllerIterator PlayerControllerIt = GetWorld()->GetPlayerControllerIterator(); PlayerControllerIt; ++PlayerControllerIt)
		{
			APlayerController* PlayerController = PlayerControllerIt->Get();
			APawn* Candidate = IsValid(PlayerController) ? PlayerController->GetPawn() : nullptr;
			if (IsValid(Candidate))
			{
				CandidatePawns.Add(Candidate);
			}
		}
		APawn* Target = SelectRidgefireSentryTarget(CandidatePawns, GetActorLocation(), CurrentTarget, SentryPathFailedTarget.Get(), SentryPathFailedTargetUntil, CurrentTime);
		if (!IsValid(Target) || Target->ActorHasTag(TEXT("Dead")))
		{
			if (bSentryPathRecoveryActive)
			{
				if (AAIController* SentryController = Cast<AAIController>(GetController()))
				{
					SentryController->StopMovement();
				}
			}
			bSentryPathRecoveryActive = false;
			SentryStuckDuration = 0.0f;
			SentryPathFailedTarget.Reset();
			SentryPathFailedTargetUntil = 0.0f;
			CurrentSentryTarget = nullptr;
			SentryFirstLineOfSightAt = -1.0f;
			bSentryOpeningAttackStarted = false;
			const bool bAttackWarningWasActive = bFoundryTurretCharging || bFoundryTurretBurstActive || bBruteRangedCharging || bBruteBurstActive;
			bFoundryTurretCharging = false;
			bFoundryTurretBurstActive = false;
			FoundryTurretShotsRemaining = 0;
			bBruteRangedCharging = false;
			bBruteBurstActive = false;
			BruteBurstShotsRemaining = 0;
			if (bAttackWarningWasActive)
			{
				HideSentryBeam();
			}
			if (bSentryTelegraphActive || bSentryMeleeTelegraphActive)
			{
				bSentryTelegraphActive = false;
				bSentryMeleeTelegraphActive = false;
				NextSentryShotTime = GetWorld()->GetTimeSeconds() + 0.5f;
				HideSentryBeam();
#if WITH_EDITOR
				OnSentryEditorEvent.Broadcast(TEXT("TelegraphCancelled"), Target);
#endif
			}
			if (bIsShooting)
			{
				StopShooting();
			}
			return;
		}
		if (CurrentTarget && CurrentTarget != Target)
		{
			const bool bHadSentryTelegraph = bSentryTelegraphActive || bSentryMeleeTelegraphActive;
			const bool bHadFoundryAttack = bFoundryTurretCharging || bFoundryTurretBurstActive;
			const bool bHadBruteAttack = bBruteRangedCharging || bBruteBurstActive;
			bSentryTelegraphActive = false;
			bSentryMeleeTelegraphActive = false;
			NextSentryShotTime = CurrentTime + 0.8f;
			NextSentryMeleeTime = CurrentTime + 0.5f;
			bFoundryTurretCharging = false;
			bFoundryTurretBurstActive = false;
			FoundryTurretShotsRemaining = 0;
			FoundryTurretChargeEndsAt = 0.0f;
			bBruteRangedCharging = false;
			bBruteBurstActive = false;
			BruteBurstShotsRemaining = 0;
			BruteRangedNextAttackTime = CurrentTime + 0.8f;
			BruteChargeEndsAt = 0.0f;
			bSentryOpeningAttackStarted = false;
			SentryFirstLineOfSightAt = -1.0f;
			if (bHadSentryTelegraph || bHadFoundryAttack || bHadBruteAttack)
			{
				HideSentryBeam();
#if WITH_EDITOR
				if (bHadSentryTelegraph)
				{
					OnSentryEditorEvent.Broadcast(TEXT("TelegraphCancelled"), CurrentTarget);
				}
				if (bHadFoundryAttack)
				{
					OnSentryEditorEvent.Broadcast(TEXT("FoundryTurretAttackCancelled"), CurrentTarget);
				}
#endif
			}
		}
		if (bSentryPathRecoveryActive && CurrentTarget != Target)
		{
			if (AAIController* SentryController = Cast<AAIController>(GetController()))
			{
				SentryController->StopMovement();
			}
			bSentryPathRecoveryActive = false;
			SentryStuckDuration = 0.0f;
		}
		CurrentSentryTarget = Target;
		const FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
		const FVector FlatToTarget(ToTarget.X, ToTarget.Y, 0.0f);
		const float Distance = FlatToTarget.Size();
		const FVector ApproachDirection = FlatToTarget.GetSafeNormal2D();
		const bool bUsesMobileRidgefireRole = bRidgefireSentry || bRidgefireSprinter || bRidgefireBrute;
		const float SentryVisualClearanceDistance = bUsesMobileRidgefireRole
			? GetSentryVisualClearanceDistance(Target, ApproachDirection)
			: 220.0f;
		if (CurrentTime - SentryMovementSampleTime >= 0.5f)
		{
			const float MovedDistance = FVector::Dist2D(GetActorLocation(), SentryMovementSampleLocation);
			if (Distance > 350.0f && MovedDistance < 18.0f)
			{
				SentryStuckDuration += CurrentTime - SentryMovementSampleTime;
			}
			else
			{
				SentryStuckDuration = 0.0f;
			}
			SentryMovementSampleLocation = GetActorLocation();
			SentryMovementSampleTime = CurrentTime;
		}

		AAIController* SentryController = Cast<AAIController>(GetController());
		if (ShouldCancelSentryPathRecovery(bUsesMobileRidgefireRole, bSentryPathRecoveryActive, Distance, SentryVisualClearanceDistance))
		{
			if (SentryController)
			{
				SentryController->StopMovement();
			}
			bSentryPathRecoveryActive = false;
			SentryPathRecoveryRetryTime = CurrentTime + 0.75f;
		}
		if (bSentryPathRecoveryActive)
		{
			const UPathFollowingComponent* PathFollowing = SentryController ? SentryController->GetPathFollowingComponent() : nullptr;
			if (PathFollowing && PathFollowing->GetStatus() != EPathFollowingStatus::Idle && CurrentTime < SentryPathRecoveryDeadline)
			{
				// Let path following clear the obstruction; combat and attack telegraphs continue below.
			}
			else
			{
				if (PathFollowing && PathFollowing->GetStatus() != EPathFollowingStatus::Idle)
				{
					SentryController->StopMovement();
				}
				if (Distance > 350.0f && CurrentTime < SentryPathRecoveryDeadline)
				{
					SentryPathFailedTarget = Target;
					SentryPathFailedTargetUntil = CurrentTime + 4.0f;
				}
				bSentryPathRecoveryActive = false;
				SentryPathRecoveryRetryTime = CurrentTime + 0.75f;
			}
		}
		if (!bSentryPathRecoveryActive && SentryStuckDuration >= 1.25f && CurrentTime >= SentryPathRecoveryRetryTime && SentryController)
		{
			const float PathAcceptanceDistance = FMath::Max(220.0f, SentryVisualClearanceDistance);
			const EPathFollowingRequestResult::Type MoveResult = SentryController->MoveToActor(Target, PathAcceptanceDistance, true, true, true, nullptr, true);
			if (MoveResult != EPathFollowingRequestResult::Failed)
			{
				bSentryPathRecoveryActive = true;
				SentryPathRecoveryDeadline = CurrentTime + 4.0f;
				SentryStuckDuration = 0.0f;
				UE_LOG(LogTemp, Verbose, TEXT("RIDGEFIRE: %s requested a recovery path to %s after stalling."), *GetName(), *Target->GetName());
			}
			else
			{
				SentryPathRecoveryRetryTime = CurrentTime + 1.5f;
				SentryPathFailedTarget = Target;
				SentryPathFailedTargetUntil = CurrentTime + 4.0f;
				SentinelStrafeDirection *= -1.0f;
			}
		}
		if (Distance < 1.0f)
		{
			return;
		}
		const FVector Approach = FlatToTarget / Distance;
		const FVector Strafe(-Approach.Y * SentinelStrafeDirection, Approach.X * SentinelStrafeDirection, 0.0f);
		const float ApproachWeight = bUsesMobileRidgefireRole
			? GetRidgefireSentryApproachWeight(bRidgefireBrute, bRidgefireSprinter, false, Distance, SentryVisualClearanceDistance)
			: Distance > 850.0f ? 1.0f : Distance < 360.0f ? -0.75f : 0.15f;
	const float StrafeWeight = bRidgefireBrute ? (bPressureStrategic ? 0.6f : 0.2f) : 0.65f;
	FVector MoveDirection = (Approach * ApproachWeight + Strafe * StrafeWeight).GetSafeNormal();
        if(bPressureStrategic && !bRidgefireBrute && !bRidgefireSprinter)
        {
            // A small assigned subset takes an oblique route while allies maintain pressure.
            const float Lateral=bPressureFlanker ? 1.25f : 0.25f;
            const float Forward=Distance>1000 ? 0.7f : Distance<400 ? -0.55f : bPressureFlanker ? 0.20f : 0.03f;
            MoveDirection=(Approach*Forward+Strafe*Lateral).GetSafeNormal();
        }
		float Speed = FMath::Clamp(GetCharacterMovement()->MaxWalkSpeed * 0.6f, 160.0f, 360.0f);
		if (bRidgefireSprinter)
		{
			const float SprinterMoveTime = GetWorld()->GetTimeSeconds();
			if (SprinterMoveTime >= SprinterNextDodgeTime)
			{
				SprinterDodgeDirection = FMath::RandBool() ? 1.0f : -1.0f;
				SprinterDodgeEndsAt = SprinterMoveTime + 0.25f;
				SprinterNextDodgeTime = SprinterMoveTime + FMath::FRandRange(0.85f, 1.25f);
			}
			if (SprinterMoveTime < SprinterDodgeEndsAt)
			{
				const float DodgeApproachWeight = GetRidgefireSentryApproachWeight(false, true, true, Distance, SentryVisualClearanceDistance);
				MoveDirection = (Approach * DodgeApproachWeight + Strafe * SprinterDodgeDirection * 1.8f).GetSafeNormal();
				Speed = FMath::Clamp(GetCharacterMovement()->MaxWalkSpeed, 520.0f, 700.0f);
			}
			else
			{
				MoveDirection = (Approach * ApproachWeight + Strafe * (bPressureStrategic ? 0.85f : 0.35f)).GetSafeNormal();
				Speed = FMath::Clamp(GetCharacterMovement()->MaxWalkSpeed * 0.7f, 340.0f, 440.0f);
			}
		}
		if (bUsesMobileRidgefireRole)
		{
			MoveDirection = GetSentryMovementWithClearance(MoveDirection, Approach, Distance, SentryVisualClearanceDistance);
			if (Distance <= SentryVisualClearanceDistance)
			{
				Speed = GetSentryContactEscapeSpeed(Speed, Target->GetVelocity(), Approach);
			}
		}
		if (!bSentryPathRecoveryActive)
		{
		FHitResult MoveHit;
		SetActorLocation(GetActorLocation() + MoveDirection * Speed * DeltaTime, true, &MoveHit);
		if (MoveHit.bBlockingHit)
		{
			FVector DetourDirection = FVector::VectorPlaneProject(MoveDirection, MoveHit.Normal).GetSafeNormal();
			if (bRidgefireSentry || bRidgefireSprinter || bRidgefireBrute)
			{
				const FVector SurfaceTangent = FVector::CrossProduct(FVector::UpVector, MoveHit.Normal).GetSafeNormal2D();
				const FVector PreferredSide = Strafe * SentinelStrafeDirection;
				const FVector CandidateDirections[] =
				{
					(MoveDirection + SurfaceTangent * 1.4f).GetSafeNormal2D(),
					(MoveDirection - SurfaceTangent * 1.4f).GetSafeNormal2D(),
					SurfaceTangent,
					-SurfaceTangent
				};
				const float ProbeDistance = 100.0f;
				const float CapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();
				const float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
				const FCollisionShape ProbeShape = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);
				FCollisionQueryParams ProbeQuery(SCENE_QUERY_STAT(RidgefireSentryObstacleDetour), false, this);
				FVector BestDirection = FVector::ZeroVector;
				float BestScore = -TNumericLimits<float>::Max();
				const FVector ProgressDirection = Distance <= SentryVisualClearanceDistance ? -Approach : Approach;
				for (const FVector& CandidateDirection : CandidateDirections)
				{
					if (CandidateDirection.IsNearlyZero())
					{
						continue;
					}
					FHitResult ProbeHit;
					const FVector ProbeEnd = GetActorLocation() + CandidateDirection * ProbeDistance;
					if (GetWorld()->SweepSingleByChannel(ProbeHit, GetActorLocation(), ProbeEnd, GetActorQuat(), ECC_Pawn, ProbeShape, ProbeQuery))
					{
						continue;
					}
					const float CandidateScore = FVector::DotProduct(CandidateDirection, ProgressDirection)
						+ FVector::DotProduct(CandidateDirection, PreferredSide) * 0.15f;
					if (CandidateScore > BestScore)
					{
						BestDirection = CandidateDirection;
						BestScore = CandidateScore;
					}
				}
				if (!BestDirection.IsNearlyZero())
				{
					DetourDirection = BestDirection;
				}
				else if (GetWorld()->GetTimeSeconds() >= SentinelNextTurnTime)
				{
					SentinelStrafeDirection *= -1.0f;
					SentinelNextTurnTime = GetWorld()->GetTimeSeconds() + 0.75f;
				}
			}
			if (!bRidgefireSentry && !bRidgefireSprinter && !bRidgefireBrute)
			{
				FHitResult SlideHit;
				SetActorLocation(GetActorLocation() + DetourDirection * Speed * DeltaTime, true, &SlideHit);
				if (SlideHit.bBlockingHit && GetWorld()->GetTimeSeconds() >= SentinelNextTurnTime)
				{
					SentinelStrafeDirection *= -1.0f;
					SentinelNextTurnTime = GetWorld()->GetTimeSeconds() + 1.0f;
				}
			}
			else if (!DetourDirection.IsNearlyZero())
			{
				FHitResult DetourHit;
				SetActorLocation(GetActorLocation() + DetourDirection * Speed * DeltaTime, true, &DetourHit);
				if (DetourHit.bBlockingHit && GetWorld()->GetTimeSeconds() >= SentinelNextTurnTime)
				{
					SentinelStrafeDirection *= -1.0f;
					SentinelNextTurnTime = GetWorld()->GetTimeSeconds() + 0.75f;
				}
			}
		}
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), Approach.Rotation(), DeltaTime, 4.0f));
		}
		FCollisionQueryParams SightQuery(SCENE_QUERY_STAT(RidgefireSentinelSight), false, this);
		SightQuery.AddIgnoredActor(this);
		FHitResult SightHit;
		const FVector SightStart = GetActorLocation() + FVector(0.0f, 0.0f, 30.0f);
		const FVector SightEnd = Target->GetActorLocation() + FVector(0.0f, 0.0f, 45.0f);
		const bool bHasLineOfSight = Distance < 1700.0f
			&& GetWorld()->LineTraceSingleByChannel(SightHit, SightStart, SightEnd, ECC_Visibility, SightQuery)
			&& SightHit.GetActor() == Target;
		const ARidgefireHealingTube* HitTube = Cast<ARidgefireHealingTube>(SightHit.GetActor());
		const bool bHasRangedLineOfSight = bHasLineOfSight || (Distance < 1700.0f && HitTube && HitTube->IsAcceptingEnemyHits());
		if (bHasRangedLineOfSight && SentryFirstLineOfSightAt < 0.0f)
		{
			SentryFirstLineOfSightAt = CurrentTime;
		}
		bool bPlayerFacingSentry = false;
		if (Target->GetController())
		{
			const FVector ToSentry = (GetActorLocation() - Target->GetActorLocation()).GetSafeNormal();
			bPlayerFacingSentry = FVector::DotProduct(Target->GetBaseAimRotation().Vector(), ToSentry) >= 0.45f;
		}
		const bool bUsesRidgefireThreatRules = bRidgefireSentry || bFoundryTurret || bRidgefireSprinter || bRidgefireBrute;
		const bool bOpeningThreatReady = !bUsesRidgefireThreatRules
			|| bSentryOpeningAttackStarted
			|| bPlayerFacingSentry
			|| (SentryFirstLineOfSightAt >= 0.0f && CurrentTime - SentryFirstLineOfSightAt >= 5.0f);
        const bool bCanShoot = (bRidgefireSentry || bRidgefireBrute || Weapon) && bHasRangedLineOfSight && bOpeningThreatReady;
        if(PressureShotsRemaining>0)
        {
            if(!bCanShoot || Distance<=SentryMeleeTriggerRange || PressureAimTarget.Get()!=Target) PressureShotsRemaining=0;
            else if(CurrentTime>=PressureNextShot)
            {
                FireSentryShot(Target,PressurePattern==2 ? 2.8f : 5.0f);
                --PressureShotsRemaining;
                PressureNextShot=CurrentTime+(PressurePattern==2 ? 0.16f : 0.25f);
            }
        }
		const bool bCanMelee = (bRidgefireSentry || bRidgefireSprinter || bRidgefireBrute) && bHasLineOfSight && bOpeningThreatReady;
		if (bRidgefireBrute)
		{
			TickRidgefireBrute(Target, bHasRangedLineOfSight && bOpeningThreatReady, Distance, FMath::Abs(ToTarget.Z), CurrentTime);
		}
		else if (bFoundryTurret)
		{
			TickFoundryTurret(Target, bCanShoot, CurrentTime);
		}
		else if (bRidgefireSentry || bRidgefireSprinter)
		{
			const bool bInMeleeRange = Distance <= SentryMeleeTriggerRange && FMath::Abs(ToTarget.Z) <= 170.0f;
			if (bInMeleeRange && bCanMelee)
			{
				if (bSentryTelegraphActive)
				{
					bSentryTelegraphActive = false;
					HideSentryBeam();
#if WITH_EDITOR
					OnSentryEditorEvent.Broadcast(TEXT("TelegraphCancelled"), Target);
#endif
				}
				if (bSentryMeleeTelegraphActive && CurrentTime >= NextSentryMeleeTime)
				{
					bSentryMeleeTelegraphActive = false;
					HideSentryBeam();
					PerformSentryMeleeAttack(Target);
					NextSentryMeleeTime = CurrentTime + FMath::FRandRange(2.6f, 3.4f);
				}
				else if (!bSentryMeleeTelegraphActive && CurrentTime >= NextSentryMeleeTime)
				{
					bSentryMeleeTelegraphActive = true;
					NextSentryMeleeTime = CurrentTime + 0.9f;
					if (!ShowSentryTelegraph(Target))
					{
						bSentryMeleeTelegraphActive = false;
						NextSentryMeleeTime = CurrentTime + 0.5f;
					}
					else
					{
						DispatchSentryTelegraphCue(Target);
						bSentryOpeningAttackStarted = true;
#if WITH_EDITOR
						OnSentryEditorEvent.Broadcast(TEXT("MeleeTelegraphStarted"), Target);
#endif
					}
				}
			}
			else
			{
				if (bSentryMeleeTelegraphActive)
				{
					bSentryMeleeTelegraphActive = false;
					NextSentryMeleeTime = CurrentTime + 0.5f;
					HideSentryBeam();
#if WITH_EDITOR
					OnSentryEditorEvent.Broadcast(TEXT("TelegraphCancelled"), Target);
#endif
				}
				if (!bCanShoot && bSentryTelegraphActive)
				{
					bSentryTelegraphActive = false;
					NextSentryShotTime = CurrentTime + 0.5f;
					HideSentryBeam();
#if WITH_EDITOR
					OnSentryEditorEvent.Broadcast(TEXT("TelegraphCancelled"), Target);
#endif
				}
				else if (bCanShoot && bSentryTelegraphActive && CurrentTime >= NextSentryShotTime)
				{
					bSentryTelegraphActive = false;
					HideSentryBeam();
                    const int32 Shots=PressurePattern==1 ? 1 : PressurePattern==2 ? FMath::RandRange(5,7) : FMath::RandRange(2,4);
                    FireSentryShot(Target,PressurePattern==1 ? 12.0f : PressurePattern==2 ? 2.8f : 5.0f);
                    PressureShotsRemaining=Shots-1;
                    PressureNextShot=CurrentTime+(PressurePattern==2 ? 0.16f : 0.25f);
                    NextSentryShotTime = CurrentTime + FMath::FRandRange(3.5f,5.0f);
				}
				else if (bCanShoot && bSentryTelegraphActive)
				{
					if (!ShowSentryTelegraph(Target))
					{
						bSentryTelegraphActive = false;
						NextSentryShotTime = CurrentTime + 0.5f;
						HideSentryBeam();
					}
				}
				else if (bCanShoot && CurrentTime >= NextSentryShotTime)
				{
					bSentryTelegraphActive = true;
                    LockPressureAim(Target);
                    NextSentryShotTime = CurrentTime + (PressurePattern==1 ? 1.2f : FMath::FRandRange(0.75f,1.0f));
					if (!ShowSentryTelegraph(Target))
					{
						bSentryTelegraphActive = false;
						NextSentryShotTime = CurrentTime + 0.5f;
					}
					else
					{
						DispatchSentryTelegraphCue(Target);
						bSentryOpeningAttackStarted = true;
#if WITH_EDITOR
						OnSentryEditorEvent.Broadcast(TEXT("TelegraphStarted"), Target);
#endif
					}
				}
			}
		}
		else if (!bRidgefireSentry && !bRidgefireBrute && bCanShoot && !bIsShooting)
		{
			StartShooting(Target);
		}
		else if (!bRidgefireSentry && !bRidgefireBrute && !bCanShoot && bIsShooting)
		{
			StopShooting();
		}
	}
}

#if WITH_EDITOR
void AShooterNPC::TickSentryContactEditorSmokeTest(float DeltaTime)
{
	if (bSentryContactSmokeFinished || !GetWorld() || !SentinelVisualRoot)
	{
		return;
	}
	ACharacter* Player = SentryContactSmokePlayer.Get();
	if (!bSentryContactSmokeStarted)
	{
		APlayerController* PlayerControllerForFixture = GetWorld()->GetFirstPlayerController();
		Player = PlayerControllerForFixture ? Cast<ACharacter>(PlayerControllerForFixture->GetPawn()) : nullptr;
		if (!Player || !Player->GetCharacterMovement())
		{
			return;
		}
		SentryContactSmokePlayer = Player;
		SentryContactSmokePlayerStart = Player->GetActorLocation();
		for (TActorIterator<AShooterNPC> It(GetWorld()); It; ++It)
		{
			if (*It != this)
			{
				It->SetActorTickEnabled(false);
				if (AAIController* ControllerToStop = Cast<AAIController>(It->GetController()))
				{
					ControllerToStop->StopMovement();
				}
			}
		}
		if (AAIController* ControllerToStop = Cast<AAIController>(GetController()))
		{
			ControllerToStop->StopMovement();
		}
		bSentryPathRecoveryActive = false;
		FCollisionQueryParams Query(SCENE_QUERY_STAT(RidgefireSentryContactFixture), false, this);
		Query.AddIgnoredActor(Player);
		const FCollisionShape Shape = FCollisionShape::MakeCapsule(
			GetCapsuleComponent()->GetScaledCapsuleRadius(), GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		FVector Direction = FVector::ZeroVector;
		FVector StartLocation = FVector::ZeroVector;
		const float PlayerGroundZ = SentryContactSmokePlayerStart.Z - Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		for (int32 DirectionIndex = 0; DirectionIndex < 8; ++DirectionIndex)
		{
			const FVector Candidate = FRotator(0.0f, DirectionIndex * 45.0f, 0.0f).Vector();
			FVector CandidateStart = SentryContactSmokePlayerStart + Candidate * 500.0f;
			CandidateStart.Z = PlayerGroundZ + GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2.0f;
			FHitResult ProbeHit;
			if (!GetWorld()->SweepSingleByChannel(ProbeHit, CandidateStart, CandidateStart + Candidate * 1000.0f,
				FQuat::Identity, ECC_Pawn, Shape, Query))
			{
				Direction = Candidate;
				StartLocation = CandidateStart;
				break;
			}
		}
		if (Direction.IsNearlyZero())
		{
			bSentryContactSmokeFinished = true;
			UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE NPC SMOKE FAIL contact fixture has no collision-probed escape lane"));
			return;
		}
		SetActorLocation(StartLocation, false, nullptr, ETeleportType::TeleportPhysics);
		SetActorRotation((-Direction).Rotation());
		const float Clearance = GetSentryVisualClearanceDistance(Player, -Direction);
		FVector PlayerStart = StartLocation - Direction * (Clearance - 35.0f);
		PlayerStart.Z = SentryContactSmokePlayerStart.Z;
		FHitResult PlacementHit;
		Player->SetActorLocation(PlayerStart, true, &PlacementHit, ETeleportType::TeleportPhysics);
		Player->GetCharacterMovement()->StopMovementImmediately();
		SentryContactSmokeInitialDistance = FVector::Dist2D(GetActorLocation(), Player->GetActorLocation());
		SentryContactSmokePlayerWalkStart = Player->GetActorLocation();
		SentryContactSmokeMinimumGap = SentryContactSmokeInitialDistance - Clearance;
		if (SentryContactSmokeInitialDistance >= Clearance || SentryContactSmokeInitialDistance < 1.0f)
		{
			bSentryContactSmokeFinished = true;
			UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE NPC SMOKE FAIL contact fixture did not begin inside the actual visible envelope (distance=%.1f clearance=%.1f blocked=%d)"),
				SentryContactSmokeInitialDistance, Clearance, PlacementHit.bBlockingHit);
			return;
		}
		bSentryContactSmokeStarted = true;
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE NPC SMOKE CONTACT_START role=%d distance=%.1f clearance=%.1f player_walk_speed=%.1f"),
			SentryContactSmokeRole, SentryContactSmokeInitialDistance, Clearance, Player->GetCharacterMovement()->MaxWalkSpeed);
	}
	if (!Player)
	{
		bSentryContactSmokeFinished = true;
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE NPC SMOKE FAIL contact fixture lost its actual possessed player"));
		return;
	}
	SentryContactSmokeElapsed += DeltaTime;
	const FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
	const float Distance = ToPlayer.Size2D();
	const float Clearance = GetSentryVisualClearanceDistance(Player, ToPlayer);
	SentryContactSmokeMinimumGap = FMath::Min(SentryContactSmokeMinimumGap, Distance - Clearance);
	if (SentryContactSmokeElapsed < 1.2f)
	{
		Player->AddMovementInput(-ToPlayer.GetSafeNormal2D(), 1.0f, true);
		return;
	}
	const float PlayerDistance = FVector::Dist2D(Player->GetActorLocation(), SentryContactSmokePlayerWalkStart);
	const float PlayerSpeed = Player->GetVelocity().Size2D();
	Player->ConsumeMovementInputVector();
	Player->GetCharacterMovement()->StopMovementImmediately();
	Player->SetActorLocation(SentryContactSmokePlayerStart, false, nullptr, ETeleportType::TeleportPhysics);
	bSentryContactSmokeFinished = true;
	const bool bClearanceRecovered = Distance >= Clearance - 5.0f && PlayerSpeed >= 100.0f && PlayerDistance >= 20.0f;
	UE_LOG(LogTemp, Display,
		TEXT("RIDGEFIRE NPC SMOKE CONTACT_RESULT role=%d start=%.1f final=%.1f clearance=%.1f min_gap=%.1f player_distance=%.1f player_speed=%.1f"),
		SentryContactSmokeRole, SentryContactSmokeInitialDistance, Distance, Clearance, SentryContactSmokeMinimumGap, PlayerDistance, PlayerSpeed);
	if (!bClearanceRecovered)
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE NPC SMOKE FAIL moving-player contact did not recover actual visible clearance (role=%d)"), SentryContactSmokeRole);
		return;
	}
	UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE NPC SMOKE PASS moving-player contact recovered actual visible clearance role=%d"), SentryContactSmokeRole);
	if (SentryContactSmokeRole >= 2)
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE NPC SMOKE PASS contact clearance completed standard sprinter brute actual actors"));
		return;
	}
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AShooterNPC* NextRole = GetWorld()->SpawnActor<AShooterNPC>(GetClass(), GetActorTransform(), SpawnParameters);
	if (!NextRole)
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE NPC SMOKE FAIL contact fixture could not spawn next real Blueprint role"));
		return;
	}
	NextRole->bSentryContactSmokeActor = true;
	NextRole->SentryContactSmokeRole = SentryContactSmokeRole + 1;
	if (NextRole->SentryContactSmokeRole == 1)
	{
		NextRole->EnableRidgefireSprinter();
	}
	else
	{
		NextRole->EnableRidgefireBrute();
	}
	SetActorTickEnabled(false);
}
#endif

void AShooterNPC::PerformSentryMeleeAttack(AActor* Target, float Damage)
{
	if (!HasAuthority() || (!bRidgefireSentry && !bRidgefireSprinter && !bRidgefireBrute) || bIsDead || !IsValid(Target) || Target->ActorHasTag(TEXT("Dead")))
	{
		return;
	}
	const FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
	if (FVector::Dist2D(GetActorLocation(), Target->GetActorLocation()) > 255.0f || FMath::Abs(ToTarget.Z) > 170.0f)
	{
		return;
	}
	FCollisionQueryParams Query(SCENE_QUERY_STAT(RidgefireSentryMelee), false, this);
	Query.AddIgnoredActor(this);
	FHitResult Hit;
	const FVector Start = GetActorLocation() + FVector(0.0f, 0.0f, 30.0f);
	const FVector End = Target->GetActorLocation() + FVector(0.0f, 0.0f, 45.0f);
	if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Query) || Hit.GetActor() != Target)
	{
		return;
	}
	UGameplayStatics::ApplyDamage(Target, Damage*RidgefireDamageScale, GetController(), this, UDamageType::StaticClass());
#if WITH_EDITOR
	OnSentryEditorEvent.Broadcast(TEXT("MeleeHit"), Target);
#endif
}

void AShooterNPC::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// clear the death timer
	GetWorld()->GetTimerManager().ClearTimer(DeathTimer);
	GetWorld()->GetTimerManager().ClearTimer(RidgefireSlowTimer);
	GetWorld()->GetTimerManager().ClearTimer(SentryBeamTimer);
}

void AShooterNPC::TickRidgefireBrute(AActor* Target, bool bHasLineOfSight, float Distance, float VerticalDistance, float CurrentTime)
{
	if (!HasAuthority() || bIsDead || !IsValid(Target))
	{
		return;
	}

	const bool bInMeleeRange = Distance <= SentryMeleeTriggerRange && VerticalDistance <= 170.0f;
	if (!bHasLineOfSight)
	{
		const bool bWasAttacking = bBruteRangedCharging || bBruteBurstActive || bSentryMeleeTelegraphActive;
		bBruteRangedCharging = false;
		bBruteBurstActive = false;
		BruteBurstShotsRemaining = 0;
		bSentryMeleeTelegraphActive = false;
		if (bWasAttacking)
		{
			NextSentryMeleeTime = CurrentTime + 0.5f;
			BruteRangedNextAttackTime = CurrentTime + 0.8f;
			HideSentryBeam();
		}
		return;
	}

	if (bInMeleeRange)
	{
		const bool bWasChargingRangedAttack = bBruteRangedCharging;
		bBruteRangedCharging = false;
		bBruteBurstActive = false;
		BruteBurstShotsRemaining = 0;
		if (bWasChargingRangedAttack)
		{
			HideSentryBeam();
		}
		if (!bSentryMeleeTelegraphActive && CurrentTime >= NextSentryMeleeTime)
		{
			bSentryMeleeTelegraphActive = true;
			NextSentryMeleeTime = CurrentTime + 0.85f;
			if (!ShowSentryTelegraph(Target, true))
			{
				bSentryMeleeTelegraphActive = false;
				NextSentryMeleeTime = CurrentTime + 0.8f;
			}
			else
			{
				DispatchSentryTelegraphCue(Target);
				bSentryOpeningAttackStarted = true;
			}
		}
		else if (bSentryMeleeTelegraphActive && CurrentTime >= NextSentryMeleeTime)
		{
			bSentryMeleeTelegraphActive = false;
			HideSentryBeam();
			PerformSentryMeleeAttack(Target, 32.0f);
			NextSentryMeleeTime = CurrentTime + FMath::FRandRange(2.8f, 3.5f);
		}
		return;
	}

	if (bSentryMeleeTelegraphActive)
	{
		bSentryMeleeTelegraphActive = false;
		NextSentryMeleeTime = CurrentTime + 0.5f;
		HideSentryBeam();
	}

	if (bBruteBurstActive)
	{
		if (CurrentTime < BruteNextBurstShotAt)
		{
			return;
		}
		FireSentryShot(Target, 15.0f);
		--BruteBurstShotsRemaining;
		if (BruteBurstShotsRemaining > 0)
		{
			BruteNextBurstShotAt = CurrentTime + 0.38f;
		}
		else
		{
			bBruteBurstActive = false;
			BruteRangedNextAttackTime = CurrentTime + FMath::FRandRange(4.8f, 5.8f);
		}
		return;
	}

	if (bBruteRangedCharging)
	{
		if (!ShowSentryTelegraph(Target, true))
		{
			bBruteRangedCharging = false;
			BruteRangedNextAttackTime = CurrentTime + 0.8f;
			HideSentryBeam();
			return;
		}
		if (CurrentTime < BruteChargeEndsAt)
		{
			return;
		}
		bBruteRangedCharging = false;
		HideSentryBeam();
		FireSentryShot(Target, 15.0f);
		bBruteBurstActive = true;
		BruteBurstShotsRemaining = 2;
		BruteNextBurstShotAt = CurrentTime + 0.38f;
		return;
	}

	if (CurrentTime >= BruteRangedNextAttackTime)
	{
		bBruteRangedCharging = true;
		LockPressureAim(Target);
		BruteChargeEndsAt = CurrentTime + 1.35f;
		if (!ShowSentryTelegraph(Target, true))
		{
			bBruteRangedCharging = false;
			BruteRangedNextAttackTime = CurrentTime + 0.8f;
		}
		else
		{
			DispatchSentryTelegraphCue(Target);
			bSentryOpeningAttackStarted = true;
		}
	}
}

void AShooterNPC::DispatchSentryTelegraphCue(AActor* Target)
{
	if (!HasAuthority())
	{
		return;
	}
	Multicast_PlaySentryTelegraphCue(GetActorLocation());
#if WITH_EDITOR
	OnSentryEditorEvent.Broadcast(TEXT("TelegraphWarningCueDispatched"), Target);
	UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE NPC SMOKE PASS sentry telegraph warning cue multicast dispatched: %s"), *GetName());
#endif
}

#if WITH_EDITOR
bool AShooterNPC::RunSentryTelegraphCueEditorSmokeTest(AActor* Target)
{
	const bool bHasSentryRole = bRidgefireSentry || bRidgefireSprinter || bFoundryTurret || bRidgefireBrute;
	if (!HasAuthority() || !IsValid(Target) || bIsDead || !bHasSentryRole || bIsShooting
		|| bSentryTelegraphActive || bSentryMeleeTelegraphActive
		|| bFoundryTurretCharging || bFoundryTurretBurstActive
		|| bBruteRangedCharging || bBruteBurstActive)
	{
		return false;
	}

	UStaticMeshComponent* PreviousTelegraphBeam = SentryTelegraphBeam;
	const bool bPreviousFallbackOverride = bForceSentryTelegraphFallbackForEditorSmokeTest;
	SentryTelegraphBeam = nullptr;
	bForceSentryTelegraphFallbackForEditorSmokeTest = true;
	const bool bFallbackTelegraphShown = ShowSentryTelegraph(Target);
	UStaticMeshComponent* FallbackTelegraphBeam = SentryTelegraphBeam;
	bool bFallbackTelegraphIsValid = bFallbackTelegraphShown && FallbackTelegraphBeam != nullptr;
	if (bFallbackTelegraphIsValid)
	{
		bFallbackTelegraphIsValid = FallbackTelegraphBeam->GetStaticMesh() != nullptr
			&& FallbackTelegraphBeam->GetMaterial(0) != nullptr
			&& !FallbackTelegraphBeam->bHiddenInGame
			&& FallbackTelegraphBeam->IsVisible();
	}
	HideSentryTelegraph();
	if (IsValid(FallbackTelegraphBeam) && FallbackTelegraphBeam != PreviousTelegraphBeam)
	{
		RemoveInstanceComponent(FallbackTelegraphBeam);
		FallbackTelegraphBeam->DestroyComponent();
	}
	SentryTelegraphBeam = PreviousTelegraphBeam;
	bForceSentryTelegraphFallbackForEditorSmokeTest = bPreviousFallbackOverride;
	if (!bFallbackTelegraphIsValid)
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE NPC SMOKE FAIL default-material fallback did not show a valid sentry telegraph: %s"), *GetName());
		return false;
	}
	UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE NPC SMOKE PASS default-material fallback showed telegraph without triggering caller cancellation: %s"), *GetName());

	int32 CueDispatchCount = 0;
	const FDelegateHandle CueDispatchHandle = OnSentryEditorEvent.AddLambda(
		[&CueDispatchCount](FName Event, AActor*)
		{
			if (Event == TEXT("TelegraphWarningCueDispatched"))
			{
				++CueDispatchCount;
			}
		});

	DispatchSentryTelegraphCue(Target);
	OnSentryEditorEvent.Remove(CueDispatchHandle);
	HideSentryTelegraph();

	if (CueDispatchCount != 1)
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE NPC SMOKE FAIL sentry telegraph warning cue dispatch count=%d expected=1: %s"), CueDispatchCount, *GetName());
		return false;
	}

	UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE NPC SMOKE PASS exactly one sentry telegraph warning cue dispatch: %s"), *GetName());
	return true;
}
#endif

void AShooterNPC::Multicast_PlaySentryTelegraphCue_Implementation(FVector Location)
{
	if (GetNetMode() != NM_DedicatedServer)
	{
		PlaySentryTelegraphCue(Location);
	}
}

void AShooterNPC::PlaySentryTelegraphCue(const FVector& Location)
{
	constexpr int32 SampleRate = 44100;
	constexpr float Duration = 0.30f;
	constexpr float PulseDuration = 0.11f;
	const int32 SampleCount = static_cast<int32>(SampleRate * Duration);
	TArray<int16> Samples;
	Samples.SetNumUninitialized(SampleCount);
	for (int32 Index = 0; Index < SampleCount; ++Index)
	{
		const float Time = static_cast<float>(Index) / SampleRate;
		const float PulseStart = Time < PulseDuration ? 0.0f : Time >= 0.17f && Time < 0.17f + PulseDuration ? 0.17f : -1.0f;
		float Value = 0.0f;
		if (PulseStart >= 0.0f)
		{
			const float PulseTime = Time - PulseStart;
			const float Progress = FMath::Clamp(PulseTime / PulseDuration, 0.0f, 1.0f);
			const float Frequency = FMath::Lerp(430.0f, 760.0f, Progress);
			const float Envelope = FMath::Sin(PI * Progress);
			Value = (FMath::Sin(2.0f * PI * Frequency * PulseTime) * 0.22f
				+ FMath::Sin(2.0f * PI * Frequency * 1.5f * PulseTime) * 0.06f) * Envelope;
		}
		Samples[Index] = static_cast<int16>(FMath::Clamp(Value, -1.0f, 1.0f) * 32767.0f);
	}

	URidgefireSynthWave* Sound = NewObject<URidgefireSynthWave>(GetTransientPackage());
	if (!Sound)
	{
		return;
	}
	Sound->Duration = Duration;
	Sound->QueueAudio(reinterpret_cast<const uint8*>(Samples.GetData()), Samples.Num() * sizeof(int16));
	if (!SentryWarningAttenuation)
	{
		SentryWarningAttenuation = NewObject<USoundAttenuation>(this);
		if (SentryWarningAttenuation)
		{
			FSoundAttenuationSettings& Settings = SentryWarningAttenuation->Attenuation;
			Settings.bAttenuate = true;
			Settings.bSpatialize = true;
			Settings.AttenuationShape = EAttenuationShape::Sphere;
			Settings.AttenuationShapeExtents = FVector::ZeroVector;
			Settings.DistanceAlgorithm = EAttenuationDistanceModel::Logarithmic;
			Settings.FalloffDistance = 1600.0f;
			Settings.NonSpatializedRadiusStart = 0.0f;
			Settings.NonSpatializedRadiusEnd = 0.0f;
		}
	}
	if (SentryWarningAttenuation)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Sound, Location, 0.72f, 1.0f, 0.0f, SentryWarningAttenuation);
	}
}

bool AShooterNPC::ShowSentryTelegraph(AActor* Target, bool bHeavyWarning)
{
	if ((!bRidgefireSentry && !bRidgefireSprinter && !bFoundryTurret && !bRidgefireBrute) || bIsDead || !IsValid(Target))
	{
		return false;
	}
	if (!SentryTelegraphBeam)
	{
		UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
		UMaterialInterface* Material = nullptr;
#if WITH_EDITOR
		if (!bForceSentryTelegraphFallbackForEditorSmokeTest)
#endif
		{
			Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Variant_Shooter/Materials/M_Ridgefire_Sentinel.M_Ridgefire_Sentinel"));
		}
		if (!Material)
		{
			Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
		}
		if (!Cylinder || !Material)
		{
			return false;
		}
		SentryTelegraphBeam = NewObject<UStaticMeshComponent>(this, TEXT("RidgefireSentryTelegraph"));
		SentryTelegraphBeam->SetStaticMesh(Cylinder);
		SentryTelegraphBeam->SetMaterial(0, Material);
		SentryTelegraphBeam->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SentryTelegraphBeam->SetCastShadow(false);
		SentryTelegraphBeam->SetupAttachment(GetRootComponent());
		AddInstanceComponent(SentryTelegraphBeam);
		SentryTelegraphBeam->RegisterComponent();
	}
	UMaterialInstanceDynamic* BeamMaterial = Cast<UMaterialInstanceDynamic>(SentryTelegraphBeam->GetMaterial(0));
	if (!BeamMaterial)
	{
		BeamMaterial = SentryTelegraphBeam->CreateDynamicMaterialInstance(0);
	}
	if (!BeamMaterial)
	{
		return false;
	}
	BeamMaterial->SetVectorParameterValue(TEXT("BaseColor"), bHeavyWarning ? FLinearColor(0.34f, 0.06f, 0.56f) : FLinearColor(0.82f, 0.34f, 0.08f));
	BeamMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), bHeavyWarning ? FLinearColor(1.8f, 0.2f, 3.0f) : FLinearColor(2.4f, 0.72f, 0.16f));
	const FVector Start = GetActorLocation() + GetActorForwardVector() * 48.0f + FVector(0.0f, 0.0f, 18.0f);
	FVector End = Target->GetActorLocation() + FVector(0.0f, 0.0f, 45.0f);
	FCollisionQueryParams WarningQuery(SCENE_QUERY_STAT(RidgefireTubeWarning), false, this);
	FHitResult WarningHit;
	if (GetWorld()->LineTraceSingleByChannel(WarningHit, Start, End, ECC_Visibility, WarningQuery)
		&& Cast<ARidgefireHealingTube>(WarningHit.GetActor()))
	{
		End = WarningHit.ImpactPoint;
	}
	const FVector BeamVector = End - Start;
	SentryTelegraphBeam->SetWorldLocation((Start + End) * 0.5f);
	SentryTelegraphBeam->SetWorldRotation(FRotationMatrix::MakeFromZ(BeamVector).Rotator());
	const float BeamWidth = bHeavyWarning ? 0.010f : 0.006f;
	SentryTelegraphBeam->SetWorldScale3D(FVector(BeamWidth, BeamWidth, BeamVector.Size() / 100.0f));
	SentryTelegraphBeam->SetHiddenInGame(false);
	return true;
}

void AShooterNPC::TickFoundryTurret(AActor* Target, bool bCanShoot, float CurrentTime)
{
	if (!bCanShoot)
	{
		if (bFoundryTurretCharging || bFoundryTurretBurstActive)
		{
			bFoundryTurretCharging = false;
			bFoundryTurretBurstActive = false;
			FoundryTurretShotsRemaining = 0;
			NextSentryShotTime = CurrentTime + 0.8f;
			HideSentryBeam();
#if WITH_EDITOR
			OnSentryEditorEvent.Broadcast(TEXT("FoundryTurretAttackCancelled"), Target);
#endif
		}
		return;
	}
	if (bFoundryTurretBurstActive)
	{
		if (CurrentTime < FoundryTurretNextBurstShotAt)
		{
			return;
		}
		FireSentryShot(Target, 8.0f);
		--FoundryTurretShotsRemaining;
		if (FoundryTurretShotsRemaining > 0)
		{
			FoundryTurretNextBurstShotAt = CurrentTime + 0.24f;
		}
		else
		{
			bFoundryTurretBurstActive = false;
			NextSentryShotTime = CurrentTime + FMath::FRandRange(4.3f, 5.4f);
#if WITH_EDITOR
			OnSentryEditorEvent.Broadcast(TEXT("FoundryTurretBurstFinished"), Target);
#endif
		}
		return;
	}
	if (bFoundryTurretCharging)
	{
		if (!ShowSentryTelegraph(Target))
		{
			bFoundryTurretCharging = false;
			NextSentryShotTime = CurrentTime + 0.8f;
			HideSentryBeam();
			return;
		}
		if (CurrentTime < FoundryTurretChargeEndsAt)
		{
			return;
		}
		bFoundryTurretCharging = false;
		HideSentryBeam();
		FireSentryShot(Target, 8.0f);
		bFoundryTurretBurstActive = true;
		FoundryTurretShotsRemaining = 2;
		FoundryTurretNextBurstShotAt = CurrentTime + 0.24f;
		return;
	}
	if (CurrentTime >= NextSentryShotTime)
	{
		bFoundryTurretCharging = true;
		LockPressureAim(Target);
		FoundryTurretChargeStartedAt = CurrentTime;
		FoundryTurretChargeEndsAt = CurrentTime + 1.1f;
		if (!ShowSentryTelegraph(Target))
		{
			bFoundryTurretCharging = false;
			NextSentryShotTime = CurrentTime + 0.8f;
			return;
		}
		bSentryOpeningAttackStarted = true;
		DispatchSentryTelegraphCue(Target);
#if WITH_EDITOR
		OnSentryEditorEvent.Broadcast(TEXT("FoundryTurretCharging"), Target);
#endif
	}
}

void AShooterNPC::FireSentryShot(AActor* Target,float Damage)
{
 if(!HasAuthority() || (!bRidgefireSentry && !bRidgefireBrute && !bFoundryTurret) || bIsDead || !IsValid(Target)) return;
 const float Now=GetWorld()->GetTimeSeconds();
 if(PressureAimTarget.Get()!=Target) LockPressureAim(Target);
 const FVector Start=GetActorLocation()+GetActorForwardVector()*48+FVector(0,0,18);
 const FVector Current=Target->GetActorLocation()+FVector(0,0,45);
 const FVector Locked=(PressureLockedAim-Start).GetSafeNormal();
 const FVector Desired=(Current-Start).GetSafeNormal();
 const float Angle=FMath::Acos(FMath::Clamp(FVector::DotProduct(Locked,Desired),-1.0f,1.0f));
 const float Allow=FMath::DegreesToRadians(bPressureStrategic ? 12.0f : 3.0f)*FMath::Min(0.4f,FMath::Max(0.0f,Now-PressureAimAt));
 const float Blend=Angle>SMALL_NUMBER ? FMath::Min(1.0f,Allow/Angle) : 1.0f;
 const FVector Aim=FMath::Lerp(Locked,Desired,Blend).GetSafeNormal();
 PressureLockedAim=Start+Aim*FVector::Distance(Start,Current);PressureAimAt=Now;
 const FVector End=Start+Aim*1900.0f;
 FCollisionQueryParams Query(SCENE_QUERY_STAT(RidgefireSentryShot),false,this);Query.AddIgnoredActor(this);
 FHitResult Hit;
 const bool Blocked=GetWorld()->LineTraceSingleByChannel(Hit,Start,End,ECC_Visibility,Query);
 auto* Tube=Blocked ? Cast<ARidgefireHealingTube>(Hit.GetActor()) : nullptr;
 if(Blocked && (Hit.GetActor()==Target || (Tube && Tube->IsAcceptingEnemyHits())))
  UGameplayStatics::ApplyDamage(Hit.GetActor(),Damage*RidgefireDamageScale,GetController(),this,UDamageType::StaticClass());
#if WITH_EDITOR
 OnSentryEditorEvent.Broadcast(TEXT("ShotFired"),Blocked ? Hit.GetActor() : nullptr);
#endif
 Multicast_ShowSentryShotBeam(Start,Blocked ? Hit.ImpactPoint : End);
}

void AShooterNPC::Multicast_ShowSentryShotBeam_Implementation(FVector Start, FVector ImpactPoint)
{
	ShowSentryShotBeam(Start, ImpactPoint);
}

void AShooterNPC::ShowSentryShotBeam(const FVector& Start, const FVector& ImpactPoint)
{
	if (!SentryShotBeam)
	{
		UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
		UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Variant_Shooter/Materials/M_Ridgefire_Sentinel.M_Ridgefire_Sentinel"));
		if (!Cylinder || !Material)
		{
			return;
		}
		SentryShotBeam = NewObject<UStaticMeshComponent>(this, TEXT("RidgefireSentryBeam"));
		SentryShotBeam->SetStaticMesh(Cylinder);
		SentryShotBeam->SetMaterial(0, Material);
		SentryShotBeam->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SentryShotBeam->SetCastShadow(false);
		SentryShotBeam->SetupAttachment(GetRootComponent());
		AddInstanceComponent(SentryShotBeam);
		SentryShotBeam->RegisterComponent();
	}
	UMaterialInstanceDynamic* BeamMaterial = Cast<UMaterialInstanceDynamic>(SentryShotBeam->GetMaterial(0));
	if (!BeamMaterial)
	{
		BeamMaterial = SentryShotBeam->CreateDynamicMaterialInstance(0);
	}
	if (BeamMaterial)
	{
		BeamMaterial->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.7f, 0.12f, 0.025f));
		BeamMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(3.0f, 0.45f, 0.04f));
	}
	const FVector BeamVector = ImpactPoint - Start;
	SentryShotBeam->SetWorldLocation((Start + ImpactPoint) * 0.5f);
	SentryShotBeam->SetWorldRotation(FRotationMatrix::MakeFromZ(BeamVector).Rotator());
	SentryShotBeam->SetWorldScale3D(FVector(0.018f, 0.018f, BeamVector.Size() / 100.0f));
	SentryShotBeam->SetHiddenInGame(false);
	GetWorld()->GetTimerManager().SetTimer(SentryBeamTimer, this, &AShooterNPC::HideSentryBeam, 0.18f, false);
}

void AShooterNPC::HideSentryBeam()
{
	if (SentryShotBeam)
	{
		SentryShotBeam->SetHiddenInGame(true);
	}
	HideSentryTelegraph();
}

void AShooterNPC::HideSentryTelegraph()
{
	if (SentryTelegraphBeam)
	{
		SentryTelegraphBeam->SetHiddenInGame(true);
	}
}

void AShooterNPC::ApplyRidgefireSlow(float Duration, float SpeedMultiplier)
{
	if (!HasAuthority() || bIsDead)
	{
		return;
	}

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!GetWorld()->GetTimerManager().IsTimerActive(RidgefireSlowTimer))
	{
		RidgefireOriginalSpeed = Movement->MaxWalkSpeed;
	}
	Movement->MaxWalkSpeed = RidgefireOriginalSpeed * FMath::Clamp(SpeedMultiplier, 0.1f, 1.0f);
	GetWorld()->GetTimerManager().SetTimer(RidgefireSlowTimer, this, &AShooterNPC::ClearRidgefireSlow, Duration, false);
}

float AShooterNPC::GetTravelDistance() const
{
	return FVector::Dist2D(SentinelSpawnLocation, GetActorLocation());
}

void AShooterNPC::ClearRidgefireSlow()
{
	if (RidgefireOriginalSpeed > 0.0f && !bIsDead)
	{
		GetCharacterMovement()->MaxWalkSpeed = RidgefireOriginalSpeed;
	}
	RidgefireOriginalSpeed = 0.0f;
}

float AShooterNPC::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority() || bIsDead || !FMath::IsFinite(Damage) || Damage <= 0.0f)
	{
		return 0.0f;
	}

    float ValidatedDamage = bRidgefireChampion && bChampionGuarded ? Damage*0.25f : Damage;
	if (bFoundryTurret && DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const FPointDamageEvent* PointDamageEvent = static_cast<const FPointDamageEvent*>(&DamageEvent);
		if (PointDamageEvent && IsFoundryTurretWeakPoint(PointDamageEvent->HitInfo.Component.Get()))
		{
			ValidatedDamage *= 3.0f;
		}
	}

	const float AppliedDamage = FMath::Min(ValidatedDamage, FMath::Max(0.0f, CurrentHP));
	if (AppliedDamage <= 0.0f)
	{
		Die();
		return 0.0f;
	}
    if(FinisherMaxHP<=0) FinisherMaxHP=CurrentHP;
    CurrentHP -= AppliedDamage;
    if(CurrentHP>0)
    {
        const float Now=GetWorld()->GetTimeSeconds();
        bool Weak=false;
        if(DamageEvent.IsOfType(FPointDamageEvent::ClassID))
        {
            const auto& Point=static_cast<const FPointDamageEvent&>(DamageEvent);
            Weak=Point.HitInfo.BoneName==TEXT("head") || IsFoundryTurretWeakPoint(Point.HitInfo.Component.Get())
                || (Point.HitInfo.Component.IsValid() && Point.HitInfo.Component->ComponentHasTag(TEXT("RidgefireHead")));
        }
        MulticastPressureHit(FMath::Clamp(AppliedDamage/80.0f,0.2f,1.0f));
        // Heavy bodies resist interruption; a refractory window prevents stun locking.
        if(!bRidgefireChampion && !bRidgefireBrute && Now>=PressureInterruptReadyAt && (AppliedDamage>=75 || Weak))
        {
            PressureInterruptUntil=Now+0.3f;PressureInterruptReadyAt=Now+2.0f;
            PressureShotsRemaining=0;bSentryTelegraphActive=false;bSentryMeleeTelegraphActive=false;
            bFoundryTurretCharging=false;bFoundryTurretBurstActive=false;FoundryTurretShotsRemaining=0;
            NextSentryShotTime=Now+0.8f;NextSentryMeleeTime=Now+0.8f;HideSentryBeam();ForceNetUpdate();
        }
    }
    if(CurrentHP>0 && CurrentHP<=FinisherMaxHP*0.25f && !bRidgefireChampion && !bFoundryTurret
        && Cast<AShooterPlayerController>(EventInstigator)) {
        FinisherUntil=GetWorld()->GetTimeSeconds()+2.5f;ForceNetUpdate();
    }
    if (bRidgefireChampion && !bChampionSecondPhase && CurrentHP > 0 && CurrentHP <= ChampionMaxHP*0.5f)
    {
        bChampionSecondPhase=true;
        GetCharacterMovement()->MaxWalkSpeed*=1.25f;
        BruteRangedNextAttackTime=GetWorld()->GetTimeSeconds()+1.2f;
        UE_LOG(LogTemp,Display,TEXT("RIDGEFIRE CHAMPION phase=2 hp=%.1f"),CurrentHP);
        ForceNetUpdate();
    }
	if (CurrentHP <= 0.0f && DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const FPointDamageEvent& Point = static_cast<const FPointDamageEvent&>(DamageEvent);
		if (IsFoundryTurretWeakPoint(Point.HitInfo.Component.Get()) || Point.HitInfo.BoneName == FName(TEXT("head")) || (Point.HitInfo.Component.IsValid() && Point.HitInfo.Component->ComponentHasTag(TEXT("RidgefireHead"))))
			if (ARidgefireGameMode* Mode = GetWorld()->GetAuthGameMode<ARidgefireGameMode>()) Mode->AwardPersonalCombatXP(EventInstigator, 3);
	}

	// Have we depleted HP?
	if (CurrentHP <= 0.0f)
	{
		Die();
	}

	return AppliedDamage;
}

void AShooterNPC::AttachWeaponMeshes(AShooterWeapon* WeaponToAttach)
{
	const FAttachmentTransformRules AttachmentRule(EAttachmentRule::SnapToTarget, false);

	// attach the weapon actor
	WeaponToAttach->AttachToActor(this, AttachmentRule);

	// attach the weapon meshes
	WeaponToAttach->GetFirstPersonMesh()->AttachToComponent(GetFirstPersonMesh(), AttachmentRule, FirstPersonWeaponSocket);
	WeaponToAttach->GetThirdPersonMesh()->AttachToComponent(GetMesh(), AttachmentRule, ThirdPersonWeaponSocket);
}

void AShooterNPC::PlayFiringMontage(UAnimMontage* Montage)
{
	// unused
}

void AShooterNPC::AddWeaponRecoil(float Recoil)
{
	// unused
}

void AShooterNPC::UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize)
{
	// unused
}

FVector AShooterNPC::GetWeaponTargetLocation()
{
	// start aiming from the camera location
	const FVector AimSource = GetFirstPersonCameraComponent()->GetComponentLocation();

	FVector AimDir, AimTarget = FVector::ZeroVector;

	// do we have an aim target?
	if (CurrentAimTarget)
	{
		// target the actor location
		AimTarget = CurrentAimTarget->GetActorLocation();

		// apply a vertical offset to target head/feet
		AimTarget.Z += FMath::RandRange(MinAimOffsetZ, MaxAimOffsetZ);

		// get the aim direction and apply randomness in a cone
		AimDir = (AimTarget - AimSource).GetSafeNormal();
		AimDir = UKismetMathLibrary::RandomUnitVectorInConeInDegrees(AimDir, AimVarianceHalfAngle);

		
	} else {

		// no aim target, so just use the camera facing
		AimDir = UKismetMathLibrary::RandomUnitVectorInConeInDegrees(GetFirstPersonCameraComponent()->GetForwardVector(), AimVarianceHalfAngle);

	}

	// calculate the unobstructed aim target location
	AimTarget = AimSource + (AimDir * AimRange);

	// run a visibility trace to see if there's obstructions
	FHitResult OutHit;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(OutHit, AimSource, AimTarget, ECC_Visibility, QueryParams);

	// return either the impact point or the trace end
	return OutHit.bBlockingHit ? OutHit.ImpactPoint : OutHit.TraceEnd;
}

void AShooterNPC::AddWeaponClass(const TSubclassOf<AShooterWeapon>& InWeaponClass)
{
	// unused
}

void AShooterNPC::OnWeaponActivated(AShooterWeapon* InWeapon)
{
	// unused
}

void AShooterNPC::OnWeaponDeactivated(AShooterWeapon* InWeapon)
{
	// unused
}

void AShooterNPC::OnSemiWeaponRefire()
{
	// are we still shooting?
	if (HasAuthority() && bIsShooting)
	{
		// fire the weapon
		if (Weapon)
		{
			Weapon->StartFiring();
		}
	}
}

void AShooterNPC::Die()
{
	// ignore if already dead
	if (!HasAuthority() || bIsDead)
	{
		return;
	}

	// raise the dead flag
	bIsDead = true;
	ForceNetUpdate();
	bSentryTelegraphActive = false;
	bSentryMeleeTelegraphActive = false;
	bFoundryTurretCharging = false;
	bFoundryTurretBurstActive = false;
	FoundryTurretShotsRemaining = 0;
	bBruteRangedCharging = false;
	bBruteBurstActive = false;
	BruteBurstShotsRemaining = 0;
	HideSentryBeam();
	if (bIsShooting)
	{
		StopShooting();
	}

	// grant the death tag to the character
	Tags.Add(DeathTag);

	// call the delegate
	OnPawnDeath.Broadcast(this);

	// increment the team score
	if (AShooterGameMode* GM = Cast<AShooterGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->IncrementTeamScore(TeamByte);
	}

	ApplyDeathPresentation();

	// schedule actor destruction
	GetWorld()->GetTimerManager().SetTimer(DeathTimer, this, &AShooterNPC::DeferredDestruction, DeferredDestructionTime, false);
}

void AShooterNPC::ApplyDeathPresentation()
{
	HideSentryBeam();
	UpdateHumanoidSentinelCollision();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->StopActiveMovement();
	const bool bUseMechanicalDeathPresentation = bRidgefireSentry || bRidgefireSprinter || bRidgefireBrute || bFoundryTurret;
	if (bUseMechanicalDeathPresentation)
	{
		if (SentinelVisualRoot)
		{
			SentinelVisualRoot->SetVisibility(true, false);
			PressureDeathAt=GetWorld()->GetTimeSeconds();PressureDeathStart=SentinelVisualRoot->GetRelativeRotation();
		}
		if (GetMesh())
		{
			GetMesh()->SetSimulatePhysics(false);
			GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			GetMesh()->SetVisibility(false, false);
		}
#if WITH_EDITOR
		if (HasAuthority())
		{
			if (bSentryHitSmokeActor)
			{
				bool bArmorCollisionDisabled = true;
				for (const UStaticMeshComponent* Part : SentinelHumanoidParts)
				{
					bArmorCollisionDisabled &= IsValid(Part) && Part->GetCollisionEnabled() == ECollisionEnabled::NoCollision;
				}
				UStaticMeshComponent* CorpseArmorPart = SentryHitSmokeArmorParts.IsEmpty() ? nullptr : SentryHitSmokeArmorParts[0].Get();
				FCollisionQueryParams CorpseQuery(SCENE_QUERY_STAT(RidgefireSentryCorpseSmoke), false);
				for (TActorIterator<APawn> PawnIterator(GetWorld()); PawnIterator; ++PawnIterator)
				{
					if (*PawnIterator != this)
					{
						CorpseQuery.AddIgnoredActor(*PawnIterator);
					}
				}
				FHitResult CorpseHit;
				const FVector CorpsePoint = IsValid(CorpseArmorPart) ? CorpseArmorPart->GetComponentLocation() : GetActorLocation();
				const FVector CorpseTraceExtent = GetActorForwardVector() * 80.0f;
				const bool bCorpseBlocked = GetWorld()->LineTraceSingleByChannel(CorpseHit, CorpsePoint + CorpseTraceExtent,
					CorpsePoint - CorpseTraceExtent, ECC_Visibility, CorpseQuery);
				if (IsValid(CorpseArmorPart) && bArmorCollisionDisabled && (!bCorpseBlocked || CorpseHit.GetActor() != this))
				{
					UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE NPC SMOKE PASS sentry corpse armor disables query collision and does not block visibility traces"));
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE NPC SMOKE FAIL sentry corpse armor remains hittable actor=%s hit_actor=%s collision_disabled=%d"),
						*GetName(), *GetNameSafe(CorpseHit.GetActor()), bArmorCollisionDisabled);
				}
				bSentryHitSmokeActor = false;
			}
			const bool bMechanicalDeathPresentationApplied = SentinelVisualRoot && SentinelVisualRoot->IsVisible()
				&& GetMesh() && !GetMesh()->IsVisible() && !GetMesh()->IsSimulatingPhysics()
				&& GetCapsuleComponent()->GetCollisionEnabled() == ECollisionEnabled::NoCollision;
			if (bMechanicalDeathPresentationApplied)
			{
				UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE NPC SMOKE PASS mechanical death hides inherited skeletal mesh and ragdoll: %s"), *GetName());
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE NPC SMOKE FAIL mechanical death presentation exposed inherited skeletal mesh or collision: %s"), *GetName());
			}
		}
#endif
		return;
	}
	if (SentinelVisualRoot)
	{
		SentinelVisualRoot->SetVisibility(false, true);
	}
	if (GetMesh())
	{
		GetMesh()->SetVisibility(true, false);
		GetMesh()->SetCollisionProfileName(RagdollCollisionProfile);
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->SetPhysicsBlendWeight(1.0f);
	}
}

void AShooterNPC::DeferredDestruction()
{
	Destroy();
}

void AShooterNPC::StartShooting(AActor* ActorToShoot)
{
	if (!HasAuthority() || bRidgefireSprinter || bRidgefireBrute)
	{
		return;
	}
	// save the aim target
	CurrentAimTarget = ActorToShoot;

	// raise the flag
	bIsShooting = true;

	// signal the weapon
	if (Weapon)
	{
		Weapon->StartFiring();
	}
}

void AShooterNPC::StopShooting()
{
	if (!HasAuthority())
	{
		return;
	}
	// lower the flag
	bIsShooting = false;

	// signal the weapon
	if (Weapon)
	{
		Weapon->StopFiring();
	}
}

void AShooterNPC::EnableRidgefireChampion(bool bGuarded, bool bFoundryRoute)
{
    if (!HasAuthority()) return;
    bRidgefireChampion=true; bChampionGuarded=bGuarded;
    ChampionMaxHP=CurrentHP;
    GetCharacterMovement()->MaxWalkSpeed=bFoundryRoute ? 230.0f : 300.0f;
    Tags.Add(bFoundryRoute ? FName(TEXT("ChampionFurnaceWarden")) : FName(TEXT("ChampionGlassMarshal")));
    ForceNetUpdate();
}
void AShooterNPC::SetRidgefireChampionGuard(bool bGuarded)
{
    if (HasAuthority() && bRidgefireChampion) { bChampionGuarded=bGuarded; ForceNetUpdate(); }
}

void AShooterNPC::SetRidgefireDamageScale(float Scale)
{
    if (HasAuthority() && FMath::IsFinite(Scale)) RidgefireDamageScale=FMath::Clamp(Scale,1.0f,5.0f);
}

bool AShooterNPC::CanRidgefireFinish() const
{
    const auto* State=GetWorld()->GetGameState();
    const float Now=GetNetMode()==NM_Client && State ? State->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();
    return !bIsDead && !bRidgefireChampion && !bFoundryTurret && CurrentHP>0 && FinisherUntil>Now;
}

void AShooterNPC::ConfigurePressureTactics(int32 Seed,int32 Arena,int32 Encounter,int32 Ordinal,int32 Party)
{
 FRandomStream R(Seed ^ (Arena*73856093) ^ (Encounter*19349663));
 const int32 PreferredFamily=Arena==2 ? 1 : Arena==3 ? 2 : 0;
 const int32 SmartFamily=R.FRand()<0.6f ? PreferredFamily : R.RandRange(0,2);
 const int32 Family=bRidgefireBrute ? 2 : bRidgefireSprinter ? 1 : 0;
 const int32 Phase=R.RandRange(0,3);
 bPressureStrategic=Family==SmartFamily || (Ordinal+Phase)%4==0;
 // One candidate per reinforcement block; route can change sides between runs.
 bPressureFlanker=bPressureStrategic && (Ordinal+Phase)%FMath::Max(4,7-FMath::Clamp(Party,1,4))==0;
 R.Initialize(Seed ^ (Arena*83492791) ^ (Encounter*19349663) ^ (Ordinal*137));
 PressurePattern=R.FRand()<0.70f ? 0 : R.RandRange(1,2);
 SentinelStrafeDirection=R.RandRange(0,1)==0 ? -1.0f : 1.0f;
}
void AShooterNPC::LockPressureAim(AActor* Target)
{
 if(!IsValid(Target)) return;
 PressureAimTarget=Target;PressureLockedAim=Target->GetActorLocation()+FVector(0,0,45);PressureAimAt=GetWorld()->GetTimeSeconds();
}
void AShooterNPC::MulticastPressureHit_Implementation(float Strength)
{
 PressureHitStrength=FMath::Clamp(Strength,0.0f,1.0f);PressureHitAt=GetWorld()->GetTimeSeconds();
}

void AShooterNPC::PlayPressureFootstep()
{
 if(GetNetMode()==NM_DedicatedServer) return;
 if(!SentryWarningAttenuation)
 {
  SentryWarningAttenuation=NewObject<USoundAttenuation>(this);
  auto& A=SentryWarningAttenuation->Attenuation;A.bAttenuate=true;A.bSpatialize=true;
  A.AttenuationShape=EAttenuationShape::Sphere;A.AttenuationShapeExtents=FVector::ZeroVector;
  A.DistanceAlgorithm=EAttenuationDistanceModel::Logarithmic;A.FalloffDistance=1600;
 }
 constexpr int32 Rate=22050,Count=2205;
 TArray<int16> PCM;PCM.SetNumUninitialized(Count);
 const float Weight=bRidgefireBrute ? 0.32f : bRidgefireSprinter ? 0.12f : 0.20f;
 for(int32 I=0;I<Count;++I)
 {
  const float T=static_cast<float>(I)/Rate;
  const float V=(FMath::Sin(2*PI*(bRidgefireBrute ? 70.0f : 115.0f)*T)*Weight+FMath::FRandRange(-0.08f,0.08f))*FMath::Exp(-T*42);
  PCM[I]=static_cast<int16>(FMath::Clamp(V,-1.0f,1.0f)*32767);
 }
 auto* Wave=NewObject<URidgefireSynthWave>(GetTransientPackage());Wave->SetSampleRate(Rate);Wave->Duration=0.1f;
 Wave->QueueAudio(reinterpret_cast<const uint8*>(PCM.GetData()),PCM.Num()*sizeof(int16));
 UGameplayStatics::PlaySoundAtLocation(this,Wave,GetActorLocation(),0.65f,1,0,SentryWarningAttenuation);
}
