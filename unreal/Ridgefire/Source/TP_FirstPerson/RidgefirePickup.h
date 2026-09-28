#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RidgefirePickup.generated.h"

class UPointLightComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class TP_FIRSTPERSON_API ARidgefirePickup : public AActor
{
	GENERATED_BODY()

public:
	ARidgefirePickup();
	void Initialize(bool bInHealthPickup);
	bool Collect(class AShooterCharacter* Gladiator);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<USphereComponent> CollectionSphere;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> PickupMesh;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UPointLightComponent> PickupLight;

	UPROPERTY(ReplicatedUsing=OnRep_PickupState)
	bool bHealthPickup = false;

	UPROPERTY(ReplicatedUsing=OnRep_PickupState)
	bool bCollected = false;

	float StartingZ = 0.0f;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	UFUNCTION()
	void OnCollectionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnRep_PickupState();
	void ApplyPickupState();
};
