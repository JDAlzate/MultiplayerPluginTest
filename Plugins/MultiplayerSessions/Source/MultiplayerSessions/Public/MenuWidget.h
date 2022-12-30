// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UObject/Object.h"
#include "MenuWidget.generated.h"

class UMultiplayerSessionsSubsystem;
class UButton;

UCLASS()
class MULTIPLAYERSESSIONS_API UMenuWidget : public UUserWidget
{
	GENERATED_BODY()

private:
	UPROPERTY(meta = (BindWidget))
	UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

private:
	UPROPERTY(Transient)
	UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	UFUNCTION()
	virtual void OnMultiplayerSessionCreated(const FName SessionName, const bool bWasSuccessful);
	
public:
	UFUNCTION(BlueprintCallable)
	void SetupMenu();

private:
	void TearDownMenu();

private:
	UFUNCTION()
	void OnHostButtonClicked();

	UFUNCTION()
	void OnJoinButtonClicked();
};
