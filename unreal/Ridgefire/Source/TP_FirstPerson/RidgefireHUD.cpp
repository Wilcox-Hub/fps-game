#include "RidgefireHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Math/RotationMatrix.h"
#include "RidgefireArenaDressing.h"
#include "RidgefireGameMode.h"
#include "ShooterCharacter.h"
#include "ShooterPlayerController.h"

void ARidgefireHUD::DrawHUD()
{
	Super::DrawHUD();
	if (!Canvas || !PlayerOwner)
	{
		return;
	}

	const float Width = Canvas->ClipX;
	const float Height = Canvas->ClipY;
	const FLinearColor Ember(1.0f, 0.46f, 0.20f, 1.0f);
	const FLinearColor Ice(0.48f, 0.92f, 0.84f, 1.0f);
	const FLinearColor Paper(0.98f, 0.91f, 0.77f, 1.0f);
	const FLinearColor Shade(0.025f, 0.035f, 0.045f, 0.72f);
	UFont* Font = GEngine ? GEngine->GetMediumFont() : nullptr;
	if (!Font)
	{
		return;
	}
	auto FitScale = [this, Font](const FString& Text, float MaximumWidth, float PreferredScale)
	{
		float TextWidth = 0.0f;
		float TextHeight = 0.0f;
		Canvas->StrLen(Font, Text, TextWidth, TextHeight);
		return TextWidth > 0.0f ? FMath::Min(PreferredScale, MaximumWidth / TextWidth) : PreferredScale;
	};
	auto DrawCentered = [this, &FitScale, Font, Width](const FString& Text, const FLinearColor& Color, float CenterY, float MaximumWidth, float PreferredScale)
	{
		const float Scale = FitScale(Text, MaximumWidth, PreferredScale);
		float TextWidth = 0.0f;
		float TextHeight = 0.0f;
		Canvas->StrLen(Font, Text, TextWidth, TextHeight);
		DrawText(Text, Color, (Width - TextWidth * Scale) * 0.5f, CenterY, Font, Scale);
	};

	const ARidgefireGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ARidgefireGameMode>() : nullptr;
	const AShooterPlayerController* RidgefireController = Cast<AShooterPlayerController>(PlayerOwner);
	const FRidgefireClientRunState* ClientRunState = RidgefireController ? &RidgefireController->GetRidgefireRunState() : nullptr;
	const bool bArmoryOpen = GameMode ? GameMode->IsArmoryOpen() : ClientRunState && ClientRunState->bArmoryOpen;
	const bool bRunOver = GameMode ? GameMode->IsRunOver() : ClientRunState && ClientRunState->bRunOver;
	const bool bPersonalArsenalOpen = ClientRunState && ClientRunState->bPersonalArsenalOpen && !bRunOver;
	const int32 RerollTokens = GameMode ? GameMode->GetWildcardRerollTokens() : ClientRunState ? ClientRunState->WildcardRerollTokens : 0;
	const int32 RerollCount = GameMode ? GameMode->GetWildcardRerollCount() : ClientRunState ? ClientRunState->WildcardRerollCount : 0;
	const bool bWildcardClaimed = GameMode ? GameMode->IsWildcardClaimed() : ClientRunState && ClientRunState->bWildcardClaimed;
	const int32 OfferCount = GameMode ? GameMode->GetOfferCount() : ClientRunState ? ClientRunState->WeaponOffers.Num() : 0;
	const int32 ActiveWeaponIndex = GameMode ? GameMode->GetActiveWeaponIndex() : ClientRunState ? ClientRunState->ActiveWeaponIndex : INDEX_NONE;
	const FRidgefireWeaponRoll* WildcardOffer = nullptr;
	auto GetOffer = [GameMode, ClientRunState](int32 Index) -> const FRidgefireWeaponRoll*
	{
		if (GameMode)
		{
			return GameMode->GetOffer(Index);
		}
		return ClientRunState && ClientRunState->WeaponOffers.IsValidIndex(Index) ? &ClientRunState->WeaponOffers[Index] : nullptr;
	};
	WildcardOffer = GetOffer(2);
	auto GetPrimarySlotOffer = [GameMode, ClientRunState](int32 Slot)
	{
		if (GameMode)
		{
			return GameMode->GetPrimarySlotOffer(Slot);
		}
		return ClientRunState && ClientRunState->PrimaryWeaponSlots.IsValidIndex(Slot)
			? ClientRunState->PrimaryWeaponSlots[Slot]
			: INDEX_NONE;
	};
	if (PlayerOwner->IsPaused())
	{
		DrawRect(FLinearColor(0.008f, 0.012f, 0.020f, 0.76f), 0.0f, 0.0f, Width, Height);
		DrawCentered(TEXT("PIT PAUSED"), Paper, Height * 0.44f, Width * 0.88f, FMath::Clamp(FMath::Min(Width, Height) / 500.0f, 0.75f, 1.35f));
		const FString ResumeText = bArmoryOpen
			? TEXT("P / ESC / MENU  TO RETURN TO THE ARMORY")
			: TEXT("P / ESC / MENU  TO RETURN TO THE FIGHT");
		DrawCentered(ResumeText, Ice, Height * 0.53f, Width * 0.88f, FMath::Clamp(FMath::Min(Width, Height) / 920.0f, 0.58f, 0.78f));
		return;
	}
	if (bArmoryOpen || bPersonalArsenalOpen)
	{
		DrawRect(FLinearColor(0.015f, 0.022f, 0.032f, 0.96f), 0.0f, 0.0f, Width, Height);
		DrawRect(Ember, 0.0f, 0.0f, Width, 7.0f);
		const float Margin = FMath::Clamp(Width * 0.04f, 16.0f, 48.0f);
		const bool bPortrait = Height > Width * 1.15f;
		const bool bCompactCards = Width < 1000.0f && !bPortrait;
		const bool bStackCards = bPortrait || bCompactCards;
		const float HeaderY = bPortrait ? Height * 0.075f : bCompactCards ? Height * 0.065f : Height * 0.16f;
		const FString ArmoryTitle = bPersonalArsenalOpen ? TEXT("ARSENAL") : TEXT("IRON SUN");
		const FString ArmorySubtitle = bPersonalArsenalOpen ? TEXT("SELECT YOUR SECONDARY WEAPON") : TEXT("CHOOSE YOUR ENTRY WEAPON");
		DrawCentered(ArmoryTitle, Paper, HeaderY, Width - Margin * 2.0f, FMath::Clamp(FMath::Min(Width, Height) / 460.0f, 0.85f, 1.55f));
		DrawCentered(ArmorySubtitle, Ice, HeaderY + FMath::Min(48.0f, Height * 0.065f), Width - Margin * 2.0f, FMath::Clamp(FMath::Min(Width, Height) / 780.0f, 0.62f, 0.92f));

		const float Gap = bStackCards ? 12.0f : 22.0f;
		const float CardWidth = bStackCards
			? Width - Margin * 2.0f
			: FMath::Min(380.0f, (Width - Margin * 2.0f - Gap * 2.0f) / 3.0f);
		const float CardHeight = bCompactCards
			? FMath::Min(120.0f, (Height * 0.56f - Gap * 2.0f) / 3.0f)
			: bPortrait
			? FMath::Min(230.0f, (Height * 0.60f - Gap * 2.0f) / 3.0f)
			: FMath::Min(230.0f, Height * 0.38f);
		const float StartX = bStackCards ? Margin : (Width - (CardWidth * 3.0f + Gap * 2.0f)) * 0.5f;
		const float CardY = bPortrait ? Height * 0.205f : bCompactCards ? Height * 0.19f : Height * 0.36f;
		const float CardScale = FMath::Clamp(FMath::Min(CardWidth / 380.0f, CardHeight / 230.0f), 0.42f, 1.0f);
		auto DrawWrappedText = [this, Font](const FString& Text, const FLinearColor& Color, float X, float Y, float MaxWidth, float MaxHeight, float Scale)
		{
			TArray<FString> Words;
			Text.ParseIntoArray(Words, TEXT(" "), true);
			FString Line;
			float LineY = Y;
			int32 LinesDrawn = 0;
			for (const FString& Word : Words)
			{
				const FString Candidate = Line.IsEmpty() ? Word : Line + TEXT(" ") + Word;
				float TextWidth = 0.0f;
				float TextHeight = 0.0f;
				Canvas->StrLen(Font, Candidate, TextWidth, TextHeight);
				if (!Line.IsEmpty() && TextWidth * Scale > MaxWidth)
				{
					if (LineY + TextHeight * Scale > Y + MaxHeight || LinesDrawn >= 4)
					{
						return;
					}
					DrawText(Line, Color, X, LineY, Font, Scale);
					LineY += TextHeight * Scale + 2.0f;
					++LinesDrawn;
					Line = Word;
				}
				else
				{
					Line = Candidate;
				}
			}
			if (!Line.IsEmpty())
			{
				float TextWidth = 0.0f;
				float TextHeight = 0.0f;
				Canvas->StrLen(Font, Line, TextWidth, TextHeight);
				if (TextWidth * Scale <= MaxWidth && LineY + TextHeight * Scale <= Y + MaxHeight)
				{
					DrawText(Line, Color, X, LineY, Font, Scale);
				}
			}
		};
		for (int32 Index = 0; Index < OfferCount; ++Index)
		{
			const FRidgefireWeaponRoll* Offer = GetOffer(Index);
			if (!Offer)
			{
				continue;
			}

			const float CardX = bStackCards ? StartX : StartX + Index * (CardWidth + Gap);
			const float ThisCardY = bStackCards ? CardY + Index * (CardHeight + Gap) : CardY;
			const FLinearColor Accent = Offer->bWildcard ? Ember : Index == 1 ? FLinearColor(0.78f, 0.84f, 0.91f, 1.0f) : Ice;
			const float TextX = CardX + FMath::Min(20.0f, CardWidth * 0.05f);
			const float TextWidth = CardWidth - (TextX - CardX) * 2.0f;
			DrawRect(FLinearColor(0.035f, 0.045f, 0.055f, 0.97f), CardX, ThisCardY, CardWidth, CardHeight);
			DrawRect(Accent, CardX, ThisCardY, CardWidth, 4.0f);
			if (bCompactCards)
			{
				const float CompactScale = FMath::Clamp(CardHeight / 115.0f, 0.78f, 0.95f);
				const float RightX = CardX + CardWidth * 0.67f;
				const float LeftWidth = RightX - TextX - 14.0f;
				const float RightWidth = CardX + CardWidth - RightX - 14.0f;
				const FString Label = FString::Printf(TEXT("0%d  /  %s"), Index + 1, Offer->bWildcard ? TEXT("WILDCARD") : TEXT("PIT-TESTED"));
				const FString Stats = FString::Printf(TEXT("DMG x%.2f   MAG %d"), Offer->DamageScale, Offer->MagazineSize);
				const FString Payload = FString::Printf(TEXT("PAYLOAD: %s"), *Offer->PayloadName);
				DrawText(Label, Accent, TextX, ThisCardY + 8.0f, Font, FitScale(Label, LeftWidth, 0.58f * CompactScale));
				DrawText(Offer->Name, Paper, TextX, ThisCardY + CardHeight * 0.25f, Font, FitScale(Offer->Name, LeftWidth, 0.9f * CompactScale));
				DrawText(Offer->ModeName, Accent, TextX, ThisCardY + CardHeight * 0.48f, Font, FitScale(Offer->ModeName, LeftWidth, 0.62f * CompactScale));
				DrawWrappedText(Offer->Description, Paper, TextX, ThisCardY + CardHeight * 0.68f, LeftWidth, CardHeight * 0.27f, 0.65f * CompactScale);
				DrawText(Stats, Ice, RightX, ThisCardY + CardHeight * 0.30f, Font, FitScale(Stats, RightWidth, 0.70f * CompactScale));
				DrawText(Payload, Accent, RightX, ThisCardY + CardHeight * 0.55f, Font, FitScale(Payload, RightWidth, 0.65f * CompactScale));
				continue;
			}
			DrawText(FString::Printf(TEXT("0%d  /  %s"), Index + 1, Offer->bWildcard ? TEXT("WILDCARD") : TEXT("PIT-TESTED")), Accent, TextX, ThisCardY + 16.0f * CardScale, Font, FitScale(FString::Printf(TEXT("0%d  /  %s"), Index + 1, Offer->bWildcard ? TEXT("WILDCARD") : TEXT("PIT-TESTED")), TextWidth, 0.78f * CardScale));
			DrawText(Offer->Name, Paper, TextX, ThisCardY + 48.0f * CardScale, Font, FitScale(Offer->Name, TextWidth, 1.0f * CardScale));
			DrawText(Offer->ModeName, Accent, TextX, ThisCardY + 79.0f * CardScale, Font, FitScale(Offer->ModeName, TextWidth, 0.72f * CardScale));
			const float DescriptionY = ThisCardY + 105.0f * CardScale;
			const float FooterY = ThisCardY + CardHeight - 42.0f * CardScale;
			DrawWrappedText(Offer->Description, Paper, TextX, DescriptionY, TextWidth, FMath::Max(0.0f, FooterY - DescriptionY - 4.0f), 0.74f * CardScale);
			const FString Stats = FString::Printf(TEXT("DMG x%.2f   MAG %d"), Offer->DamageScale, Offer->MagazineSize);
			const FString Payload = FString::Printf(TEXT("PAYLOAD: %s"), *Offer->PayloadName);
			DrawText(Stats, Ice, TextX, FooterY, Font, FitScale(Stats, TextWidth, 0.68f * CardScale));
			DrawText(Payload, Accent, TextX, ThisCardY + CardHeight - 20.0f * CardScale, Font, FitScale(Payload, TextWidth, 0.62f * CardScale));
		}

		if (bPersonalArsenalOpen)
		{
			DrawCentered(TEXT("1 / 2 / 3  OR  A / B / X  TO SELECT"), Paper, Height * (bPortrait ? 0.84f : 0.78f), Width - Margin * 2.0f, FMath::Clamp(FMath::Min(Width, Height) / 800.0f, 0.62f, 0.9f));
			DrawCentered(TEXT("PICK AN OFFER NOT ALREADY IN PRIMARY 1 OR 2"), Ember, Height * (bPortrait ? 0.89f : 0.83f), Width - Margin * 2.0f, FMath::Clamp(FMath::Min(Width, Height) / 1050.0f, 0.48f, 0.68f));
			DrawCentered(TEXT("T / VIEW TO CLOSE  /  SELECTION ONLY AFFECTS YOU"), Ice, Height * (bPortrait ? 0.94f : 0.89f), Width - Margin * 2.0f, FMath::Clamp(FMath::Min(Width, Height) / 1000.0f, 0.5f, 0.7f));
		}
		else
		{
			DrawCentered(TEXT("1 / 2 / 3  OR  A / B / X  TO ENTER THE PIT"), Paper, Height * (bPortrait ? 0.84f : 0.78f), Width - Margin * 2.0f, FMath::Clamp(FMath::Min(Width, Height) / 800.0f, 0.62f, 0.9f));
			DrawCentered(TEXT("THE WILDCARD IS DIFFERENT EVERY RUN. THE PIT OWES YOU NO BALANCE."), Ember, Height * (bPortrait ? 0.89f : 0.83f), Width - Margin * 2.0f, FMath::Clamp(FMath::Min(Width, Height) / 1050.0f, 0.48f, 0.68f));
			const int32 ArmorySecondsRemaining = GameMode ? GameMode->GetArmorySecondsRemaining() : ClientRunState ? ClientRunState->ArmorySecondsRemaining : 0;
			DrawCentered(FString::Printf(TEXT("AUTO-DEPLOY IN %02d"), ArmorySecondsRemaining), Ice, Height * (bPortrait ? 0.94f : 0.89f), Width - Margin * 2.0f, FMath::Clamp(FMath::Min(Width, Height) / 1000.0f, 0.5f, 0.7f));
		}
		return;
	}

	if (bRunOver)
	{
		DrawRect(FLinearColor(0.008f, 0.012f, 0.020f, 0.86f), 0.0f, 0.0f, Width, Height);
		DrawRect(Ember, 0.0f, 0.0f, Width, 7.0f);
		DrawCentered(TEXT("THE PIT CLAIMS ANOTHER"), Ember, Height * 0.34f, Width * 0.88f, FMath::Clamp(FMath::Min(Width, Height) / 500.0f, 0.75f, 1.35f));
		const int32 Wave = GameMode ? GameMode->GetWave() : ClientRunState ? ClientRunState->Wave : 0;
		const int32 Score = GameMode ? GameMode->GetScore() : ClientRunState ? ClientRunState->Score : 0;
		DrawCentered(FString::Printf(TEXT("WAVE %02d     /     SCORE %06d"), Wave, Score), Paper, Height * 0.44f, Width * 0.88f, FMath::Clamp(FMath::Min(Width, Height) / 680.0f, 0.7f, 1.0f));
		const bool bGamepadControls = RidgefireController && RidgefireController->IsUsingGamepad();
		const FString GameOverControls = bGamepadControls
			? TEXT("R3 TO ROLL AGAIN    /    MENU KEEPS GAME OPEN")
			: TEXT("R TO ROLL AGAIN    /    ESC KEEPS GAME OPEN");
		DrawCentered(GameOverControls, Ice, Height * 0.55f, Width * 0.88f, FMath::Clamp(FMath::Min(Width, Height) / 840.0f, 0.58f, 0.82f));
		return;
	}

	const FString CurrentArenaName = GameMode ? GameMode->GetArenaDisplayName() : ClientRunState ? ClientRunState->ArenaName : FString();
	if (CurrentArenaName.Equals(TEXT("IRON SUN ARENA"), ESearchCase::IgnoreCase) && !bArmoryOpen && !bPersonalArsenalOpen)
	{
		const APawn* PlayerPawn = PlayerOwner->GetPawn();
		FVector ArsenalStationLocation = FVector::ZeroVector;
		bool bFoundArsenalStation = false;
		for (TActorIterator<ARidgefireArenaDressing> It(GetWorld()); It; ++It)
		{
			if (It->GetArsenalStationWorldLocation(ArsenalStationLocation))
			{
				bFoundArsenalStation = true;
				break;
			}
		}

		if (IsValid(PlayerPawn) && bFoundArsenalStation
			&& FVector::DistSquared2D(PlayerPawn->GetActorLocation(), ArsenalStationLocation) <= FMath::Square(500.0f))
		{
			const FVector ToArsenal = (ArsenalStationLocation - PlayerPawn->GetActorLocation()).GetSafeNormal2D();
			const FRotator ViewYaw(0.0f, PlayerOwner->GetControlRotation().Yaw, 0.0f);
			const FVector ViewForward = FRotationMatrix(ViewYaw).GetUnitAxis(EAxis::X);
			const FVector ViewRight = FRotationMatrix(ViewYaw).GetUnitAxis(EAxis::Y);
			const float ForwardAlignment = FVector::DotProduct(ToArsenal, ViewForward);
			const float RightAlignment = FVector::DotProduct(ToArsenal, ViewRight);
			const TCHAR* Direction = ForwardAlignment > 0.65f ? TEXT("AHEAD")
				: ForwardAlignment < -0.65f ? TEXT("BEHIND")
				: RightAlignment >= 0.0f ? TEXT("RIGHT") : TEXT("LEFT");
			const FString Action = RidgefireController && RidgefireController->IsUsingGamepad() ? TEXT("VIEW") : TEXT("T");
			const FString ArsenalPrompt = FString::Printf(TEXT("ARSENAL %s  /  %s TO OPEN"), Direction, *Action);
			const float PromptScale = FMath::Clamp(FMath::Min(Width / 1280.0f, Height / 720.0f), 0.72f, 1.10f);
			const float PromptMargin = FMath::Max(20.0f, 30.0f * PromptScale);
			const float PromptWidth = FMath::Min(360.0f * PromptScale, Width - PromptMargin * 2.0f);
			const float PromptHeight = 34.0f * PromptScale;
			const float PromptX = (Width - PromptWidth) * 0.5f;
			const float PromptY = Height * 0.68f;
			DrawRect(Shade, PromptX, PromptY, PromptWidth, PromptHeight);
			DrawRect(Ember, PromptX, PromptY, 4.0f * PromptScale, PromptHeight);
			DrawText(ArsenalPrompt, Paper, PromptX + 14.0f * PromptScale, PromptY + 8.0f * PromptScale, Font,
				FitScale(ArsenalPrompt, PromptWidth - 28.0f * PromptScale, 0.88f * PromptScale));
		}
	}

	const float HudScale = FMath::Clamp(FMath::Min(Width / 1280.0f, Height / 720.0f), 0.72f, 1.10f);
	const float HudMargin = FMath::Max(20.0f, 30.0f * HudScale);
	const bool bPortraitCombatLayout = Height > Width * 1.15f;
	const float LeftPanelWidth = bPortraitCombatLayout
		? Width - HudMargin * 2.0f
		: FMath::Min(420.0f * HudScale, Width * 0.40f);
	const float LeftPanelHeight = 180.0f * HudScale;
	const FString ArenaName = GameMode ? GameMode->GetArenaDisplayName() : ClientRunState ? ClientRunState->ArenaName : TEXT("IRON SUN ARENA");
	DrawRect(Shade, HudMargin, HudMargin, LeftPanelWidth, LeftPanelHeight);
	DrawRect(Ember, HudMargin, HudMargin, 4.0f * HudScale, LeftPanelHeight);
	DrawText(TEXT("IRON SUN"), Paper, HudMargin + 14.0f * HudScale, HudMargin + 9.0f * HudScale, Font,
		FitScale(TEXT("IRON SUN"), LeftPanelWidth - 28.0f * HudScale, 1.1f * HudScale));
	DrawText(ArenaName, Ember, HudMargin + 14.0f * HudScale, HudMargin + 32.0f * HudScale, Font,
		FitScale(ArenaName, LeftPanelWidth - 28.0f * HudScale, 0.92f * HudScale));

	if (GameMode || ClientRunState)
	{
		const int32 Wave = GameMode ? GameMode->GetWave() : ClientRunState->Wave;
		const int32 EnemiesRemaining = GameMode ? GameMode->GetEnemiesRemaining() : ClientRunState->EnemiesRemaining;
		const int32 CurrentScore = GameMode ? GameMode->GetScore() : ClientRunState->Score;
		const int32 Streak = GameMode ? GameMode->GetStreak() : ClientRunState->Streak;
		const FString WaveStatus = FString::Printf(TEXT("WAVE %02d     /     SENTRIES %02d"), Wave, EnemiesRemaining);
		const FString Score = FString::Printf(TEXT("SCORE %06d     /     STREAK x%d"), CurrentScore, FMath::Min(8, 1 + Streak / 3));
		DrawText(WaveStatus, Ice, HudMargin + 14.0f * HudScale, HudMargin + 55.0f * HudScale, Font,
			FitScale(WaveStatus, LeftPanelWidth - 28.0f * HudScale, 1.0f * HudScale));
		DrawText(Score, Paper, HudMargin + 14.0f * HudScale, HudMargin + 78.0f * HudScale, Font,
			FitScale(Score, LeftPanelWidth - 28.0f * HudScale, 0.94f * HudScale));

		FString Objective = GameMode ? GameMode->GetObjectiveDisplayText() : ClientRunState->ObjectiveText;
		FString OptionalObjective;
		const int32 OptionalStart = Objective.Find(TEXT(" / OPTIONAL:"));
		if (OptionalStart != INDEX_NONE)
		{
			OptionalObjective = Objective.Mid(OptionalStart + 3);
			Objective = Objective.Left(OptionalStart);
		}
		DrawText(Objective, Ice, HudMargin + 14.0f * HudScale, HudMargin + 101.0f * HudScale, Font,
			FitScale(Objective, LeftPanelWidth - 28.0f * HudScale, 0.94f * HudScale));
		if (OptionalObjective.Contains(TEXT("STABILIZE THE FURNACE ANCHOR")))
		{
			if (OptionalObjective.Contains(TEXT("OPTIONAL COMPLETE")))
			{
				OptionalObjective = TEXT("FURNACE ANCHOR SECURED  /  RESUPPLY CLAIMED");
			}
			else if (OptionalObjective.Contains(TEXT("OPTIONAL LOST")))
			{
				OptionalObjective = TEXT("FURNACE ANCHOR LOST");
			}
			else
			{
				OptionalObjective = RidgefireController && RidgefireController->IsUsingGamepad()
					? TEXT("OPTIONAL: D-LEFT AT CORE FOR RESUPPLY")
					: TEXT("OPTIONAL: H AT CORE FOR RESUPPLY");
			}
		}
		if (!OptionalObjective.IsEmpty())
		{
			DrawText(OptionalObjective, Ember, HudMargin + 14.0f * HudScale, HudMargin + 124.0f * HudScale, Font,
				FitScale(OptionalObjective, LeftPanelWidth - 28.0f * HudScale, 0.9f * HudScale));
		}

		const int32 SecondarySlot = GameMode
			? GameMode->GetSecondarySlotOffer()
			: ClientRunState ? ClientRunState->SecondaryWeaponOfferIndex : INDEX_NONE;
		const FRidgefireWeaponRoll* SecondaryOffer = GetOffer(SecondarySlot);
		const AShooterCharacter* EquippedCharacter = Cast<AShooterCharacter>(PlayerOwner->GetPawn());
		const FString EquippedWeaponName = EquippedCharacter ? EquippedCharacter->GetActiveWeaponName() : FString();
		const bool bSecondaryEquipped = SecondaryOffer
			&& EquippedWeaponName.Equals(SecondaryOffer->Name, ESearchCase::IgnoreCase);
		const FRidgefireWeaponRoll* PrimaryActiveOffer = GetOffer(ActiveWeaponIndex);
		const FRidgefireWeaponRoll* Offer = bSecondaryEquipped ? SecondaryOffer : PrimaryActiveOffer;
		if (Offer)
		{
			const float ArsenalWidth = bPortraitCombatLayout
				? Width - HudMargin * 2.0f - 28.0f * HudScale
				: FMath::Min(520.0f * HudScale, Width * 0.48f);
			const float ArsenalX = bPortraitCombatLayout
				? HudMargin + 14.0f * HudScale
				: Width - HudMargin - ArsenalWidth;
			const float ArsenalPanelX = ArsenalX - 14.0f * HudScale;
			const float ArsenalPanelWidth = ArsenalWidth + 28.0f * HudScale;
			const float ArsenalPanelHeight = 158.0f * HudScale;
			const float ArsenalPanelY = bPortraitCombatLayout
				? HudMargin + LeftPanelHeight + 10.0f * HudScale
				: HudMargin;
			const int32 PrimaryOne = GetPrimarySlotOffer(0);
			const int32 PrimaryTwo = GetPrimarySlotOffer(1);
			const int32 ActivePrimarySlot = PrimaryOne == ActiveWeaponIndex ? 0 : PrimaryTwo == ActiveWeaponIndex ? 1 : INDEX_NONE;
			FString EquippedSlotLabel = TEXT("WEAPON");
			if (bSecondaryEquipped)
			{
				EquippedSlotLabel = TEXT("SECONDARY");
			}
			else if (ActivePrimarySlot != INDEX_NONE)
			{
				EquippedSlotLabel = FString::Printf(TEXT("PRIMARY %d"), ActivePrimarySlot + 1);
			}
			FString WeaponLabel = FString::Printf(TEXT("%s  /  %s"), *EquippedSlotLabel, *Offer->Name);
			if (SecondaryOffer)
			{
				WeaponLabel += RidgefireController && RidgefireController->IsUsingGamepad()
					? TEXT("  /  Y TOGGLE")
					: TEXT("  /  B TOGGLE");
			}
			const FString Traits = FString::Printf(TEXT("%s  +  %s  +  %s"), *Offer->ModeName, *Offer->PayloadName, *Offer->ConditionName);
			const FString SlotLine = FString::Printf(TEXT("P1 %s   /   P2 %s   /   SEC %s"),
				GetOffer(PrimaryOne) ? *GetOffer(PrimaryOne)->Name : TEXT("EMPTY"),
				GetOffer(PrimaryTwo) ? *GetOffer(PrimaryTwo)->Name : TEXT("EMPTY"),
				SecondaryOffer ? *SecondaryOffer->Name : TEXT("EMPTY"));
			FString WildcardPreview;
			FString WildcardStatus;
			if (bWildcardClaimed)
			{
				WildcardPreview = TEXT("WILDCARD CLAIMED");
				WildcardStatus = TEXT("REROLL LOCKED");
			}
			else if (RerollTokens > 0 && WildcardOffer)
			{
				WildcardPreview = FString::Printf(TEXT("WILDCARD PREVIEW  /  %s"), *WildcardOffer->Name);
				WildcardStatus = FString::Printf(TEXT("ROLL %d  /  TOKENS %d  /  %s REROLL"), RerollCount + 1, RerollTokens,
					RidgefireController && RidgefireController->IsUsingGamepad() ? TEXT("D-RIGHT") : TEXT("G"));
			}
			else if (WildcardOffer)
			{
				WildcardPreview = FString::Printf(TEXT("WILDCARD PREVIEW  /  %s"), *WildcardOffer->Name);
				WildcardStatus = TEXT("CLEAR A WAVE TO EARN A REROLL TOKEN");
			}

			DrawRect(Shade, ArsenalPanelX, ArsenalPanelY, ArsenalPanelWidth, ArsenalPanelHeight);
			DrawRect(Offer->bWildcard ? Ember : Ice, ArsenalPanelX, ArsenalPanelY, 4.0f * HudScale, ArsenalPanelHeight);
			DrawText(WeaponLabel, Offer->bWildcard ? Ember : Ice, ArsenalX, ArsenalPanelY + 11.0f * HudScale, Font,
				FitScale(WeaponLabel, ArsenalWidth, 1.0f * HudScale));
			DrawText(Traits, Paper, ArsenalX, ArsenalPanelY + 37.0f * HudScale, Font,
				FitScale(Traits, ArsenalWidth, 0.88f * HudScale));
			DrawText(SlotLine, Ice, ArsenalX, ArsenalPanelY + 63.0f * HudScale, Font,
				FitScale(SlotLine, ArsenalWidth, 0.86f * HudScale));
			if (!WildcardPreview.IsEmpty())
			{
				DrawText(WildcardPreview, bWildcardClaimed ? Ember : Ice, ArsenalX, ArsenalPanelY + 91.0f * HudScale, Font,
					FitScale(WildcardPreview, ArsenalWidth, 0.92f * HudScale));
				DrawText(WildcardStatus, bWildcardClaimed ? Ember : Paper, ArsenalX, ArsenalPanelY + 117.0f * HudScale, Font,
					FitScale(WildcardStatus, ArsenalWidth, 0.84f * HudScale));
			}
		}
	}

	if (const AShooterCharacter* Gladiator = Cast<AShooterCharacter>(PlayerOwner->GetPawn()))
	{
		const float HealthRatio = Gladiator->GetHealthRatio();
		const float BarWidth = FMath::Min(360.0f * HudScale, Width * 0.33f);
		const float BarX = HudMargin + 8.0f * HudScale;
		const float BarY = Height - 74.0f * HudScale;
		DrawRect(Shade, BarX - 8.0f * HudScale, BarY - 20.0f * HudScale, BarWidth + 16.0f * HudScale, 56.0f * HudScale);
		DrawRect(FLinearColor(0.16f, 0.18f, 0.20f, 1.0f), BarX, BarY, BarWidth, 8.0f * HudScale);
		DrawRect(HealthRatio < 0.3f ? Ember : Ice, BarX, BarY, BarWidth * HealthRatio, 8.0f * HudScale);
		DrawText(FString::Printf(TEXT("VITALS  %03d%%"), FMath::RoundToInt(HealthRatio * 100.0f)), Paper, BarX, BarY - 17.0f * HudScale, Font, HudScale * 0.95f);
		DrawText(FString::Printf(TEXT("PATCHES  %d"), Gladiator->GetFieldPatches()), Ember, BarX + BarWidth + 22.0f * HudScale, BarY - 17.0f * HudScale, Font, FitScale(FString::Printf(TEXT("PATCHES  %d"), Gladiator->GetFieldPatches()), Width - BarX - BarWidth - 30.0f * HudScale, HudScale * 0.95f));
		const FString Ammo = FString::Printf(TEXT("AMMO  %02d / %03d"), Gladiator->GetRidgefireAmmo(), Gladiator->GetRidgefireReserveAmmo());
		const float AmmoScale = FitScale(Ammo, Width * 0.42f, HudScale * 1.05f);
		float AmmoWidth = 0.0f;
		float AmmoHeight = 0.0f;
		Canvas->StrLen(Font, Ammo, AmmoWidth, AmmoHeight);
		DrawText(Ammo, Paper, Width - HudMargin - AmmoWidth * AmmoScale, Height - 108.0f * HudScale, Font, AmmoScale);

		const FString SurgeLabel = Gladiator->GetIonCharge() >= 100.0f
			? TEXT("E  /  ION SURGE READY")
			: FString::Printf(TEXT("ION SURGE  %02d%%"), FMath::RoundToInt(Gladiator->GetIonCharge()));
		const float SurgeScale = FitScale(SurgeLabel, Width * 0.48f, HudScale * 1.0f);
		float SurgeWidth = 0.0f;
		float SurgeHeight = 0.0f;
		Canvas->StrLen(Font, SurgeLabel, SurgeWidth, SurgeHeight);
		DrawText(SurgeLabel, Ice, Width - HudMargin - SurgeWidth * SurgeScale, Height - 62.0f * HudScale, Font, SurgeScale);
	}

	const float CenterX = Width * 0.5f;
	const float CenterY = Height * 0.5f;
	float ShotFeedback = 0.0f;
	float HitFeedback = 0.0f;
	float KillFeedback = 0.0f;
	if (const AShooterCharacter* Gladiator = Cast<AShooterCharacter>(PlayerOwner->GetPawn()))
	{
		ShotFeedback = Gladiator->GetRidgefireShotFeedbackAlpha();
		HitFeedback = Gladiator->GetRidgefireHitFeedbackAlpha();
		KillFeedback = Gladiator->GetRidgefireKillFeedbackAlpha();
	}
	const float ReticleGap = 3.0f + ShotFeedback * 5.0f;
	const float ReticleArm = 6.0f + ShotFeedback * 4.0f;
	const FLinearColor ReticleColor = FMath::Lerp(Ice, Paper, ShotFeedback);
	const float ReticleThickness = 2.0f + ShotFeedback * 1.5f;
	DrawLine(CenterX - ReticleGap - ReticleArm, CenterY, CenterX - ReticleGap, CenterY, ReticleColor, ReticleThickness);
	DrawLine(CenterX + ReticleGap, CenterY, CenterX + ReticleGap + ReticleArm, CenterY, ReticleColor, ReticleThickness);
	DrawLine(CenterX, CenterY - ReticleGap - ReticleArm, CenterX, CenterY - ReticleGap, ReticleColor, ReticleThickness);
	DrawLine(CenterX, CenterY + ReticleGap, CenterX, CenterY + ReticleGap + ReticleArm, ReticleColor, ReticleThickness);
	if (HitFeedback > 0.0f || KillFeedback > 0.0f)
	{
		const float MarkerInner = 8.0f + KillFeedback * 2.0f;
		const float MarkerOuter = 13.0f + KillFeedback * 3.0f;
		FLinearColor MarkerColor = FMath::Lerp(Paper, Ember, KillFeedback);
		MarkerColor.A = FMath::Max(HitFeedback, KillFeedback);
		DrawLine(CenterX - MarkerInner, CenterY - MarkerInner, CenterX - MarkerOuter, CenterY - MarkerOuter, MarkerColor, 2.0f);
		DrawLine(CenterX + MarkerInner, CenterY - MarkerInner, CenterX + MarkerOuter, CenterY - MarkerOuter, MarkerColor, 2.0f);
		DrawLine(CenterX - MarkerInner, CenterY + MarkerInner, CenterX - MarkerOuter, CenterY + MarkerOuter, MarkerColor, 2.0f);
		DrawLine(CenterX + MarkerInner, CenterY + MarkerInner, CenterX + MarkerOuter, CenterY + MarkerOuter, MarkerColor, 2.0f);
		if (KillFeedback > 0.0f)
		{
			FLinearColor KillLabelColor = Ember;
			KillLabelColor.A = KillFeedback;
			DrawCentered(TEXT("SENTINEL DOWN"), KillLabelColor, CenterY + 22.0f * HudScale, Width * 0.36f, 0.58f * HudScale);
		}
	}
	if (const AShooterCharacter* Gladiator = Cast<AShooterCharacter>(PlayerOwner->GetPawn()))
	{
		const float DamageFeedback = Gladiator->GetRidgefireDamageFeedbackAlpha();
		if (DamageFeedback > 0.0f)
		{
			const float DamageAngle = FMath::DegreesToRadians(Gladiator->GetRidgefireDamageFeedbackYaw());
			const float OutwardX = FMath::Sin(DamageAngle);
			const float OutwardY = -FMath::Cos(DamageAngle);
			const float Radius = 54.0f * HudScale;
			const float TipX = CenterX + OutwardX * Radius;
			const float TipY = CenterY + OutwardY * Radius;
			const float HeadX = TipX + OutwardX * 12.0f * HudScale;
			const float HeadY = TipY + OutwardY * 12.0f * HudScale;
			const float TangentX = FMath::Cos(DamageAngle) * 8.0f * HudScale;
			const float TangentY = FMath::Sin(DamageAngle) * 8.0f * HudScale;
			FLinearColor DamageColor(1.0f, 0.16f, 0.09f, DamageFeedback);
			DrawLine(TipX, TipY, HeadX + TangentX, HeadY + TangentY, DamageColor, 2.6f * HudScale);
			DrawLine(TipX, TipY, HeadX - TangentX, HeadY - TangentY, DamageColor, 2.6f * HudScale);
			DrawLine(HeadX, HeadY, HeadX + OutwardX * 11.0f * HudScale, HeadY + OutwardY * 11.0f * HudScale, DamageColor, 2.0f * HudScale);
		}
	}
	const AShooterPlayerController* ShooterController = Cast<AShooterPlayerController>(PlayerOwner);
	const bool bCompactControls = Width < 1000.0f;
	TArray<FString> ControlsHints;
	if (ShooterController && ShooterController->IsUsingGamepad())
	{
		if (bCompactControls)
		{
			ControlsHints.Add(TEXT("LS MOVE / RT FIRE / A-B-X SELECT / D-UP/D-DN REPLACE PRIMARY"));
			ControlsHints.Add(TEXT("D-RIGHT REROLL / Y TOGGLE / R3 RELOAD / LB PATCH / RB SURGE"));
			ControlsHints.Add(TEXT("MENU PAUSE"));
		}
		else
		{
			ControlsHints.Add(TEXT("LS MOVE   /   RT FIRE   /   A-B-X SELECT   /   D-UP/D-DN REPLACE PRIMARY   /   D-RIGHT REROLL   /   Y TOGGLE   /   R3 RELOAD   /   LB PATCH   /   RB SURGE   /   MENU PAUSE"));
		}
	}
	else
	{
		if (bCompactControls)
		{
			ControlsHints.Add(TEXT("WASD MOVE / LMB FIRE / 1-2-3 SELECT / 4-5 REPLACE PRIMARY"));
			ControlsHints.Add(TEXT("SHIFT SPRINT / G REROLL / B TOGGLE / R RELOAD / F PATCH / E SURGE"));
			ControlsHints.Add(TEXT("ESC PAUSE"));
		}
		else
		{
			ControlsHints.Add(TEXT("WASD MOVE   /   LMB FIRE   /   1-2-3 SELECT   /   4-5 REPLACE PRIMARY   /   G REROLL   /   B TOGGLE   /   SHIFT SPRINT   /   R RELOAD   /   F PATCH   /   E SURGE"));
		}
	}
	if (ShooterController && ShooterController->GetPendingPrimaryReplacementSlot() != INDEX_NONE)
	{
		const FString ReplacementHint = FString::Printf(TEXT("REPLACING PRIMARY SLOT %d — SELECT OFFER 1 / 2 / 3"), ShooterController->GetPendingPrimaryReplacementSlot() + 1);
		DrawCentered(ReplacementHint, Ember, Height - 55.0f * HudScale, Width - HudMargin * 2.0f, FitScale(ReplacementHint, Width - HudMargin * 2.0f, 0.75f * HudScale));
	}
	if (bCompactControls)
	{
		const float HintStartY = Height * 0.76f;
		for (int32 Index = 0; Index < ControlsHints.Num(); ++Index)
		{
			DrawCentered(ControlsHints[Index], Paper, HintStartY + Index * 18.0f * HudScale, Width - HudMargin * 2.0f, 0.98f * HudScale);
		}
	}
	else if (!ControlsHints.IsEmpty())
	{
		DrawCentered(ControlsHints[0], Paper, Height - 30.0f * HudScale, Width - HudMargin * 2.0f, 0.9f * HudScale);
	}
}
