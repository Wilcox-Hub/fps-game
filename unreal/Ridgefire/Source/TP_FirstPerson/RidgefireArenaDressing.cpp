#include "RidgefireArenaDressing.h"
#include "Variant_Shooter/Weapons/ShooterWeapon.h"
#include "Variant_Shooter/Weapons/ShooterPickup.h"

#include "Components/SceneComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/SkyLight.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"

ARidgefireArenaDressing::ARidgefireArenaDressing()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ArenaDressingRoot"));
	SetRootComponent(SceneRoot);
}

void ARidgefireArenaDressing::BuildArena(const FVector& ArenaCenter, float GroundZ)
{
	if (!HasAuthority())
	{
		return;
	}
	ReplicatedArenaCenter = ArenaCenter;
	ReplicatedGroundZ = GroundZ;
	bReplicatedFoundryArena = false;
	++ReplicatedBuildRevision;
	SetActorLocation(FVector(ArenaCenter.X, ArenaCenter.Y, GroundZ));
	ForceNetUpdate();
	RebuildArenaFromReplicatedState();
}

void ARidgefireArenaDressing::BuildFoundryArena(const FVector& ArenaCenter, float GroundZ)
{
	if (!HasAuthority())
	{
		return;
	}
	ReplicatedArenaCenter = ArenaCenter;
	ReplicatedGroundZ = GroundZ;
	bReplicatedFoundryArena = true;
	++ReplicatedBuildRevision;
	SetActorLocation(FVector(ArenaCenter.X, ArenaCenter.Y, GroundZ));
	ForceNetUpdate();
	RebuildArenaFromReplicatedState();
}

bool ARidgefireArenaDressing::GetArsenalStationWorldLocation(FVector& OutLocation) const
{
	if (ReplicatedBuildRevision == 0 || bReplicatedFoundryArena)
	{
		return false;
	}
	OutLocation = GetActorTransform().TransformPosition(FVector(0.0f, 620.0f, 0.0f));
	return true;
}

void ARidgefireArenaDressing::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ARidgefireArenaDressing, ReplicatedArenaCenter);
	DOREPLIFETIME(ARidgefireArenaDressing, ReplicatedGroundZ);
	DOREPLIFETIME(ARidgefireArenaDressing, bReplicatedFoundryArena);
	DOREPLIFETIME(ARidgefireArenaDressing, ReplicatedBuildRevision);
}

void ARidgefireArenaDressing::OnRep_ArenaBuildState()
{
	RebuildArenaFromReplicatedState();
}

void ARidgefireArenaDressing::RebuildArenaFromReplicatedState()
{
	if (ReplicatedBuildRevision == 0 || !IsValid(SceneRoot))
	{
		return;
	}
	ClearGeneratedComponents();
	SetActorLocation(FVector(ReplicatedArenaCenter.X, ReplicatedArenaCenter.Y, ReplicatedGroundZ));
	SuppressLegacyBlockout();
	if (bReplicatedFoundryArena)
	{
		BuildFoundryArenaGeometry(ReplicatedArenaCenter, ReplicatedGroundZ);
	}
	else
	{
		BuildArenaGeometry(ReplicatedArenaCenter, ReplicatedGroundZ);
	}
	UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE ARENA STATE role=%s kind=%s revision=%u components=%d center=%s ground=%.1f"),
		HasAuthority() ? TEXT("authority") : TEXT("client"),
	bReplicatedFoundryArena ? TEXT("brassfall") : TEXT("iron-sun"),
	ReplicatedBuildRevision,
	GeneratedComponents.Num(),
	*ReplicatedArenaCenter.ToCompactString(),
	ReplicatedGroundZ);
}

void ARidgefireArenaDressing::ClearGeneratedComponents()
{
	for (UActorComponent* Component : GeneratedComponents)
	{
		if (IsValid(Component))
		{
			Component->DestroyComponent();
		}
	}
	GeneratedComponents.Reset();
}

void ARidgefireArenaDressing::SuppressLegacyBlockout()
{
	UMaterialInterface* SandSurface = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Variant_Shooter/Materials/M_Ridgefire_ArenaSand.M_Ridgefire_ArenaSand"));
	if (!SandSurface)
	{
		SandSurface = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Variant_Shooter/Materials/MI_Ridgefire_SandGrid.MI_Ridgefire_SandGrid"));
	}
	UMaterialInterface* WallSurface = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Variant_Shooter/Materials/M_Ridgefire_ObsidianPanel.M_Ridgefire_ObsidianPanel"));
	if (!WallSurface)
	{
		WallSurface = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Variant_Shooter/Materials/MI_Ridgefire_ObsidianGrid.MI_Ridgefire_ObsidianGrid"));
	}
	int32 SuppressedBlockout = 0;
	int32 RethemedComponents = 0;
	for (TActorIterator<AActor> ActorIt(GetWorld()); ActorIt; ++ActorIt)
	{
		if (*ActorIt == this)
		{
			continue;
		}
		if (AStaticMeshActor* StaticActor = Cast<AStaticMeshActor>(*ActorIt))
		{
			StaticActor->SetActorHiddenInGame(true);
			StaticActor->SetActorEnableCollision(false);
			++SuppressedBlockout;
			continue;
		}
		if (ActorIt->GetClass()->GetName().Contains(TEXT("WobbleTarget")))
		{
			ActorIt->SetActorHiddenInGame(true);
			ActorIt->SetActorEnableCollision(false);
		}
		if (ActorIt->IsA<AShooterWeapon>() && !ActorIt->GetOwner())
		{
			ActorIt->SetActorHiddenInGame(true);
			ActorIt->SetActorEnableCollision(false);
		}
		if (ActorIt->IsA<AShooterPickup>())
		{
			ActorIt->SetActorHiddenInGame(true);
			ActorIt->SetActorEnableCollision(false);
			ActorIt->SetActorTickEnabled(false);
		}
		if (!SandSurface || !WallSurface)
		{
			continue;
		}
		TArray<UStaticMeshComponent*> Components;
		ActorIt->GetComponents<UStaticMeshComponent>(Components);
		for (UStaticMeshComponent* Component : Components)
		{
			if (!IsValid(Component))
			{
				continue;
			}
			const FVector Extents = Component->Bounds.BoxExtent;
			const bool bGroundSurface = Component->Bounds.Origin.Z < ReplicatedGroundZ + 200.0f
				&& Extents.Z < FMath::Max(20.0f, FMath::Min(Extents.X, Extents.Y) * 0.25f);
			for (int32 MaterialIndex = 0; MaterialIndex < Component->GetNumMaterials(); ++MaterialIndex)
			{
				UMaterialInterface* Material = Component->GetMaterial(MaterialIndex);
				if (!Material)
				{
					continue;
				}
				const FString Name = Material->GetName();
				if (Name.Contains(TEXT("PrototypeGrid")) || Name.Contains(TEXT("FlatCol")) || Name.Contains(TEXT("DefaultColorway")) || Name.Contains(TEXT("FirstPersonColorway")))
				{
					Component->SetMaterial(MaterialIndex, bGroundSurface ? SandSurface : WallSurface);
					++RethemedComponents;
				}
			}
		}
	}
	UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE: Client/server arena setup suppressed %d legacy meshes and rethemed %d components."), SuppressedBlockout, RethemedComponents);
}

void ARidgefireArenaDressing::BuildArenaGeometry(const FVector& ArenaCenter, float GroundZ)
{

	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (!Cube || !Sphere || !BaseMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("RIDGEFIRE: Arena dressing could not load one or more engine primitives/materials."));
		return;
	}

	const FLinearColor Obsidian(0.035f, 0.052f, 0.075f, 1.0f);
	const FLinearColor Brass(0.72f, 0.31f, 0.075f, 1.0f);
	const FLinearColor Ember(1.0f, 0.19f, 0.035f, 1.0f);
	const FLinearColor Ice(0.025f, 0.52f, 0.82f, 1.0f);
	AddArenaFootprint(Cube, BaseMaterial, FLinearColor(0.43f, 0.23f, 0.12f, 1.0f));
	for (int32 Index = 0; Index < 8; ++Index)
	{
		const float Angle = Index * 45.0f + 22.5f;
		const FVector Direction = FRotator(0.0f, Angle, 0.0f).Vector();
		const FVector Location = Direction * 2250.0f + FVector(0.0f, 0.0f, 62.5f);
		AddPhysicalMesh(Cube, Location, FVector(2.45f, 0.55f, 1.25f), FRotator(0.0f, Angle + 90.0f, 0.0f), Obsidian, BaseMaterial);
		AddMesh(Cube, Location + FVector(0.0f, 0.0f, 66.0f), FVector(2.48f, 0.58f, 0.045f), FRotator(0.0f, Angle + 90.0f, 0.0f), Index % 2 == 0 ? Brass : Ice, BaseMaterial);
	}
	AddArsenalStation(FVector(0.0f, 620.0f, 0.0f), Cube, LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")), Sphere, BaseMaterial);
	int32 SunLightsTuned = 0;
	for (TActorIterator<ADirectionalLight> LightIt(GetWorld()); LightIt; ++LightIt)
	{
		if (UDirectionalLightComponent* Sun = Cast<UDirectionalLightComponent>(LightIt->GetComponentByClass(UDirectionalLightComponent::StaticClass())))
		{
			FRotator SunRotation = LightIt->GetActorRotation();
			SunRotation.Pitch = -12.0f;
			SunRotation.Roll = 0.0f;
			LightIt->SetActorRotation(SunRotation);
			Sun->SetUseTemperature(true);
			Sun->SetTemperature(3900.0f);
			Sun->SetIntensity(1.8f);
			Sun->SetAtmosphereSunLight(true);
			Sun->SetAtmosphereSunLightIndex(0);
			++SunLightsTuned;
		}
	}
	int32 SkyLightsTuned = 0;
	for (TActorIterator<ASkyLight> LightIt(GetWorld()); LightIt; ++LightIt)
	{
		if (USkyLightComponent* Sky = LightIt->GetLightComponent())
		{
			Sky->SetIntensity(1.4f);
			Sky->SetLightColor(FLinearColor(0.86f, 0.72f, 0.58f));
			++SkyLightsTuned;
		}
	}
	int32 AtmospheresTuned = 0;
	for (TActorIterator<ASkyAtmosphere> AtmosphereIt(GetWorld()); AtmosphereIt; ++AtmosphereIt)
	{
		if (USkyAtmosphereComponent* Atmosphere = AtmosphereIt->GetComponent())
		{
			Atmosphere->SetSkyLuminanceFactor(FLinearColor(0.48f, 0.27f, 0.15f));
			++AtmospheresTuned;
		}
	}
	int32 FogComponentsTuned = 0;
	for (TActorIterator<AExponentialHeightFog> FogIt(GetWorld()); FogIt; ++FogIt)
	{
		if (UExponentialHeightFogComponent* Fog = FogIt->GetComponent())
		{
			Fog->SetFogDensity(0.008f);
			Fog->SetFogInscatteringColor(FLinearColor(0.58f, 0.29f, 0.17f));
			++FogComponentsTuned;
		}
	}
	int32 ExposureVolumesTuned = 0;
	for (TActorIterator<APostProcessVolume> VolumeIt(GetWorld()); VolumeIt; ++VolumeIt)
	{
		FPostProcessSettings& Settings = VolumeIt->Settings;
		Settings.bOverride_AutoExposureBias = true;
		Settings.AutoExposureBias = 0.0f;
		++ExposureVolumesTuned;
	}
	int32 SuppressedTemplateTargets = 0;
	int32 SuppressedTemplateWeapons = 0;
	int32 SuppressedTemplatePickups = 0;
	UMaterialInterface* SandSurface = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Variant_Shooter/Materials/M_Ridgefire_ArenaSand.M_Ridgefire_ArenaSand"));
	if (!SandSurface)
	{
		SandSurface = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Variant_Shooter/Materials/MI_Ridgefire_SandGrid.MI_Ridgefire_SandGrid"));
	}
	UMaterialInterface* WallSurface = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Variant_Shooter/Materials/M_Ridgefire_ObsidianPanel.M_Ridgefire_ObsidianPanel"));
	if (!WallSurface)
	{
		WallSurface = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Variant_Shooter/Materials/MI_Ridgefire_ObsidianGrid.MI_Ridgefire_ObsidianGrid"));
	}
	int32 RethemedComponents = 0;
	int32 SuppressedLegacyBlockout = 0;
	if (SandSurface && WallSurface)
	{
		for (TActorIterator<AActor> ActorIt(GetWorld()); ActorIt; ++ActorIt)
		{
			if (*ActorIt == this)
			{
				continue;
			}
			if (AStaticMeshActor* StaticActor = Cast<AStaticMeshActor>(*ActorIt))
			{
				StaticActor->SetActorHiddenInGame(true);
				StaticActor->SetActorEnableCollision(false);
				++SuppressedLegacyBlockout;
				continue;
			}
			if (ActorIt->GetClass()->GetName().Contains(TEXT("WobbleTarget")))
			{
				ActorIt->SetActorHiddenInGame(true);
				ActorIt->SetActorEnableCollision(false);
				++SuppressedTemplateTargets;
			}
			if (ActorIt->IsA<AShooterWeapon>() && !ActorIt->GetOwner())
			{
				ActorIt->SetActorHiddenInGame(true);
				ActorIt->SetActorEnableCollision(false);
				++SuppressedTemplateWeapons;
			}
			if (ActorIt->IsA<AShooterPickup>())
			{
				ActorIt->SetActorHiddenInGame(true);
				ActorIt->SetActorEnableCollision(false);
				ActorIt->SetActorTickEnabled(false);
				++SuppressedTemplatePickups;
			}

			TArray<UStaticMeshComponent*> Components;
			ActorIt->GetComponents<UStaticMeshComponent>(Components);
			for (UStaticMeshComponent* Component : Components)
			{
				if (!IsValid(Component))
				{
					continue;
				}
				const FVector Extents = Component->Bounds.BoxExtent;
				const float SmallestHorizontalExtent = FMath::Min(Extents.X, Extents.Y);
				const bool bGroundSurface = Component->Bounds.Origin.Z < GroundZ + 200.0f
					&& Extents.Z < FMath::Max(20.0f, SmallestHorizontalExtent * 0.25f);
				for (int32 MaterialIndex = 0; MaterialIndex < Component->GetNumMaterials(); ++MaterialIndex)
				{
					UMaterialInterface* ExistingMaterial = Component->GetMaterial(MaterialIndex);
					if (ExistingMaterial)
					{
						const FString MaterialName = ExistingMaterial->GetName();
						const bool bBlockoutMaterial = MaterialName.Contains(TEXT("PrototypeGrid"))
							|| MaterialName.Contains(TEXT("FlatCol"))
							|| MaterialName.Contains(TEXT("DefaultColorway"))
							|| MaterialName.Contains(TEXT("FirstPersonColorway"));
						if (bBlockoutMaterial)
						{
							Component->SetMaterial(MaterialIndex, bGroundSurface ? SandSurface : WallSurface);
							++RethemedComponents;
						}
					}
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("RIDGEFIRE: Sand/obsidian palette materials could not be loaded; leaving prototype surfaces unchanged."));
	}

	for (int32 Index = 0; Index < 24; ++Index)
	{
		const float Angle = Index * 15.0f;
		const FRotator Rotation(0.0f, Angle, 0.0f);
		const FVector Direction = Rotation.Vector();
		const FVector MarkerLocation = Direction * 2880.0f + FVector(0.0f, 0.0f, 8.0f);
		AddMesh(Cube, MarkerLocation, FVector(0.13f, 0.72f, 0.035f), Rotation, Index % 3 == 0 ? Ember : Brass, BaseMaterial);
	}

	UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE: Built 6400 cm collision arena with eight cover panels and arsenal station; suppressed %d legacy blockout meshes, %d template targets, %d template weapons, and %d template pickups; rethemed %d remaining components; tuned %d suns, %d skylights, %d atmospheres, %d fog components, and %d exposure volumes for sunset."), SuppressedLegacyBlockout, SuppressedTemplateTargets, SuppressedTemplateWeapons, SuppressedTemplatePickups, RethemedComponents, SunLightsTuned, SkyLightsTuned, AtmospheresTuned, FogComponentsTuned, ExposureVolumesTuned);
}

void ARidgefireArenaDressing::BuildFoundryArenaGeometry(const FVector& ArenaCenter, float GroundZ)
{
	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (!Cube || !Cylinder || !Sphere || !BaseMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("RIDGEFIRE: Brassfall dressing could not load one or more engine primitives/materials."));
		return;
	}

	const FLinearColor Iron(0.025f, 0.037f, 0.047f, 1.0f);
	const FLinearColor Brass(0.72f, 0.31f, 0.075f, 1.0f);
	const FLinearColor Furnace(1.0f, 0.17f, 0.025f, 1.0f);
	const FLinearColor Ion(0.025f, 0.62f, 0.78f, 1.0f);
	AddArenaFootprint(Cube, BaseMaterial, FLinearColor(0.13f, 0.17f, 0.19f, 1.0f));

	AddMesh(Cylinder, FVector(0.0f, 0.0f, 14.0f), FVector(19.0f, 19.0f, 0.12f), FRotator::ZeroRotator, Iron, BaseMaterial);
	AddMesh(Cylinder, FVector(0.0f, 0.0f, 28.0f), FVector(15.5f, 15.5f, 0.035f), FRotator::ZeroRotator, Brass, BaseMaterial);
	AddMesh(Cylinder, FVector(0.0f, 0.0f, 62.0f), FVector(5.8f, 5.8f, 0.55f), FRotator::ZeroRotator, Iron, BaseMaterial);
	AddMesh(Cylinder, FVector(0.0f, 0.0f, 168.0f), FVector(4.4f, 4.4f, 1.55f), FRotator::ZeroRotator, Brass, BaseMaterial);
	AddMesh(Sphere, FVector(0.0f, 0.0f, 352.0f), FVector(1.55f), FRotator::ZeroRotator, Furnace, BaseMaterial);
	AddMesh(Sphere, FVector(0.0f, 0.0f, 352.0f), FVector(0.72f), FRotator::ZeroRotator, Ion, BaseMaterial);

	for (int32 Index = 0; Index < 4; ++Index)
	{
		const float Angle = 45.0f + Index * 90.0f;
		const FRotator Rotation(0.0f, Angle, 0.0f);
		const FVector Location = Rotation.Vector() * 1470.0f;
		AddMesh(Cylinder, Location + FVector(0.0f, 0.0f, 12.0f), FVector(1.9f, 1.9f, 0.12f), Rotation, Brass, BaseMaterial);
		AddMesh(Cylinder, Location + FVector(0.0f, 0.0f, 280.0f), FVector(1.1f, 1.1f, 2.7f), Rotation, Iron, BaseMaterial);
		AddMesh(Cube, Location + FVector(0.0f, 0.0f, 520.0f), FVector(0.16f, 1.55f, 0.13f), Rotation, Furnace, BaseMaterial);
		AddMesh(Sphere, Location + FVector(0.0f, 0.0f, 570.0f), FVector(0.5f), FRotator::ZeroRotator, Index % 2 == 0 ? Furnace : Ion, BaseMaterial);
	}

	for (int32 Index = 0; Index < 8; ++Index)
	{
		const float Angle = Index * 45.0f;
		const FRotator Rotation(0.0f, Angle, 0.0f);
		const FVector Location = Rotation.Vector() * 2220.0f;
		AddMesh(Cube, Location + FVector(0.0f, 0.0f, 275.0f), FVector(0.42f, 0.42f, 2.75f), Rotation, Iron, BaseMaterial);
		AddMesh(Cube, Location + FVector(0.0f, 0.0f, 560.0f), FVector(0.58f, 0.58f, 0.12f), Rotation, Index % 2 == 0 ? Furnace : Ion, BaseMaterial);
	}

	for (int32 Index = 0; Index < 4; ++Index)
	{
		const FRotator Rotation(0.0f, Index * 45.0f, 0.0f);
		AddMesh(Cube, FVector(0.0f, 0.0f, 34.0f), FVector(0.28f, 13.8f, 0.08f), Rotation, Index % 2 == 0 ? Furnace : Brass, BaseMaterial);
	}

	UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE: Built 6400 cm collidable Brassfall footprint with a central furnace anchor, four crucible stacks, radial gantry supports, and heated floor lanes."));
}

void ARidgefireArenaDressing::AddArenaFootprint(UStaticMesh* Cube, UMaterialInterface* BaseMaterial, const FLinearColor& FloorColor)
{
	const FLinearColor BoundaryColor(0.025f, 0.037f, 0.047f, 1.0f);
	AddPhysicalMesh(Cube, FVector(0.0f, 0.0f, -70.0f), FVector(64.0f, 64.0f, 1.4f), FRotator::ZeroRotator, FloorColor, BaseMaterial);
	AddPhysicalMesh(Cube, FVector(0.0f, 3100.0f, 130.0f), FVector(62.0f, 0.65f, 2.6f), FRotator::ZeroRotator, BoundaryColor, BaseMaterial);
	AddPhysicalMesh(Cube, FVector(0.0f, -3100.0f, 130.0f), FVector(62.0f, 0.65f, 2.6f), FRotator::ZeroRotator, BoundaryColor, BaseMaterial);
	AddPhysicalMesh(Cube, FVector(3100.0f, 0.0f, 130.0f), FVector(0.65f, 62.0f, 2.6f), FRotator::ZeroRotator, BoundaryColor, BaseMaterial);
	AddPhysicalMesh(Cube, FVector(-3100.0f, 0.0f, 130.0f), FVector(0.65f, 62.0f, 2.6f), FRotator::ZeroRotator, BoundaryColor, BaseMaterial);
}

void ARidgefireArenaDressing::AddArsenalStation(const FVector& StationLocation, UStaticMesh* Cube, UStaticMesh* Cylinder, UStaticMesh* Sphere, UMaterialInterface* BaseMaterial)
{
	if (!Cube || !Cylinder || !Sphere || !BaseMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("RIDGEFIRE: Arsenal station could not load one or more engine primitives/materials."));
		return;
	}

	const FLinearColor Obsidian(0.025f, 0.037f, 0.05f, 1.0f);
	const FLinearColor Brass(0.72f, 0.31f, 0.075f, 1.0f);
	const FLinearColor Phasma(0.62f, 0.82f, 0.9f, 1.0f);
	const FLinearColor Wildcard(1.0f, 0.19f, 0.035f, 1.0f);
	AddPhysicalMesh(Cylinder, StationLocation + FVector(0.0f, 0.0f, 8.0f), FVector(5.8f, 5.8f, 0.18f), FRotator::ZeroRotator, Obsidian, BaseMaterial);
	AddPhysicalMesh(Cylinder, StationLocation + FVector(0.0f, 0.0f, 20.0f), FVector(4.8f, 4.8f, 0.08f), FRotator::ZeroRotator, Brass, BaseMaterial);
	AddPhysicalMesh(Cube, StationLocation + FVector(0.0f, 0.0f, 54.0f), FVector(2.4f, 2.4f, 0.6f), FRotator::ZeroRotator, Obsidian, BaseMaterial);
	AddPhysicalMesh(Cylinder, StationLocation + FVector(0.0f, 0.0f, 145.0f), FVector(0.72f, 0.72f, 1.15f), FRotator::ZeroRotator, Brass, BaseMaterial);
	AddMesh(Sphere, StationLocation + FVector(0.0f, 0.0f, 225.0f), FVector(0.92f), FRotator::ZeroRotator, FLinearColor(0.025f, 0.62f, 0.82f, 1.0f), BaseMaterial);
	AddMesh(Cylinder, StationLocation + FVector(0.0f, 0.0f, 226.0f), FVector(0.96f, 0.96f, 0.055f), FRotator::ZeroRotator, Phasma, BaseMaterial);

	const FLinearColor DisplayColors[] = {FLinearColor(0.025f, 0.62f, 0.82f, 1.0f), Phasma, Wildcard};
	for (int32 Index = 0; Index < 3; ++Index)
	{
		const float Angle = 30.0f + Index * 120.0f;
		const FVector DisplayLocation = StationLocation + FRotator(0.0f, Angle, 0.0f).Vector() * 176.0f;
		AddPhysicalMesh(Cube, DisplayLocation + FVector(0.0f, 0.0f, 96.0f), FVector(0.74f, 0.74f, 0.24f), FRotator(0.0f, Angle, 0.0f), Brass, BaseMaterial);
		AddMesh(Cube, DisplayLocation + FVector(0.0f, 0.0f, 146.0f), FVector(0.22f, 0.22f, 0.62f), FRotator(0.0f, Angle, 0.0f), DisplayColors[Index], BaseMaterial);
		AddMesh(Sphere, DisplayLocation + FVector(0.0f, 0.0f, 210.0f), FVector(0.3f), FRotator::ZeroRotator, DisplayColors[Index], BaseMaterial);
	}

	for (int32 Index = 0; Index < 4; ++Index)
	{
		const float Angle = Index * 90.0f;
		UTextRenderComponent* Sign = NewObject<UTextRenderComponent>(this);
		if (!Sign)
		{
			continue;
		}
		Sign->SetIsReplicated(false);
		Sign->SetNetAddressable();
		Sign->SetupAttachment(SceneRoot);
		Sign->SetMobility(EComponentMobility::Movable);
		Sign->SetRelativeLocation(StationLocation + FRotator(0.0f, Angle, 0.0f).Vector() * 250.0f + FVector(0.0f, 0.0f, 340.0f));
		Sign->SetRelativeRotation(FRotator(0.0f, Angle + 180.0f, 0.0f));
	Sign->SetText(FText::FromString(TEXT("ARSENAL\nT / VIEW TO TRADE")));
		Sign->SetTextRenderColor(FColor(135, 225, 255));
		Sign->SetWorldSize(42.0f);
		Sign->SetHorizontalAlignment(EHTA_Center);
		Sign->SetVerticalAlignment(EVRTA_TextCenter);
		Sign->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Sign->RegisterComponent();
		GeneratedComponents.Add(Sign);
	}

	UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE: Built central Arsenal station with three weapon display plinths and four T/View interaction signs."));
}

void ARidgefireArenaDressing::AddPhysicalMesh(UStaticMesh* Mesh, const FVector& Location, const FVector& Scale, const FRotator& Rotation, const FLinearColor& Color, UMaterialInterface* BaseMaterial)
{
	if (!Mesh || !BaseMaterial)
	{
		return;
	}

	UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(this);
	if (!Part)
	{
		return;
	}
	Part->SetIsReplicated(false);
	Part->SetNetAddressable();
	Part->SetStaticMesh(Mesh);
	Part->SetupAttachment(SceneRoot);
	Part->SetMobility(EComponentMobility::Movable);
	Part->SetRelativeLocation(Location);
	Part->SetRelativeScale3D(Scale);
	Part->SetRelativeRotation(Rotation);
	Part->SetCollisionProfileName(TEXT("BlockAll"));
	Part->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Part->SetGenerateOverlapEvents(false);
	Part->CanCharacterStepUpOn = ECB_Yes;
	Part->CastShadow = true;
	if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, this))
	{
		Material->SetVectorParameterValue(TEXT("Color"), Color);
		Material->SetVectorParameterValue(TEXT("BaseColor"), Color);
		Material->SetVectorParameterValue(TEXT("EmissiveColor"), Color * 0.08f);
		Material->SetScalarParameterValue(TEXT("Emissive"), 0.08f);
		Material->SetScalarParameterValue(TEXT("Metallic"), 0.68f);
		Material->SetScalarParameterValue(TEXT("Roughness"), 0.36f);
		Part->SetMaterial(0, Material);
	}
	Part->RegisterComponent();
	GeneratedComponents.Add(Part);
}

void ARidgefireArenaDressing::AddMesh(UStaticMesh* Mesh, const FVector& Location, const FVector& Scale, const FRotator& Rotation, const FLinearColor& Color, UMaterialInterface* BaseMaterial)
{
	if (!Mesh || !BaseMaterial)
	{
		return;
	}

	UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(this);
	if (!Part)
	{
		return;
	}
	Part->SetIsReplicated(false);
	Part->SetNetAddressable();
	Part->SetStaticMesh(Mesh);
	Part->SetupAttachment(SceneRoot);
	Part->SetMobility(EComponentMobility::Movable);
	Part->SetRelativeLocation(Location);
	Part->SetRelativeScale3D(Scale);
	Part->SetRelativeRotation(Rotation);
	Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Part->SetGenerateOverlapEvents(false);
	Part->CastShadow = false;
	if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, this))
	{
		Material->SetVectorParameterValue(TEXT("Color"), Color);
		Material->SetVectorParameterValue(TEXT("BaseColor"), Color);
		Material->SetVectorParameterValue(TEXT("EmissiveColor"), Color * 2.5f);
		Material->SetScalarParameterValue(TEXT("Emissive"), 2.5f);
		Material->SetScalarParameterValue(TEXT("Metallic"), 0.55f);
		Material->SetScalarParameterValue(TEXT("Roughness"), 0.32f);
		Part->SetMaterial(0, Material);
	}
	Part->RegisterComponent();
	GeneratedComponents.Add(Part);
}
