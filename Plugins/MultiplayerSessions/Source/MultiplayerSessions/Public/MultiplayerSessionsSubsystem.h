// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MultiplayerSessionsSubsystem.generated.h"

UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

private:
	IOnlineSessionPtr OnlineSessionInterface;
	
public:
	UMultiplayerSessionsSubsystem();
	
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

private:
	void BindDelegates();

public:
	void RequestCreateSession(const int32 NumPublicConnections, const FString& MatchType);
	void OnCreateSessionComplete(const FName SessionName, const bool bWasSuccessful);
	
	void DestroySession();
	void OnDestroySessionComplete(const FName SessionName, const bool bWasSuccessful);
	
	void FindSessions(int32 MaxSearchResults);
	void OnFindSessionsComplete(const bool bWasSuccessful);
	
	void JoinSession();
	void OnJoinSessionComplete(const FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	void StartSession();
	void OnStartSessionComplete(const FName SessionName, const bool bWasSuccessful);

public:
	UFUNCTION(BlueprintPure)
	static UMultiplayerSessionsSubsystem* Get(const UGameInstance* GameInstance);
};
