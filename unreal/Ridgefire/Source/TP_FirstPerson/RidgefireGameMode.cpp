#include "RidgefireGameMode.h"
#include "RidgefireHUD.h"
#include "RidgefirePickup.h"
#include "RidgefireArenaDressing.h"
#include "Variant_Shooter/ShooterPlayerController.h"

#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "ShooterCharacter.h"
#include "ShooterPlayerController.h"
#include "TimerManager.h"
#include "Variant_Shooter/AI/ShooterNPC.h"
#include "Variant_Shooter/AI/ShooterNPCSpawner.h"

namespace
{
	constexpr float BrassfallEntryReserveRefillFraction = 0.10f;

	struct FWeaponModeOption
	{
		ERidgefireFireMode Mode;
		const TCHAR* Name;
		TArray<FString> Words;
	};

	FString DescribeModeEffect(ERidgefireFireMode Mode, int32 Pellets)
	{
		switch (Mode)
		{
		case ERidgefireFireMode::Auto: return TEXT("Rapid single-target shots");
		case ERidgefireFireMode::Scatter: return FString::Printf(TEXT("%d spread pellets"), Pellets);
		case ERidgefireFireMode::Pierce: return TEXT("Pierces targets in a line");
		case ERidgefireFireMode::Arc: return TEXT("Chains to up to 4 nearby sentries");
		case ERidgefireFireMode::Burst: return TEXT("Repeats 3 hits per trigger");
		case ERidgefireFireMode::Mortar: return TEXT("280 cm blast on direct hit");
		case ERidgefireFireMode::Echo: return TEXT("Hits the target twice");
		case ERidgefireFireMode::Roulette: return TEXT("Wild aim and wildly rolled damage");
		case ERidgefireFireMode::Phasma: return TEXT("Chrome plasma, 460 cm shock blast");
		default: return TEXT("Single-target shot");
		}
	}

	FString DescribePayloadEffect(ERidgefirePayload Payload)
	{
		switch (Payload)
		{
		case ERidgefirePayload::Burn: return TEXT("adds 28% hit damage");
		case ERidgefirePayload::Frost: return TEXT("slows targets to 40% speed for 2.5s");
		case ERidgefirePayload::Siphon: return TEXT("heals 8 on hit; kills heal an extra 2, 8, 20, or 45");
		case ERidgefirePayload::Salvage: return TEXT("kills roll 2-30 reserve rounds");
		case ERidgefirePayload::Volatile: return TEXT("kills explode for 10-75% damage nearby");
		case ERidgefirePayload::Ricochet: return TEXT("can jump to a target within 220 cm");
		case ERidgefirePayload::Split: return TEXT("can jump to a target within 330 cm");
		case ERidgefirePayload::Glass: return TEXT("damage wildly rolls from 0.12x to 8x");
		case ERidgefirePayload::Leaden: return TEXT("deals 38%; 15% chance to hurt you");
		default: return TEXT("no added payload effect");
		}
	}

	FString DescribeConditionEffect(ERidgefireCondition Condition)
	{
		switch (Condition)
		{
		case ERidgefireCondition::Close: return TEXT("4x inside 240 cm; 0.35x farther");
		case ERidgefireCondition::Longshot: return TEXT("3.5x beyond 700 cm; 0.45x closer");
		case ERidgefireCondition::Sprint: return TEXT("2.8x while sprinting; 0.5x otherwise");
		case ERidgefireCondition::LowHealth: return TEXT("4.5x below 28% health; 0.6x otherwise");
		case ERidgefireCondition::FullHealth: return TEXT("3x above 85% health; 0.4x otherwise");
		case ERidgefireCondition::LastShot: return TEXT("7x on an emptying shot; 0.65x otherwise");
		case ERidgefireCondition::Streak: return TEXT("3x at 3+ streak; 0.65x otherwise");
		case ERidgefireCondition::AfterReload: return TEXT("5x after reload; 0.55x otherwise");
		case ERidgefireCondition::Desperate: return TEXT("4x at 2 or fewer rounds; 0.55x otherwise");
		default: return TEXT("no trigger condition");
		}
	}

	int32 PickIndex(int32 Count, FRandomStream* RandomStream = nullptr)
	{
		return RandomStream ? RandomStream->RandRange(0, Count - 1) : FMath::RandRange(0, Count - 1);
	}

#if WITH_EDITOR
	bool bSmokeTestRestarted = false;
	bool bSmokeTestRunComplete = false;

	bool ExecuteSmokeInputBinding(UInputComponent* InputComponent, const FKey& Key, EInputEvent Event, bool bRequirePauseBinding = false)
	{
		if (!InputComponent)
		{
			return false;
		}
		for (const FInputKeyBinding& Binding : InputComponent->KeyBindings)
		{
			if (Binding.Chord.Key == Key && Binding.KeyEvent == Event && (!bRequirePauseBinding || Binding.bExecuteWhenPaused))
			{
				Binding.KeyDelegate.Execute(Key);
				return true;
			}
		}
		return false;
	}

	bool ExecuteSmokeInputBinding(AShooterPlayerController* Controller, const FKey& Key, EInputEvent Event, bool bRequirePauseBinding = false)
	{
		return ExecuteSmokeInputBinding(Controller ? Controller->InputComponent : nullptr, Key, Event, bRequirePauseBinding);
	}
#endif
}

ARidgefireGameMode::ARidgefireGameMode()
{
	PickupClass = ARidgefirePickup::StaticClass();
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClass(TEXT("/Game/Variant_Shooter/Blueprints/BP_ShooterCharacter"));
	if (PlayerPawnClass.Succeeded())
	{
		DefaultPawnClass = PlayerPawnClass.Class;
	}

	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerClassAsset(TEXT("/Game/Variant_Shooter/Blueprints/BP_ShooterPlayerController"));
	if (PlayerControllerClassAsset.Succeeded())
	{
		PlayerControllerClass = PlayerControllerClassAsset.Class;
	}

	static ConstructorHelpers::FClassFinder<AShooterNPC> SentinelBlueprint(TEXT("/Game/Variant_Shooter/Blueprints/AI/BP_ShooterNPC"));
	if (SentinelBlueprint.Succeeded())
	{
		SentinelClass = SentinelBlueprint.Class;
	}

	HUDClass = ARidgefireHUD::StaticClass();
}

void ARidgefireGameMode::BeginPlay()
{
	AGameModeBase::BeginPlay();

	TArray<AActor*> Spawners;
	UGameplayStatics::GetAllActorsOfClass(this, AShooterNPCSpawner::StaticClass(), Spawners);
	for (AActor* Spawner : Spawners)
	{
		if (IsValid(Spawner))
		{
			SentinelEntrances.Add(Spawner->GetActorTransform());
			Spawner->Destroy();
		}
	}

	if (SentinelEntrances.IsEmpty())
	{
		FVector InitialArenaCenter = FVector::ZeroVector;
		if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
		{
			if (APawn* PlayerPawn = PlayerController->GetPawn())
			{
				InitialArenaCenter = PlayerPawn->GetActorLocation();
			}
		}
		if (InitialArenaCenter.IsNearlyZero())
		{
			if (TActorIterator<APlayerStart> StartIt(GetWorld()); StartIt)
			{
				InitialArenaCenter = StartIt->GetActorLocation();
			}
		}

		for (int32 Index = 0; Index < 8; ++Index)
		{
			const FRotator Direction(0.0f, Index * 45.0f, 0.0f);
			const FVector SpawnLocation = InitialArenaCenter + Direction.Vector() * 1600.0f + FVector(0.0f, 0.0f, 20.0f);
			SentinelEntrances.Add(FTransform((Direction + FRotator(0.0f, 180.0f, 0.0f)).GetNormalized(), SpawnLocation));
		}
	}
	else
	{
		FVector InitialArenaCenter = FVector::ZeroVector;
		if (TActorIterator<APlayerStart> StartIt(GetWorld()); StartIt)
		{
			InitialArenaCenter = StartIt->GetActorLocation();
		}
		for (int32 Index = SentinelEntrances.Num(); Index < 8; ++Index)
		{
			const FRotator Direction(0.0f, Index * 45.0f, 0.0f);
			const FVector SpawnLocation = InitialArenaCenter + Direction.Vector() * 1500.0f + FVector(0.0f, 0.0f, 30.0f);
			SentinelEntrances.Add(FTransform((Direction + FRotator(0.0f, 180.0f, 0.0f)).GetNormalized(), SpawnLocation));
		}
	}

	FVector InitialArenaCenter = FVector::ZeroVector;
	float GroundZ = 0.0f;
	if (const AShooterCharacter* StartingPlayer = GetGladiator())
	{
		InitialArenaCenter = StartingPlayer->GetActorLocation();
		GroundZ = InitialArenaCenter.Z - StartingPlayer->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	}
	else if (TActorIterator<APlayerStart> StartIt(GetWorld()); StartIt)
	{
		InitialArenaCenter = StartIt->GetActorLocation();
		GroundZ = InitialArenaCenter.Z - 90.0f;
	}
	else if (!SentinelEntrances.IsEmpty())
	{
		InitialArenaCenter = SentinelEntrances[0].GetLocation();
		GroundZ = InitialArenaCenter.Z - 90.0f;
	}
	this->ArenaCenter = InitialArenaCenter;
	ArenaGroundZ = GroundZ;
	if (ARidgefireArenaDressing* Dressing = GetWorld()->SpawnActor<ARidgefireArenaDressing>())
	{
		Dressing->BuildArena(InitialArenaCenter, GroundZ);
		ActiveArenaDressing = Dressing;
	}

	CreateWeaponOffers();
	GetWorldTimerManager().SetTimer(ArmoryTimeoutTimer, this, &ARidgefireGameMode::AutoSelectStartingWeapon, 12.0f, false);
	GetWorldTimerManager().SetTimer(ArmoryStateTimer, this, &ARidgefireGameMode::RefreshRidgefireRunStates, 1.0f, true);
#if WITH_EDITOR
	bRunSmokeTest = FParse::Param(FCommandLine::Get(), TEXT("RidgefireSmokeTest"));
	bRunCoopCombatSmokeTest = FParse::Param(FCommandLine::Get(), TEXT("RidgefireCoopCombatSmoke"));
	if (bRunCoopCombatSmokeTest && HasAuthority())
	{
		GetWorldTimerManager().SetTimer(CoopCombatSmokeTimer, this, &ARidgefireGameMode::RunCoopCombatSmokeTest, 0.25f, true);
	}
	if (bRunSmokeTest && bSmokeTestRunComplete)
	{
		bSmokeTestRestarted = false;
		bSmokeTestRunComplete = false;
	}
	if (bRunSmokeTest && bSmokeTestRestarted)
	{
		GetWorldTimerManager().SetTimer(SmokeTestTimer, this, &ARidgefireGameMode::VerifyRestartSmokeTest, 2.0f, false);
	}
#endif
	UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE: Opening armory with %d offers and %d sentinel entrances."), WeaponOffers.Num(), SentinelEntrances.Num());
	RefreshRidgefireRunStates();
}

void ARidgefireGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (AShooterPlayerController* PlayerController = Cast<AShooterPlayerController>(NewPlayer))
	{
		GetOrCreatePlayerWeaponLoadout(PlayerController);
		SendRidgefireRunState(PlayerController);
	}
}

FRidgefirePlayerWeaponLoadout& ARidgefireGameMode::GetOrCreatePlayerWeaponLoadout(AShooterPlayerController* PlayerController)
{
	FRidgefirePlayerWeaponLoadout& Loadout = PlayerWeaponLoadouts.FindOrAdd(TWeakObjectPtr<AShooterPlayerController>(PlayerController));
	if (Loadout.PrimaryWeaponSlots.Num() != 2)
	{
		Loadout.PrimaryWeaponSlots.Init(INDEX_NONE, 2);
	}
	const int32 PreviousAmmoCount = Loadout.WeaponAmmo.Num();
	Loadout.WeaponAmmo.SetNum(WeaponOffers.Num());
	Loadout.WeaponReserveAmmo.SetNum(WeaponOffers.Num());
	for (int32 OfferIndex = PreviousAmmoCount; OfferIndex < WeaponOffers.Num(); ++OfferIndex)
	{
		Loadout.WeaponAmmo[OfferIndex] = WeaponOffers[OfferIndex].MagazineSize;
		Loadout.WeaponReserveAmmo[OfferIndex] = WeaponOffers[OfferIndex].ReserveAmmo;
	}
	return Loadout;
}

bool ARidgefireGameMode::AreCurrentPlayersReadyForWave() const
{
	if (!GetWorld())
	{
		return false;
	}

	int32 ReadyPlayers = 0;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AShooterPlayerController* PlayerController = Cast<AShooterPlayerController>(It->Get());
		if (!IsValid(PlayerController))
		{
			continue;
		}

		if (!Cast<AShooterCharacter>(PlayerController->GetPawn()))
		{
			return false;
		}

		const FRidgefirePlayerWeaponLoadout* Loadout = PlayerWeaponLoadouts.Find(TWeakObjectPtr<AShooterPlayerController>(PlayerController));
		if (!Loadout || !Loadout->PrimaryWeaponSlots.IsValidIndex(0) || !WeaponOffers.IsValidIndex(Loadout->PrimaryWeaponSlots[0]))
		{
			return false;
		}
		++ReadyPlayers;
	}

	return ReadyPlayers > 0;
}

void ARidgefireGameMode::MirrorFirstPlayerWeaponLoadout(AShooterPlayerController* PlayerController, const FRidgefirePlayerWeaponLoadout& Loadout)
{
	if (GetWorld() && GetWorld()->GetFirstPlayerController() == PlayerController)
	{
		PrimaryWeaponSlots = Loadout.PrimaryWeaponSlots;
		WeaponAmmo = Loadout.WeaponAmmo;
		WeaponReserveAmmo = Loadout.WeaponReserveAmmo;
		ActiveWeaponIndex = Loadout.ActiveWeaponIndex;
	}
}

void ARidgefireGameMode::CreateWeaponOffers(FRandomStream* TestRandomStream)
{
	WeaponOffers.Reset();
	WeaponAmmo.Reset();
	WeaponReserveAmmo.Reset();
	PrimaryWeaponSlots.Init(INDEX_NONE, 2);
	ActiveWeaponIndex = INDEX_NONE;

	FRidgefireWeaponRoll Gale;
	Gale.Name = TEXT("Gale Repeater");
	Gale.Description = TEXT("Fast, clean shots. Reliable until the crowd gets close.");
	Gale.ModeName = TEXT("REPEATER");
	Gale.PayloadName = TEXT("BARE METAL");
	Gale.ConditionName = TEXT("ALWAYS");
	Gale.Mode = ERidgefireFireMode::Auto;
	Gale.Payload = ERidgefirePayload::Plain;
	Gale.Condition = ERidgefireCondition::Steady;
	Gale.DamageScale = 1.0f;
	Gale.RefireDelay = 0.16f;
	Gale.MagazineSize = 24;
	Gale.ReserveAmmo = 144;
	WeaponOffers.Add(Gale);

	FRidgefireWeaponRoll Phasma;
	Phasma.Name = TEXT("Phasma Rifle");
	Phasma.Description = TEXT("Mirror-chrome plasma. Heavy shots collapse into a bassy shock ring.");
	Phasma.ModeName = TEXT("PHASMA BLASTER");
	Phasma.PayloadName = TEXT("BARE METAL");
	Phasma.ConditionName = TEXT("ALWAYS");
	Phasma.Mode = ERidgefireFireMode::Phasma;
	Phasma.Payload = ERidgefirePayload::Plain;
	Phasma.Condition = ERidgefireCondition::Steady;
	Phasma.DamageScale = 2.8f;
	Phasma.RefireDelay = 0.78f;
	Phasma.MagazineSize = 6;
	Phasma.ReserveAmmo = 30;
	WeaponOffers.Add(Phasma);

	const TArray<FWeaponModeOption> Modes = {
		{ERidgefireFireMode::Auto, TEXT("REPEATER"), {TEXT("rapid"), TEXT("steady"), TEXT("automatic")}},
		{ERidgefireFireMode::Scatter, TEXT("SCATTER"), {TEXT("wide"), TEXT("shattered"), TEXT("many-barreled")}},
		{ERidgefireFireMode::Pierce, TEXT("RAIL LANCE"), {TEXT("piercing"), TEXT("rail-fed"), TEXT("needlepoint")}},
		{ERidgefireFireMode::Arc, TEXT("ARC COIL"), {TEXT("chaining"), TEXT("forked"), TEXT("storm-fed")}},
		{ERidgefireFireMode::Burst, TEXT("BURST DRIVER"), {TEXT("triplet"), TEXT("stuttering"), TEXT("three-beat")}},
		{ERidgefireFireMode::Mortar, TEXT("MORTAR"), {TEXT("lobbed"), TEXT("volatile"), TEXT("thunderous")}},
		{ERidgefireFireMode::Echo, TEXT("ECHO GUN"), {TEXT("echoing"), TEXT("double-image"), TEXT("aftershock")}},
		{ERidgefireFireMode::Roulette, TEXT("MISFIRE"), {TEXT("unlicensed"), TEXT("unpredictable"), TEXT("questionable")}},
		{ERidgefireFireMode::Phasma, TEXT("PHASMA BLASTER"), {TEXT("chrome"), TEXT("seismic"), TEXT("starfall")}}
	};
	const TArray<TPair<ERidgefirePayload, FString>> Payloads = {
		{ERidgefirePayload::Burn, TEXT("SUNFIRE")},
		{ERidgefirePayload::Frost, TEXT("COLD IRON")},
		{ERidgefirePayload::Siphon, TEXT("BLOOD TITHE")},
		{ERidgefirePayload::Salvage, TEXT("SCRAP GOSPEL")},
		{ERidgefirePayload::Volatile, TEXT("LAST RITES")},
		{ERidgefirePayload::Ricochet, TEXT("PINBALL SAINT")},
		{ERidgefirePayload::Split, TEXT("MANY MOUTHS")},
		{ERidgefirePayload::Glass, TEXT("GLASS VERDICT")},
		{ERidgefirePayload::Leaden, TEXT("LEAD SERMON")}
	};
	const TArray<TPair<ERidgefireCondition, FString>> Conditions = {
		{ERidgefireCondition::Close, TEXT("KISS THE TARGET")},
		{ERidgefireCondition::Longshot, TEXT("ACROSS THE PIT")},
		{ERidgefireCondition::Sprint, TEXT("RUN LIKE PREY")},
		{ERidgefireCondition::LowHealth, TEXT("LAST BREATH")},
		{ERidgefireCondition::FullHealth, TEXT("UNTOUCHED")},
		{ERidgefireCondition::LastShot, TEXT("ONE IN THE CHAMBER")},
		{ERidgefireCondition::Streak, TEXT("BLOOD REMEMBERS")},
		{ERidgefireCondition::AfterReload, TEXT("RELOAD RITUAL")},
		{ERidgefireCondition::Desperate, TEXT("BAD ODDS")}
	};
	const TArray<FString> Adjectives = {TEXT("Saint"), TEXT("Problem"), TEXT("Promise"), TEXT("Mistake"), TEXT("Encore"), TEXT("Inheritance"), TEXT("Last Word"), TEXT("Miracle"), TEXT("Bad Idea"), TEXT("Omen"), TEXT("Gambit"), TEXT("Apology")};
	const TArray<FString> Nouns = {TEXT("of the Pit"), TEXT("from Nowhere"), TEXT("No. 7"), TEXT("the Unmaker"), TEXT("of Questionable Origin"), TEXT("That Hums"), TEXT("for a Friend"), TEXT("Mk. Maybe")};
	const TArray<float> DamageRolls = {0.25f, 0.4f, 0.65f, 0.8f, 1.0f, 1.2f, 1.6f, 2.4f, 4.0f, 7.0f};
	const TArray<float> TempoRolls = {0.38f, 0.55f, 0.75f, 1.0f, 1.0f, 1.25f, 1.8f, 2.7f};
	const TArray<int32> MagazineRolls = {3, 5, 8, 13, 21, 34};
	const TArray<int32> PelletRolls = {3, 5, 7, 11};
	const TArray<int32> ReserveRolls = {2, 4, 6, 9};

	const FWeaponModeOption& Mode = Modes[PickIndex(Modes.Num(), TestRandomStream)];
	const TPair<ERidgefirePayload, FString>& Payload = Payloads[PickIndex(Payloads.Num(), TestRandomStream)];
	const bool bConditional = (TestRandomStream ? TestRandomStream->FRand() : FMath::FRand()) < 0.76f;
	const TPair<ERidgefireCondition, FString>* Condition = bConditional ? &Conditions[PickIndex(Conditions.Num(), TestRandomStream)] : nullptr;
	const float DamageScale = DamageRolls[PickIndex(DamageRolls.Num(), TestRandomStream)];
	const float TempoScale = TempoRolls[PickIndex(TempoRolls.Num(), TestRandomStream)];
	const int32 Magazine = MagazineRolls[PickIndex(MagazineRolls.Num(), TestRandomStream)];
	const int32 Pellets = PelletRolls[PickIndex(PelletRolls.Num(), TestRandomStream)];
	const FString Name = FString::Printf(TEXT("%s %s %s"), *Mode.Words[PickIndex(Mode.Words.Num(), TestRandomStream)], *Adjectives[PickIndex(Adjectives.Num(), TestRandomStream)], *Nouns[PickIndex(Nouns.Num(), TestRandomStream)]);

	FRidgefireWeaponRoll Wildcard;
	Wildcard.Name = Name;
	Wildcard.ModeName = Mode.Name;
	Wildcard.PayloadName = Payload.Value;
	Wildcard.ConditionName = Condition ? Condition->Value : TEXT("ALWAYS");
	Wildcard.Mode = Mode.Mode;
	Wildcard.Payload = Payload.Key;
	Wildcard.Condition = Condition ? Condition->Key : ERidgefireCondition::Steady;
	Wildcard.DamageScale = DamageScale;
	Wildcard.RefireDelay = FMath::Max(0.055f, 0.19f * TempoScale);
	Wildcard.MagazineSize = Magazine;
	Wildcard.ReserveAmmo = Magazine * ReserveRolls[PickIndex(ReserveRolls.Num(), TestRandomStream)];
	Wildcard.Pellets = Pellets;
	Wildcard.Description = FString::Printf(TEXT("%s. %s. %s."), *DescribeModeEffect(Wildcard.Mode, Wildcard.Pellets), *DescribePayloadEffect(Wildcard.Payload), Condition ? *DescribeConditionEffect(Wildcard.Condition) : TEXT("No trigger condition"));
	Wildcard.bWildcard = true;
	WeaponOffers.Add(Wildcard);
}

void ARidgefireGameMode::SelectWeaponOffer(int32 Index)
{
	SelectWeaponOffer(Index, INDEX_NONE);
}

void ARidgefireGameMode::SelectWeaponOffer(int32 Index, int32 ReplacementSlot)
{
	SelectWeaponOffer(GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr, Index, ReplacementSlot);
}

void ARidgefireGameMode::SelectWeaponOffer(APlayerController* PlayerController, int32 Index, int32 ReplacementSlot)
{
	AShooterPlayerController* RidgefireController = Cast<AShooterPlayerController>(PlayerController);
	if (!HasAuthority() || !IsValid(RidgefireController) || RidgefireController->GetWorld() != GetWorld() || bRunOver || !WeaponOffers.IsValidIndex(Index))
	{
		return;
	}

	AShooterCharacter* Gladiator = Cast<AShooterCharacter>(RidgefireController->GetPawn());
	if (!IsValid(Gladiator))
	{
		UE_LOG(LogTemp, Warning, TEXT("RIDGEFIRE: Deferring armory selection until player %s is ready."), *GetNameSafe(RidgefireController));
		return;
	}

	FRidgefirePlayerWeaponLoadout& Loadout = GetOrCreatePlayerWeaponLoadout(RidgefireController);
	const bool bFirstSelection = Loadout.PrimaryWeaponSlots[0] == INDEX_NONE && Loadout.PrimaryWeaponSlots[1] == INDEX_NONE;
	if (bFirstSelection)
	{
		Loadout.PrimaryWeaponSlots[0] = Index;
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE: Player %s selected starting offer %d (%s)."), *GetNameSafe(RidgefireController), Index + 1, *WeaponOffers[Index].Name);
	}
	else if (Loadout.ActiveWeaponIndex == Index)
	{
		return;
	}
	else if (!Loadout.PrimaryWeaponSlots.Contains(Index))
	{
		const int32 EmptySlot = Loadout.PrimaryWeaponSlots.IndexOfByKey(INDEX_NONE);
		if (EmptySlot != INDEX_NONE)
		{
			Loadout.PrimaryWeaponSlots[EmptySlot] = Index;
		}
		else if (Loadout.PrimaryWeaponSlots.IsValidIndex(ReplacementSlot))
		{
			Loadout.PrimaryWeaponSlots[ReplacementSlot] = Index;
		}
		else
		{
			return;
		}
	}
	if (Loadout.WeaponAmmo.IsValidIndex(Loadout.ActiveWeaponIndex))
	{
		Loadout.WeaponAmmo[Loadout.ActiveWeaponIndex] = Gladiator->GetRidgefireAmmo();
		Loadout.WeaponReserveAmmo[Loadout.ActiveWeaponIndex] = Gladiator->GetRidgefireReserveAmmo();
	}
	Loadout.ActiveWeaponIndex = Index;
	Gladiator->EquipRidgefireWeapon(WeaponOffers[Index], Loadout.WeaponAmmo[Index], Loadout.WeaponReserveAmmo[Index]);
	MirrorFirstPlayerWeaponLoadout(RidgefireController, Loadout);
	if (bArmoryOpen && AreCurrentPlayersReadyForWave())
	{
		bArmoryOpen = false;
		GetWorldTimerManager().ClearTimer(ArmoryTimeoutTimer);
		GetWorldTimerManager().SetTimer(WaveTimer, this, &ARidgefireGameMode::BeginNextWave, 2.5f, false);
	}
	if (!bArmoryOpen)
	{
		GetWorldTimerManager().ClearTimer(ArmoryStateTimer);
	}
	RefreshRidgefireRunStates();
}

void ARidgefireGameMode::CycleWeaponOffer()
{
	CycleWeaponOffer(GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr);
}

void ARidgefireGameMode::CycleWeaponOffer(APlayerController* PlayerController)
{
	AShooterPlayerController* RidgefireController = Cast<AShooterPlayerController>(PlayerController);
	if (!HasAuthority() || !IsValid(RidgefireController) || bArmoryOpen || bRunOver)
	{
		return;
	}
	const FRidgefirePlayerWeaponLoadout* Loadout = PlayerWeaponLoadouts.Find(TWeakObjectPtr<AShooterPlayerController>(RidgefireController));
	if (!Loadout || Loadout->PrimaryWeaponSlots.Num() < 2)
	{
		return;
	}
	const int32 CurrentSlot = Loadout->PrimaryWeaponSlots.IndexOfByKey(Loadout->ActiveWeaponIndex);
	const int32 NextSlot = CurrentSlot == 0 ? 1 : 0;
	if (Loadout->PrimaryWeaponSlots.IsValidIndex(NextSlot) && WeaponOffers.IsValidIndex(Loadout->PrimaryWeaponSlots[NextSlot]))
	{
		SelectWeaponOffer(RidgefireController, Loadout->PrimaryWeaponSlots[NextSlot]);
	}
}

void ARidgefireGameMode::SelectSecondaryWeaponOffer(APlayerController* PlayerController, int32 Index)
{
	AShooterPlayerController* RidgefireController = Cast<AShooterPlayerController>(PlayerController);
	if (!HasAuthority() || !IsValid(RidgefireController) || RidgefireController->GetWorld() != GetWorld() || bRunOver
		|| !WeaponOffers.IsValidIndex(Index))
	{
		return;
	}

	FRidgefirePlayerWeaponLoadout& Loadout = GetOrCreatePlayerWeaponLoadout(RidgefireController);
	if (Loadout.PrimaryWeaponSlots.Contains(Index))
	{
		return;
	}

	Loadout.SecondaryWeaponOfferIndex = Index;
	SendRidgefireRunState(RidgefireController);
}

bool ARidgefireGameMode::SetPlayerArsenalOpen(AShooterPlayerController* PlayerController, bool bOpen)
{
	if (!HasAuthority() || !IsValid(PlayerController) || PlayerController->GetWorld() != GetWorld())
	{
		return false;
	}
	FRidgefirePlayerWeaponLoadout& Loadout = GetOrCreatePlayerWeaponLoadout(PlayerController);
	if (bOpen && (bArmoryOpen || bRunOver || !IsPlayerNearArsenal(PlayerController)))
	{
		return false;
	}
	if (Loadout.bPersonalArsenalOpen != bOpen)
	{
		Loadout.bPersonalArsenalOpen = bOpen;
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE ARSENAL: player=%s open=%s stationRange=%s"),
			*GetNameSafe(PlayerController), bOpen ? TEXT("true") : TEXT("false"),
			bOpen ? TEXT("validated") : TEXT("not-required"));
	}
	SendRidgefireRunState(PlayerController);
	return true;
}

bool ARidgefireGameMode::IsPlayerNearArsenal(const AShooterPlayerController* PlayerController) const
{
	if (!IsValid(PlayerController) || PlayerController->GetWorld() != GetWorld() || !IsValid(ActiveArenaDressing))
	{
		return false;
	}
	const APawn* PlayerPawn = PlayerController->GetPawn();
	FVector ArsenalLocation = FVector::ZeroVector;
	if (!PlayerPawn || !ActiveArenaDressing->GetArsenalStationWorldLocation(ArsenalLocation))
	{
		return false;
	}
	return FVector::DistSquared2D(PlayerPawn->GetActorLocation(), ArsenalLocation) <= FMath::Square(500.0f);
}

bool ARidgefireGameMode::SelectSecondaryWeaponOfferFromArsenal(AShooterPlayerController* PlayerController, int32 Index)
{
	if (!HasAuthority() || !IsValid(PlayerController) || PlayerController->GetWorld() != GetWorld() || bRunOver)
	{
		return false;
	}
	FRidgefirePlayerWeaponLoadout& Loadout = GetOrCreatePlayerWeaponLoadout(PlayerController);
	if (!Loadout.bPersonalArsenalOpen)
	{
		return false;
	}
	if (!IsPlayerNearArsenal(PlayerController))
	{
		SetPlayerArsenalOpen(PlayerController, false);
		return false;
	}
	if (!WeaponOffers.IsValidIndex(Index) || Loadout.PrimaryWeaponSlots.Contains(Index))
	{
		return false;
	}

	SelectSecondaryWeaponOffer(PlayerController, Index);
	if (Loadout.SecondaryWeaponOfferIndex != Index)
	{
		return false;
	}
	Loadout.bPersonalArsenalOpen = false;
	SendRidgefireRunState(PlayerController);
	return true;
}

int32 ARidgefireGameMode::GetActiveWeaponIndex() const
{
	return ActiveWeaponIndex;
}

int32 ARidgefireGameMode::GetPrimarySlotOffer(int32 Slot) const
{
	return PrimaryWeaponSlots.IsValidIndex(Slot) ? PrimaryWeaponSlots[Slot] : INDEX_NONE;
}

int32 ARidgefireGameMode::GetSecondarySlotOffer() const
{
	return GetSecondarySlotOffer(GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr);
}

int32 ARidgefireGameMode::GetSecondarySlotOffer(APlayerController* PlayerController) const
{
	AShooterPlayerController* RidgefireController = Cast<AShooterPlayerController>(PlayerController);
	if (!IsValid(RidgefireController))
	{
		return INDEX_NONE;
	}
	const FRidgefirePlayerWeaponLoadout* Loadout = PlayerWeaponLoadouts.Find(TWeakObjectPtr<AShooterPlayerController>(RidgefireController));
	return Loadout ? Loadout->SecondaryWeaponOfferIndex : INDEX_NONE;
}

int32 ARidgefireGameMode::GetWildcardRerollTokens() const
{
	return WildcardRerollTokens;
}

int32 ARidgefireGameMode::GetWildcardRerollCount() const
{
	return WildcardRerollCount;
}

bool ARidgefireGameMode::IsWildcardClaimed() const
{
	if (PrimaryWeaponSlots.Contains(2))
	{
		return true;
	}
	for (const TPair<TWeakObjectPtr<AShooterPlayerController>, FRidgefirePlayerWeaponLoadout>& Entry : PlayerWeaponLoadouts)
	{
		if (Entry.Key.IsValid() && (Entry.Value.PrimaryWeaponSlots.Contains(2) || Entry.Value.SecondaryWeaponOfferIndex == 2))
		{
			return true;
		}
	}
	return false;
}

bool ARidgefireGameMode::TryRerollWildcardOffer()
{
	if (!HasAuthority() || bRunOver || WildcardRerollTokens <= 0 || IsWildcardClaimed() || !WeaponOffers.IsValidIndex(2) || !GetWorld())
	{
		return false;
	}

	const double CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastWildcardRerollTime < 0.3)
	{
		return false;
	}

	const TArray<FRidgefireWeaponRoll> SavedOffers = WeaponOffers;
	const TArray<int32> SavedAmmo = WeaponAmmo;
	const TArray<int32> SavedReserveAmmo = WeaponReserveAmmo;
	const TArray<int32> SavedPrimaryWeaponSlots = PrimaryWeaponSlots;
	const int32 SavedActiveWeaponIndex = ActiveWeaponIndex;
	CreateWeaponOffers();
	const FRidgefireWeaponRoll NewWildcardOffer = WeaponOffers[2];
	WeaponOffers = SavedOffers;
	WeaponOffers[2] = NewWildcardOffer;
	WeaponAmmo = SavedAmmo;
	WeaponReserveAmmo = SavedReserveAmmo;
	if (WeaponAmmo.IsValidIndex(2))
	{
		WeaponAmmo[2] = NewWildcardOffer.MagazineSize;
	}
	if (WeaponReserveAmmo.IsValidIndex(2))
	{
		WeaponReserveAmmo[2] = NewWildcardOffer.ReserveAmmo;
	}
	for (TPair<TWeakObjectPtr<AShooterPlayerController>, FRidgefirePlayerWeaponLoadout>& Entry : PlayerWeaponLoadouts)
	{
		if (Entry.Key.IsValid() && Entry.Value.WeaponAmmo.IsValidIndex(2) && Entry.Value.WeaponReserveAmmo.IsValidIndex(2))
		{
			Entry.Value.WeaponAmmo[2] = NewWildcardOffer.MagazineSize;
			Entry.Value.WeaponReserveAmmo[2] = NewWildcardOffer.ReserveAmmo;
		}
	}
	PrimaryWeaponSlots = SavedPrimaryWeaponSlots;
	ActiveWeaponIndex = SavedActiveWeaponIndex;
	--WildcardRerollTokens;
	++WildcardRerollCount;
	LastWildcardRerollTime = CurrentTime;
	RefreshRidgefireRunStates();
	return true;
}

void ARidgefireGameMode::AwardWildcardRerollToken()
{
	if (HasAuthority() && !bRunOver && WildcardRerollTokens < 3)
	{
		++WildcardRerollTokens;
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE: Wave clear earned a wildcard reroll (%d available)."), WildcardRerollTokens);
	}
}

void ARidgefireGameMode::HandlePlayerDefeated()
{
	if (!HasAuthority() || bRunOver)
	{
		return;
	}

	bRunOver = true;
	bArmoryOpen = false;
	GetWorldTimerManager().ClearTimer(WaveTimer);
	GetWorldTimerManager().ClearTimer(ArmoryTimeoutTimer);
	GetWorldTimerManager().ClearTimer(StreakTimer);
	GetWorldTimerManager().ClearTimer(ArmoryStateTimer);
#if WITH_EDITOR
	if (!bRunSmokeTest)
	{
		GetWorldTimerManager().ClearTimer(SmokeTestTimer);
	}
#endif
	UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE: Run over at wave %d with score %d."), CurrentWave, TotalScore);
	RefreshRidgefireRunStates();
}

void ARidgefireGameMode::RestartRun()
{
	if (!bRunOver)
	{
		return;
	}
	GetWorldTimerManager().ClearTimer(WaveTimer);
	GetWorldTimerManager().ClearTimer(ArmoryTimeoutTimer);
	GetWorldTimerManager().ClearTimer(StreakTimer);
#if WITH_EDITOR
	GetWorldTimerManager().ClearTimer(SmokeTestTimer);
#endif
	FString MapName = GetWorld()->GetMapName();
	if (MapName.StartsWith(TEXT("UEDPIE_")))
	{
		int32 PrefixEnd = INDEX_NONE;
		if (MapName.FindChar(TEXT('_'), PrefixEnd) && PrefixEnd + 1 < MapName.Len())
		{
			const int32 MapNameStart = MapName.Find(TEXT("_"), ESearchCase::CaseSensitive, ESearchDir::FromStart, PrefixEnd + 1);
			if (MapNameStart != INDEX_NONE)
			{
				MapName.RightChopInline(MapNameStart + 1);
			}
		}
	}
	UGameplayStatics::OpenLevel(this, FName(*MapName));
}

bool ARidgefireGameMode::IsRunOver() const
{
	return bRunOver;
}

void ARidgefireGameMode::AutoSelectStartingWeapon()
{
	if (bRunOver || !bArmoryOpen)
	{
		return;
	}
	#if WITH_EDITOR
	if (bRunSmokeTest)
	{
		if (!IsValid(GetGladiator()))
		{
			GetWorldTimerManager().SetTimer(ArmoryTimeoutTimer, this, &ARidgefireGameMode::AutoSelectStartingWeapon, 1.0f, false);
			return;
		}
		const FString ExpectedWeaponName = WeaponOffers.IsValidIndex(2) ? WeaponOffers[2].Name : FString();
		if (AShooterPlayerController* Controller = Cast<AShooterPlayerController>(GetWorld()->GetFirstPlayerController()))
		{
			const bool bOfferBindingFound = ExecuteSmokeInputBinding(Controller, EKeys::Gamepad_FaceButton_Left, IE_Pressed);
			if (bOfferBindingFound && !bArmoryOpen && GetGladiator()->GetActiveWeaponName() == ExpectedWeaponName)
			{
				UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: gamepad X selects the wildcard offer."));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: gamepad X did not select the wildcard offer."));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: gamepad offer selection has no player controller."));
		}
		if (bArmoryOpen)
		{
			SelectWeaponOffer(2);
		}
		return;
	}
	#endif
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AShooterPlayerController* PlayerController = Cast<AShooterPlayerController>(It->Get());
		AShooterCharacter* Player = PlayerController ? Cast<AShooterCharacter>(PlayerController->GetPawn()) : nullptr;
		if (!IsValid(PlayerController) || !IsValid(Player))
		{
			continue;
		}

		const FRidgefirePlayerWeaponLoadout& Loadout = GetOrCreatePlayerWeaponLoadout(PlayerController);
		if (!WeaponOffers.IsValidIndex(Loadout.PrimaryWeaponSlots.IsEmpty() ? INDEX_NONE : Loadout.PrimaryWeaponSlots[0]))
		{
			SelectWeaponOffer(PlayerController, 0);
		}
	}
	if (bArmoryOpen)
	{
		GetWorldTimerManager().SetTimer(ArmoryTimeoutTimer, this, &ARidgefireGameMode::AutoSelectStartingWeapon, 1.0f, false);
	}
}

void ARidgefireGameMode::StartWave()
{
	if (!HasAuthority() || bRunOver || bArmoryOpen)
	{
		return;
	}
	if (!SentinelClass)
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE: Sentinel Blueprint could not be loaded; wave could not start."));
		bRunOver = true;
		bArmoryOpen = false;
		GetWorldTimerManager().ClearTimer(WaveTimer);
		GetWorldTimerManager().ClearTimer(StreakTimer);
		RefreshRidgefireRunStates();
		return;
	}
	if (SentinelEntrances.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE: No sentinel entrances are available; wave could not start."));
		bRunOver = true;
		bArmoryOpen = false;
		GetWorldTimerManager().ClearTimer(WaveTimer);
		GetWorldTimerManager().ClearTimer(StreakTimer);
		RefreshRidgefireRunStates();
		return;
	}
	const int32 WaveToStart = CurrentWave + 1;
	UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE: Starting wave %d."), WaveToStart);

	const int32 PlannedCount = FMath::Min(3 + WaveToStart * 2, 14);
	EnemiesRemaining = 0;
	int32 FoundryTurretsSpawned = 0;
	int32 SprintersSpawned = 0;
	int32 BrutesSpawned = 0;

	for (int32 Index = 0; Index < PlannedCount; ++Index)
	{
		FTransform SpawnTransform = SentinelEntrances[Index % SentinelEntrances.Num()];
		SpawnTransform.AddToTranslation(FVector(FMath::RandRange(-130.0f, 130.0f), FMath::RandRange(-130.0f, 130.0f), 30.0f));

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		AShooterNPC* Sentinel = GetWorld()->SpawnActor<AShooterNPC>(SentinelClass, SpawnTransform, SpawnParameters);
		if (!Sentinel)
		{
			UE_LOG(LogTemp, Warning, TEXT("RIDGEFIRE: Could not spawn sentinel %d of wave %d at %s."), Index + 1, CurrentWave, *SpawnTransform.GetLocation().ToString());
			continue;
		}

		const bool bSprinter = WaveToStart >= 2 && Index == 5;
		const bool bBrute = WaveToStart >= 3 && Index == PlannedCount - 1;
		Sentinel->CurrentHP = 120.0f + WaveToStart * 12.0f;
		Sentinel->SetActorScale3D(FVector(1.35f));
		if (bSprinter)
		{
			Sentinel->EnableRidgefireSprinter();
			Sentinel->SetActorScale3D(FVector(1.05f));
			++SprintersSpawned;
		}
		else if (bBrute)
		{
			Sentinel->EnableRidgefireBrute();
			Sentinel->CurrentHP = 260.0f + WaveToStart * 25.0f;
			++BrutesSpawned;
		}
		const float MinimumSpawnZ = ArenaGroundZ + Sentinel->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 5.0f;
		if (Sentinel->GetActorLocation().Z < MinimumSpawnZ)
		{
			Sentinel->SetActorLocation(FVector(Sentinel->GetActorLocation().X, Sentinel->GetActorLocation().Y, MinimumSpawnZ),
				false, nullptr, ETeleportType::TeleportPhysics);
		}
		Sentinel->ForceNetUpdate();
		Sentinel->OnPawnDeath.AddDynamic(this, &ARidgefireGameMode::HandleSentinelDeath);
		++EnemiesRemaining;
	}
#if WITH_EDITOR
	if (bRunCoopCombatSmokeTest && !bCoopSmokeScenarioStarted && EnemiesRemaining > 0)
	{
		APlayerController* RemoteController = nullptr;
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* Candidate = It->Get();
			if (Candidate && !Candidate->IsLocalController() && Cast<AShooterCharacter>(Candidate->GetPawn()))
			{
				RemoteController = Candidate;
				CoopSmokeRemoteCharacter = Cast<AShooterCharacter>(Candidate->GetPawn());
				break;
			}
		}
		if (RemoteController && CoopSmokeRemoteCharacter.IsValid())
		{
			TArray<AActor*> Sentinels;
			UGameplayStatics::GetAllActorsOfClass(this, AShooterNPC::StaticClass(), Sentinels);
			for (AActor* Actor : Sentinels)
			{
				if (AShooterNPC* Sentinel = Cast<AShooterNPC>(Actor); IsValid(Sentinel))
				{
					Sentinel->CurrentHP = 1000.0f;
					Sentinel->SetActorTickEnabled(false);
					Sentinel->ForceNetUpdate();
				}
			}

			UCameraComponent* Camera = CoopSmokeRemoteCharacter->GetFirstPersonCameraComponent();
			AShooterNPC* TestSentinel = Sentinels.IsEmpty() ? nullptr : Cast<AShooterNPC>(Sentinels[0]);
			AShooterNPC* VisibleSentinel = nullptr;
			const FVector OriginalLocation = IsValid(TestSentinel) ? TestSentinel->GetActorLocation() : FVector::ZeroVector;
			FHitResult SelectedAimHit;
			FHitResult SelectedMuzzleHit;
			int32 PositionsTested = 0;
			int32 PlacementRejected = 0;
			int32 TraceRejected = 0;
			if (Camera && IsValid(TestSentinel))
			{
				const UCapsuleComponent* Capsule = TestSentinel->GetCapsuleComponent();
				const FVector CameraLocation = Camera->GetComponentLocation();
				const FVector PlayerLocation = CoopSmokeRemoteCharacter->GetActorLocation();
				const float BaseYaw = CoopSmokeRemoteCharacter->GetActorRotation().Yaw;
				const float SearchRadii[] = {220.0f, 320.0f, 450.0f, 650.0f, 900.0f, 1200.0f, 1500.0f};
				FCollisionQueryParams PlacementParams(SCENE_QUERY_STAT(RidgefireCoopSmokePlacement), false, TestSentinel);
				PlacementParams.AddIgnoredActor(TestSentinel);
				for (float Radius : SearchRadii)
				{
					for (int32 Sector = 0; Sector < 24 && !VisibleSentinel; ++Sector)
					{
						const float Yaw = BaseYaw + Sector * 15.0f;
						FVector TrialLocation = PlayerLocation + FRotator(0.0f, Yaw, 0.0f).Vector() * Radius;
						TrialLocation.Z = OriginalLocation.Z;
						++PositionsTested;
						if (Capsule && GetWorld()->OverlapBlockingTestByChannel(TrialLocation, FQuat::Identity, ECC_Pawn,
							FCollisionShape::MakeCapsule(Capsule->GetScaledCapsuleRadius(), Capsule->GetScaledCapsuleHalfHeight()), PlacementParams))
						{
							++PlacementRejected;
							continue;
						}
						TestSentinel->SetActorLocation(TrialLocation, false, nullptr, ETeleportType::TeleportPhysics);

						const FVector AimPoint = TrialLocation + FVector(0.0f, 0.0f, 35.0f);
						const FRotator AimRotation = (AimPoint - CameraLocation).Rotation();
						const FTransform AimTransform(AimRotation, CameraLocation, Camera->GetComponentScale());
						const FVector MuzzleStart = AimTransform.TransformPosition(FVector(97.0f, 33.0f, -28.0f));
						const FVector AimEnd = CameraLocation + AimRotation.Vector() * 10000.0f;
						FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(RidgefireCoopSmokeVisibility), true, CoopSmokeRemoteCharacter.Get());
						QueryParams.AddIgnoredActor(CoopSmokeRemoteCharacter.Get());
						FHitResult AimHit;
						const bool bAimHit = GetWorld()->LineTraceSingleByChannel(AimHit, CameraLocation, AimEnd, ECC_Visibility, QueryParams);
						FHitResult MuzzleHit;
						const FVector ShotDirection = ((bAimHit ? AimHit.ImpactPoint : AimEnd) - MuzzleStart).GetSafeNormal();
						const bool bMuzzleHit = GetWorld()->LineTraceSingleByChannel(MuzzleHit, MuzzleStart, MuzzleStart + ShotDirection * 10000.0f, ECC_Visibility, QueryParams);
						if (!bAimHit || AimHit.GetActor() != TestSentinel || !bMuzzleHit || MuzzleHit.GetActor() != TestSentinel)
						{
							++TraceRejected;
							UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP SMOKE LOS_REJECT location=%s aimActor=%s muzzleActor=%s"),
								*TrialLocation.ToCompactString(), *GetNameSafe(bAimHit ? AimHit.GetActor() : nullptr), *GetNameSafe(bMuzzleHit ? MuzzleHit.GetActor() : nullptr));
							continue;
						}

						VisibleSentinel = TestSentinel;
						SelectedAimHit = AimHit;
						SelectedMuzzleHit = MuzzleHit;
					}
				}
			}
			if (!VisibleSentinel && IsValid(TestSentinel))
			{
				TestSentinel->SetActorLocation(OriginalLocation, false, nullptr, ETeleportType::TeleportPhysics);
			}

			if (VisibleSentinel)
			{
				CoopSmokeTarget = VisibleSentinel;
				AShooterNPC* Target = VisibleSentinel;
				Target->CurrentHP = 1.0f;
				Target->ForceNetUpdate();
				CoopSmokeInitialScore = TotalScore;
				CoopSmokeInitialEnemies = EnemiesRemaining;
				CoopSmokeInitialAmmo = CoopSmokeRemoteCharacter->GetRidgefireAmmo();
				CoopSmokeInitialHostAmmo = CoopSmokeHostCharacter.IsValid() ? CoopSmokeHostCharacter->GetRidgefireAmmo() : -1;
				CoopSmokeInitialRewardCharge = GetGladiator() ? GetGladiator()->GetIonCharge() : 0.0f;
				bCoopSmokeScenarioStarted = true;
				UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP SMOKE LOS_PASS target=%s aimHit=%s muzzleHit=%s"),
					*GetNameSafe(Target), *GetNameSafe(SelectedAimHit.GetActor()), *GetNameSafe(SelectedMuzzleHit.GetActor()));
				UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP SMOKE TARGET_READY remote=%s target=%s hp=%.0f"),
					*GetNameSafe(CoopSmokeRemoteCharacter.Get()), *GetNameSafe(Target), Target->CurrentHP);
			}
			else
			{
				bCoopSmokeScenarioStarted = true;
				UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE COOP SMOKE FAIL no unobstructed test position after %d samples (placement rejects=%d, camera/muzzle trace rejects=%d)"),
					PositionsTested, PlacementRejected, TraceRejected);
			}
		}
	}
#endif
	UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE: Wave %d role composition: %d Foundry Turrets, %d Sprinters, %d Brutes, %d baseline sentries."),
		WaveToStart, FoundryTurretsSpawned, SprintersSpawned, BrutesSpawned, EnemiesRemaining - FoundryTurretsSpawned - SprintersSpawned - BrutesSpawned);

	if (EnemiesRemaining == 0)
	{
		++ConsecutiveWaveSpawnFailures;
		if (ConsecutiveWaveSpawnFailures >= 3)
		{
			UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE: Wave %d failed to spawn after %d attempts; ending run so it can be restarted."), WaveToStart, ConsecutiveWaveSpawnFailures);
			bRunOver = true;
			GetWorldTimerManager().ClearTimer(WaveTimer);
			GetWorldTimerManager().ClearTimer(StreakTimer);
			RefreshRidgefireRunStates();
			return;
		}
		UE_LOG(LogTemp, Warning, TEXT("RIDGEFIRE: Wave %d spawned no enemies; retrying attempt %d shortly."), WaveToStart, ConsecutiveWaveSpawnFailures + 1);
		GetWorldTimerManager().SetTimer(WaveTimer, this, &ARidgefireGameMode::StartWave, 2.0f, false);
	}
	else
	{
		CurrentWave = WaveToStart;
		ConsecutiveWaveSpawnFailures = 0;
		WaveObjective.Id = TEXT("ClearWave");
		WaveObjective.Title = TEXT("CLEAR THE ARENA");
		WaveObjective.Progress = 0;
		WaveObjective.Goal = EnemiesRemaining;
		WaveObjective.bOptional = false;
		WaveObjective.bCompleted = false;
		WaveObjective.bFailed = false;
		OptionalObjectives.Reset();
		if (bFoundryArenaActive && WaveToStart == 2 && !bFoundryAnchorCleared)
		{
			RegisterOptionalObjective(TEXT("BrassfallFurnaceAnchor"), TEXT("STABILIZE THE FURNACE ANCHOR"), 1);
		}
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE: Wave %d is live with %d sentinels."), CurrentWave, EnemiesRemaining);
		RefreshRidgefireRunStates();
	#if WITH_EDITOR
		if (bRunSmokeTest && !bSmokeTestRestarted && CurrentWave == 1)
		{
			GetWorldTimerManager().SetTimer(SmokeTestTimer, this, &ARidgefireGameMode::RunGameplaySmokeTest, 0.25f, false);
		}
	#endif
	}
}

#if WITH_EDITOR
bool ARidgefireGameMode::StartSentryBehaviorSmokeTest(AShooterCharacter* Player, AShooterPlayerController* Controller)
{
	if (!HasAuthority() || CurrentWave != 1 || !IsValid(Player) || !IsValid(Controller))
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: sentry behavior check requires wave one, an authoritative game mode, and a live player/controller."));
		return false;
	}

	TArray<AActor*> SentinelActors;
	UGameplayStatics::GetAllActorsOfClass(this, AShooterNPC::StaticClass(), SentinelActors);
	AShooterNPC* TestSentry = nullptr;
	float ClosestVisibleDistance = TNumericLimits<float>::Max();
	for (AActor* Actor : SentinelActors)
	{
		AShooterNPC* Candidate = Cast<AShooterNPC>(Actor);
		if (!IsValid(Candidate) || Candidate->CurrentHP <= 0.0f || Candidate->ActorHasTag(TEXT("Dead"))
			|| !FMath::IsNearlyEqual(Candidate->CurrentHP, 132.0f)
			|| !Candidate->GetController() || Candidate->IsFoundryTurretRole()
			|| Candidate->IsRidgefireSprinterRole() || Candidate->IsRidgefireBruteRole())
		{
			continue;
		}

		const float Distance = FVector::Dist2D(Candidate->GetActorLocation(), Player->GetActorLocation());
		if (Distance < 500.0f || Distance > 1700.0f || Distance >= ClosestVisibleDistance)
		{
			continue;
		}

		FCollisionQueryParams SightQuery(SCENE_QUERY_STAT(RidgefireSentryBehaviorSmokeSight), false, Candidate);
		SightQuery.AddIgnoredActor(Candidate);
		FHitResult SightHit;
		const FVector SightStart = Candidate->GetActorLocation() + FVector(0.0f, 0.0f, 30.0f);
		const FVector SightEnd = Player->GetActorLocation() + FVector(0.0f, 0.0f, 45.0f);
		if (GetWorld()->LineTraceSingleByChannel(SightHit, SightStart, SightEnd, ECC_Visibility, SightQuery)
			&& SightHit.GetActor() == Player)
		{
			TestSentry = Candidate;
			ClosestVisibleDistance = Distance;
		}
	}

	if (!TestSentry)
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: no naturally spawned, possessed baseline sentry had clear line of sight to the player within 500-1700 cm."));
		return false;
	}

	SentryBehaviorSmokeSentry = TestSentry;
	SentryBehaviorSmokePlayer = Player;
	SentryBehaviorSmokeStartLocation = TestSentry->GetActorLocation();
	SentryBehaviorSmokeTargetLocation = Player->GetActorLocation();
	SentryBehaviorSmokeStartedAt = GetWorld()->GetTimeSeconds();
	SentryBehaviorSmokeOriginalControlRotation = Controller->GetControlRotation();
	const FRotator FaceSentryRotation = (TestSentry->GetActorLocation() - Player->GetActorLocation()).Rotation();
	Controller->SetControlRotation(FaceSentryRotation);

	SentryBehaviorSmokeEventHandle = TestSentry->GetSentryEditorEventForSmokeTest().AddLambda(
		[this, Player](FName Event, AActor* AttackTarget)
		{
			if (AttackTarget != Player)
			{
				return;
			}
			if (Event == TEXT("TelegraphStarted") || Event == TEXT("MeleeTelegraphStarted"))
			{
				bSentryBehaviorSmokeTelegraphed = true;
				SentryBehaviorSmokeTelegraphPlayerHealth = Player->GetHealthRatio();
			}
			if (Event == TEXT("ShotFired") || Event == TEXT("MeleeHit"))
			{
				bSentryBehaviorSmokeAttackDelivered = true;
				bSentryBehaviorSmokeDamagedPlayer = Player->GetHealthRatio() < SentryBehaviorSmokeTelegraphPlayerHealth;
			}
		});

	UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE: observing naturally spawned sentry %s pursuing reachable player at %.0f cm."),
		*GetNameSafe(TestSentry), ClosestVisibleDistance);
	GetWorldTimerManager().SetTimer(SentryBehaviorSmokeTimer, this, &ARidgefireGameMode::VerifySentryBehaviorSmokeTest, 0.2f, true);
	return true;
}

void ARidgefireGameMode::VerifySentryBehaviorSmokeTest()
{
	AShooterNPC* TestSentry = SentryBehaviorSmokeSentry.Get();
	AShooterCharacter* Player = SentryBehaviorSmokePlayer.Get();
	AShooterPlayerController* Controller = Cast<AShooterPlayerController>(GetWorld()->GetFirstPlayerController());
	const float Elapsed = GetWorld()->GetTimeSeconds() - SentryBehaviorSmokeStartedAt;
	const FVector TowardPlayer = (SentryBehaviorSmokeTargetLocation - SentryBehaviorSmokeStartLocation).GetSafeNormal2D();
	const float TowardProgress = IsValid(TestSentry)
		? FVector::DotProduct(TestSentry->GetActorLocation() - SentryBehaviorSmokeStartLocation, TowardPlayer)
		: 0.0f;
	const bool bMovedTowardPlayer = IsValid(TestSentry) && TowardProgress >= 80.0f;
	const bool bAttackEvidenceComplete = bSentryBehaviorSmokeTelegraphed
		&& bSentryBehaviorSmokeAttackDelivered && bSentryBehaviorSmokeDamagedPlayer;
	const bool bPassed = bMovedTowardPlayer && bAttackEvidenceComplete;
	const bool bTimedOut = Elapsed >= 12.0f || !IsValid(TestSentry) || !IsValid(Player);
	if (!bPassed && !bTimedOut)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(SentryBehaviorSmokeTimer);
	if (IsValid(TestSentry))
	{
		TestSentry->GetSentryEditorEventForSmokeTest().Remove(SentryBehaviorSmokeEventHandle);
	}
	if (IsValid(Controller))
	{
		Controller->SetControlRotation(SentryBehaviorSmokeOriginalControlRotation);
	}
	bSentryBehaviorSmokeFinished = true;

	if (bPassed)
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: natural sentry moved %.0f cm toward its reachable player and telegraphed, delivered, and damaged the player with an attack (%.1f s)."), TowardProgress, Elapsed);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: natural sentry behavior assertion failed (valid=%d toward-player=%.0f/80cm telegraphed=%d delivered=%d player-damaged=%d elapsed=%.1f/12.0s)."),
			IsValid(TestSentry) && IsValid(Player), TowardProgress, bSentryBehaviorSmokeTelegraphed,
			bSentryBehaviorSmokeAttackDelivered, bSentryBehaviorSmokeDamagedPlayer, Elapsed);
	}
	RunGameplaySmokeTest();
}

void ARidgefireGameMode::RunGameplaySmokeTest()
{
	AShooterCharacter* Gladiator = GetGladiator();
	AShooterPlayerController* Controller = Cast<AShooterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (!IsValid(Gladiator) || !IsValid(Controller))
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: player or Ridgefire controller missing; aborting destructive smoke checks."));
		return;
	}
	if (!bSentryBehaviorSmokeStarted)
	{
		bSentryBehaviorSmokeStarted = true;
		if (StartSentryBehaviorSmokeTest(Gladiator, Controller))
		{
			return;
		}
		bSentryBehaviorSmokeFinished = true;
	}
	if (!bSentryBehaviorSmokeFinished)
	{
		return;
	}
	const FRidgefireWeaponRoll* WildcardOffer = GetOffer(2);
	const FString ExpectedModeEffect = WildcardOffer ? DescribeModeEffect(WildcardOffer->Mode, WildcardOffer->Pellets) : FString();
	const FString ExpectedPayloadEffect = WildcardOffer ? DescribePayloadEffect(WildcardOffer->Payload) : FString();
	const FString ExpectedConditionEffect = WildcardOffer && WildcardOffer->Condition != ERidgefireCondition::Steady
		? DescribeConditionEffect(WildcardOffer->Condition)
		: TEXT("No trigger condition");
	const bool bSiphonDescriptionAccurate = DescribePayloadEffect(ERidgefirePayload::Siphon)
		== TEXT("heals 8 on hit; kills heal an extra 2, 8, 20, or 45");
	const bool bWildcardValid = WildcardOffer && WildcardOffer->bWildcard
		&& FMath::IsFinite(WildcardOffer->DamageScale) && WildcardOffer->DamageScale >= 0.25f && WildcardOffer->DamageScale <= 7.0f
		&& FMath::IsFinite(WildcardOffer->RefireDelay) && WildcardOffer->RefireDelay >= 0.055f
		&& WildcardOffer->MagazineSize > 0 && WildcardOffer->ReserveAmmo >= WildcardOffer->MagazineSize
		&& WildcardOffer->Pellets > 0
		&& WildcardOffer->Description.Contains(ExpectedModeEffect)
		&& WildcardOffer->Description.Contains(ExpectedPayloadEffect)
		&& WildcardOffer->Description.Contains(ExpectedConditionEffect);
	bool bWildcardSamplesValid = true;
	const TArray<FRidgefireWeaponRoll> SavedOffers = WeaponOffers;
	const TArray<int32> SavedAmmo = WeaponAmmo;
	const TArray<int32> SavedReserveAmmo = WeaponReserveAmmo;
	const TArray<int32> SavedPrimaryWeaponSlots = PrimaryWeaponSlots;
	const TMap<TWeakObjectPtr<AShooterPlayerController>, FRidgefirePlayerWeaponLoadout> SavedPlayerWeaponLoadouts = PlayerWeaponLoadouts;
	TArray<TPair<TWeakObjectPtr<AShooterPlayerController>, FRidgefireClientRunState>> SavedPlayerRunStates;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (AShooterPlayerController* PlayerController = Cast<AShooterPlayerController>(It->Get()))
		{
			SavedPlayerRunStates.Emplace(PlayerController, PlayerController->GetRidgefireRunState());
		}
	}
	const int32 SavedActiveWeaponIndex = ActiveWeaponIndex;
	const int32 SavedRerollTokens = WildcardRerollTokens;
	const int32 SavedRerollCount = WildcardRerollCount;
	const double SavedLastRerollTime = LastWildcardRerollTime;
	const FString SavedWildcardName = WeaponOffers.IsValidIndex(2) ? WeaponOffers[2].Name : FString();
	const int32 ClaimedTestTokenCount = FMath::Max(1, WildcardRerollTokens);
	PrimaryWeaponSlots.Init(INDEX_NONE, 2);
	PrimaryWeaponSlots[0] = 2;
	WildcardRerollTokens = ClaimedTestTokenCount;
	const bool bClaimedRerollRejected = !TryRerollWildcardOffer()
		&& WildcardRerollTokens == ClaimedTestTokenCount
		&& WildcardRerollCount == SavedRerollCount
		&& WeaponOffers.IsValidIndex(2) && WeaponOffers[2].Name == SavedWildcardName;
	PrimaryWeaponSlots = SavedPrimaryWeaponSlots;
	WildcardRerollTokens = SavedRerollTokens;
	WildcardRerollCount = SavedRerollCount;
	LastWildcardRerollTime = SavedLastRerollTime;
	if (bClaimedRerollRejected)
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: a claimed wildcard rejects reroll without spending a token or changing the weapon."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: a claimed wildcard was rerolled or consumed a token."));
	}
	constexpr int32 WildcardSmokeSeed = 0x51A7;
	FRandomStream WildcardSmokeRandomStream(WildcardSmokeSeed);
	const TArray<int32> ValidMagazineSizes = {3, 5, 8, 13, 21, 34};
	const TArray<int32> ValidReserveMultipliers = {2, 4, 6, 9};
	const TArray<int32> ValidPelletCounts = {3, 5, 7, 11};
	int32 FirstInvalidWildcardSample = INDEX_NONE;
	for (int32 SampleIndex = 0; SampleIndex < 100; ++SampleIndex)
	{
		CreateWeaponOffers(&WildcardSmokeRandomStream);
		const FRidgefireWeaponRoll* Sample = GetOffer(2);
		const FString ModeEffect = Sample ? DescribeModeEffect(Sample->Mode, Sample->Pellets) : FString();
		const FString PayloadEffect = Sample ? DescribePayloadEffect(Sample->Payload) : FString();
		const FString ConditionEffect = Sample && Sample->Condition != ERidgefireCondition::Steady
			? DescribeConditionEffect(Sample->Condition)
			: TEXT("No trigger condition");
		const bool bSampleValid = Sample && Sample->bWildcard
			&& FMath::IsFinite(Sample->DamageScale) && Sample->DamageScale >= 0.25f && Sample->DamageScale <= 7.0f
			&& FMath::IsFinite(Sample->RefireDelay) && Sample->RefireDelay >= 0.055f && Sample->RefireDelay <= 0.19f * 2.7f
			&& ValidMagazineSizes.Contains(Sample->MagazineSize)
			&& Sample->ReserveAmmo >= Sample->MagazineSize * 2
			&& Sample->ReserveAmmo <= Sample->MagazineSize * 9
			&& Sample->ReserveAmmo % Sample->MagazineSize == 0
			&& ValidReserveMultipliers.Contains(Sample->ReserveAmmo / Sample->MagazineSize)
			&& ValidPelletCounts.Contains(Sample->Pellets)
			&& Sample->Description.Contains(ModeEffect)
			&& Sample->Description.Contains(PayloadEffect)
			&& Sample->Description.Contains(ConditionEffect)
			&& (Sample->Mode != ERidgefireFireMode::Scatter || Sample->Description.Contains(FString::Printf(TEXT("%d spread pellets"), Sample->Pellets)));
		bWildcardSamplesValid &= bSampleValid;
		if (!bSampleValid && FirstInvalidWildcardSample == INDEX_NONE)
		{
			FirstInvalidWildcardSample = SampleIndex;
		}
	}
	WeaponOffers = SavedOffers;
	WildcardOffer = SavedOffers.IsValidIndex(2) ? &WeaponOffers[2] : nullptr;
	WeaponAmmo = SavedAmmo;
	WeaponReserveAmmo = SavedReserveAmmo;
	PrimaryWeaponSlots = SavedPrimaryWeaponSlots;
	PlayerWeaponLoadouts = SavedPlayerWeaponLoadouts;
	ActiveWeaponIndex = SavedActiveWeaponIndex;
	WildcardRerollTokens = SavedRerollTokens;
	WildcardRerollCount = SavedRerollCount;
	LastWildcardRerollTime = SavedLastRerollTime;
	for (const TPair<TWeakObjectPtr<AShooterPlayerController>, FRidgefireClientRunState>& SavedState : SavedPlayerRunStates)
	{
		if (AShooterPlayerController* PlayerController = SavedState.Key.Get())
		{
			PlayerController->SetRidgefireRunState(SavedState.Value);
		}
	}
	if (bWildcardValid)
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: the selected wildcard has bounded live stats and describes its rolled mode, payload, and condition."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: wildcard %s; mode=%s; payload=%s; condition=%s; damage=%.3f; delay=%.3f; mag=%d; reserve=%d; pellets=%d."),
			WildcardOffer ? *WildcardOffer->Description : TEXT("<missing>"), *ExpectedModeEffect, *ExpectedPayloadEffect, *ExpectedConditionEffect,
			WildcardOffer ? WildcardOffer->DamageScale : 0.0f, WildcardOffer ? WildcardOffer->RefireDelay : 0.0f,
			WildcardOffer ? WildcardOffer->MagazineSize : 0, WildcardOffer ? WildcardOffer->ReserveAmmo : 0, WildcardOffer ? WildcardOffer->Pellets : 0);
	}
	if (bSiphonDescriptionAccurate)
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: Siphon offer text identifies its on-hit heal and discrete kill-heal rewards."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: Siphon offer text does not match its on-hit and kill-heal rewards."));
	}
	if (bWildcardSamplesValid)
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: deterministic wildcard property smoke seed=%d validated 100 rolls, finite bounds, legal ammo/pellet values, and mode/payload/condition descriptions."), WildcardSmokeSeed);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: deterministic wildcard property smoke seed=%d failed at sample=%d (0-based)."), WildcardSmokeSeed, FirstInvalidWildcardSample);
	}
	const float WalkingSpeed = Gladiator->GetCharacterMovement()->MaxWalkSpeed;
	bool bSprintBindingsFound = true;
	bool bSprintSpeedApplied = false;
	bool bWalkingSpeedRestored = false;
	for (int32 InputCycle = 0; InputCycle < 3; ++InputCycle)
	{
		bSprintBindingsFound &= ExecuteSmokeInputBinding(Controller, EKeys::LeftShift, IE_Pressed);
		bSprintSpeedApplied |= Gladiator->GetCharacterMovement()->MaxWalkSpeed > WalkingSpeed;
		bSprintBindingsFound &= ExecuteSmokeInputBinding(Controller, EKeys::LeftShift, IE_Released);
		bWalkingSpeedRestored |= FMath::IsNearlyEqual(Gladiator->GetCharacterMovement()->MaxWalkSpeed, WalkingSpeed);
	}
	if (bSprintBindingsFound && bSprintSpeedApplied && bWalkingSpeedRestored)
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: repeated keyboard sprint press/release restores walking speed."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: repeated keyboard sprint input did not restore walking speed."));
	}
	if (UCharacterMovementComponent* Movement = Gladiator->GetCharacterMovement())
	{
		const float StandardWalkSpeed = Movement->MaxWalkSpeed;
		const float AlternateWalkSpeed = StandardWalkSpeed * 0.8f;
		Movement->MaxWalkSpeed = AlternateWalkSpeed;
		const bool bAlternateSprintPressed = ExecuteSmokeInputBinding(Controller, EKeys::LeftShift, IE_Pressed);
		const bool bAlternateSprintSpeedApplied = FMath::IsNearlyEqual(Movement->MaxWalkSpeed, AlternateWalkSpeed * 1.5f);
		const bool bAlternateSprintReleased = ExecuteSmokeInputBinding(Controller, EKeys::LeftShift, IE_Released);
		const bool bAlternateWalkSpeedRestored = FMath::IsNearlyEqual(Movement->MaxWalkSpeed, AlternateWalkSpeed);
		Movement->MaxWalkSpeed = StandardWalkSpeed;
		if (bAlternateSprintPressed && bAlternateSprintSpeedApplied && bAlternateSprintReleased && bAlternateWalkSpeedRestored)
		{
			UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: sprint scales and restores a nonstandard character walk speed."));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: sprint did not preserve a nonstandard character walk speed."));
		}
	}
	if (UCharacterMovementComponent* Movement = Gladiator->GetCharacterMovement())
	{
		const float WalkSpeedBeforePause = Movement->MaxWalkSpeed;
		const bool bSprintPressed = ExecuteSmokeInputBinding(Controller, EKeys::LeftShift, IE_Pressed);
		const bool bSprintStarted = Movement->MaxWalkSpeed > WalkSpeedBeforePause;
		const bool bPausePressed = ExecuteSmokeInputBinding(Controller, EKeys::Gamepad_Special_Right, IE_Pressed, true);
		const bool bPaused = Controller->IsPaused();
		const bool bSprintRestored = FMath::IsNearlyEqual(Movement->MaxWalkSpeed, WalkSpeedBeforePause);
		const bool bResumePressed = ExecuteSmokeInputBinding(Controller, EKeys::Gamepad_Special_Right, IE_Pressed, true);
		const bool bResumed = !Controller->IsPaused();
		ExecuteSmokeInputBinding(Controller, EKeys::LeftShift, IE_Released);
		if (bSprintPressed && bSprintStarted && bPausePressed && bPaused && bSprintRestored && bResumePressed && bResumed)
		{
			UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: pausing during held sprint restores walk speed and resumes cleanly."));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: pausing during held sprint left speed or pause state inconsistent."));
			if (Controller->IsPaused())
			{
				Controller->TogglePause();
			}
			Movement->MaxWalkSpeed = WalkSpeedBeforePause;
			Gladiator->DoStopFiring();
		}
	}
	{
		const bool bMenuBindingFound = ExecuteSmokeInputBinding(Controller, EKeys::Gamepad_Special_Right, IE_Pressed, true);
		const bool bPausedOnce = Controller->IsPaused();
		const bool bMenuBindingFoundAgain = ExecuteSmokeInputBinding(Controller, EKeys::Gamepad_Special_Right, IE_Pressed, true);
		if (bMenuBindingFound && bMenuBindingFoundAgain && bPausedOnce && !Controller->IsPaused())
		{
			UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: gamepad menu pauses and resumes the live run."));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: gamepad menu did not pause and resume the live run."));
			if (Controller->IsPaused())
			{
				Controller->TogglePause();
			}
		}
	}
	const int32 WaveBeforeRapidOfferInput = CurrentWave;
	const int32 EnemiesBeforeRapidOfferInput = EnemiesRemaining;
	const int32 ActiveOfferBeforeRapidInput = ActiveWeaponIndex;
	const TArray<FKey> RapidOfferKeys = {EKeys::One, EKeys::Two, EKeys::Three};
	bool bRapidOfferBindingsFound = true;
	for (int32 InputCycle = 0; InputCycle < 3; ++InputCycle)
	{
		for (const FKey& Key : RapidOfferKeys)
		{
			bRapidOfferBindingsFound &= ExecuteSmokeInputBinding(Controller, Key, IE_Pressed);
		}
	}
	const bool bRapidOfferInputDidNotDuplicateWave = CurrentWave == WaveBeforeRapidOfferInput && EnemiesRemaining == EnemiesBeforeRapidOfferInput;
	SelectWeaponOffer(ActiveOfferBeforeRapidInput);
	if (bRapidOfferBindingsFound && bRapidOfferInputDidNotDuplicateWave)
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: rapid repeated weapon-selection inputs do not duplicate a live wave."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: rapid repeated weapon-selection inputs changed the live wave state."));
	}
	const int32 StartingAmmo = Gladiator->GetRidgefireAmmo();
	SmokeInitialReserveAmmo = Gladiator->GetRidgefireReserveAmmo();
	const bool bMouseFirePressed = ExecuteSmokeInputBinding(Gladiator->InputComponent, EKeys::LeftMouseButton, IE_Pressed);
	const bool bMouseFireReleased = ExecuteSmokeInputBinding(Gladiator->InputComponent, EKeys::LeftMouseButton, IE_Released);
	if (Gladiator->IsRidgefireBeamVisibleForSmokeTest())
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: the bound mouse fire input creates a visible muzzle-origin beam."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: bound mouse fire input created no visible muzzle-origin beam; pressed=%s; released=%s."), bMouseFirePressed ? TEXT("true") : TEXT("false"), bMouseFireReleased ? TEXT("true") : TEXT("false"));
	}
	if (Gladiator->GetRidgefireAmmo() >= StartingAmmo)
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: firing did not consume ammunition."));
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: firing consumed ammunition (%d -> %d)."), StartingAmmo, Gladiator->GetRidgefireAmmo());
	}
	if (Gladiator->GetRidgefireShotFeedbackAlpha() > 0.9f)
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: firing triggers the immediate reticle feedback pulse."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: firing did not trigger the reticle feedback pulse."));
	}
	const int32 AmmoBeforeSwap = Gladiator->GetRidgefireAmmo();
	const int32 WeaponBeforeSwap = ActiveWeaponIndex;
	const int32 ActiveSlot = PrimaryWeaponSlots.IndexOfByKey(WeaponBeforeSwap);
	const int32 OtherSlot = ActiveSlot == 0 ? 1 : 0;
	const int32 OtherWeapon = GetPrimarySlotOffer(OtherSlot);
	bool bOtherWeaponEquipped = false;
	if (WeaponOffers.IsValidIndex(OtherWeapon))
	{
		SelectWeaponOffer(OtherWeapon);
		bOtherWeaponEquipped = ActiveWeaponIndex == OtherWeapon && Gladiator->GetActiveWeaponName() == WeaponOffers[OtherWeapon].Name;
	}
	SelectWeaponOffer(WeaponBeforeSwap);
	const bool bOriginalAmmoRestored = ActiveWeaponIndex == WeaponBeforeSwap && Gladiator->GetRidgefireAmmo() == AmmoBeforeSwap;
	if (bOtherWeaponEquipped && bOriginalAmmoRestored)
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: switching between carried primaries preserves each gun's ammunition (%d rounds restored)."), AmmoBeforeSwap);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: carried-primary swap failed; other weapon equipped=%s, original ammo restored=%s, expected=%d, actual=%d."),
			bOtherWeaponEquipped ? TEXT("true") : TEXT("false"), bOriginalAmmoRestored ? TEXT("true") : TEXT("false"), AmmoBeforeSwap, Gladiator->GetRidgefireAmmo());
	}

	TArray<AActor*> Sentinels;
	UGameplayStatics::GetAllActorsOfClass(this, AShooterNPC::StaticClass(), Sentinels);
	bool bOpeningWaveHasOnlyBaselineSentries = CurrentWave == 1 && Sentinels.Num() == 5;
	for (AActor* Actor : Sentinels)
	{
		if (const AShooterNPC* Sentinel = Cast<AShooterNPC>(Actor))
		{
			bOpeningWaveHasOnlyBaselineSentries &= !Sentinel->IsFoundryTurretRole()
				&& !Sentinel->IsRidgefireSprinterRole() && !Sentinel->IsRidgefireBruteRole();
		}
	}
	if (bOpeningWaveHasOnlyBaselineSentries)
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: opening wave has five baseline sentries and no special roles."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: opening wave role composition changed (wave=%d, NPCs=%d)."), CurrentWave, Sentinels.Num());
	}
	bool bSentinelMoved = false;
	for (AActor* Actor : Sentinels)
	{
		if (const AShooterNPC* Sentinel = Cast<AShooterNPC>(Actor); Sentinel && Sentinel->GetTravelDistance() > 10.0f)
		{
			bSentinelMoved = true;
			break;
		}
	}
	if (bSentinelMoved)
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: sentinels maneuver after spawning."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: sentinels did not maneuver after spawning."));
	}
	AShooterNPC* SmokeTarget = nullptr;
	float NearestSentinelDistance = TNumericLimits<float>::Max();
	const FVector PlayerViewLocation = Gladiator->GetPawnViewLocation();
	const FRotator OriginalControlRotation = Controller->GetControlRotation();
	bool bSmokeTargetVisible = false;
	for (AActor* Actor : Sentinels)
	{
		AShooterNPC* Sentinel = Cast<AShooterNPC>(Actor);
		if (Sentinel && Sentinel->CurrentHP > 0.0f)
		{
			const float Distance = FVector::DistSquared(Gladiator->GetActorLocation(), Sentinel->GetActorLocation());
			if (Distance < NearestSentinelDistance)
			{
				SmokeTarget = Sentinel;
				NearestSentinelDistance = Distance;
			}
			const FVector AimPoint = Sentinel->GetActorLocation() + FVector(0.0f, 0.0f, 35.0f);
			const FVector AimDirection = (AimPoint - PlayerViewLocation).GetSafeNormal();
			FCollisionQueryParams AimQuery(SCENE_QUERY_STAT(RidgefireSmokeAim), true, Gladiator);
			FHitResult AimHit;
			if (GetWorld()->LineTraceSingleByChannel(AimHit, PlayerViewLocation, AimPoint, ECC_Visibility, AimQuery)
				&& AimHit.GetActor() == Sentinel)
			{
				SmokeTarget = Sentinel;
				NearestSentinelDistance = Distance;
				bSmokeTargetVisible = true;
				Controller->SetControlRotation(AimDirection.Rotation());
				Gladiator->GetFirstPersonCameraComponent()->SetWorldRotation(AimDirection.Rotation());
				break;
			}
		}
	}
	if (SmokeTarget)
	{
		SelectWeaponOffer(0);
		const float HealthBeforeShot = SmokeTarget->CurrentHP;
		Gladiator->DoStartFiring();
		Gladiator->DoStopFiring();
		Controller->SetControlRotation(OriginalControlRotation);
		Gladiator->GetFirstPersonCameraComponent()->SetWorldRotation(OriginalControlRotation);
		if (SmokeTarget->CurrentHP < HealthBeforeShot)
		{
			UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: an aimed player shot damages a naturally positioned visible sentinel."));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: an aimed player shot missed a live sentinel; visible target acquired=%s, distance=%.1f."), bSmokeTargetVisible ? TEXT("true") : TEXT("false"), FMath::Sqrt(NearestSentinelDistance));
		}
		AShooterNPC* SentryShotTarget = nullptr;
		for (AActor* Actor : Sentinels)
		{
			AShooterNPC* Sentinel = Cast<AShooterNPC>(Actor);
			if (!Sentinel || Sentinel->CurrentHP <= 0.0f || FVector::DistSquared(Sentinel->GetActorLocation(), Gladiator->GetActorLocation()) >= FMath::Square(1700.0f))
			{
				continue;
			}
			const FVector ShotStart = Sentinel->GetActorLocation() + Sentinel->GetActorForwardVector() * 48.0f + FVector(0.0f, 0.0f, 18.0f);
			const FVector ShotEnd = Gladiator->GetActorLocation() + FVector(0.0f, 0.0f, 45.0f);
			FCollisionQueryParams ShotQuery(SCENE_QUERY_STAT(RidgefireSmokeSentryShot), false, Sentinel);
			ShotQuery.AddIgnoredActor(Sentinel);
			FHitResult ShotHit;
			const bool bShotHit = GetWorld()->LineTraceSingleByChannel(ShotHit, ShotStart, ShotEnd, ECC_Visibility, ShotQuery);
			if (bShotHit && ShotHit.GetActor() == Gladiator)
			{
				SentryShotTarget = Sentinel;
				break;
			}
		}
		const float PlayerHealthBeforeSentryShot = Gladiator->GetHealthRatio();
		if (SentryShotTarget)
		{
			SentryShotTarget->FireSentryShot(Gladiator);
		}
		if (SentryShotTarget && Gladiator->GetHealthRatio() < PlayerHealthBeforeSentryShot)
		{
			UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: a sentry energy shot damages the player."));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: no unobstructed sentry energy shot damaged the player."));
		}
		if (Gladiator->GetRidgefireDamageFeedbackAlpha() > 0.9f)
		{
			UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: incoming sentry damage triggers the directional HUD cue."));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: incoming sentry damage did not trigger the directional HUD cue."));
		}
		SelectWeaponOffer(2);
		Gladiator->RunRidgefireHitFeedbackSmokeTest(SmokeTarget, 100.0f);
		if (SmokeTarget->CurrentHP > 0.0f && Gladiator->GetRidgefireHitFeedbackAlpha() > 0.9f && Gladiator->GetRidgefireKillFeedbackAlpha() <= 0.0f)
		{
			UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: a surviving sentinel hit shows hit feedback without kill confirmation."));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: a surviving sentinel hit did not produce hit-only feedback."));
		}

		Gladiator->RunRidgefireHitFeedbackSmokeTest(SmokeTarget, 0.0f);
		if (SmokeTarget->CurrentHP <= 0.0f && Gladiator->GetRidgefireHitFeedbackAlpha() <= 0.0f && Gladiator->GetRidgefireKillFeedbackAlpha() <= 0.0f)
		{
			UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: an already-defeated sentinel does not retrigger hit or kill feedback."));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: an already-defeated sentinel retriggered feedback."));
		}

		AShooterNPC* SecondarySmokeTarget = nullptr;
		for (AActor* Actor : Sentinels)
		{
			AShooterNPC* Sentinel = Cast<AShooterNPC>(Actor);
			if (Sentinel && Sentinel != SmokeTarget && Sentinel->CurrentHP > 0.0f)
			{
				SecondarySmokeTarget = Sentinel;
				break;
			}
		}
		if (SecondarySmokeTarget)
		{
			Gladiator->RunRidgefireSecondaryDamageSmokeTest(SecondarySmokeTarget);
			if (SecondarySmokeTarget->CurrentHP <= 0.0f && Gladiator->GetRidgefireKillFeedbackAlpha() > 0.9f)
			{
				UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: secondary sentinel damage triggers kill confirmation."));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: secondary sentinel damage missed kill confirmation."));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: no second living sentinel was available for chain-feedback verification."));
		}

		Gladiator->RunRidgefireHitFeedbackSmokeTest(SmokeTarget, 1.0f);
		if (SmokeTarget->CurrentHP <= 0.0f && Gladiator->GetRidgefireHitFeedbackAlpha() > 0.9f && Gladiator->GetRidgefireKillFeedbackAlpha() > 0.9f)
		{
			UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: a lethal direct hit triggers distinct kill confirmation."));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: a lethal direct hit did not trigger kill confirmation."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: no living sentinel was available for hit-marker verification."));
	}
	for (AActor* Sentinel : Sentinels)
	{
		UGameplayStatics::ApplyDamage(Sentinel, 100000.0f, Gladiator ? Gladiator->GetController() : nullptr, Gladiator, UDamageType::StaticClass());
	}
	UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE: Applied lethal damage to %d first-wave sentinels."), Sentinels.Num());
	bool bDeadSentriesHideTemplateMesh = !Sentinels.IsEmpty();
	for (const AActor* Actor : Sentinels)
	{
		const AShooterNPC* Sentinel = Cast<AShooterNPC>(Actor);
		bDeadSentriesHideTemplateMesh &= Sentinel && Sentinel->ActorHasTag(TEXT("Dead")) && Sentinel->GetMesh()
			&& !Sentinel->GetMesh()->IsVisible() && !Sentinel->GetMesh()->IsSimulatingPhysics();
	}
	if (bDeadSentriesHideTemplateMesh)
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: defeated sentry skeletal mesh stays hidden and non-simulating."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: defeated sentry revealed or simulated its skeletal mesh."));
	}
	GetWorldTimerManager().SetTimer(SmokeTestTimer, this, &ARidgefireGameMode::VerifySecondWaveSmokeTest, 6.0f, false);
}

void ARidgefireGameMode::VerifySecondWaveSmokeTest()
{
	if (CurrentWave >= 2 && EnemiesRemaining == 7)
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: first wave cleared and wave two spawned seven sentinels."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: expected wave two with seven sentinels; current wave %d has %d."), CurrentWave, EnemiesRemaining);
	}
	TArray<AActor*> WaveTwoSentinels;
	UGameplayStatics::GetAllActorsOfClass(this, AShooterNPC::StaticClass(), WaveTwoSentinels);
	int32 FoundryCount = 0;
	int32 SprinterCount = 0;
	int32 BruteCount = 0;
	int32 LiveWaveTwoCount = 0;
	for (AActor* Actor : WaveTwoSentinels)
	{
		if (AShooterNPC* Sentinel = Cast<AShooterNPC>(Actor))
		{
			if (Sentinel->ActorHasTag(TEXT("Dead")))
			{
				continue;
			}
			++LiveWaveTwoCount;
			if (Sentinel->IsFoundryTurretRole())
			{
				++FoundryCount;
			}
			SprinterCount += Sentinel->IsRidgefireSprinterRole() ? 1 : 0;
			BruteCount += Sentinel->IsRidgefireBruteRole() ? 1 : 0;
		}
	}
	const bool bWaveTwoRoleComposition = CurrentWave == 2 && LiveWaveTwoCount == 7
		&& FoundryCount == 0 && SprinterCount == 1 && BruteCount == 0;
	if (bWaveTwoRoleComposition)
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: wave two has no Foundry Turrets, one Sprinter, no Brute, and six baseline sentries."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: wave two role composition mismatch (wave=%d, count=%d, turret=%d, sprinter=%d, brute=%d)."),
			CurrentWave, LiveWaveTwoCount, FoundryCount, SprinterCount, BruteCount);
	}
	const bool bFoundryObjectiveReady = bFoundryArenaActive && ArenaDisplayName == TEXT("BRASSFALL FOUNDRY") && !bFoundryAnchorCleared
		&& OptionalObjectives.ContainsByPredicate([](const FRidgefireObjectiveState& Objective)
		{
			return Objective.Id == FName(TEXT("BrassfallFurnaceAnchor")) && Objective.Goal == 1 && Objective.Progress == 0 && !Objective.bCompleted;
		});
	if (bFoundryObjectiveReady)
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: wave-clear transition entered Brassfall and registered its one-time furnace anchor objective."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: Brassfall transition or one-time furnace anchor objective was not ready on wave two."));
	}
	if (bFoundryObjectiveReady)
	{
		AShooterCharacter* AnchorTester = GetGladiator();
		AShooterPlayerController* AnchorController = AnchorTester ? Cast<AShooterPlayerController>(AnchorTester->GetController()) : nullptr;
		if (IsValid(AnchorTester) && IsValid(AnchorController))
		{
			const FVector OriginalLocation = AnchorTester->GetActorLocation();
			const int32 ReserveBeforeAnchor = AnchorTester->GetRidgefireReserveAmmo();
			const bool bOutOfRangeRejected = FVector::DistSquared2D(OriginalLocation, ArenaCenter) > FMath::Square(420.0f)
				? !ActivateFoundryFurnaceAnchor(AnchorController) : true;
			AnchorTester->SetActorLocation(ArenaCenter + FVector(0.0f, 0.0f, 120.0f), false);
			const bool bActivated = ActivateFoundryFurnaceAnchor(AnchorController);
			const int32 ReserveAfterAnchor = AnchorTester->GetRidgefireReserveAmmo();
			const bool bRepeatRejected = !ActivateFoundryFurnaceAnchor(AnchorController)
				&& AnchorTester->GetRidgefireReserveAmmo() == ReserveAfterAnchor;
			AnchorTester->SetActorLocation(OriginalLocation, false);
			if (bOutOfRangeRejected && bActivated && bRepeatRejected && ReserveAfterAnchor == ReserveBeforeAnchor + 36)
			{
				UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: Brassfall furnace anchor rejects distant use, rewards once, and rejects repeat use."));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: Brassfall furnace anchor range/reward-once check failed (range=%d, activate=%d, repeat=%d, reserve=%d->%d)."), bOutOfRangeRejected, bActivated, bRepeatRejected, ReserveBeforeAnchor, ReserveAfterAnchor);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: no player/controller available for Brassfall furnace anchor interaction check."));
		}
	}
	if (AShooterCharacter* Gladiator = GetGladiator(); Gladiator && Gladiator->GetRidgefireReserveAmmo() >= SmokeInitialReserveAmmo + 24)
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: wave clear replenished reserve ammunition."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: wave clear did not replenish reserve ammunition."));
	}

	TArray<AActor*> Pickups;
	UGameplayStatics::GetAllActorsOfClass(this, ARidgefirePickup::StaticClass(), Pickups);
	AShooterCharacter* PickupCollector = GetGladiator();
	if (!Pickups.IsEmpty() && PickupCollector)
	{
		const int32 ReserveBeforePickup = PickupCollector->GetRidgefireReserveAmmo();
		UGameplayStatics::ApplyDamage(PickupCollector, 40.0f, nullptr, nullptr, UDamageType::StaticClass());
		const float HealthAfterDamage = PickupCollector->GetHealthRatio();
		if (ARidgefirePickup* Pickup = Cast<ARidgefirePickup>(Pickups[0]); IsValid(Pickup))
		{
			const bool bCollected = Pickup->Collect(PickupCollector);
			const bool bRewardApplied = PickupCollector->GetHealthRatio() > HealthAfterDamage || PickupCollector->GetRidgefireReserveAmmo() >= ReserveBeforePickup + 18;
			const float HealthAfterFirstCollection = PickupCollector->GetHealthRatio();
			const int32 ReserveAfterFirstCollection = PickupCollector->GetRidgefireReserveAmmo();
			const bool bRepeatedCollectionRejected = !Pickup->Collect(PickupCollector)
				&& FMath::IsNearlyEqual(PickupCollector->GetHealthRatio(), HealthAfterFirstCollection)
				&& PickupCollector->GetRidgefireReserveAmmo() == ReserveAfterFirstCollection;
			if (bCollected && bRewardApplied && bRepeatedCollectionRejected && Pickup->GetIsReplicated())
			{
				UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: replicated sentinel field drop granted once; repeated collection rejected."));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: field drop replication/reward-once check failed (collected=%d reward=%d duplicate-rejected=%d replicated=%d)."),
					bCollected, bRewardApplied, bRepeatedCollectionRejected, Pickup->GetIsReplicated());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: field drop actor was invalid during collection."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: guaranteed smoke-test field drop was not spawned."));
	}

	AShooterCharacter* Gladiator = GetGladiator();
	if (!Gladiator)
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: player character missing before defeat test."));
		return;
	}
	UGameplayStatics::ApplyDamage(Gladiator, 100000.0f, nullptr, nullptr, UDamageType::StaticClass());
	if (IsRunOver() && Gladiator->IsDead())
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: player defeat enters game over without destroying the pawn."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: lethal player damage did not end the run."));
	}
	SmokeDefeatedGladiator = Gladiator;
	GetWorldTimerManager().SetTimer(SmokeTestTimer, this, &ARidgefireGameMode::VerifyPlayerNoRespawnSmokeTest, 5.5f, false);
}

void ARidgefireGameMode::VerifyPlayerNoRespawnSmokeTest()
{
	if (IsRunOver() && SmokeDefeatedGladiator.IsValid() && GetGladiator() == SmokeDefeatedGladiator.Get())
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: defeated player remains on game-over screen without an automatic respawn."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: defeated player respawned or left game-over state unexpectedly."));
	}
	if (IsRunOver())
	{
		bSmokeTestRestarted = true;
		RestartRun();
	}
}

void ARidgefireGameMode::VerifyRestartSmokeTest()
{
	const AShooterCharacter* Gladiator = GetGladiator();
	TArray<AActor*> RemainingSentinels;
	UGameplayStatics::GetAllActorsOfClass(this, AShooterNPC::StaticClass(), RemainingSentinels);
	const bool bFreshRunState = !IsRunOver() && IsArmoryOpen() && CurrentWave == 0 && EnemiesRemaining == 0
		&& TotalScore == 0 && CurrentStreak == 0 && GetOfferCount() == 3 && ActiveWeaponIndex == INDEX_NONE
		&& !bFoundryArenaActive && !bFoundryAnchorCleared && ArenaDisplayName == TEXT("IRON SUN ARENA")
		&& WeaponAmmo.IsEmpty() && WeaponReserveAmmo.IsEmpty() && RemainingSentinels.IsEmpty()
		&& !GetWorldTimerManager().IsTimerActive(WaveTimer) && !GetWorldTimerManager().IsTimerActive(StreakTimer);
	if (bFreshRunState && IsValid(Gladiator) && !Gladiator->IsDead())
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: restart restores a living player and fresh armory with no stale sentries, score, ammo, or wave/streak timers."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: restart retained stale run state (run-over=%d, armory=%d, wave=%d, enemies=%d, score=%d, streak=%d, offers=%d, active-weapon=%d, sentries=%d, wave-timer=%d, streak-timer=%d, player=%d)."),
			IsRunOver(), IsArmoryOpen(), CurrentWave, EnemiesRemaining, TotalScore, CurrentStreak, GetOfferCount(), ActiveWeaponIndex,
			RemainingSentinels.Num(), GetWorldTimerManager().IsTimerActive(WaveTimer), GetWorldTimerManager().IsTimerActive(StreakTimer), IsValid(Gladiator));
	}
	bSmokeTestRunComplete = true;
}
#endif

void ARidgefireGameMode::HandleSentinelDeath(AActor* DeadSentinel)
{
	if (!HasAuthority() || bRunOver || !IsValid(DeadSentinel))
	{
		return;
	}
	const TWeakObjectPtr<AActor> DeathKey(DeadSentinel);
	if (ProcessedSentinelDeaths.Contains(DeathKey))
	{
		return;
	}
	ProcessedSentinelDeaths.Add(DeathKey);
	EnemiesRemaining = FMath::Max(0, EnemiesRemaining - 1);
	AdvanceObjective(WaveObjective, 1);
	++CurrentStreak;
	const int32 Multiplier = FMath::Min(8, 1 + CurrentStreak / 3);
	TotalScore += 110 * Multiplier;
#if WITH_EDITOR
	if (bRunCoopCombatSmokeTest && DeadSentinel == CoopSmokeTarget.Get() && !bCoopSmokeTargetKilled)
	{
		bCoopSmokeTargetKilled = true;
		GetWorldTimerManager().SetTimer(CoopCombatSmokeTimer, this, &ARidgefireGameMode::VerifyCoopCombatSmokeTest, 2.0f, false);
	}
#endif
	GetWorldTimerManager().SetTimer(StreakTimer, this, &ARidgefireGameMode::ResetStreak, 2.7f, false);

	if (AShooterCharacter* Gladiator = GetGladiator())
	{
		Gladiator->AddIonCharge(14.0f);
		Gladiator->ApplyRidgefireKillReward();
	}

	bool bDropPickup = FMath::FRand() < 0.24f;
#if WITH_EDITOR
	if (bRunCoopCombatSmokeTest && DeadSentinel == CoopSmokeTarget.Get())
	{
		bDropPickup = true;
	}
	if (bRunSmokeTest && !bSmokeDropSpawned)
	{
		bDropPickup = true;
		bSmokeDropSpawned = true;
	}
#endif
	if (bDropPickup && IsValid(DeadSentinel) && PickupClass)
	{
		const FTransform PickupTransform(FRotator::ZeroRotator, DeadSentinel->GetActorLocation() + FVector(0.0f, 0.0f, 65.0f));
		if (ARidgefirePickup* Pickup = GetWorld()->SpawnActor<ARidgefirePickup>(PickupClass, PickupTransform))
		{
			Pickup->Initialize(FMath::RandBool());
		}
	}

	if (EnemiesRemaining == 0)
	{
		AwardWildcardRerollToken();
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE: Wave %d cleared. Next wave in five seconds."), CurrentWave);
		GetWorldTimerManager().SetTimer(WaveTimer, this, &ARidgefireGameMode::BeginNextWave, 5.0f, false);
	}
	RefreshRidgefireRunStates();
}

#if WITH_EDITOR
void ARidgefireGameMode::RunCoopCombatSmokeTest()
{
	if (!HasAuthority() || bCoopSmokeScenarioStarted)
	{
		GetWorldTimerManager().ClearTimer(CoopCombatSmokeTimer);
		return;
	}
	if (bArmoryOpen && !bCoopSmokeArmoryTestComplete)
	{
		AShooterPlayerController* HostController = Cast<AShooterPlayerController>(GetWorld()->GetFirstPlayerController());
		AShooterPlayerController* RemoteController = nullptr;
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			AShooterPlayerController* Candidate = Cast<AShooterPlayerController>(It->Get());
			if (Candidate && !Candidate->IsLocalController() && Cast<AShooterCharacter>(Candidate->GetPawn()))
			{
				RemoteController = Candidate;
				break;
			}
		}
		if (HostController && Cast<AShooterCharacter>(HostController->GetPawn()) && RemoteController)
		{
			SelectWeaponOffer(HostController, 0);
			const bool bArmoryStayedOpenForUnselectedRemote = bArmoryOpen && RemoteController->GetRidgefireRunState().bArmoryOpen;
			SelectWeaponOffer(RemoteController, 1);
			const bool bArmoryClosedAfterBothSelected = !bArmoryOpen
				&& HostController->GetRidgefireRunState().PrimaryWeaponSlots.IsValidIndex(0)
				&& HostController->GetRidgefireRunState().PrimaryWeaponSlots[0] == 0
				&& RemoteController->GetRidgefireRunState().PrimaryWeaponSlots.IsValidIndex(0)
				&& RemoteController->GetRidgefireRunState().PrimaryWeaponSlots[0] == 1;
			if (!bArmoryStayedOpenForUnselectedRemote || !bArmoryClosedAfterBothSelected)
			{
				UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE COOP SMOKE FAIL team armory host_open=%s remote_open=%s closed_after_both=%s"),
					bArmoryOpen ? TEXT("true") : TEXT("false"),
					RemoteController->GetRidgefireRunState().bArmoryOpen ? TEXT("true") : TEXT("false"),
					bArmoryClosedAfterBothSelected ? TEXT("true") : TEXT("false"));
				GetWorldTimerManager().ClearTimer(CoopCombatSmokeTimer);
				return;
			}
			bCoopSmokeArmoryTestComplete = true;
			UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP SMOKE PASS opening armory stays open for both clients until each selects a starting primary"));
		}
	}
	TArray<APawn*> PlayerPawns;
	AShooterPlayerController* RemoteController = nullptr;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();
		AShooterCharacter* Character = Cast<AShooterCharacter>(PlayerController ? PlayerController->GetPawn() : nullptr);
		if (Character)
		{
			PlayerPawns.Add(Character);
			if (!PlayerController->IsLocalController())
			{
				RemoteController = Cast<AShooterPlayerController>(PlayerController);
				CoopSmokeRemoteCharacter = Character;
			}
		}
	}
	if (PlayerPawns.Num() < 2 || !RemoteController)
	{
		return;
	}
	if (PlayerPawns[0] == PlayerPawns[1])
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE COOP SMOKE FAIL player controllers share one pawn"));
		GetWorldTimerManager().ClearTimer(CoopCombatSmokeTimer);
		return;
	}
	AShooterPlayerController* HostController = Cast<AShooterPlayerController>(GetWorld()->GetFirstPlayerController());
	AShooterCharacter* HostCharacter = HostController ? Cast<AShooterCharacter>(HostController->GetPawn()) : nullptr;
	if (!IsValid(HostController) || !IsValid(HostCharacter) || !IsValid(RemoteController))
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE COOP SMOKE FAIL player loadout controllers missing"));
		GetWorldTimerManager().ClearTimer(CoopCombatSmokeTimer);
		return;
	}
	const bool bEitherPlayerAlreadyUnavailable = HostCharacter->ActorHasTag(TEXT("Dead")) || CoopSmokeRemoteCharacter->ActorHasTag(TEXT("Dead"));
	if (bEitherPlayerAlreadyUnavailable)
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE COOP SMOKE FAIL sentry target eligibility setup requires two living players"));
		GetWorldTimerManager().ClearTimer(CoopCombatSmokeTimer);
		return;
	}
	FActorSpawnParameters SmokeSentrySpawnParameters;
	SmokeSentrySpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const FTransform SmokeSentryTransform(FRotator::ZeroRotator, HostCharacter->GetActorLocation() + FVector(0.0f, 0.0f, 250.0f));
	AShooterNPC* CueSmokeSentry = SentinelClass
		? GetWorld()->SpawnActor<AShooterNPC>(SentinelClass, SmokeSentryTransform, SmokeSentrySpawnParameters)
		: nullptr;
	if (CueSmokeSentry)
	{
		CueSmokeSentry->SetActorTickEnabled(false);
	}
	const bool bWarningCueDispatchedExactlyOnce = CueSmokeSentry && CueSmokeSentry->RunSentryTelegraphCueEditorSmokeTest(HostCharacter);
	if (IsValid(CueSmokeSentry))
	{
		CueSmokeSentry->Destroy();
	}
	if (!bWarningCueDispatchedExactlyOnce)
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE COOP SMOKE FAIL temporary sentry did not dispatch exactly one warning cue for a living target"));
		GetWorldTimerManager().ClearTimer(CoopCombatSmokeTimer);
		return;
	}
	UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP SMOKE PASS exactly one sentry warning cue dispatch for living target; RPC dispatch only, audio playback/client delivery unverified"));
	const float TargetSelectionTime = GetWorld()->GetTimeSeconds();
	const FName UnavailablePlayerTag(TEXT("Dead"));
	HostCharacter->Tags.AddUnique(UnavailablePlayerTag);
	const APawn* SentryHandoffTarget = AShooterNPC::SelectRidgefireSentryTarget(PlayerPawns, HostCharacter->GetActorLocation(), HostCharacter, nullptr, 0.0f, TargetSelectionTime);
	HostCharacter->Tags.Remove(UnavailablePlayerTag);
	if (SentryHandoffTarget != CoopSmokeRemoteCharacter.Get())
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE COOP SMOKE FAIL sentry target handoff when current player marked unavailable target=%s expected=%s"),
			*GetNameSafe(SentryHandoffTarget), *GetNameSafe(CoopSmokeRemoteCharacter.Get()));
		GetWorldTimerManager().ClearTimer(CoopCombatSmokeTimer);
		return;
	}
	UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP SMOKE PASS sentry target handoff to living player after current target marked unavailable"));
	TArray<APawn*> LoneEligiblePlayers;
	LoneEligiblePlayers.Add(CoopSmokeRemoteCharacter.Get());
	const APawn* SentryFallbackTarget = AShooterNPC::SelectRidgefireSentryTarget(LoneEligiblePlayers, HostCharacter->GetActorLocation(), nullptr, CoopSmokeRemoteCharacter.Get(), TargetSelectionTime + 4.0f, TargetSelectionTime);
	if (SentryFallbackTarget != CoopSmokeRemoteCharacter.Get())
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE COOP SMOKE FAIL sole eligible path-deprioritized player was not retained as sentry fallback target=%s expected=%s"),
			*GetNameSafe(SentryFallbackTarget), *GetNameSafe(CoopSmokeRemoteCharacter.Get()));
		GetWorldTimerManager().ClearTimer(CoopCombatSmokeTimer);
		return;
	}
	UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP SMOKE PASS sole eligible player remains sentry fallback despite temporary path deprioritization"));
	SelectWeaponOffer(HostController, 0);
	SelectWeaponOffer(RemoteController, 1);
	SelectWeaponOffer(HostController, 2);
	SelectWeaponOffer(RemoteController, 2);
	CycleWeaponOffer(HostController);
	CycleWeaponOffer(RemoteController);
	FVector ArsenalStationLocation = FVector::ZeroVector;
	AShooterCharacter* RemoteCharacter = CoopSmokeRemoteCharacter.Get();
	if (IsValid(ActiveArenaDressing) && ActiveArenaDressing->GetArsenalStationWorldLocation(ArsenalStationLocation)
		&& IsValid(HostCharacter) && IsValid(RemoteCharacter))
	{
		const FTransform HostOriginalTransform = HostCharacter->GetActorTransform();
		const FTransform RemoteOriginalTransform = RemoteCharacter->GetActorTransform();
		HostCharacter->SetActorLocation(ArsenalStationLocation + FVector(-140.0f, 0.0f, 90.0f), false, nullptr, ETeleportType::TeleportPhysics);
		RemoteCharacter->SetActorLocation(ArsenalStationLocation + FVector(700.0f, 0.0f, 90.0f), false, nullptr, ETeleportType::TeleportPhysics);
		const bool bHostOpenedPersonalArsenal = SetPlayerArsenalOpen(HostController, true)
			&& HostController->GetRidgefireRunState().bPersonalArsenalOpen
			&& !RemoteController->GetRidgefireRunState().bPersonalArsenalOpen;
		const bool bOutOfRangeOpenRejected = !SetPlayerArsenalOpen(RemoteController, true)
			&& !RemoteController->GetRidgefireRunState().bPersonalArsenalOpen;
		RemoteCharacter->SetActorLocation(ArsenalStationLocation + FVector(140.0f, 0.0f, 90.0f), false, nullptr, ETeleportType::TeleportPhysics);
		const bool bRemoteOpenedIndependently = SetPlayerArsenalOpen(RemoteController, true)
			&& HostController->GetRidgefireRunState().bPersonalArsenalOpen
			&& RemoteController->GetRidgefireRunState().bPersonalArsenalOpen;
		const bool bBothClosed = SetPlayerArsenalOpen(HostController, false)
			&& SetPlayerArsenalOpen(RemoteController, false)
			&& !HostController->GetRidgefireRunState().bPersonalArsenalOpen
			&& !RemoteController->GetRidgefireRunState().bPersonalArsenalOpen;
		HostCharacter->SetActorTransform(HostOriginalTransform, false, nullptr, ETeleportType::TeleportPhysics);
		RemoteCharacter->SetActorTransform(RemoteOriginalTransform, false, nullptr, ETeleportType::TeleportPhysics);
		if (!bHostOpenedPersonalArsenal || !bOutOfRangeOpenRejected || !bRemoteOpenedIndependently || !bBothClosed)
		{
			UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE COOP SMOKE FAIL personal arsenal range/isolation host_open=%s range_rejected=%s remote_independent=%s closed=%s"),
				bHostOpenedPersonalArsenal ? TEXT("true") : TEXT("false"),
				bOutOfRangeOpenRejected ? TEXT("true") : TEXT("false"),
				bRemoteOpenedIndependently ? TEXT("true") : TEXT("false"),
				bBothClosed ? TEXT("true") : TEXT("false"));
			GetWorldTimerManager().ClearTimer(CoopCombatSmokeTimer);
			return;
		}
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP SMOKE PASS personal arsenal range-gated and per-player; host/remote open/close independent, out-of-range denied"));
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP SMOKE SKIP personal arsenal range fixture unavailable in this arena"));
	}
	SelectSecondaryWeaponOffer(HostController, 1);
	const bool bHostSecondaryDidNotChangeRemote = HostController->GetRidgefireRunState().SecondaryWeaponOfferIndex == 1
		&& RemoteController->GetRidgefireRunState().SecondaryWeaponOfferIndex == INDEX_NONE;
	SelectSecondaryWeaponOffer(RemoteController, 0);
	const bool bDifferentSecondaryOffersStayedIndependent = HostController->GetRidgefireRunState().SecondaryWeaponOfferIndex == 1
		&& RemoteController->GetRidgefireRunState().SecondaryWeaponOfferIndex == 0;
	SelectSecondaryWeaponOffer(HostController, INDEX_NONE);
	SelectSecondaryWeaponOffer(RemoteController, 2);
	const bool bInvalidAndOwnedSecondaryOffersRejected = HostController->GetRidgefireRunState().SecondaryWeaponOfferIndex == 1
		&& RemoteController->GetRidgefireRunState().SecondaryWeaponOfferIndex == 0;
	if (!bHostSecondaryDidNotChangeRemote || !bDifferentSecondaryOffersStayedIndependent || !bInvalidAndOwnedSecondaryOffersRejected)
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE COOP SMOKE FAIL secondary offers host=%d remote=%d isolated=%s distinct=%s rejected_invalid_or_owned=%s"),
			HostController->GetRidgefireRunState().SecondaryWeaponOfferIndex,
			RemoteController->GetRidgefireRunState().SecondaryWeaponOfferIndex,
			bHostSecondaryDidNotChangeRemote ? TEXT("true") : TEXT("false"),
			bDifferentSecondaryOffersStayedIndependent ? TEXT("true") : TEXT("false"),
			bInvalidAndOwnedSecondaryOffersRejected ? TEXT("true") : TEXT("false"));
		GetWorldTimerManager().ClearTimer(CoopCombatSmokeTimer);
		return;
	}
	UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP SMOKE PASS independent secondary offers host=%d remote=%d; invalid and primary-owned offers rejected"),
		HostController->GetRidgefireRunState().SecondaryWeaponOfferIndex,
		RemoteController->GetRidgefireRunState().SecondaryWeaponOfferIndex);
	FVector SecondaryStationLocation = FVector::ZeroVector;
	AShooterCharacter* SecondaryRemoteCharacter = CoopSmokeRemoteCharacter.Get();
	if (IsValid(ActiveArenaDressing) && ActiveArenaDressing->GetArsenalStationWorldLocation(SecondaryStationLocation)
		&& IsValid(HostCharacter) && IsValid(SecondaryRemoteCharacter))
	{
		const FTransform HostOriginalTransform = HostCharacter->GetActorTransform();
		const FTransform RemoteOriginalTransform = SecondaryRemoteCharacter->GetActorTransform();
		const bool bWasGlobalArmoryOpen = bArmoryOpen;
		HostCharacter->SetActorLocation(SecondaryStationLocation + FVector(-140.0f, 0.0f, 90.0f), false, nullptr, ETeleportType::TeleportPhysics);
		SecondaryRemoteCharacter->SetActorLocation(SecondaryStationLocation + FVector(140.0f, 0.0f, 90.0f), false, nullptr, ETeleportType::TeleportPhysics);
		const bool bBothPersonalPanelsOpened = SetPlayerArsenalOpen(HostController, true)
			&& SetPlayerArsenalOpen(RemoteController, true);
		const bool bHostSelectedOnlyForHost = bBothPersonalPanelsOpened
			&& SelectSecondaryWeaponOfferFromArsenal(HostController, 1)
			&& HostController->GetRidgefireRunState().SecondaryWeaponOfferIndex == 1
			&& !HostController->GetRidgefireRunState().bPersonalArsenalOpen
			&& RemoteController->GetRidgefireRunState().bPersonalArsenalOpen
			&& RemoteController->GetRidgefireRunState().SecondaryWeaponOfferIndex == 0;
		const bool bRemoteSelectedOnlyForRemote = bHostSelectedOnlyForHost
			&& SelectSecondaryWeaponOfferFromArsenal(RemoteController, 0)
			&& RemoteController->GetRidgefireRunState().SecondaryWeaponOfferIndex == 0
			&& !RemoteController->GetRidgefireRunState().bPersonalArsenalOpen;
		SetPlayerArsenalOpen(HostController, false);
		SetPlayerArsenalOpen(RemoteController, false);
		HostCharacter->SetActorTransform(HostOriginalTransform, false, nullptr, ETeleportType::TeleportPhysics);
		SecondaryRemoteCharacter->SetActorTransform(RemoteOriginalTransform, false, nullptr, ETeleportType::TeleportPhysics);
		if (!bBothPersonalPanelsOpened || !bHostSelectedOnlyForHost || !bRemoteSelectedOnlyForRemote || bArmoryOpen != bWasGlobalArmoryOpen)
		{
			UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE COOP SMOKE FAIL station offer selection both_open=%s host_isolated=%s remote_isolated=%s global_armory_unchanged=%s"),
				bBothPersonalPanelsOpened ? TEXT("true") : TEXT("false"),
				bHostSelectedOnlyForHost ? TEXT("true") : TEXT("false"),
				bRemoteSelectedOnlyForRemote ? TEXT("true") : TEXT("false"),
				bArmoryOpen == bWasGlobalArmoryOpen ? TEXT("true") : TEXT("false"));
			GetWorldTimerManager().ClearTimer(CoopCombatSmokeTimer);
			return;
		}
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP SMOKE PASS station offer selection stayed per-player; each panel closed after selection and global armory stayed unchanged"));
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP SMOKE SKIP station offer-selection fixture unavailable in this arena"));
	}
	const FRidgefirePlayerWeaponLoadout* HostLoadout = PlayerWeaponLoadouts.Find(TWeakObjectPtr<AShooterPlayerController>(HostController));
	const FRidgefirePlayerWeaponLoadout* RemoteLoadout = PlayerWeaponLoadouts.Find(TWeakObjectPtr<AShooterPlayerController>(RemoteController));
	const bool bIndependentPrimaryLoadouts = HostLoadout && RemoteLoadout
		&& HostLoadout->PrimaryWeaponSlots.Contains(0) && HostLoadout->PrimaryWeaponSlots.Contains(2)
		&& RemoteLoadout->PrimaryWeaponSlots.Contains(1) && RemoteLoadout->PrimaryWeaponSlots.Contains(2)
		&& HostLoadout->ActiveWeaponIndex == 0 && RemoteLoadout->ActiveWeaponIndex == 1
		&& HostCharacter->GetActiveWeaponName() == WeaponOffers[0].Name
		&& CoopSmokeRemoteCharacter->GetActiveWeaponName() == WeaponOffers[1].Name
		&& HostCharacter->GetRidgefireAmmo() == WeaponOffers[0].MagazineSize
		&& CoopSmokeRemoteCharacter->GetRidgefireAmmo() == WeaponOffers[1].MagazineSize;
	if (!bIndependentPrimaryLoadouts)
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE COOP SMOKE FAIL independent loadouts host=%s/%d remote=%s/%d"),
			*HostCharacter->GetActiveWeaponName(), HostCharacter->GetRidgefireAmmo(),
			*CoopSmokeRemoteCharacter->GetActiveWeaponName(), CoopSmokeRemoteCharacter->GetRidgefireAmmo());
		GetWorldTimerManager().ClearTimer(CoopCombatSmokeTimer);
		return;
	}
	CoopSmokeHostCharacter = HostCharacter;
	UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP SMOKE PASS independent primary loadouts host=%s ammo=%d remote=%s ammo=%d"),
		*HostCharacter->GetActiveWeaponName(), HostCharacter->GetRidgefireAmmo(),
		*CoopSmokeRemoteCharacter->GetActiveWeaponName(), CoopSmokeRemoteCharacter->GetRidgefireAmmo());
	UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP SMOKE PASS unique possessed pawns: %s and %s"),
		*GetNameSafe(PlayerPawns[0]), *GetNameSafe(PlayerPawns[1]));
	GetWorldTimerManager().ClearTimer(CoopCombatSmokeTimer);
}

void ARidgefireGameMode::VerifyCoopCombatSmokeTest()
{
	const bool bTargetDied = bCoopSmokeTargetKilled && (!CoopSmokeTarget.IsValid() || CoopSmokeTarget->CurrentHP <= 0.0f);
	const bool bSingleKillReward = TotalScore == CoopSmokeInitialScore + 110 && EnemiesRemaining == CoopSmokeInitialEnemies - 1;
	const AShooterCharacter* RewardRecipient = GetGladiator();
	const bool bSingleIonReward = RewardRecipient && FMath::IsNearlyEqual(RewardRecipient->GetIonCharge(), CoopSmokeInitialRewardCharge + 14.0f, 0.1f);
	const bool bHostAmmoUnaffected = CoopSmokeHostCharacter.IsValid() && CoopSmokeHostCharacter->GetRidgefireAmmo() == CoopSmokeInitialHostAmmo;
	if (!CoopSmokeRemoteCharacter.IsValid() || CoopSmokeRemoteCharacter->GetRidgefireAmmo() >= CoopSmokeInitialAmmo || !bHostAmmoUnaffected || !bTargetDied || !bSingleKillReward || !bSingleIonReward)
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE COOP SMOKE FAIL server authority remote_ammo=%d host_ammo=%d host_ammo_unchanged=%s targetDead=%s scoreDelta=%d enemiesDelta=%d ion_delta=%.1f"),
			CoopSmokeRemoteCharacter.IsValid() ? CoopSmokeRemoteCharacter->GetRidgefireAmmo() : -1,
			CoopSmokeHostCharacter.IsValid() ? CoopSmokeHostCharacter->GetRidgefireAmmo() : -1,
			bHostAmmoUnaffected ? TEXT("true") : TEXT("false"),
			bTargetDied ? TEXT("true") : TEXT("false"), TotalScore - CoopSmokeInitialScore, EnemiesRemaining - CoopSmokeInitialEnemies,
			RewardRecipient ? RewardRecipient->GetIonCharge() - CoopSmokeInitialRewardCharge : -1.0f);
		return;
	}
	const int32 ScoreBeforeDuplicate = TotalScore;
	const int32 EnemiesBeforeDuplicate = EnemiesRemaining;
	const float RewardBeforeDuplicate = RewardRecipient->GetIonCharge();
	if (AShooterNPC* Target = CoopSmokeTarget.Get())
	{
		HandleSentinelDeath(Target);
		HandleSentinelDeath(Target);
	}
	if (TotalScore != ScoreBeforeDuplicate || EnemiesRemaining != EnemiesBeforeDuplicate || !FMath::IsNearlyEqual(RewardRecipient->GetIonCharge(), RewardBeforeDuplicate, 0.1f))
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE COOP SMOKE FAIL repeated death callback duplicated score or wave progress"));
		return;
	}
	if (AShooterCharacter* HostCharacter = CoopSmokeHostCharacter.Get())
	{
		HostCharacter->RunRidgefireSecondaryWeaponSmokeTest();
	}
	CoopSmokeRemoteCharacter->RunRidgefireSecondaryWeaponSmokeTest();
	UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP SMOKE PASS server-authoritative remote fire consumed ammo only on remote, killed replicated target, score=110 and ion=14 once despite repeated death callbacks"));
	TransitionToBrassfallFoundry();
	if (bFoundryArenaActive && IsValid(ActiveArenaDressing))
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE COOP SMOKE PASS transitioned host arena to Brassfall for client replication"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE COOP SMOKE FAIL host arena did not transition to Brassfall"));
	}
}
#endif

void ARidgefireGameMode::ResetStreak()
{
	CurrentStreak = 0;
	RefreshRidgefireRunStates();
}

void ARidgefireGameMode::BeginNextWave()
{
	if (bRunOver || bArmoryOpen)
	{
		return;
	}
	if (CurrentWave > 0)
	{
		if (AShooterCharacter* Gladiator = GetGladiator())
		{
			Gladiator->RestoreHealth(60.0f);
			Gladiator->AddRidgefireReserveAmmo(24);
			Gladiator->AddIonCharge(18.0f);
			if (CurrentWave % 3 == 0)
			{
				Gladiator->GrantFieldPatch();
			}
		}
	}
	if (CurrentWave == 1 && !bFoundryArenaActive)
	{
		TransitionToBrassfallFoundry();
	}

	StartWave();
}

void ARidgefireGameMode::GrantBrassfallEntryAmmoRefill()
{
	if (!HasAuthority() || bBrassfallEntryAmmoRefillGranted || !GetWorld())
	{
		return;
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AShooterPlayerController* PlayerController = Cast<AShooterPlayerController>(It->Get());
		if (!IsValid(PlayerController))
		{
			continue;
		}

		FRidgefirePlayerWeaponLoadout& Loadout = GetOrCreatePlayerWeaponLoadout(PlayerController);
		AShooterCharacter* Character = Cast<AShooterCharacter>(PlayerController->GetPawn());
		TSet<int32> RefreshedOffers;
		int32 TotalReserveGranted = 0;
		for (const int32 OfferIndex : Loadout.PrimaryWeaponSlots)
		{
			if (!WeaponOffers.IsValidIndex(OfferIndex) || RefreshedOffers.Contains(OfferIndex)
				|| !Loadout.WeaponReserveAmmo.IsValidIndex(OfferIndex))
			{
				continue;
			}
			RefreshedOffers.Add(OfferIndex);

			const int32 ReserveLimit = FMath::Max(0, WeaponOffers[OfferIndex].ReserveAmmo);
			const bool bActiveOffer = Loadout.ActiveWeaponIndex == OfferIndex && IsValid(Character);
			const int32 CurrentReserve = bActiveOffer ? Character->GetRidgefireReserveAmmo() : Loadout.WeaponReserveAmmo[OfferIndex];
			const int32 RequestedRefill = FMath::CeilToInt(static_cast<float>(ReserveLimit) * BrassfallEntryReserveRefillFraction);
			const int32 Refill = FMath::Min(RequestedRefill, FMath::Max(0, ReserveLimit - CurrentReserve));
			if (bActiveOffer && Refill > 0)
			{
				Character->AddRidgefireReserveAmmo(Refill);
			}
			else if (!bActiveOffer)
			{
				Loadout.WeaponReserveAmmo[OfferIndex] = CurrentReserve + Refill;
			}
			if (bActiveOffer)
			{
				Loadout.WeaponReserveAmmo[OfferIndex] = Character->GetRidgefireReserveAmmo();
			}
			TotalReserveGranted += Refill;
		}
		MirrorFirstPlayerWeaponLoadout(PlayerController, Loadout);
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE: Brassfall entry reserve refill player=%s fraction=%.0f%% carried-primary-rounds=%d (each weapon capped at its offer reserve)."),
			*GetNameSafe(PlayerController), BrassfallEntryReserveRefillFraction * 100.0f, TotalReserveGranted);
	}
	bBrassfallEntryAmmoRefillGranted = true;
}

void ARidgefireGameMode::TransitionToBrassfallFoundry()
{
	if (!HasAuthority() || bRunOver || bFoundryArenaActive || bFoundryAnchorCleared || bBrassfallEntryAmmoRefillGranted)
	{
		return;
	}
	ARidgefireArenaDressing* FoundryDressing = GetWorld()->SpawnActor<ARidgefireArenaDressing>();
	if (!IsValid(FoundryDressing))
	{
		UE_LOG(LogTemp, Warning, TEXT("RIDGEFIRE: Brassfall transition kept the current arena because foundry dressing could not spawn."));
		return;
	}
	FoundryDressing->BuildFoundryArena(ArenaCenter, ArenaGroundZ);
	if (IsValid(ActiveArenaDressing))
	{
		ActiveArenaDressing->Destroy();
	}
	ActiveArenaDressing = FoundryDressing;
	for (TPair<TWeakObjectPtr<AShooterPlayerController>, FRidgefirePlayerWeaponLoadout>& PlayerLoadout : PlayerWeaponLoadouts)
	{
		PlayerLoadout.Value.bPersonalArsenalOpen = false;
	}

	SentinelEntrances.Reset();
	for (int32 Index = 0; Index < 8; ++Index)
	{
		const FRotator Direction(0.0f, Index * 45.0f, 0.0f);
		const FVector SpawnLocation = ArenaCenter + Direction.Vector() * 1650.0f + FVector(0.0f, 0.0f, 30.0f);
		const FRotator FacingCenter = (ArenaCenter - SpawnLocation).Rotation();
		SentinelEntrances.Add(FTransform(FacingCenter, SpawnLocation));
	}
	bFoundryArenaActive = true;
	ArenaDisplayName = TEXT("BRASSFALL FOUNDRY");
#if WITH_EDITOR
	struct FEntryAmmoSmokePlayer
	{
		TWeakObjectPtr<AShooterPlayerController> Controller;
		TWeakObjectPtr<AShooterCharacter> Character;
		FRidgefirePlayerWeaponLoadout SavedLoadout;
		int32 SavedCharacterAmmo = 0;
		int32 SavedCharacterReserveAmmo = 0;
		TArray<int32> OfferIndexes;
		TArray<int32> ExpectedReserveAfterRefill;
	};
	const bool bRunEntryAmmoSmoke = bRunSmokeTest || bRunCoopCombatSmokeTest;
	bool bEntryAmmoSmokePrepared = bRunEntryAmmoSmoke;
	TArray<FEntryAmmoSmokePlayer> EntryAmmoSmokePlayers;
	if (bRunEntryAmmoSmoke)
	{
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			AShooterPlayerController* PlayerController = Cast<AShooterPlayerController>(It->Get());
			AShooterCharacter* Character = PlayerController ? Cast<AShooterCharacter>(PlayerController->GetPawn()) : nullptr;
			if (!IsValid(PlayerController) || !IsValid(Character))
			{
				bEntryAmmoSmokePrepared = false;
				continue;
			}

			FRidgefirePlayerWeaponLoadout& Loadout = GetOrCreatePlayerWeaponLoadout(PlayerController);
			FEntryAmmoSmokePlayer& Snapshot = EntryAmmoSmokePlayers.AddDefaulted_GetRef();
			Snapshot.Controller = PlayerController;
			Snapshot.Character = Character;
			Snapshot.SavedLoadout = Loadout;
			Snapshot.SavedCharacterAmmo = Character->GetRidgefireAmmo();
			Snapshot.SavedCharacterReserveAmmo = Character->GetRidgefireReserveAmmo();
			TSet<int32> SeenOffers;
			for (const int32 OfferIndex : Loadout.PrimaryWeaponSlots)
			{
				if (!WeaponOffers.IsValidIndex(OfferIndex) || SeenOffers.Contains(OfferIndex)
					|| !Loadout.WeaponReserveAmmo.IsValidIndex(OfferIndex))
				{
					continue;
				}
				SeenOffers.Add(OfferIndex);
				const int32 ReserveLimit = FMath::Max(0, WeaponOffers[OfferIndex].ReserveAmmo);
				const int32 SmokeStartingReserve = ReserveLimit / 2;
				if (Loadout.ActiveWeaponIndex == OfferIndex)
				{
					Character->EquipRidgefireWeapon(WeaponOffers[OfferIndex], Snapshot.SavedCharacterAmmo, SmokeStartingReserve);
				}
				else
				{
					Loadout.WeaponReserveAmmo[OfferIndex] = SmokeStartingReserve;
				}
				Snapshot.OfferIndexes.Add(OfferIndex);
				Snapshot.ExpectedReserveAfterRefill.Add(SmokeStartingReserve
					+ FMath::Min(FMath::CeilToInt(static_cast<float>(ReserveLimit) * BrassfallEntryReserveRefillFraction), ReserveLimit - SmokeStartingReserve));
			}
			MirrorFirstPlayerWeaponLoadout(PlayerController, Loadout);
			bEntryAmmoSmokePrepared &= !Snapshot.OfferIndexes.IsEmpty();
		}
		bEntryAmmoSmokePrepared &= !EntryAmmoSmokePlayers.IsEmpty();
	}
#endif
	GrantBrassfallEntryAmmoRefill();
#if WITH_EDITOR
	if (bRunEntryAmmoSmoke)
	{
		auto HasExpectedEntryReserves = [this](const FEntryAmmoSmokePlayer& Snapshot)
		{
			AShooterPlayerController* PlayerController = Snapshot.Controller.Get();
			AShooterCharacter* Character = Snapshot.Character.Get();
			const FRidgefirePlayerWeaponLoadout* Loadout = PlayerController
				? PlayerWeaponLoadouts.Find(TWeakObjectPtr<AShooterPlayerController>(PlayerController)) : nullptr;
			if (!PlayerController || !Character || !Loadout || Snapshot.OfferIndexes.Num() != Snapshot.ExpectedReserveAfterRefill.Num())
			{
				return false;
			}
			for (int32 OfferPosition = 0; OfferPosition < Snapshot.OfferIndexes.Num(); ++OfferPosition)
			{
				const int32 OfferIndex = Snapshot.OfferIndexes[OfferPosition];
				if (!WeaponOffers.IsValidIndex(OfferIndex) || !Loadout->WeaponReserveAmmo.IsValidIndex(OfferIndex))
				{
					return false;
				}
				const int32 CurrentReserve = Loadout->ActiveWeaponIndex == OfferIndex
					? Character->GetRidgefireReserveAmmo() : Loadout->WeaponReserveAmmo[OfferIndex];
				if (CurrentReserve != Snapshot.ExpectedReserveAfterRefill[OfferPosition]
					|| CurrentReserve > WeaponOffers[OfferIndex].ReserveAmmo)
				{
					return false;
				}
			}
			return true;
		};

		bool bEachPlayerReceivedEntryRefill = bEntryAmmoSmokePrepared;
		for (const FEntryAmmoSmokePlayer& Snapshot : EntryAmmoSmokePlayers)
		{
			bEachPlayerReceivedEntryRefill &= HasExpectedEntryReserves(Snapshot);
		}
		TransitionToBrassfallFoundry();
		bool bDuplicateTransitionDidNotRefill = true;
		for (const FEntryAmmoSmokePlayer& Snapshot : EntryAmmoSmokePlayers)
		{
			bDuplicateTransitionDidNotRefill &= HasExpectedEntryReserves(Snapshot);
		}
		for (const FEntryAmmoSmokePlayer& Snapshot : EntryAmmoSmokePlayers)
		{
			AShooterPlayerController* PlayerController = Snapshot.Controller.Get();
			AShooterCharacter* Character = Snapshot.Character.Get();
			if (!PlayerController || !Character)
			{
				continue;
			}
			PlayerWeaponLoadouts.FindOrAdd(TWeakObjectPtr<AShooterPlayerController>(PlayerController)) = Snapshot.SavedLoadout;
			if (WeaponOffers.IsValidIndex(Snapshot.SavedLoadout.ActiveWeaponIndex))
			{
				Character->EquipRidgefireWeapon(WeaponOffers[Snapshot.SavedLoadout.ActiveWeaponIndex],
					Snapshot.SavedCharacterAmmo, Snapshot.SavedCharacterReserveAmmo);
			}
			MirrorFirstPlayerWeaponLoadout(PlayerController, Snapshot.SavedLoadout);
		}
		if (bEachPlayerReceivedEntryRefill && bDuplicateTransitionDidNotRefill)
		{
			UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SMOKE PASS: Brassfall entry reserve refill granted once to %d connected players at %.0f%% per carried primary, capped; duplicate transition granted zero."),
				EntryAmmoSmokePlayers.Num(), BrassfallEntryReserveRefillFraction * 100.0f);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SMOKE FAIL: Brassfall entry refill player-isolation=%s duplicate-no-refill=%s players=%d."),
				bEachPlayerReceivedEntryRefill ? TEXT("true") : TEXT("false"),
				bDuplicateTransitionDidNotRefill ? TEXT("true") : TEXT("false"), EntryAmmoSmokePlayers.Num());
		}
	}
#endif
	UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE: Wave clear transitioned the run to Brassfall Foundry with %d furnace-facing entrances."), SentinelEntrances.Num());
	RefreshRidgefireRunStates();
}

bool ARidgefireGameMode::ActivateFoundryFurnaceAnchor(AShooterPlayerController* PlayerController)
{
	if (!HasAuthority() || !bFoundryArenaActive || CurrentWave != 2 || bFoundryAnchorCleared || !IsValid(PlayerController)
		|| (PlayerController->GetPawn() && (PlayerController->GetPawn()->GetActorLocation() - ArenaCenter).SizeSquared2D() > FMath::Square(420.0f)))
	{
		return false;
	}
	AShooterCharacter* Player = Cast<AShooterCharacter>(PlayerController->GetPawn());
	if (!IsValid(Player) || Player->IsDead())
	{
		return false;
	}
	const FName FurnaceAnchorId(TEXT("BrassfallFurnaceAnchor"));
	const FRidgefireObjectiveState* FurnaceObjective = OptionalObjectives.FindByPredicate([FurnaceAnchorId](const FRidgefireObjectiveState& Objective) { return Objective.Id == FurnaceAnchorId; });
	if (!FurnaceObjective || FurnaceObjective->bCompleted || FurnaceObjective->bFailed)
	{
		return false;
	}

	AdvanceOptionalObjective(FurnaceAnchorId, 1);
	FurnaceObjective = OptionalObjectives.FindByPredicate([FurnaceAnchorId](const FRidgefireObjectiveState& Objective) { return Objective.Id == FurnaceAnchorId; });
	bFoundryAnchorCleared = FurnaceObjective && FurnaceObjective->bCompleted;
	if (bFoundryAnchorCleared)
	{
		Player->AddRidgefireReserveAmmo(36);
		Player->RestoreHealth(35.0f);
		Player->AddIonCharge(20.0f);
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE: Brassfall furnace anchor stabilized; optional reserve, health, and ion charge granted."));
	}
	RefreshRidgefireRunStates();
	return bFoundryAnchorCleared;
}

AShooterCharacter* ARidgefireGameMode::GetGladiator() const
{
	return Cast<AShooterCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
}

void ARidgefireGameMode::RefreshRidgefireRunStates()
{
	if (!HasAuthority() || !GetWorld())
	{
		return;
	}
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		SendRidgefireRunState(Cast<AShooterPlayerController>(It->Get()));
	}
}

void ARidgefireGameMode::SendRidgefireRunState(AShooterPlayerController* PlayerController)
{
	if (!HasAuthority() || !IsValid(PlayerController))
	{
		return;
	}
	FRidgefireClientRunState State;
	const FRidgefirePlayerWeaponLoadout& Loadout = GetOrCreatePlayerWeaponLoadout(PlayerController);
	State.Wave = CurrentWave;
	State.EnemiesRemaining = EnemiesRemaining;
	State.Score = TotalScore;
	State.Streak = CurrentStreak;
	State.ArmorySecondsRemaining = GetArmorySecondsRemaining();
	State.ActiveWeaponIndex = Loadout.ActiveWeaponIndex;
	State.bArmoryOpen = bArmoryOpen;
	State.bRunOver = bRunOver;
	State.ObjectiveText = GetObjectiveDisplayText();
	State.ArenaName = GetArenaDisplayName();
	State.WeaponOffers = WeaponOffers;
	State.PrimaryWeaponSlots = Loadout.PrimaryWeaponSlots;
	State.SecondaryWeaponOfferIndex = Loadout.SecondaryWeaponOfferIndex;
	State.bPersonalArsenalOpen = Loadout.bPersonalArsenalOpen;
	State.WildcardRerollTokens = WildcardRerollTokens;
	State.WildcardRerollCount = WildcardRerollCount;
	State.bWildcardClaimed = IsWildcardClaimed();
	PlayerController->SetRidgefireRunState(State);
}

int32 ARidgefireGameMode::GetWave() const
{
	return CurrentWave;
}

int32 ARidgefireGameMode::GetEnemiesRemaining() const
{
	return EnemiesRemaining;
}

int32 ARidgefireGameMode::GetScore() const
{
	return TotalScore;
}

int32 ARidgefireGameMode::GetStreak() const
{
	return CurrentStreak;
}

FString ARidgefireGameMode::GetArenaDisplayName() const
{
	return ArenaDisplayName;
}

FString ARidgefireGameMode::GetObjectiveDisplayText() const
{
	FString DisplayText;
	if (bArmoryOpen)
	{
		DisplayText = TEXT("CHOOSE YOUR STARTING WEAPON");
	}
	else if (bRunOver)
	{
		DisplayText = TEXT("RUN COMPLETE");
	}
	else if (WaveObjective.bCompleted)
	{
		DisplayText = TEXT("ARENA CLEARED  /  NEXT WAVE INCOMING");
	}
	else if (WaveObjective.Goal > 0)
	{
		DisplayText = FString::Printf(TEXT("%s  %02d / %02d"), *WaveObjective.Title, WaveObjective.Progress, WaveObjective.Goal);
	}
	else
	{
		DisplayText = TEXT("PREPARE FOR THE NEXT WAVE");
	}

	for (const FRidgefireObjectiveState& Objective : OptionalObjectives)
	{
		if (Objective.bFailed)
		{
			return FString::Printf(TEXT("%s  /  OPTIONAL LOST: %s"), *DisplayText, *Objective.Title);
		}
		if (Objective.bCompleted)
		{
			return FString::Printf(TEXT("%s  /  OPTIONAL COMPLETE: %s"), *DisplayText, *Objective.Title);
		}
		if (!Objective.bCompleted && !Objective.bFailed)
		{
			return FString::Printf(TEXT("%s  /  OPTIONAL: %s %d/%d"), *DisplayText, *Objective.Title, Objective.Progress, Objective.Goal);
		}
	}
	return DisplayText;
}

void ARidgefireGameMode::RegisterOptionalObjective(FName Id, const FString& Title, int32 Goal)
{
	if (Id.IsNone() || Title.IsEmpty() || Goal <= 0 || (Id == FName(TEXT("BrassfallFurnaceAnchor")) && bFoundryAnchorCleared))
	{
		return;
	}
	if (OptionalObjectives.ContainsByPredicate([Id](const FRidgefireObjectiveState& Objective) { return Objective.Id == Id; }))
	{
		return;
	}
	FRidgefireObjectiveState Objective;
	Objective.Id = Id;
	Objective.Title = Title;
	Objective.Goal = Goal;
	Objective.bOptional = true;
	OptionalObjectives.Add(MoveTemp(Objective));
	RefreshRidgefireRunStates();
}

void ARidgefireGameMode::AdvanceOptionalObjective(FName Id, int32 Amount)
{
	if (Amount <= 0 || (Id == FName(TEXT("BrassfallFurnaceAnchor")) && bFoundryAnchorCleared))
	{
		return;
	}
	if (FRidgefireObjectiveState* Objective = OptionalObjectives.FindByPredicate([Id](const FRidgefireObjectiveState& Candidate) { return Candidate.Id == Id; }))
	{
		AdvanceObjective(*Objective, Amount);
		RefreshRidgefireRunStates();
	}
}

void ARidgefireGameMode::FailOptionalObjective(FName Id)
{
	if (FRidgefireObjectiveState* Objective = OptionalObjectives.FindByPredicate([Id](const FRidgefireObjectiveState& Candidate) { return Candidate.Id == Id; }))
	{
		if (Objective->bOptional && !Objective->bCompleted)
		{
			Objective->bFailed = true;
			RefreshRidgefireRunStates();
		}
	}
}

void ARidgefireGameMode::AdvanceObjective(FRidgefireObjectiveState& Objective, int32 Amount)
{
	if (Amount <= 0 || Objective.Goal <= 0 || Objective.bCompleted || Objective.bFailed)
	{
		return;
	}
	Objective.Progress = Amount >= Objective.Goal - Objective.Progress ? Objective.Goal : Objective.Progress + Amount;
	Objective.bCompleted = Objective.Progress == Objective.Goal;
}

bool ARidgefireGameMode::IsArmoryOpen() const
{
	return bArmoryOpen;
}

int32 ARidgefireGameMode::GetArmorySecondsRemaining() const
{
	if (!bArmoryOpen || !GetWorld())
	{
		return 0;
	}
	return FMath::Max(0, FMath::CeilToInt(GetWorld()->GetTimerManager().GetTimerRemaining(ArmoryTimeoutTimer)));
}

int32 ARidgefireGameMode::GetOfferCount() const
{
	return WeaponOffers.Num();
}

const FRidgefireWeaponRoll* ARidgefireGameMode::GetOffer(int32 Index) const
{
	return WeaponOffers.IsValidIndex(Index) ? &WeaponOffers[Index] : nullptr;
}
