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
