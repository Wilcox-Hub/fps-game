#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RidgefireArenaDressing.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class USceneComponent;
class UActorComponent;

UCLASS()
class TP_FIRSTPERSON_API ARidgefireArenaDressing : public AActor
{
	GENERATED_BODY()

public:
	ARidgefireArenaDressing();
	void BuildArena(const FVector& ArenaCenter, float GroundZ);
	void BuildFoundryArena(const FVector& ArenaCenter, float GroundZ);
	bool GetArsenalStationWorldLocation(FVector& OutLocation) const;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UFUNCTION()
	void OnRep_ArenaBuildState();

private:
	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> SceneRoot;
	UPROPERTY(Replicated)
	FVector ReplicatedArenaCenter = FVector::ZeroVector;
	UPROPERTY(Replicated)
	float ReplicatedGroundZ = 0.0f;
	UPROPERTY(Replicated)
	bool bReplicatedFoundryArena = false;
	UPROPERTY(ReplicatedUsing=OnRep_ArenaBuildState)
	uint8 ReplicatedBuildRevision = 0;
	UPROPERTY(Transient)
	TArray<TObjectPtr<UActorComponent>> GeneratedComponents;

	void RebuildArenaFromReplicatedState();
	void ClearGeneratedComponents();
	void SuppressLegacyBlockout();
	void BuildArenaGeometry(const FVector& ArenaCenter, float GroundZ);
	void BuildFoundryArenaGeometry(const FVector& ArenaCenter, float GroundZ);
	void AddMesh(UStaticMesh* Mesh, const FVector& Location, const FVector& Scale, const FRotator& Rotation, const FLinearColor& Color, UMaterialInterface* BaseMaterial);
	void AddPhysicalMesh(UStaticMesh* Mesh, const FVector& Location, const FVector& Scale, const FRotator& Rotation, const FLinearColor& Color, UMaterialInterface* BaseMaterial);
	void AddArenaFootprint(UStaticMesh* Cube, UMaterialInterface* BaseMaterial, const FLinearColor& FloorColor);
	void AddArsenalStation(const FVector& StationLocation, UStaticMesh* Cube, UStaticMesh* Cylinder, UStaticMesh* Sphere, UMaterialInterface* BaseMaterial);
};
