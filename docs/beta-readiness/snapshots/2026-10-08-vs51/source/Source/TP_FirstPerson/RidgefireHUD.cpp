#include "RidgefireHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Math/RotationMatrix.h"
#include "RidgefireArenaDressing.h"
#include "RidgefireGameMode.h"
#include "RidgefireHealingTube.h"
#include "Variant_Shooter/AI/ShooterNPC.h"
#include "ShooterCharacter.h"
#include "ShooterPlayerController.h"

void ARidgefireHUD::DrawHUD()
{
	Super::DrawHUD();
	if (!Canvas || !PlayerOwner)
	{
		return;
	}

    if (DrawSessionScreen()) return;
    if (DrawSetupScreen()) return;
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
	auto FitScale = [this, Font](const FString& Text, float MaximumWidth, float PreferredScale, float MaximumHeight = MAX_flt)
	{
		float TextWidth = 0.0f;
		float TextHeight = 0.0f;
		Canvas->StrLen(Font, Text, TextWidth, TextHeight);
		return RidgefireHudLayout::FitTextScale(TextWidth, TextHeight, MaximumWidth, MaximumHeight, PreferredScale);
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
	const AShooterCharacter* OwnedCharacter = Cast<AShooterCharacter>(PlayerOwner->GetPawn());
	const bool bPlayerDefeated = OwnedCharacter && OwnedCharacter->IsDead();
	const int32 RerollTokens = GameMode ? GameMode->GetWildcardRerollTokens() : ClientRunState ? ClientRunState->WildcardRerollTokens : 0;
	const int32 RerollCount = GameMode ? GameMode->GetWildcardRerollCount() : ClientRunState ? ClientRunState->WildcardRerollCount : 0;
	const bool bWildcardClaimed = GameMode ? GameMode->IsWildcardClaimed() : ClientRunState && ClientRunState->bWildcardClaimed;
	const int32 OfferCount = FMath::Min(3,GameMode ? GameMode->GetOfferCount() : ClientRunState ? ClientRunState->WeaponOffers.Num() : 0);
	const int32 ActiveWeaponIndex = GameMode ? GameMode->GetActiveWeaponIndex() : ClientRunState ? ClientRunState->ActiveWeaponIndex : INDEX_NONE;
	const FRidgefireWeaponRoll* WildcardOffer = nullptr;
	auto GetOffer = [this, GameMode, ClientRunState](int32 Index) -> const FRidgefireWeaponRoll*
	{
		if(ClientRunState && ClientRunState->WeaponOffers.IsValidIndex(Index)) return &ClientRunState->WeaponOffers[Index];
		if (GameMode)
		{
			return GameMode->GetPlayerWeapon(PlayerOwner,Index);
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
	const bool bSecondarySetup = ClientRunState && bArmoryOpen && ClientRunState->SetupStage == ERidgefireSetupStage::Secondary;
	if (RidgefireController && (RidgefireController->IsPersonalPerksOpen() || (ClientRunState && bArmoryOpen && ClientRunState->SetupStage == ERidgefireSetupStage::PersonalPerks)))
	{
		DrawRect(FLinearColor(0.015f, 0.022f, 0.032f, 0.98f), 0, 0, Width, Height);
		const FRidgefireProgressionProfile& Profile = RidgefireController->GetPersonalProgression();
		DrawCentered(ClientRunState && ClientRunState->SetupStage == ERidgefireSetupStage::PersonalPerks && bArmoryOpen ? FString::Printf(TEXT("PERSONAL PERKS / %d XP / %02d SECONDS"), Profile.XP, ClientRunState->SetupSeconds) : FString::Printf(TEXT("PERSONAL PERKS / %d XP"), Profile.XP), Paper, Height * 0.035f, Width * 0.92f, 1.1f);
		const auto* First = RidgefireProgression::GetPerk(Profile.FirstPerk);
		const auto* Second = RidgefireProgression::GetPerk(Profile.SecondPerk);
		DrawCentered(FString::Printf(TEXT("1: %s    2: %s"), First->Name, Second->Name), Ice, Height * 0.11f, Width * 0.92f, 0.8f);
		const float RowHeight = Height * 0.065f;
		for (int32 Node = 0; Node < RidgefireProgression::PerkCount; ++Node)
		{
			const auto* Perk = RidgefireProgression::GetPerk(Node);
			const bool bSelected = Node == RidgefireController->GetPersonalPerkCursor();
			const bool bUnlocked = Profile.XP >= Perk->UnlockXP;
			const float Y = Height * 0.20f + RowHeight * Node;
			if (bSelected) DrawRect(FLinearColor(0.14f, 0.19f, 0.23f, 1), Width * 0.04f, Y - 2, Width * 0.92f, RowHeight);
			const FString Text = FString::Printf(TEXT("%s%s [%s] / %s%s"), bSelected ? TEXT("> ") : TEXT(""), Perk->Name, Perk->Category, Perk->Description,
				bUnlocked ? TEXT("") : *FString::Printf(TEXT(" / LOCKED %d XP"), Perk->UnlockXP));
			DrawText(Text, bSelected ? Ember : bUnlocked ? Paper : FLinearColor(0.48f, 0.50f, 0.54f, 1), Width * 0.06f, Y, Font,
				FitScale(Text, Width * 0.88f, 1.05f, RowHeight * 0.80f));
		}
		DrawCentered(ClientRunState && ClientRunState->SetupStage == ERidgefireSetupStage::PersonalPerks && bArmoryOpen ? TEXT("UP/DOWN OR D-PAD / 1 OR A: SLOT 1 / 2 OR X: SLOT 2 / CHOICES LOCK AT TIMEOUT") : TEXT("UP/DOWN OR D-PAD / 1 OR A: SLOT 1 / 2 OR X: SLOT 2 / K OR L3: CLOSE"), Ice, Height * 0.875f, Width * 0.92f, 0.7f);
		DrawCentered(RidgefireController->IsProgressionPersistent() ? TEXT("PERSISTENT PROFILE / XP SAVED AT WAVE END AND DEFEAT") : TEXT("SESSION PROFILE / AUTOMATION AND GUEST XP IS NOT SAVED TO DISK"), Paper, Height * 0.93f, Width * 0.92f, 0.6f);
		return;
	}
	if (ClientRunState && ClientRunState->bStartingTeamVoteOpen && bArmoryOpen && !PlayerOwner->IsPaused())
	{
		DrawRect(FLinearColor(0.015f, 0.022f, 0.032f, 0.98f), 0, 0, Width, Height);
		DrawCentered(FString::Printf(TEXT("STARTING TEAM PERK / %02d SECONDS"), ClientRunState->StartingTeamVoteSeconds), Paper, Height * 0.12f, Width * 0.90f, 1.3f);
		const int32 Total = ClientRunState->ShieldVotes + ClientRunState->ReloadVotes;
		const int32 ShieldOdds = Total ? FMath::RoundToInt(100.0f * ClientRunState->ShieldVotes / Total) : 100;
		DrawCentered(FString::Printf(TEXT("F1 / A: TEAM SHIELD / TAKE 5%% LESS DAMAGE / %d VOTES / %d%% ODDS"), ClientRunState->ShieldVotes, ShieldOdds), Ice, Height * 0.33f, Width * 0.92f, 1.0f);
		DrawCentered(FString::Printf(TEXT("F2 / X: TEAM QUICKLOAD / RELOAD 10%% FASTER / %d VOTES / %d%% ODDS"), ClientRunState->ReloadVotes, Total ? 100 - ShieldOdds : 0), Ice, Height * 0.46f, Width * 0.92f, 1.0f);
		DrawCentered(TEXT("F3 / B: SKIP / EVERYONE MUST SKIP TO END EARLY"), Ember, Height * 0.59f, Width * 0.92f, 0.9f);
		const TCHAR* Selection = ClientRunState->StartingTeamVoteChoice == 0 ? TEXT("YOUR VOTE: SHIELD") : ClientRunState->StartingTeamVoteChoice == 1 ? TEXT("YOUR VOTE: QUICKLOAD") : ClientRunState->StartingTeamVoteChoice == 2 ? TEXT("YOUR VOTE: SKIP") : TEXT("YOUR VOTE: ABSTAIN / NO BALLOTS DEFAULTS TO SHIELD");
		DrawCentered(Selection, Paper, Height * 0.73f, Width * 0.92f, 0.9f);
		DrawCentered(TEXT("ONE BALLOT PER PLAYER / RECAST TO CHANGE / RUN PERKS RESET EACH RUN"), Paper, Height * 0.82f, Width * 0.92f, 0.8f);
		DrawCentered(TEXT("PERKS AND SECONDARY LOCKED / ENTRY WEAPON FOLLOWS THE VOTE"), Ice, Height * 0.91f, Width * 0.92f, 0.9f);
		return;
	}
	if (bPlayerDefeated && !bRunOver && !PlayerOwner->IsPaused())
	{
		DrawRect(FLinearColor(0.008f, 0.012f, 0.020f, 0.88f), 0.0f, 0.0f, Width, Height);
		DrawRect(Ember, 0.0f, 0.0f, Width, 7.0f);
		const float OverlayScale = FMath::Clamp(FMath::Min(Width, Height) / 500.0f, 1.0f, 1.35f);
		DrawCentered(OwnedCharacter->IsRidgefireDowned() ? TEXT("YOU ARE DOWN") : TEXT("YOU ARE OUT"), Ember, Height * 0.34f, Width * 0.88f, OverlayScale);
		const FString RescueState = OwnedCharacter->GetRidgefireHealingTube() ? TEXT("DEPOSITED IN THE RESCUE STATION")
			: OwnedCharacter->GetRidgefireCarrier() ? TEXT("YOUR TEAMMATE IS CARRYING YOU") : TEXT("A TEAMMATE CAN PICK YOU UP");
		DrawCentered(RescueState, Paper, Height * 0.45f, Width * 0.88f, OverlayScale * 0.90f);
		const ARidgefireHealingTube* RescueTube = OwnedCharacter->GetRidgefireHealingTube();
		const bool bWaitingForExit = RescueTube && RescueTube->GetRecoverySeconds() >= 60.0f
			&& RescueTube->GetRecoveryPauseRemaining() <= 0.0f;
		const FString RecoveryText = bWaitingForExit ? TEXT("RECOVERY CHARGED")
			: RescueTube ? FString::Printf(TEXT("RECOVERY %.0f / 60 SECONDS"), RescueTube->GetRecoverySeconds())
			: TEXT("WAITING FOR RESCUE");
		const FString RecoveryDetail = bWaitingForExit ? TEXT("TEAMMATES: CLEAR THE TUBE EXIT")
			: RescueTube && RescueTube->GetRecoveryPauseRemaining() > 0.0f
			? FString::Printf(TEXT("TUBE HIT / PAUSED %.1f SECONDS"), RescueTube->GetRecoveryPauseRemaining())
			: RescueTube ? TEXT("TEAMMATES: DEFEND THE TUBE")
			: TEXT("YOUR TEAMMATE MUST TAKE YOU TO A RESCUE TUBE");
		DrawCentered(RecoveryText, Ice, Height * 0.56f, Width * 0.88f, OverlayScale);
		DrawCentered(RecoveryDetail, Paper, Height * 0.67f, Width * 0.88f, OverlayScale * 0.85f);
		return;
	}
	if (PlayerOwner->IsPaused())
	{
		DrawRect(FLinearColor(0.008f, 0.012f, 0.020f, 0.76f), 0.0f, 0.0f, Width, Height);
		const float PauseScale = FMath::Clamp(FMath::Min(Width, Height) / 500.0f, 1.0f, 1.35f);
		DrawCentered(TEXT("PIT PAUSED"), Paper, Height * 0.44f, Width * 0.88f, PauseScale);
		const FString ResumeText = bPlayerDefeated && !bRunOver
			? TEXT("P / ESC / MENU  TO RETURN TO RESCUE")
			: bArmoryOpen ? TEXT("P / ESC / MENU  TO RETURN TO THE ARMORY")
			: TEXT("P / ESC / MENU  TO RETURN TO THE FIGHT");
		DrawCentered(ResumeText, Ice, Height * 0.56f, Width * 0.88f, PauseScale * 0.90f);
		return;
	}
	if (bPersonalArsenalOpen && ClientRunState && ClientRunState->bEconomyEnabled && DrawWeaponShop()) return;
	if (bArmoryOpen || bPersonalArsenalOpen)
	{
		DrawRect(FLinearColor(0.015f, 0.022f, 0.032f, 0.96f), 0.0f, 0.0f, Width, Height);
		DrawRect(Ember, 0.0f, 0.0f, Width, 7.0f);
		const float Margin = FMath::Clamp(Width * 0.04f, 16.0f, 48.0f);
		const RidgefireHudLayout::EArmoryCardLayout CardLayout = RidgefireHudLayout::GetArmoryCardLayout(Width, Height);
		const bool bPortrait = CardLayout == RidgefireHudLayout::EArmoryCardLayout::PortraitStacked;
		const bool bCompactCards = CardLayout == RidgefireHudLayout::EArmoryCardLayout::CompactStacked;
		const bool bStackCards = CardLayout != RidgefireHudLayout::EArmoryCardLayout::WideColumns;
		const float HeaderY = bPortrait ? Height * 0.075f : bCompactCards ? Height * 0.065f : Height * 0.16f;
		const FString ArmoryTitle = bPersonalArsenalOpen ? TEXT("ARSENAL") : TEXT("IRON SUN");
		const FString ArmorySubtitle = bPersonalArsenalOpen ? TEXT("SELECT YOUR SECONDARY WEAPON") : bSecondarySetup ? TEXT("SELECT YOUR SECONDARY / 1, 2, 3 OR A, B, X") : TEXT("DEPENDABLE ENTRY / 1 OR 2 / WILDCARD FINDS AFTER SETUP");
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
				const RidgefireHudLayout::FCompactArmoryTextRows TextRows = RidgefireHudLayout::GetCompactArmoryTextRows(CardHeight);
				const float RightX = CardX + CardWidth * 0.67f;
				const float LeftWidth = RightX - TextX - 14.0f;
				const float RightWidth = CardX + CardWidth - RightX - 14.0f;
				const FString Label = FString::Printf(TEXT("0%d  /  %s"), Index + 1, ClientRunState && !bSecondarySetup && !bPersonalArsenalOpen && ClientRunState->SecondaryWeaponOfferIndex == Index ? TEXT("RESERVED SECONDARY") : Offer->bWildcard ? TEXT("WILDCARD") : TEXT("PIT-TESTED"));
				const FString Stats = FString::Printf(TEXT("DMG x%.2f   MAG %d"), Offer->DamageScale, Offer->MagazineSize);
				const FString Payload = FString::Printf(TEXT("PAYLOAD: %s"), *Offer->PayloadName);
				const FString ModeLabel = TextRows.bShowDescription
					? Offer->ModeName : Offer->ModeName + TEXT(" / ") + Offer->ConditionName;
				DrawText(Label, Accent, TextX, ThisCardY + TextRows.LabelY, Font,
					FitScale(Label, LeftWidth, 0.58f * CompactScale, TextRows.NameY - TextRows.LabelY - 2.0f));
				DrawText(Offer->Name, Paper, TextX, ThisCardY + TextRows.NameY, Font,
					FitScale(Offer->Name, LeftWidth, 0.9f * CompactScale, TextRows.ModeY - TextRows.NameY - 2.0f));
				DrawText(ModeLabel, Accent, TextX, ThisCardY + TextRows.ModeY, Font,
					FitScale(ModeLabel, LeftWidth, 0.62f * CompactScale, TextRows.DescriptionY - TextRows.ModeY - 2.0f));
				if (TextRows.bShowDescription)
				{
					DrawWrappedText(Offer->Description, Paper, TextX, ThisCardY + TextRows.DescriptionY, LeftWidth,
						CardHeight * 0.27f, 0.65f * CompactScale);
				}
				DrawText(Stats, Ice, RightX, ThisCardY + CardHeight * 0.30f, Font,
					FitScale(Stats, RightWidth, 0.70f * CompactScale, CardHeight * 0.25f - 2.0f));
				DrawText(Payload, Accent, RightX, ThisCardY + CardHeight * 0.55f, Font,
					FitScale(Payload, RightWidth, 0.65f * CompactScale, CardHeight * 0.40f));
				continue;
			}
			const float BodyScale = RidgefireHudLayout::GetArmoryBodyTextScale(CardScale);
			const FString OfferLabel = FString::Printf(TEXT("0%d  /  %s"), Index + 1, ClientRunState && !bSecondarySetup && !bPersonalArsenalOpen && ClientRunState->SecondaryWeaponOfferIndex == Index ? TEXT("RESERVED SECONDARY") : Offer->bWildcard ? TEXT("WILDCARD") : TEXT("PIT-TESTED"));
			DrawText(OfferLabel, Accent, TextX, ThisCardY + 16.0f * CardScale, Font,
				FitScale(OfferLabel, TextWidth, 0.85f * BodyScale, 32.0f * CardScale - 2.0f));
			DrawText(Offer->Name, Paper, TextX, ThisCardY + 48.0f * CardScale, Font,
				FitScale(Offer->Name, TextWidth, 1.25f * BodyScale, 31.0f * CardScale - 2.0f));
			DrawText(Offer->ModeName, Accent, TextX, ThisCardY + 79.0f * CardScale, Font,
				FitScale(Offer->ModeName, TextWidth, 0.95f * BodyScale, 26.0f * CardScale - 2.0f));
			const float DescriptionY = ThisCardY + 105.0f * CardScale;
			const float FooterY = ThisCardY + CardHeight - 42.0f * CardScale;
			DrawWrappedText(Offer->Description, Paper, TextX, DescriptionY, TextWidth, FMath::Max(0.0f, FooterY - DescriptionY - 4.0f), BodyScale);
			const FString Stats = FString::Printf(TEXT("DMG x%.2f   MAG %d"), Offer->DamageScale, Offer->MagazineSize);
			const FString Payload = FString::Printf(TEXT("PAYLOAD: %s"), *Offer->PayloadName);
			DrawText(Stats, Ice, TextX, FooterY, Font,
				FitScale(Stats, TextWidth, 0.85f * BodyScale, 22.0f * CardScale - 2.0f));
			DrawText(Payload, Accent, TextX, ThisCardY + CardHeight - 20.0f * CardScale, Font,
				FitScale(Payload, TextWidth, 0.85f * BodyScale, 20.0f * CardScale - 2.0f));
		}

		if (bSecondarySetup)
		{
			const int32 Selected = ClientRunState->SecondaryWeaponOfferIndex;
			const FString Selection = ClientRunState->WeaponOffers.IsValidIndex(Selected) ? ClientRunState->WeaponOffers[Selected].Name : TEXT("Phasma Rifle (timeout default)");
			DrawCentered(FString::Printf(TEXT("SECONDARY: %s"), *Selection), Paper, Height * (bPortrait ? 0.84f : 0.78f), Width - Margin * 2, 0.8f);
			DrawCentered(TEXT("ONE PRIMARY + ONE SECONDARY / CHANGE UNTIL THE TIMER ENDS"), Ember, Height * (bPortrait ? 0.89f : 0.83f), Width - Margin * 2, 0.65f);
			DrawCentered(FString::Printf(TEXT("TEAM VOTE IN %02d SECONDS"), ClientRunState->SetupSeconds), Ice, Height * (bPortrait ? 0.94f : 0.89f), Width - Margin * 2, 0.7f);
		}
		else if (bPersonalArsenalOpen)
		{
			DrawCentered(TEXT("1 / 2 / 3  OR  A / B / X  TO SELECT"), Paper, Height * (bPortrait ? 0.84f : 0.78f), Width - Margin * 2.0f, FMath::Clamp(FMath::Min(Width, Height) / 800.0f, 0.62f, 0.9f));
			DrawCentered(TEXT("PICK AN OFFER NOT ALREADY IN PRIMARY 1 OR 2"), Ember, Height * (bPortrait ? 0.89f : 0.83f), Width - Margin * 2.0f, FMath::Clamp(FMath::Min(Width, Height) / 1050.0f, 0.48f, 0.68f));
			DrawCentered(TEXT("T / VIEW TO CLOSE  /  SELECTION ONLY AFFECTS YOU"), Ice, Height * (bPortrait ? 0.94f : 0.89f), Width - Margin * 2.0f, FMath::Clamp(FMath::Min(Width, Height) / 1000.0f, 0.5f, 0.7f));
		}
		else
		{
			DrawCentered(TEXT("1 / 2 / 3  OR  A / B / X  TO ENTER THE PIT"), Paper, Height * (bPortrait ? 0.84f : 0.78f), Width - Margin * 2.0f, FMath::Clamp(FMath::Min(Width, Height) / 800.0f, 0.62f, 0.9f));
			DrawCentered(ClientRunState && ClientRunState->WeaponOffers.IsValidIndex(ClientRunState->SecondaryWeaponOfferIndex) ? FString::Printf(TEXT("SECONDARY: %s / TEAM: %s"), *ClientRunState->WeaponOffers[ClientRunState->SecondaryWeaponOfferIndex].Name, ClientRunState->TeamArenaPerks & 4 ? TEXT("SHIELD") : ClientRunState->TeamArenaPerks & 8 ? TEXT("QUICKLOAD") : TEXT("SKIPPED")) : FString(TEXT("CHOOSE YOUR ENTRY WEAPON")), Ember, Height * (bPortrait ? 0.89f : 0.83f), Width - Margin * 2.0f, FMath::Clamp(FMath::Min(Width, Height) / 1050.0f, 0.48f, 0.68f));
			const int32 ArmorySecondsRemaining = GameMode ? GameMode->GetArmorySecondsRemaining() : ClientRunState ? ClientRunState->ArmorySecondsRemaining : 0;
			DrawCentered(FString::Printf(TEXT("AUTO-DEPLOY IN %02d"), ArmorySecondsRemaining), Ice, Height * (bPortrait ? 0.94f : 0.89f), Width - Margin * 2.0f, FMath::Clamp(FMath::Min(Width, Height) / 1000.0f, 0.5f, 0.7f));
		}
		return;
	}

    if (ClientRunState && ClientRunState->bRouteVoteOpen && !PlayerOwner->IsPaused())
    {
        DrawRect(FLinearColor(0.008f,0.012f,0.020f,0.94f),0,0,Width,Height);
        DrawCentered(FString::Printf(TEXT("ARENA CLEARED / CHOOSE A ROUTE / %02d SECONDS"),ClientRunState->RouteSeconds),Paper,Height*0.12f,Width*0.92f,1.15f);
        for (int32 Index=0;Index<ClientRunState->RouteBriefs.Num();++Index)
        {
            TArray<FString> Lines; ClientRunState->RouteBriefs[Index].ParseIntoArray(Lines,TEXT("|"));
            float Y=Height*(0.29f+Index*0.28f);
            DrawCentered(FString::Printf(TEXT("%s: %s / %d VOTES%s"),Index==0 ? TEXT("1 / A") : TEXT("2 / X"),Lines.Num() ? *Lines[0] : TEXT("ROUTE"),Index==0 ? ClientRunState->LeftRouteVotes : ClientRunState->RightRouteVotes,ClientRunState->RouteChoice==Index ? TEXT(" / YOUR CHOICE") : TEXT("")),Ember,Y,Width*0.9f,1.05f);
            for (int32 Line=1;Line<Lines.Num();++Line) DrawCentered(Lines[Line],Ice,Y+Height*(0.046f*Line),Width*0.9f,0.8f);
        }
        DrawCentered(TEXT("MAJORITY CHOOSES / TIES AND ABSTENTIONS USE THE RUN SEED"),Paper,Height*0.9f,Width*0.92f,0.8f);
        return;
    }
    if (ClientRunState && ClientRunState->bChampionActive && !PlayerOwner->IsPaused())
    {
        DrawRect(FLinearColor(0.008f,0.012f,0.020f,0.9f),Width*0.14f,Height*0.335f,Width*0.72f,Height*0.07f);
        DrawCentered(FString::Printf(TEXT("CHAMPION / %.0f HP / %s"),ClientRunState->ChampionHealth,ClientRunState->bChampionGuarded ? TEXT("LENSES REDUCE DAMAGE BY 75%") : TEXT("GUARD DOWN")),Ember,Height*0.35f,Width*0.88f,0.85f);
    }
	if (bRunOver)
	{
		DrawRect(FLinearColor(0.008f, 0.012f, 0.020f, 0.86f), 0.0f, 0.0f, Width, Height);
		DrawRect(Ember, 0.0f, 0.0f, Width, 7.0f);
		DrawCentered(ClientRunState && ClientRunState->bRunWon ? TEXT("CHAMPION DEFEATED / RUN COMPLETE") : TEXT("THE PIT CLAIMS ANOTHER"), Ember, Height * 0.34f, Width * 0.88f, FMath::Clamp(FMath::Min(Width, Height) / 500.0f, 0.75f, 1.35f));
		const int32 Wave = GameMode ? GameMode->GetWave() : ClientRunState ? ClientRunState->Wave : 0;
		const int32 Score = GameMode ? GameMode->GetScore() : ClientRunState ? ClientRunState->Score : 0;
		DrawCentered(FString::Printf(TEXT("WAVE %02d     /     SCORE %06d"), Wave, Score), Paper, Height * 0.44f, Width * 0.88f, FMath::Clamp(FMath::Min(Width, Height) / 680.0f, 0.7f, 1.0f));
		const bool bGamepadControls = RidgefireController && RidgefireController->IsUsingGamepad();
		const FString GameOverControls = bGamepadControls
			? TEXT("R3: ROLL AGAIN / L3: PERSONAL PERKS / MENU: PAUSE")
			: TEXT("R: ROLL AGAIN / K: PERSONAL PERKS / ESC: PAUSE");
		DrawCentered(GameOverControls, Ice, Height * 0.55f, Width * 0.88f, FMath::Clamp(FMath::Min(Width, Height) / 840.0f, 0.58f, 0.82f));
		return;
	}

	bool bRescueHelpVisible = false;
	if (!bRunOver && !bArmoryOpen)
	{
		for (TActorIterator<ARidgefireHealingTube> Tube(GetWorld()); Tube; ++Tube)
		{
			FVector2D Screen;
			if (PlayerOwner->ProjectWorldLocationToScreen(Tube->GetActorLocation() + FVector(0.0f, 0.0f, 40.0f), Screen, true)
				&& Screen.X > Width * 0.20f && Screen.X < Width * 0.80f && Screen.Y > Height * 0.30f && Screen.Y < Height * 0.72f)
			{
				const FString Label = Tube->GetOccupant() ? FString::Printf(TEXT("RESCUE %.0f / 60s  /  %s"),
					Tube->GetRecoverySeconds(), Tube->GetRecoveryPauseRemaining() > 0.0f ? TEXT("HIT: PAUSED")
					: Tube->GetRecoverySeconds() >= 60.0f ? TEXT("CLEAR AN EXIT") : TEXT("DEFEND")) : TEXT("RESCUE TUBE / ONE TEAMMATE AT A TIME");
				const float LabelWidth = FMath::Min(340.0f, Width * 0.7f);
				const float LabelHeight = FMath::Min(30.0f, Height * 0.08f);
				const float LabelLeft = FMath::Clamp(Screen.X - LabelWidth * 0.5f, 12.0f, Width - LabelWidth - 12.0f);
				const float LabelTop = FMath::Clamp(Screen.Y - 4.0f, Height * 0.48f, Height * 0.57f - LabelHeight);
				const float Scale = FitScale(Label, LabelWidth - 12.0f, 1.4f, LabelHeight - 8.0f);
				DrawRect(Shade, LabelLeft, LabelTop, LabelWidth, LabelHeight);
				DrawText(Label, Ice, LabelLeft + 6.0f, LabelTop + 4.0f, Font, Scale);
				bRescueHelpVisible = true;
			}
		}
	}
	if (OwnedCharacter && !bRunOver && (OwnedCharacter->IsCarryingRidgefirePlayer() || OwnedCharacter->FindRidgefireCarryTarget()))
	{
		bRescueHelpVisible = true;
		const bool bCarrying = OwnedCharacter->IsCarryingRidgefirePlayer();
		const FString Action = RidgefireController && RidgefireController->IsUsingGamepad() ? TEXT("VIEW") : TEXT("V");
		const bool bSecondaryReady = OwnedCharacter->IsRidgefireSecondaryEquipped()
			|| (ClientRunState && ClientRunState->WeaponOffers.IsValidIndex(ClientRunState->SecondaryWeaponOfferIndex));
		const ARidgefireHealingTube* NearbyTube = bCarrying ? OwnedCharacter->FindNearbyRidgefireHealingTube() : nullptr;
		const FString CarryPrompt = NearbyTube ? (NearbyTube->GetOccupant() ? TEXT("RESCUE STATION OCCUPIED  /  KEEP CARRYING")
			: Action + TEXT(" TO DEPOSIT  /  DEFEND FOR 60 SECONDS"))
			: !bCarrying && !bSecondaryReady ? TEXT("SELECT A SECONDARY AT THE ARSENAL TO CARRY")
			: Action + (bCarrying ? TEXT(" TO SET TEAMMATE DOWN  /  SECONDARY ONLY") : TEXT(" TO CARRY DOWNED TEAMMATE"));
		DrawCentered(CarryPrompt,
			Ice, Height * 0.62f, Width * 0.88f, FMath::Clamp(FMath::Min(Width, Height) / 780.0f, 0.72f, 1.0f));
	}
	const FString CurrentArenaName = GameMode ? GameMode->GetArenaDisplayName() : ClientRunState ? ClientRunState->ArenaName : FString();
	if (CurrentArenaName.Equals(TEXT("IRON SUN ARENA"), ESearchCase::IgnoreCase) && !bArmoryOpen && !bPersonalArsenalOpen && !bRescueHelpVisible)
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
			const float PromptY = Height * 0.52f;
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
		const FString WaveStatus = ClientRunState && ClientRunState->bRegroupOpen
            ? FString::Printf(TEXT("REGROUP %ds / %d/%d READY / N OR PAD B"),ClientRunState->RegroupSeconds,ClientRunState->RegroupReadyCount,ClientRunState->RegroupEligible)
            : ClientRunState && ClientRunState->ArenaEncounters>0
            ? FString::Printf(TEXT("FIGHT %d/%d / LIVE %d +%d"),ClientRunState->ArenaWave,ClientRunState->ArenaEncounters,EnemiesRemaining,ClientRunState->ReinforcementsRemaining)
            : FString::Printf(TEXT("WAVE %02d     /     SENTRIES %02d"), Wave, EnemiesRemaining);
		const FString Score = FString::Printf(TEXT("SCORE %06d     /     STREAK x%d"), CurrentScore, FMath::Min(8, 1 + Streak / 3));
		DrawText(WaveStatus, Ice, HudMargin + 14.0f * HudScale, HudMargin + 55.0f * HudScale, Font,
			FitScale(WaveStatus, LeftPanelWidth - 28.0f * HudScale, 1.0f * HudScale));
		DrawText(Score, Paper, HudMargin + 14.0f * HudScale, HudMargin + 78.0f * HudScale, Font,
			FitScale(Score, LeftPanelWidth - 28.0f * HudScale, 0.94f * HudScale));

		FString Objective = GameMode ? GameMode->GetObjectiveDisplayText() : ClientRunState->ObjectiveText;
		FString OptionalObjective;
		const int32 OptionalStart = Objective.Find(TEXT("OPTIONAL"));
		if (OptionalStart != INDEX_NONE)
		{
			OptionalObjective = Objective.Mid(OptionalStart);
			Objective = Objective.Left(OptionalStart).TrimEnd();
			Objective.RemoveFromEnd(TEXT("/"));
			Objective.TrimEndInline();
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

		if (RidgefireController)
		{
			const uint8 TeamPerks = RidgefireController->GetRidgefireRunState().TeamArenaPerks;
			const FString PerkText = FString::Printf(TEXT("XP %d / CASH %d%s%s%s%s"), RidgefireController->GetPersonalProgression().XP, ClientRunState ? ClientRunState->Cash : 0,
				TeamPerks & 1 ? TEXT(" / MOVE") : TEXT(""), TeamPerks & 2 ? TEXT(" / DAMAGE") : TEXT(""),
				TeamPerks & 4 ? TEXT(" / SHIELD") : TEXT(""), TeamPerks & 8 ? TEXT(" / QUICKLOAD") : TEXT(""));
			DrawText(PerkText, Ice, HudMargin + 14.0f * HudScale, HudMargin + 147.0f * HudScale, Font,
				FitScale(PerkText, LeftPanelWidth - 28.0f * HudScale, 0.90f * HudScale, 20.0f * HudScale));
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
			const int32 ActivePrimarySlot = PrimaryOne == ActiveWeaponIndex ? 0 : INDEX_NONE;
			FString EquippedSlotLabel = TEXT("WEAPON");
			if (bSecondaryEquipped)
			{
				EquippedSlotLabel = TEXT("SECONDARY");
			}
			else if (ActivePrimarySlot != INDEX_NONE)
			{
				EquippedSlotLabel = TEXT("PRIMARY");
			}
			FString WeaponLabel = FString::Printf(TEXT("%s  /  %s"), *EquippedSlotLabel, *Offer->Name);
			if (OwnedCharacter && OwnedCharacter->IsCarryingRidgefirePlayer())
			{
				WeaponLabel += TEXT("  /  CARRY LOCK");
			}
			else if (SecondaryOffer)
			{
				WeaponLabel += RidgefireController && RidgefireController->IsUsingGamepad()
					? TEXT("  /  Y TOGGLE")
					: TEXT("  /  B TOGGLE");
			}
			const FString Traits = FString::Printf(TEXT("%s  +  %s  +  %s"), *Offer->ModeName, *Offer->PayloadName, *Offer->ConditionName);
			const FString SlotLine = FString::Printf(TEXT("1 PRIMARY %s   /   2 SECONDARY %s"),
				GetOffer(PrimaryOne) ? *GetOffer(PrimaryOne)->Name : TEXT("EMPTY"),
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
		const float BarY = Height - 132.0f * HudScale;
		DrawRect(Shade, BarX - 8.0f * HudScale, BarY - 20.0f * HudScale, BarWidth + 16.0f * HudScale, 56.0f * HudScale);
		DrawRect(FLinearColor(0.16f, 0.18f, 0.20f, 1.0f), BarX, BarY, BarWidth, 8.0f * HudScale);
		DrawRect(HealthRatio < 0.3f ? Ember : Ice, BarX, BarY, BarWidth * HealthRatio, 8.0f * HudScale);
		DrawText(FString::Printf(TEXT("VITALS  %03d%%"), FMath::RoundToInt(HealthRatio * 100.0f)), Paper, BarX, BarY - 17.0f * HudScale, Font, HudScale * 0.95f);
		DrawText(FString::Printf(TEXT("PATCHES  %d"), Gladiator->GetFieldPatches()), Ember, BarX, BarY + 15.0f * HudScale, Font,
			FitScale(FString::Printf(TEXT("PATCHES  %d"), Gladiator->GetFieldPatches()), BarWidth, HudScale * 0.90f));
		const FString Ammo = FString::Printf(TEXT("AMMO  %02d / %03d"), Gladiator->GetRidgefireAmmo(), Gladiator->GetRidgefireReserveAmmo());
		const FBox2D ResourcePanel = RidgefireHudLayout::GetCombatResourcePanel(Width, Height);
		const float ResourceX = ResourcePanel.Min.X + 14.0f * HudScale;
		const float ResourceTextWidth = ResourcePanel.GetSize().X - 28.0f * HudScale;
		DrawRect(FLinearColor(0.015f, 0.022f, 0.032f, 0.94f), ResourcePanel.Min.X, ResourcePanel.Min.Y,
			ResourcePanel.GetSize().X, ResourcePanel.GetSize().Y);
		const RidgefireHudLayout::EAmmoState AmmoState = RidgefireHudLayout::GetAmmoState(
			Gladiator->GetRidgefireAmmo(), Gladiator->GetRidgefireReserveAmmo(), Gladiator->IsRidgefireReloading());
		const bool bAmmoWarning = AmmoState == RidgefireHudLayout::EAmmoState::EmptyMagazine
			|| AmmoState == RidgefireHudLayout::EAmmoState::OutOfAmmo;
		DrawRect(bAmmoWarning ? Ember : Ice, ResourcePanel.Min.X, ResourcePanel.Min.Y, 4.0f * HudScale, ResourcePanel.GetSize().Y);
		DrawText(Ammo, bAmmoWarning ? Ember : Paper, ResourceX, ResourcePanel.Min.Y + 10.0f * HudScale, Font,
			FitScale(Ammo, ResourceTextWidth, HudScale * 1.18f));
		FString AmmoStatus;
		if (AmmoState == RidgefireHudLayout::EAmmoState::Reloading)
		{
			AmmoStatus = TEXT("RELOADING");
		}
		else if (AmmoState == RidgefireHudLayout::EAmmoState::EmptyMagazine)
		{
			AmmoStatus = RidgefireController && RidgefireController->IsUsingGamepad() ? TEXT("DPAD UP / RELOAD") : TEXT("R / RELOAD");
		}
		else if (AmmoState == RidgefireHudLayout::EAmmoState::OutOfAmmo)
		{
			AmmoStatus = TEXT("OUT OF AMMO / SWITCH WEAPON");
		}
		else if (Gladiator->GetRidgefireReserveAmmo() <= 0)
		{
			AmmoStatus = TEXT("LAST MAGAZINE");
		}
		if (!AmmoStatus.IsEmpty())
		{
			DrawText(AmmoStatus, bAmmoWarning ? Ember : Ice, ResourceX, ResourcePanel.Min.Y + 37.0f * HudScale, Font,
				FitScale(AmmoStatus, ResourceTextWidth, HudScale * 0.90f));
		}

		const FString SurgeLabel = Gladiator->GetIonCharge() >= 100.0f
			? RidgefireController && RidgefireController->IsUsingGamepad() ? TEXT("RB / ION SURGE READY") : TEXT("E / ION SURGE READY")
			: FString::Printf(TEXT("ION SURGE  %02d%%"), FMath::RoundToInt(Gladiator->GetIonCharge()));
		DrawText(SurgeLabel, Ice, ResourceX, ResourcePanel.Min.Y + 64.0f * HudScale, Font,
			FitScale(SurgeLabel, ResourceTextWidth, HudScale * 0.90f));
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
        const FString Blast=Gladiator->GetRidgefireBlastFeedback();
        if(!Blast.IsEmpty()) {
            const float PanelWidth=FMath::Min(280*HudScale,Width*.48f);
            DrawRect(FLinearColor(.015f,.022f,.032f,.85f),(Width-PanelWidth)*.5f,Height*.59f-4*HudScale,PanelWidth,26*HudScale);
            DrawCentered(Blast,Ice,Height*.59f,PanelWidth-12*HudScale,0.85f*HudScale);
        }
	}
	const float Handling = OwnedCharacter ? OwnedCharacter->GetRidgefireHandlingRecovery() : 0;
	const float ReticleGap = 3.0f + ShotFeedback * 5.0f + Handling * 4.0f;
	const float ReticleArm = 6.0f + ShotFeedback * 4.0f;
	const FLinearColor ReticleColor = FMath::Lerp(Ice, Paper, ShotFeedback);
	const float ReticleThickness = 2.0f + ShotFeedback * 1.5f;
	DrawLine(CenterX - ReticleGap - ReticleArm, CenterY, CenterX - ReticleGap, CenterY, ReticleColor, ReticleThickness);
	DrawLine(CenterX + ReticleGap, CenterY, CenterX + ReticleGap + ReticleArm, CenterY, ReticleColor, ReticleThickness);
	DrawLine(CenterX, CenterY - ReticleGap - ReticleArm, CenterX, CenterY - ReticleGap, ReticleColor, ReticleThickness);
	DrawLine(CenterX, CenterY + ReticleGap, CenterX, CenterY + ReticleGap + ReticleArm, ReticleColor, ReticleThickness);
    if(OwnedCharacter && !OwnedCharacter->IsDead() && !bRunOver) {
        FHitResult Hit;FCollisionQueryParams Query(SCENE_QUERY_STAT(RidgefireFinisherPrompt),true,OwnedCharacter);
        const FVector Start=OwnedCharacter->GetPawnViewLocation();
        if(GetWorld()->LineTraceSingleByChannel(Hit,Start,Start+PlayerOwner->GetControlRotation().Vector()*190,ECC_Visibility,Query))
            if(const auto* Target=Cast<AShooterNPC>(Hit.GetActor());Target && Target->CanRidgefireFinish())
                DrawCentered(RidgefireController && RidgefireController->IsUsingGamepad() ? TEXT("D-DOWN: FINISH / +90 PERSONAL CASH") : TEXT("M / MIDDLE MOUSE: FINISH / +90 PERSONAL CASH"),Ember,CenterY+44*HudScale,Width*.60f,0.90f*HudScale);
    }
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
	TArray<FString> ControlsHints;
	if (OwnedCharacter && OwnedCharacter->IsCarryingRidgefirePlayer())
	{
		const bool bAtTube = OwnedCharacter->FindNearbyRidgefireHealingTube() != nullptr;
		const bool bPrimaryAllowed = OwnedCharacter->CanUsePrimaryWhileCarrying();
  const TCHAR* FireLabel = bPrimaryAllowed ? TEXT("FIRE") : TEXT("SECONDARY FIRE");
  const TCHAR* RescueAction = bAtTube ? TEXT("DEPOSIT") : TEXT("SET DOWN");
  ControlsHints.Add(ShooterController && ShooterController->IsUsingGamepad()
   ? FString::Printf(TEXT("LS MOVE / RT %s / DPAD UP RELOAD / VIEW %s / MENU PAUSE"), FireLabel, RescueAction)
   : FString::Printf(TEXT("WASD MOVE / LMB %s / R RELOAD / V %s / ESC PAUSE"), FireLabel, RescueAction));
  ControlsHints.Add(OwnedCharacter->CanUsePrimaryWhileCarrying()
   ? TEXT("RESCUE HARNESS / PRIMARY FIRE ALLOWED / B OR Y: SWITCH / NO SPRINT")
   : TEXT("CARRYING TEAMMATE / SECONDARY ONLY / NO SPRINT OR PRIMARY SWITCH"));
	}
	else if (ShooterController && ShooterController->IsUsingGamepad())
	{
		ControlsHints.Add(TEXT("LS MOVE / RT FIRE / LT AIM / R3 CROUCH-SLIDE / DPAD UP RELOAD / MENU PAUSE"));
		ControlsHints.Add(TEXT("VIEW ARSENAL / Y SWITCH WEAPONS / D-DOWN FINISHER"));
		ControlsHints.Add(TEXT("D-RIGHT REROLL / Y SECONDARY / LB PATCH / RB SURGE"));
	}
	else
	{
		ControlsHints.Add(TEXT("WASD MOVE / LMB FIRE / RMB AIM / SHIFT SPRINT / C-CTRL CROUCH-SLIDE / R RELOAD / ESC PAUSE"));
		ControlsHints.Add(TEXT("1 PRIMARY / 2 SECONDARY / T ARSENAL / G REROLL"));
		ControlsHints.Add(TEXT("B SECONDARY / F PATCH / E SURGE"));
	}
	const bool bReplacingPrimary = ShooterController && ShooterController->GetPendingPrimaryReplacementSlot() != INDEX_NONE;
	if (bReplacingPrimary)
	{
		const FString ReplacementHint = FString::Printf(TEXT("REPLACING PRIMARY SLOT %d — SELECT OFFER 1 / 2 / 3"), ShooterController->GetPendingPrimaryReplacementSlot() + 1);
		ControlsHints.Last() = ReplacementHint;
	}
	const float HintStartY = Height - 70.0f * HudScale;
	DrawRect(FLinearColor(0.015f, 0.022f, 0.032f, 0.94f), HudMargin, HintStartY,
		Width - HudMargin * 2.0f, 62.0f * HudScale);
	for (int32 Index = 0; Index < ControlsHints.Num(); ++Index)
	{
		DrawCentered(ControlsHints[Index], bReplacingPrimary && Index == ControlsHints.Num() - 1 ? Ember : Paper,
			HintStartY + (8.0f + Index * 18.0f) * HudScale, Width - HudMargin * 2.0f - 20.0f * HudScale, 0.92f * HudScale);
	}
}

