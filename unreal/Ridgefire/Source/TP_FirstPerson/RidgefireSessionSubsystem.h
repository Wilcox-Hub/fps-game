#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RidgefireSessionSubsystem.generated.h"

class FOnlineSessionSearch;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRidgefireSessionOperationComplete, bool, bWasSuccessful, const FString&, Message);

UCLASS()
class TP_FIRSTPERSON_API URidgefireSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category="Ridgefire|Online")
	bool HostSession(int32 MaxPlayers = 4, bool bUseLAN = true);

	UFUNCTION(BlueprintCallable, Category="Ridgefire|Online")
	bool FindSessions(bool bUseLAN = true);

	UFUNCTION(BlueprintCallable, Category="Ridgefire|Online")
	bool JoinSession(int32 SearchResultIndex);

	UFUNCTION(BlueprintCallable, Category="Ridgefire|Online")
	bool LeaveSession();

	UFUNCTION(BlueprintPure, Category="Ridgefire|Online")
	TArray<FString> GetSessionSummaries() const;

	UPROPERTY(BlueprintAssignable, Category="Ridgefire|Online")
	FRidgefireSessionOperationComplete OnSessionOperationComplete;

private:
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	FDelegateHandle CreateSessionDelegateHandle;
	FDelegateHandle FindSessionsDelegateHandle;
	FDelegateHandle JoinSessionDelegateHandle;
	FDelegateHandle DestroySessionDelegateHandle;
	TArray<FString> SessionSummaries;
	TArray<int32> SessionResultIndices;
	bool bOperationInFlight = false;
	int32 PendingMaxPlayers = 4;
	bool bPendingLAN = true;
	bool bSessionSmokeHost = false;
	bool bSessionSmokeClient = false;
	float SessionSmokeWaitSeconds = 0.0f;
	int32 SessionSmokeSearchAttempts = 0;
	FTSTicker::FDelegateHandle SessionSmokeTickerHandle;

	bool TickSessionSmokeStartup(float DeltaTime);
	bool ResolveSessionInterface();
	void ClearDelegates();
	void CompleteOperation(bool bWasSuccessful, const FString& Message);
	void HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleFindSessionsComplete(bool bWasSuccessful);
	void HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful);
};
