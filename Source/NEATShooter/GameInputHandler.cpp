//Copyright 2018 Raphael Haucke
//
//Licensed under the Apache License, Version 2.0 (the "License");
//you may not use this file except in compliance with the License.
//You may obtain a copy of the License at
//
//http ://www.apache.org/licenses/LICENSE-2.0
//
//Unless required by applicable law or agreed to in writing, software
//distributed under the License is distributed on an "AS IS" BASIS,
//WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//See the License for the specific language governing permissions and
//limitations under the License.

#include "GameInputHandler.h"
#include "Components/InputComponent.h"
#include "MyGameMode.h"
#include "Engine/World.h"


// Sets default values
AGameInputHandler::AGameInputHandler(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGameInputHandler::BeginPlay()
{
	Super::BeginPlay();
	
	m_World = GetWorld();
	m_GameMode = Cast<AMyGameMode>(m_World->GetAuthGameMode());
	if (!m_GameMode)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Couldn't find GameMode in GameInputHandler"));
	}
}

// Called every frame
void AGameInputHandler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AGameInputHandler::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	InputComponent->BindAction("AccelerateGame", IE_Pressed, this, &AGameInputHandler::AccelerateGame);
	InputComponent->BindAction("ResetGameSpeed", IE_Pressed, this, &AGameInputHandler::ResetGameSpeed);
	InputComponent->BindAction("FastForward", IE_Pressed, this, &AGameInputHandler::FastForward);
	InputComponent->BindAction("ReturnToTraining", IE_Pressed, this, &AGameInputHandler::ReturnToTraining);

	InputComponent->BindAction("PlayerVsShip1", IE_Pressed, this, &AGameInputHandler::PlayVsShip1);
	InputComponent->BindAction("PlayerVsShip2", IE_Pressed, this, &AGameInputHandler::PlayVsShip2);
	InputComponent->BindAction("PlayerVsShip3", IE_Pressed, this, &AGameInputHandler::PlayVsShip3);
	InputComponent->BindAction("PlayerVsShip4", IE_Pressed, this, &AGameInputHandler::PlayVsShip4);
	InputComponent->BindAction("PlayerVsShip5", IE_Pressed, this, &AGameInputHandler::PlayVsShip5);

	InputComponent->BindAction("PlayShip1", IE_Pressed, this, &AGameInputHandler::LetShip1Play);
	InputComponent->BindAction("PlayShip2", IE_Pressed, this, &AGameInputHandler::LetShip2Play);
	InputComponent->BindAction("PlayShip3", IE_Pressed, this, &AGameInputHandler::LetShip3Play);
	InputComponent->BindAction("PlayShip4", IE_Pressed, this, &AGameInputHandler::LetShip4Play);
	InputComponent->BindAction("PlayShip5", IE_Pressed, this, &AGameInputHandler::LetShip5Play);

	InputComponent->BindAction("PlayerVsBest", IE_Pressed, this, &AGameInputHandler::PlayerVsBest);
}

void AGameInputHandler::AccelerateGame()
{
	GetWorldSettings()->TimeDilation += 1.f;
}

void AGameInputHandler::ResetGameSpeed()
{
	GetWorldSettings()->TimeDilation = 1.f;
}

void AGameInputHandler::FastForward()
{
	GetWorldSettings()->TimeDilation += 50.f;
}

void AGameInputHandler::ReturnToTraining()
{
	m_GameMode->ReturnToTraining();
}

void AGameInputHandler::LetShip1Play()
{
	if (m_GameMode->GetCurrentGeneration() > 1)
	{
		m_GameMode->LetBestShipNrPlay(1);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Can't use this until first generation is done!"));
	}
}

void AGameInputHandler::LetShip2Play()
{
	if (m_GameMode->GetCurrentGeneration() > 1)
	{
		m_GameMode->LetBestShipNrPlay(2);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Can't use this until first generation is done!"));
	}
}

void AGameInputHandler::LetShip3Play()
{
	if (m_GameMode->GetCurrentGeneration() > 1)
	{
		m_GameMode->LetBestShipNrPlay(3);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Can't use this until first generation is done!"));
	}
}

void AGameInputHandler::LetShip4Play()
{
	if (m_GameMode->GetCurrentGeneration() > 1)
	{
		m_GameMode->LetBestShipNrPlay(4);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Can't use this until first generation is done!"));
	}
}

void AGameInputHandler::LetShip5Play()
{
	if (m_GameMode->GetCurrentGeneration() > 1)
	{
		m_GameMode->LetBestShipNrPlay(5);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Can't use this until first generation is done!"));
	}
}

void AGameInputHandler::PlayVsShip1()
{
	if (m_GameMode->GetCurrentGeneration() > 1)
	{
		m_GameMode->PlayVsBestShipNr(1);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Can't use this until first generation is done!"));
	}
}

void AGameInputHandler::PlayVsShip2()
{
	if (m_GameMode->GetCurrentGeneration() > 1)
	{
		m_GameMode->PlayVsBestShipNr(2);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Can't use this until first generation is done!"));
	}
}

void AGameInputHandler::PlayVsShip3()
{
	if (m_GameMode->GetCurrentGeneration() > 1)
	{
		m_GameMode->PlayVsBestShipNr(3);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Can't use this until first generation is done!"));
	}
}

void AGameInputHandler::PlayVsShip4()
{
	if (m_GameMode->GetCurrentGeneration() > 1)
	{
		m_GameMode->PlayVsBestShipNr(4);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Can't use this until first generation is done!"));
	}
}

void AGameInputHandler::PlayVsShip5()
{
	if (m_GameMode->GetCurrentGeneration() > 1)
	{
		m_GameMode->PlayVsBestShipNr(5);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Can't use this until first generation is done!"));
	}
}

void AGameInputHandler::PlayerVsBest()
{
	if (m_GameMode->GetCurrentGeneration() > 1)
	{
		m_GameMode->PlayVsBestShipNr(0);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Can't use this until first generation is done!"));
	}
}