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

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameInputHandler.generated.h"


class AMyGameMode;


//Used for User-Input related to changing the mode of the simulation
UCLASS()
class NEATSHOOTER_API AGameInputHandler : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGameInputHandler(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY()
		UWorld* m_World;
	UPROPERTY()
		AMyGameMode* m_GameMode;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void AccelerateGame();
	void ResetGameSpeed();
	void FastForward();
	void ReturnToTraining();

	//Those functions give the game mode the indices to the best spaceship that the user wants to select
	void LetShip1Play();
	void LetShip2Play();
	void LetShip3Play();
	void LetShip4Play();
	void LetShip5Play();

	void PlayVsShip1();
	void PlayVsShip2();
	void PlayVsShip3();
	void PlayVsShip4();
	void PlayVsShip5();

	//gives shipNR 0
	void PlayerVsBest();
};
