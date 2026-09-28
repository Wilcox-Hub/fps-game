#include "RidgefireSessionSubsystem.h"

#include "Containers/Ticker.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

void URidgefireSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	bSessionSmokeHost = FParse::Param(FCommandLine::Get(), TEXT("RidgefireSessionSmokeHost"));
	bSessionSmokeClient = FParse::Param(FCommandLine::Get(), TEXT("RidgefireSessionSmokeClient"));
	if (bSessionSmokeHost || bSessionSmokeClient)
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SESSION SMOKE START %s"), bSessionSmokeHost ? TEXT("HOST") : TEXT("CLIENT"));
		SessionSmokeTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateUObject(this, &URidgefireSessionSubsystem::TickSessionSmokeStartup), 0.25f);
	}
}

void URidgefireSessionSubsystem::Deinitialize()
{
	if (SessionSmokeTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(SessionSmokeTickerHandle);
		SessionSmokeTickerHandle.Reset();
	}
	ClearDelegates();
	if (SessionInterface.IsValid() && SessionInterface->GetNamedSession(NAME_GameSession))
	{
		SessionInterface->DestroySession(NAME_GameSession);
	}
	SessionSearch.Reset();
	SessionResultIndices.Reset();
	SessionInterface.Reset();
	bOperationInFlight = false;
	Super::Deinitialize();
}

bool URidgefireSessionSubsystem::TickSessionSmokeStartup(float DeltaTime)
{
	SessionSmokeWaitSeconds += DeltaTime;
	UGameInstance* GameInstance = GetGameInstance();
	UWorld* World = GameInstance ? GameInstance->GetWorld() : nullptr;
	if (World && World->GetFirstPlayerController())
	{
		SessionSmokeTickerHandle.Reset();
		if (bSessionSmokeHost)
		{
			if (!HostSession(2, true))
			{
				UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SESSION SMOKE FAIL host request rejected"));
			}
		}
		else if (!FindSessions(true))
		{
			UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SESSION SMOKE FAIL search request rejected"));
		}
		else
		{
			++SessionSmokeSearchAttempts;
		}
		return false;
	}
	if (SessionSmokeWaitSeconds >= 60.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SESSION SMOKE FAIL startup timed out waiting for world/player controller"));
		SessionSmokeTickerHandle.Reset();
		return false;
	}
	return true;
}

bool URidgefireSessionSubsystem::HostSession(int32 MaxPlayers, bool bUseLAN)
{
	if (bOperationInFlight)
	{
		OnSessionOperationComplete.Broadcast(false, TEXT("Another session operation is still running."));
		return false;
	}
	if (!ResolveSessionInterface())
	{
		OnSessionOperationComplete.Broadcast(false, TEXT("Online sessions are unavailable."));
		return false;
	}
	if (SessionInterface->GetNamedSession(NAME_GameSession))
	{
		OnSessionOperationComplete.Broadcast(false, TEXT("Leave the current session before hosting another."));
		return false;
	}

	PendingMaxPlayers = FMath::Clamp(MaxPlayers, 2, 4);
	bPendingLAN = bUseLAN;
	FOnlineSessionSettings Settings;
	Settings.bIsLANMatch = bUseLAN;
	Settings.NumPublicConnections = PendingMaxPlayers;
	Settings.NumPrivateConnections = 0;
	Settings.bShouldAdvertise = true;
	Settings.bAllowJoinInProgress = true;
	Settings.bUsesPresence = false;
	Settings.Set(SETTING_MAPNAME, GetWorld() ? GetWorld()->GetMapName() : FString(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	CreateSessionDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &URidgefireSessionSubsystem::HandleCreateSessionComplete));
	bOperationInFlight = true;
	if (!SessionInterface->CreateSession(0, NAME_GameSession, Settings))
	{
		ClearDelegates();
		CompleteOperation(false, TEXT("The session provider rejected the host request."));
		return false;
	}
	return true;
}

bool URidgefireSessionSubsystem::FindSessions(bool bUseLAN)
{
	if (bOperationInFlight)
	{
		OnSessionOperationComplete.Broadcast(false, TEXT("Another session operation is still running."));
		return false;
	}
	if (!ResolveSessionInterface())
	{
		OnSessionOperationComplete.Broadcast(false, TEXT("Online sessions are unavailable."));
		return false;
	}

	SessionSummaries.Reset();
	SessionResultIndices.Reset();
	SessionSearch = MakeShared<FOnlineSessionSearch>();
	SessionSearch->bIsLanQuery = bUseLAN;
	SessionSearch->MaxSearchResults = 100;
	FindSessionsDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &URidgefireSessionSubsystem::HandleFindSessionsComplete));
	bOperationInFlight = true;
	if (!SessionInterface->FindSessions(0, SessionSearch.ToSharedRef()))
	{
		ClearDelegates();
		SessionSearch.Reset();
		CompleteOperation(false, TEXT("The session provider rejected the search request."));
		return false;
	}
	return true;
}

bool URidgefireSessionSubsystem::JoinSession(int32 SearchResultIndex)
{
	if (bOperationInFlight)
	{
		OnSessionOperationComplete.Broadcast(false, TEXT("Another session operation is still running."));
		return false;
	}
	if (!ResolveSessionInterface() || !SessionSearch.IsValid() || !SessionResultIndices.IsValidIndex(SearchResultIndex))
	{
		OnSessionOperationComplete.Broadcast(false, TEXT("That session is no longer available."));
		return false;
	}
	if (SessionInterface->GetNamedSession(NAME_GameSession))
	{
		OnSessionOperationComplete.Broadcast(false, TEXT("Leave the current session before joining another."));
		return false;
	}

	JoinSessionDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &URidgefireSessionSubsystem::HandleJoinSessionComplete));
	bOperationInFlight = true;
	if (!SessionInterface->JoinSession(0, NAME_GameSession, SessionSearch->SearchResults[SessionResultIndices[SearchResultIndex]]))
	{
		ClearDelegates();
		CompleteOperation(false, TEXT("The session provider rejected the join request."));
		return false;
	}
	return true;
}

bool URidgefireSessionSubsystem::LeaveSession()
{
	if (bOperationInFlight)
	{
		OnSessionOperationComplete.Broadcast(false, TEXT("Another session operation is still running."));
		return false;
	}
	if (!ResolveSessionInterface())
	{
		OnSessionOperationComplete.Broadcast(false, TEXT("Online sessions are unavailable."));
		return false;
	}
	if (!SessionInterface->GetNamedSession(NAME_GameSession))
	{
		OnSessionOperationComplete.Broadcast(true, TEXT("No active session to leave."));
		return true;
	}

	DestroySessionDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
		FOnDestroySessionCompleteDelegate::CreateUObject(this, &URidgefireSessionSubsystem::HandleDestroySessionComplete));
	bOperationInFlight = true;
	if (!SessionInterface->DestroySession(NAME_GameSession))
	{
		ClearDelegates();
		CompleteOperation(false, TEXT("The session provider rejected the leave request."));
		return false;
	}
	return true;
}

TArray<FString> URidgefireSessionSubsystem::GetSessionSummaries() const
{
	return SessionSummaries;
}

bool URidgefireSessionSubsystem::ResolveSessionInterface()
{
	if (SessionInterface.IsValid())
	{
		return true;
	}
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem)
	{
		OnlineSubsystem = IOnlineSubsystem::Get(FName(TEXT("NULL")));
	}
	if (!OnlineSubsystem)
	{
		return false;
	}
	SessionInterface = OnlineSubsystem->GetSessionInterface();
	return SessionInterface.IsValid();
}

void URidgefireSessionSubsystem::ClearDelegates()
{
	if (!SessionInterface.IsValid())
	{
		return;
	}
	if (CreateSessionDelegateHandle.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionDelegateHandle);
		CreateSessionDelegateHandle.Reset();
	}
	if (FindSessionsDelegateHandle.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsDelegateHandle);
		FindSessionsDelegateHandle.Reset();
	}
	if (JoinSessionDelegateHandle.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionDelegateHandle);
		JoinSessionDelegateHandle.Reset();
	}
	if (DestroySessionDelegateHandle.IsValid())
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionDelegateHandle);
		DestroySessionDelegateHandle.Reset();
	}
}

void URidgefireSessionSubsystem::CompleteOperation(bool bWasSuccessful, const FString& Message)
{
	ClearDelegates();
	bOperationInFlight = false;
	if (!bWasSuccessful && (bSessionSmokeHost || bSessionSmokeClient))
	{
		UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SESSION SMOKE FAIL operation: %s"), *Message);
	}
	OnSessionOperationComplete.Broadcast(bWasSuccessful, Message);
}

void URidgefireSessionSubsystem::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		CompleteOperation(false, TEXT("Could not create the online session."));
		return;
	}
	if (!GetWorld())
	{
		SessionInterface->DestroySession(SessionName);
		CompleteOperation(false, TEXT("Session created, but no world is available to host it."));
		return;
	}

	FString MapPackage = GetWorld()->GetOutermost()->GetName();
	const int32 PIEPrefixIndex = MapPackage.Find(TEXT("UEDPIE_"));
	if (PIEPrefixIndex != INDEX_NONE)
	{
		const int32 PrefixEnd = MapPackage.Find(TEXT("_"), ESearchCase::CaseSensitive, ESearchDir::FromStart, PIEPrefixIndex + 7);
		if (PrefixEnd != INDEX_NONE)
		{
			MapPackage.RemoveAt(PIEPrefixIndex, PrefixEnd - PIEPrefixIndex + 1);
		}
	}
	if (!GetWorld()->ServerTravel(FString::Printf(TEXT("%s?listen"), *MapPackage)))
	{
		SessionInterface->DestroySession(SessionName);
		CompleteOperation(false, TEXT("Session created, but the arena could not start as a listen server."));
		return;
	}
	CompleteOperation(true, FString::Printf(TEXT("Hosting %d-player %s session."), PendingMaxPlayers, bPendingLAN ? TEXT("LAN") : TEXT("online")));
	if (bSessionSmokeHost)
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SESSION SMOKE HOST READY"));
	}
}

void URidgefireSessionSubsystem::HandleFindSessionsComplete(bool bWasSuccessful)
{
	SessionSummaries.Reset();
	SessionResultIndices.Reset();
	if (bWasSuccessful && SessionSearch.IsValid())
	{
		for (int32 ResultIndex = 0; ResultIndex < SessionSearch->SearchResults.Num(); ++ResultIndex)
		{
			const FOnlineSessionSearchResult& Result = SessionSearch->SearchResults[ResultIndex];
			if (!Result.IsValid() || Result.Session.NumOpenPublicConnections <= 0)
			{
				continue;
			}
			SessionResultIndices.Add(ResultIndex);
			SessionSummaries.Add(FString::Printf(TEXT("%s  |  %d OPEN  |  PING %d ms"),
				*Result.Session.OwningUserName, Result.Session.NumOpenPublicConnections, Result.PingInMs));
		}
	}
	if (!bWasSuccessful)
	{
		SessionSearch.Reset();
	}
	CompleteOperation(bWasSuccessful, bWasSuccessful
		? FString::Printf(TEXT("Found %d joinable sessions."), SessionSummaries.Num())
		: TEXT("Session search failed."));
	if (bSessionSmokeClient)
	{
		if (!bWasSuccessful || SessionResultIndices.IsEmpty())
		{
			if (SessionSmokeSearchAttempts < 5)
			{
				UE_LOG(LogTemp, Warning, TEXT("RIDGEFIRE SESSION SMOKE retry discovery (%d/5)"), SessionSmokeSearchAttempts + 1);
				SessionSmokeTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
					FTickerDelegate::CreateUObject(this, &URidgefireSessionSubsystem::TickSessionSmokeStartup), 2.0f);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SESSION SMOKE FAIL discovery returned no joinable sessions after %d attempts"), SessionSmokeSearchAttempts);
			}
		}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SESSION SMOKE DISCOVERED %d"), SessionResultIndices.Num());
			if (!JoinSession(0))
			{
				UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SESSION SMOKE FAIL join request rejected"));
			}
		}
	}
}

void URidgefireSessionSubsystem::HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		if (SessionInterface.IsValid() && SessionInterface->GetNamedSession(SessionName))
		{
			SessionInterface->DestroySession(SessionName);
		}
		CompleteOperation(false, TEXT("The session could not be joined."));
		if (bSessionSmokeClient)
		{
			UE_LOG(LogTemp, Error, TEXT("RIDGEFIRE SESSION SMOKE FAIL join callback result %d"), static_cast<int32>(Result));
		}
		return;
	}
	FString ConnectString;
	if (!SessionInterface.IsValid() || !SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
	{
		if (SessionInterface.IsValid())
		{
			SessionInterface->DestroySession(SessionName);
		}
		CompleteOperation(false, TEXT("Joined, but the host address could not be resolved."));
		return;
	}
	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PlayerController)
	{
		SessionInterface->DestroySession(SessionName);
		CompleteOperation(false, TEXT("Joined, but no player controller is available to connect."));
		return;
	}
	CompleteOperation(true, TEXT("Joined session; connecting to host."));
	if (bSessionSmokeClient)
	{
		UE_LOG(LogTemp, Display, TEXT("RIDGEFIRE SESSION SMOKE JOINED; client travel requested"));
	}
	PlayerController->ClientTravel(ConnectString, TRAVEL_Absolute);
}

void URidgefireSessionSubsystem::HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		SessionSearch.Reset();
		SessionSummaries.Reset();
		SessionResultIndices.Reset();
	}
	CompleteOperation(bWasSuccessful, bWasSuccessful ? TEXT("Left the online session.") : TEXT("Could not leave the online session."));
}
