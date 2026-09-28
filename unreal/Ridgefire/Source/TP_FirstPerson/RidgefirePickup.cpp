#include "RidgefirePickup.h"

#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "ShooterCharacter.h"
#include "Net/UnrealNetwork.h"

ARidgefirePickup::ARidgefirePickup()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	CollectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollectionSphere"));
	SetRootComponent(CollectionSphere);
	CollectionSphere->SetSphereRadius(115.0f);
	CollectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ARidgefirePickup::OnCollectionSphereBeginOverlap);

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	PickupMesh->SetupAttachment(CollectionSphere);
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMesh.Succeeded())
	{
		PickupMesh->SetStaticMesh(SphereMesh.Object);
	}
	PickupMesh->SetRelativeScale3D(FVector(0.32f));

	PickupLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PickupLight"));
	PickupLight->SetupAttachment(PickupMesh);
	PickupLight->SetIntensity(1800.0f);
	PickupLight->SetAttenuationRadius(420.0f);
}

void ARidgefirePickup::Initialize(bool bInHealthPickup)
{
	bHealthPickup = bInHealthPickup;
	ApplyPickupState();
	ForceNetUpdate();
}

void ARidgefirePickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ARidgefirePickup, bHealthPickup);
	DOREPLIFETIME(ARidgefirePickup, bCollected);
}

void ARidgefirePickup::OnRep_PickupState()
{
	ApplyPickupState();
}

void ARidgefirePickup::ApplyPickupState()
{
	if (PickupLight)
	{
		PickupLight->SetLightColor(bHealthPickup ? FLinearColor(1.0f, 0.27f, 0.12f) : FLinearColor(0.14f, 0.92f, 0.78f));
	}
	if (bCollected)
	{
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
	}
}

void ARidgefirePickup::BeginPlay()
{
	Super::BeginPlay();
	StartingZ = GetActorLocation().Z;
	SetLifeSpan(22.0f);
	ApplyPickupState();
#if WITH_EDITOR
	if (!HasAuthority() && FParse::Param(FCommandLine::Get(), TEXT("RidgefireCoopCombatSmoke")))
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP SMOKE PASS client received replicated %s field drop actor."), bHealthPickup ? TEXT("health") : TEXT("ammo"));
	}
#endif
}

void ARidgefirePickup::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	AddActorLocalRotation(FRotator(0.0f, 85.0f * DeltaSeconds, 0.0f));
	FVector Location = GetActorLocation();
	Location.Z = StartingZ + FMath::Sin(GetWorld()->GetTimeSeconds() * 2.6f) * 16.0f;
	SetActorLocation(Location);
}

bool ARidgefirePickup::Collect(AShooterCharacter* Gladiator)
{
	if (!HasAuthority() || bCollected || !Gladiator || Gladiator->IsDead())
	{
		return false;
	}
	if (bHealthPickup)
	{
		if (Gladiator->GetHealthRatio() >= 1.0f)
		{
			return false;
		}
		Gladiator->RestoreHealth(120.0f);
	}
	else
	{
		Gladiator->AddRidgefireReserveAmmo(18);
	}
	bCollected = true;
	ApplyPickupState();
	ForceNetUpdate();
	SetLifeSpan(0.75f);
	return true;
}

void ARidgefirePickup::OnCollectionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AShooterCharacter* Gladiator = Cast<AShooterCharacter>(OtherActor);
	Collect(Gladiator);
}
