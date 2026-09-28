// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/AI/ShooterNPC.h"
#include "ShooterWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "ShooterGameMode.h"
#include "RidgefireGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
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
#include "TimerManager.h"

AShooterNPC::AShooterNPC()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
}

void AShooterNPC::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShooterNPC, CurrentHP);
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
		AddSentinelPart(PartMesh, Position * 1.08f, Scale * 1.08f, Rotation, Color);
		if (SentinelVisualParts.Num() > 0)
		{
			SentinelHumanoidParts.Add(SentinelVisualParts.Last());
		}
	};

	AddHumanoidPart(Cube, FVector(-3.0f, 0.0f, 69.0f), FVector(0.38f, 0.31f, 0.39f), FRotator::ZeroRotator, Armor);
	AddHumanoidPart(Cube, FVector(12.0f, 0.0f, 79.0f), FVector(0.29f, 0.32f, 0.3f), FRotator::ZeroRotator, Steel);
	AddHumanoidPart(Cube, FVector(-4.0f, 0.0f, 37.0f), FVector(0.28f, 0.27f, 0.2f), FRotator::ZeroRotator, Steel);
	AddHumanoidPart(Cylinder, FVector(0.0f, 0.0f, 99.0f), FVector(0.13f, 0.13f, 0.13f), FRotator::ZeroRotator, Ember);
	AddHumanoidPart(Sphere, FVector(1.0f, 0.0f, 122.0f), FVector(0.27f, 0.25f, 0.29f), FRotator::ZeroRotator, Armor);
	AddHumanoidPart(Cube, FVector(15.0f, 0.0f, 123.0f), FVector(0.12f, 0.235f, 0.065f), FRotator::ZeroRotator, Visor);
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
}

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
#if WITH_EDITOR
	bool bArmorPartsAreVisualOnly = SentinelVisualParts.Num() - FirstArmorPartIndex == 12;
	float ArmorTop = -TNumericLimits<float>::Max();
	float ArmorHalfWidth = 0.0f;
	for (int32 PartIndex = FirstArmorPartIndex; PartIndex < SentinelVisualParts.Num(); ++PartIndex)
	{
		const UStaticMeshComponent* Part = SentinelVisualParts[PartIndex];
		bArmorPartsAreVisualOnly = bArmorPartsAreVisualOnly && IsValid(Part)
			&& Part->GetAttachParent() == SentinelVisualRoot.Get()
			&& Part->GetCollisionEnabled() == ECollisionEnabled::NoCollision;
		if (IsValid(Part))
		{
			const FVector RelativeLocation = Part->GetRelativeLocation();
			const FVector RelativeScale = Part->GetRelativeScale3D();
			ArmorTop = FMath::Max(ArmorTop, RelativeLocation.Z + RelativeScale.Z * 50.0f);
			ArmorHalfWidth = FMath::Max(ArmorHalfWidth, FMath::Abs(RelativeLocation.Y) + RelativeScale.Y * 50.0f);
		}
	}
	bArmorPartsAreVisualOnly = bArmorPartsAreVisualOnly && ArmorTop >= 245.0f && ArmorHalfWidth >= 80.0f;
	if (bArmorPartsAreVisualOnly)
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE NPC SMOKE PASS first-arena armored sentry added 12 attached non-colliding parts with top %.1f and half-width %.1f: %s"), ArmorTop, ArmorHalfWidth, *GetName());
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

void AShooterNPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CreateFirstArenaArmoredSentinelVisuals();
	if (SentinelVisualRoot && !bIsDead)
	{
		SentinelHoverPhase += DeltaTime * 1.7f;
		SentinelVisualRoot->SetRelativeLocation(FVector(0.0f, 0.0f, -70.0f + FMath::Sin(SentinelHoverPhase) * 1.5f));
		SentinelVisualRoot->SetRelativeRotation(FRotator(0.0f, 0.0f, FMath::Cos(SentinelHoverPhase) * 1.5f));
		UpdateFoundryTurretVisuals(DeltaTime);
		if (!HasAuthority())
		{
			UpdateSentryTelegraphVisual();
			return;
		}

		const float CurrentTime = GetWorld()->GetTimeSeconds();
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
			const EPathFollowingRequestResult::Type MoveResult = SentryController->MoveToActor(Target, 220.0f, true, true, true, nullptr, true);
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
	const float ApproachWeight = Distance > 850.0f ? 1.0f : bRidgefireBrute ? 0.55f : bRidgefireSprinter ? 0.45f : Distance < 360.0f ? -0.75f : 0.15f;
	const float StrafeWeight = bRidgefireBrute ? 0.2f : 0.65f;
	FVector MoveDirection = (Approach * ApproachWeight + Strafe * StrafeWeight).GetSafeNormal();
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
				MoveDirection = (Approach * 0.18f + Strafe * SprinterDodgeDirection * 1.8f).GetSafeNormal();
				Speed = FMath::Clamp(GetCharacterMovement()->MaxWalkSpeed, 520.0f, 700.0f);
			}
			else
			{
				MoveDirection = (Approach * ApproachWeight + Strafe * 0.35f).GetSafeNormal();
				Speed = FMath::Clamp(GetCharacterMovement()->MaxWalkSpeed * 0.7f, 340.0f, 440.0f);
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
					const float CandidateScore = FVector::DotProduct(CandidateDirection, Approach)
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
		if (bHasLineOfSight && SentryFirstLineOfSightAt < 0.0f)
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
		const bool bCanShoot = (bRidgefireSentry || bRidgefireBrute || Weapon) && bHasLineOfSight && bOpeningThreatReady;
		const bool bCanMelee = (bRidgefireSentry || bRidgefireSprinter || bRidgefireBrute) && bHasLineOfSight && bOpeningThreatReady;
		if (bRidgefireBrute)
		{
			TickRidgefireBrute(Target, bHasLineOfSight && bOpeningThreatReady, Distance, FMath::Abs(ToTarget.Z), CurrentTime);
		}
		else if (bFoundryTurret)
		{
			TickFoundryTurret(Target, bCanShoot, CurrentTime);
		}
		else if (bRidgefireSentry || bRidgefireSprinter)
		{
			const bool bInMeleeRange = Distance <= 235.0f && FMath::Abs(ToTarget.Z) <= 170.0f;
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
					FireSentryShot(Target);
					NextSentryShotTime = CurrentTime + FMath::FRandRange(3.8f, 5.2f);
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
					NextSentryShotTime = CurrentTime + 0.9f;
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
	UGameplayStatics::ApplyDamage(Target, Damage, GetController(), this, UDamageType::StaticClass());
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

	const bool bInMeleeRange = Distance <= 235.0f && VerticalDistance <= 170.0f;
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
	const FVector End = Target->GetActorLocation() + FVector(0.0f, 0.0f, 45.0f);
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

void AShooterNPC::FireSentryShot(AActor* Target, float Damage)
{
	if (!HasAuthority() || (!bRidgefireSentry && !bRidgefireBrute && !bFoundryTurret) || bIsDead || !IsValid(Target))
	{
		return;
	}
	const FVector Start = GetActorLocation() + GetActorForwardVector() * 48.0f + FVector(0.0f, 0.0f, 18.0f);
	const FVector End = Target->GetActorLocation() + FVector(0.0f, 0.0f, 45.0f);
	FCollisionQueryParams Query(SCENE_QUERY_STAT(RidgefireSentryShot), false, this);
	Query.AddIgnoredActor(this);
	FHitResult Hit;
	if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Query) || Hit.GetActor() != Target)
	{
		return;
	}
	UGameplayStatics::ApplyDamage(Target, Damage, GetController(), this, UDamageType::StaticClass());
#if WITH_EDITOR
	OnSentryEditorEvent.Broadcast(TEXT("ShotFired"), Target);
#endif
	Multicast_ShowSentryShotBeam(Start, Hit.ImpactPoint);
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

	float ValidatedDamage = Damage;
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
	CurrentHP -= AppliedDamage;

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
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->StopActiveMovement();
	const bool bUseMechanicalDeathPresentation = bRidgefireSentry || bRidgefireSprinter || bRidgefireBrute || bFoundryTurret;
	if (bUseMechanicalDeathPresentation)
	{
		if (SentinelVisualRoot)
		{
			SentinelVisualRoot->SetVisibility(true, false);
			SentinelVisualRoot->SetRelativeRotation(FRotator(72.0f, 0.0f, 18.0f));
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
