// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/AI/ShooterNPCSpawner.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ArrowComponent.h"
#include "TimerManager.h"
#include "ShooterNPC.h"
#include "Kismet/GameplayStatics.h"
#include "ShooterGameMode.h"

// Sets default values
AShooterNPCSpawner::AShooterNPCSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	// create the root
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// create the reference spawn capsule
	SpawnCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Spawn Capsule"));
	SpawnCapsule->SetupAttachment(RootComponent);

	SpawnCapsule->SetRelativeLocation(FVector(0.0f, 0.0f, 90.0f));
	SpawnCapsule->SetCapsuleSize(35.0f, 90.0f);
	SpawnCapsule->SetCollisionProfileName(FName("NoCollision"));

	SpawnDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("Spawn Direction"));
	SpawnDirection->SetupAttachment(RootComponent);
}

void AShooterNPCSpawner::BeginPlay()
{
	Super::BeginPlay();
	if (!HasAuthority())
	{
		return;
	}
	
	// ignore if enemies are disabled at the GameMode level
	if (AShooterGameMode* GM = Cast<AShooterGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		if (!GM->ShouldSpawnEnemyNPCs())
		{
			return;
		}
	}

	// ensure we don't spawn NPCs if our initial spawn count is zero
	if (SpawnCount > 0)
	{
		// schedule the first NPC spawn
		GetWorld()->GetTimerManager().SetTimer(SpawnTimer, this, &AShooterNPCSpawner::SpawnNPC, InitialSpawnDelay);
	}
}

void AShooterNPCSpawner::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// clear the spawn timer
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
}

void AShooterNPCSpawner::SpawnNPC()
{
	// ensure the NPC class is valid
	if (HasAuthority() && IsValid(NPCClass))
	{
		// spawn the NPC at the reference capsule's transform
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AShooterNPC* SpawnedNPC = GetWorld()->SpawnActor<AShooterNPC>(NPCClass, SpawnCapsule->GetComponentTransform(), SpawnParams);

		// was the NPC successfully created?
		if (SpawnedNPC)
		{
			const int32 SelectedRoleCount = static_cast<int32>(bSpawnFoundryTurret)
				+ static_cast<int32>(bSpawnRidgefireSprinter)
				+ static_cast<int32>(bSpawnRidgefireBrute);
			if (SelectedRoleCount > 1)
			{
				UE_LOG(LogTemp, Warning, TEXT("RIDGEFIRE: Spawner %s selected multiple optional NPC roles; Foundry Turret, Sprinter, then Brute takes precedence."), *GetName());
			}
			if (bSpawnFoundryTurret)
			{
				SpawnedNPC->EnableFoundryTurret();
			}
			else if (bSpawnRidgefireSprinter)
			{
				SpawnedNPC->EnableRidgefireSprinter();
			}
			else if (bSpawnRidgefireBrute)
			{
				SpawnedNPC->EnableRidgefireBrute();
			}
			// subscribe to the death delegate
			SpawnedNPC->OnPawnDeath.AddDynamic(this, &AShooterNPCSpawner::OnNPCDied);
		}
	}
}

void AShooterNPCSpawner::OnNPCDied(AActor* DeadActor)
{
	(void)DeadActor;
	// decrease the spawn counter
	--SpawnCount;

	// is this the last NPC we should spawn?
	if (SpawnCount <= 0)
	{
		return;
	}

	// schedule the next NPC spawn
	GetWorld()->GetTimerManager().SetTimer(SpawnTimer, this, &AShooterNPCSpawner::SpawnNPC, RespawnDelay);
}
