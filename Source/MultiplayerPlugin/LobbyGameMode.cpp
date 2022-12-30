// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"


void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (GameState)
	{
		const int32 PlayerCount = GameState->PlayerArray.Num();
		GEngine->AddOnScreenDebugMessage(1, 60.f, FColor::Yellow, FString::Printf(TEXT("Players in game: %d"), PlayerCount));

		if (const APlayerState* PlayerState = NewPlayer->GetPlayerState<APlayerState>())
		{
			const FString PlayerName = PlayerState->GetPlayerName();
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 60.f, FColor::Cyan, FString::Printf(TEXT("%s has joined the game!"), *PlayerName));
		}
	}
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	if (GameState)
	{
		const int32 PlayerCount = GameState->PlayerArray.Num();
		GEngine->AddOnScreenDebugMessage(1, 60.f, FColor::Yellow, FString::Printf(TEXT("Players in game: %d"), PlayerCount - 1));

		if (const APlayerState* PlayerState = Exiting->GetPlayerState<APlayerState>())
		{
			const FString PlayerName = PlayerState->GetPlayerName();
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 60.f, FColor::Cyan, FString::Printf(TEXT("%s has exited the game!"), *PlayerName));
		}
	}
	
	Super::Logout(Exiting);
}
