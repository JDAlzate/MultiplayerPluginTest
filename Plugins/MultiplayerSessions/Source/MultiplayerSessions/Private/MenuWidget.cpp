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

void UMenuWidget::SetupMenu()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

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
	GEngine->AddOnScreenDebugMessage(INDEX_NONE, 15.f, FColor::Green, TEXT("Host clicked!"));

	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->RequestCreateSession(4, FString("FreeForAll"));
	}
}

void UMenuWidget::OnJoinButtonClicked()
{
	GEngine->AddOnScreenDebugMessage(INDEX_NONE, 15.f, FColor::Green, TEXT("Join clicked!"));

	if (MultiplayerSessionsSubsystem)
	{
	}
}
