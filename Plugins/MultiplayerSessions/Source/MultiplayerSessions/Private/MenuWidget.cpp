// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuWidget.h"

#include "MultiplayerSessionsSubsystem.h"
#include "Components/Button.h"

void UMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &UMenuWidget::OnHostButtonClicked);
	}

	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &UMenuWidget::OnJoinButtonClicked);
	}
}

void UMenuWidget::NativeDestruct()
{
	TearDownMenu();
	Super::NativeDestruct();
}

void UMenuWidget::OnMultiplayerSessionCreated(const FName SessionName, const bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		GetWorld()->ServerTravel(TEXT("/Game/ThirdPerson/Maps/Lobby?listen"));
	}
}

void UMenuWidget::OnMultiplayerSessionsFound(const TArray<FOnlineSessionSearchResult>& SearchResults, const bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(INDEX_NONE, 90.f, FColor::Cyan, TEXT("UMenuWidget::OnMultiplayerSessionsFound Called!!"));

	if (!MultiplayerSessionsSubsystem)
	{
		return;
	}

	for (const FOnlineSessionSearchResult& SearchResult : SearchResults)
	{
		FString ResultMatchType;
		SearchResult.Session.SessionSettings.Get(TEXT("MatchType"), ResultMatchType);

		if (ResultMatchType == MatchType)
		{
			MultiplayerSessionsSubsystem->JoinSession(SearchResult);
			return;
		}
	}
}

void UMenuWidget::OnMultiplayerSessionJoined(const EOnJoinSessionCompleteResult::Type Result)
{
	if (!MultiplayerSessionsSubsystem)
	{
		return;
	}

	IOnlineSessionPtr OnlineSessionInterface = MultiplayerSessionsSubsystem->GetOnlineSubsystemInterface();
	if (!OnlineSessionInterface.IsValid())
	{
		return;
	}

	FString Address;
	OnlineSessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

	if (APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController())
	{
		PlayerController->ClientTravel(Address, TRAVEL_Absolute);
	}
}

void UMenuWidget::OnMultiplayerSessionDestroyed(const FName SessionName, const bool bWasSuccessful)
{
}

void UMenuWidget::OnMultiplayerSessionStarted(const FName SessionName, const bool bWasSuccessful)
{
}

void UMenuWidget::SetupMenu(const int32 NewMaxSearchSessions, const FString& NewMatchType)
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	MaxSessionSearches = NewMaxSearchSessions;
	MatchType = NewMatchType;

	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		FInputModeUIOnly InputModeData;
		InputModeData.SetWidgetToFocus(TakeWidget());
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerController->SetInputMode(InputModeData);
		PlayerController->SetShowMouseCursor(true);
	}

	MultiplayerSessionsSubsystem = UMultiplayerSessionsSubsystem::Get(GetGameInstance());
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->OnMultiplayerSessionCreatedDelegate.AddDynamic(this, &UMenuWidget::OnMultiplayerSessionCreated);
		MultiplayerSessionsSubsystem->OnMultiplayerFindSessionsComplete.AddUObject(this, &UMenuWidget::OnMultiplayerSessionsFound);
		MultiplayerSessionsSubsystem->OnMultiplayerJoinSessionComplete.AddUObject(this, &UMenuWidget::OnMultiplayerSessionJoined);
		MultiplayerSessionsSubsystem->OnMultiplayerSessionDestroyed.AddDynamic(this, &UMenuWidget::OnMultiplayerSessionDestroyed);
		MultiplayerSessionsSubsystem->OnMultiplayerSessionStarted.AddDynamic(this, &UMenuWidget::OnMultiplayerSessionStarted);
	}
}

void UMenuWidget::TearDownMenu()
{
	RemoveFromParent();

	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->SetShowMouseCursor(false);
	}
}

void UMenuWidget::OnHostButtonClicked()
{
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->RequestCreateSession(4, MatchType);
	}
}

void UMenuWidget::OnJoinButtonClicked()
{
	GEngine->AddOnScreenDebugMessage(INDEX_NONE, 90.f, FColor::Cyan, TEXT("OnJoinButtonClicked!"));
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->FindSessions(MaxSessionSearches);
	}
}
