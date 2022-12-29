// Copyright Epic Games, Inc. All Rights Reserved.

#include "MultiplayerPluginCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"


//////////////////////////////////////////////////////////////////////////
// AMultiplayerPluginCharacter

AMultiplayerPluginCharacter::AMultiplayerPluginCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AMultiplayerPluginCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (const APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (const IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get())
	{
		OnlineSessionInterface = OnlineSubsystem->GetSessionInterface();
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 15.f, FColor::Cyan, FString::Printf(TEXT("Found subsystem %s"), *OnlineSubsystem->GetSubsystemName().ToString()));
	}

	if (OnlineSessionInterface)
	{
		const auto OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &AMultiplayerPluginCharacter::OnCreateSessionComplete);
		OnlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);

		const auto OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &AMultiplayerPluginCharacter::OnFindSessionsComplete);
		OnlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);

		const auto OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &AMultiplayerPluginCharacter::OnJoinSessionComplete);
		OnlineSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);
	}
}

void AMultiplayerPluginCharacter::RequestCreateGameSession()
{
	if (!OnlineSessionInterface.IsValid())
	{
		OnCreateSessionComplete(NAME_GameSession, false);
		return;
	}

	if (OnlineSessionInterface->GetNamedSession(NAME_GameSession)) // If session exists, destroy it and create a new one when destroy is completed
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 15.f, FColor::Yellow, FString::Printf(TEXT("Session already existed... attempting destroy!")));

		const auto OnDestroyCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateWeakLambda(this,
			[this](const FName SessionName, const bool bWasSuccessful)
			{
				if (bWasSuccessful)
				{
					GEngine->AddOnScreenDebugMessage(INDEX_NONE, 15.f, FColor::Green, FString::Printf(TEXT("Session destroyed successfully!")));
					CreateGameSession();
				}
				else
				{
					GEngine->AddOnScreenDebugMessage(INDEX_NONE, 15.f, FColor::Red, FString::Printf(TEXT("Session NOT destroyed successfully!")));
				}
			});
		OnlineSessionInterface->DestroySession(NAME_GameSession, OnDestroyCompleteDelegate);
	}
	else // Otherwise create the session
	{
		CreateGameSession();
	}
}

void AMultiplayerPluginCharacter::CreateGameSession()
{
	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = false;
	SessionSettings.NumPublicConnections = 4;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bAllowJoinViaPresence = true;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bUseLobbiesIfAvailable = true;
	SessionSettings.Set<FString>(TEXT("MatchType"), TEXT("FreeForAll"), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	
	if (const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController())
	{
		OnlineSessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionSettings);
	}	
}

void AMultiplayerPluginCharacter::OnCreateSessionComplete(const FName SessionName, const bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 15.f, FColor::Cyan, FString::Printf(TEXT("Created session: %s"), *SessionName.ToString()));
		GetWorld()->ServerTravel(TEXT("/Game/ThirdPerson/Maps/Lobby?listen"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 15.f, FColor::Red, FString("Failed to create session!"));
	}
}

void AMultiplayerPluginCharacter::RequestJoinGameSession()
{
	if (!OnlineSessionInterface.IsValid())
	{
		return;
	}
	
	// Find game sessions
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->MaxSearchResults = 10'000'000;
	SessionSearch->bIsLanQuery = false;
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	if (const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController())
	{
		OnlineSessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());
	}	
}

void AMultiplayerPluginCharacter::OnFindSessionsComplete(const bool bWasSuccessful)
{
	for	(const FOnlineSessionSearchResult& Result : SessionSearch->SearchResults)
	{
		const FString SessionId = Result.GetSessionIdStr();
		const FString& User = Result.Session.OwningUserName;
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 15.f, FColor::Cyan, FString::Printf(TEXT("Found session! Id: %s, User: %s"), *SessionId, *User));

		FString MatchType;
		Result.Session.SessionSettings.Get(TEXT("MatchType"), MatchType);
		
		if (MatchType == TEXT("FreeForAll"))
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 15.f, FColor::Cyan, FString::Printf(TEXT("Joining Match Type: %s"), *MatchType));

			if (const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController())
			{
				OnlineSessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, Result);
			}
		}
	}
}

void AMultiplayerPluginCharacter::OnJoinSessionComplete(const FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!OnlineSessionInterface.IsValid())
	{
		return;
	}

	FString Address;
	const bool bSuccessfulConnect = OnlineSessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

	if (bSuccessfulConnect)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 15.f, FColor::Yellow, FString::Printf(TEXT("Connection address: %s"), *Address));

		if (APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController())
		{
			PlayerController->ClientTravel(Address, TRAVEL_Absolute);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AMultiplayerPluginCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMultiplayerPluginCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMultiplayerPluginCharacter::Look);
	}
}

void AMultiplayerPluginCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AMultiplayerPluginCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}
