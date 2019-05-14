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


#include "Globals.h"

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyGameMode.generated.h"


class UGeneticAlgorithm;
class UNNInput;
class ANNSpaceShip;
class APlayerEndboss;
class UParameters;


//Central controller of this project. Handels the rules for the game and manages the training of the neural nets. Runs the genetic algorithm afterwards and keeps track of everything
UCLASS()
class NEATSHOOTER_API AMyGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
		UWorld* m_World;

	UPROPERTY()
		//this object holds all simulation parameters
		UParameters* m_Parameters;

	//keep track of spawn times
	float m_fTimeTillNextSpawnDestructible;
	float m_fTimeTillNextSpawnEnemy;

	UPROPERTY()
		//contains the dimensions of the spawn zone
		FSpawnZone m_SpawnZone;

	UPROPERTY()
		//handles the calculation of the play-area input
		UNNInput* m_InputProvider;

	UPROPERTY()
		//updated every tick then given to the current playing NN for its calculations
		TArray<double> m_InputsForTheNN;

	UPROPERTY()
		//actor for the human player
		APlayerEndboss* m_Endboss;
	UPROPERTY()
		//used to give controll to the human player
		APlayerController* m_BossController;

	//indicates what mode the simulation is in
	//default is -1: the NN are training one after the other
	//0: Human player plays vs the best NN of all time
	//1, 2, 3, 4, 5: Best NN of last gen with Nr is playing the game / vs human player
	int m_iGameState;

	//indicates the amount of spawned enemies
	//default is -1: only 1 destructible is spawned, it respawns when destroyed
	//currently not used
	int m_iTrainingsStage; //TODO

	//true if NN are training, false if human player is playing
	bool m_bTraining;

	//number of enemies currently in the scene
	int m_iNumEnemies;//TODO

	//first enemy every level spawned?
	bool m_bFirstEnemySpawned;//TODO

	//index to our currently playing spaceship
	int m_iCurrentPlayerID;
	//time current player has played
	float m_fTimePlayed;
	//true if all organisms of the current generation have played
	bool m_bAllPlayed;
	bool m_bGameReadyForCurrentPlayer;

	UPROPERTY()
		//where the actors spawn to play the game
		FVector m_PlayerStartPosition; //Feature: use PlayerSpawn asset for use in the Editor
	UPROPERTY()
		FVector m_EndbossStartPosition;

	//unique ID everytime simulation is started to log experiment data
	FString m_SimID;



	//Returns a valid random spawn location for an enemy. Currently has 10 "spawn points" 
	FTransform GenerateSpawnLocation();

	//Deletes all enemy objects and resets some values
	void ResetGame();

	//After prev. organism is done playing ready playing field for next
	void ResetGameForCurrentPlayer();
	void ResetGameForBestPlayer(int shipIndex);

	//Used to calculate the position input from the organism
	float GetCurrentPlayerYValue();

	//Log Data and calc stats
	void LogDataToFile(const TArray<double> &genotypeFitness);

	//Selects the right Update-function depending on the current simulation mode
	bool UpdateNN(run_type runType, float DeltaTime);
	//Called in UpdateNEAT to update the NN for the currently training organism, returns false if there was an error
	bool UpdateTraining(run_type runType, float DeltaTime);
	//Called in UpdateNEAT to update the NN for the currently playing top 5 organism, returns false if there was an error
	bool UpdateTopPlayer(run_type runType, float DeltaTime, int bestPlayerIndex);
	//Called in UpdateNEAT to update the NN for the currently playing all time best organism, returns false if there was an error
	bool UpdateTheBest(run_type runType, float DeltaTime);

	//Functionality as the NEATController
private:
	UPROPERTY()
		//storage for the entire population of genotypes (chromosomes)
		UGeneticAlgorithm* m_Population;

	UPROPERTY()
		//storage for all organisms (player)
		TArray<ANNSpaceShip*> m_SpaceShips;

	int m_NumberSpaceShips;

	//was stupid to not include them as the last ships in the general population above. Feature
	UPROPERTY()
		//copies of the best performing organisms
		TArray<ANNSpaceShip*> m_BestSpaceShips;
	UPROPERTY()
		ANNSpaceShip* m_BestSpaceShip;

	//all time best fitness
	double m_dBestFitness;

	//generation counter
	int	m_iGeneration;

	UPROPERTY()
		//stores the fitness of the current generation. After it is done used to create the next one
		TArray<double> m_GenotypeFitness;


	//Spawns all the required actors for the organisms in the world
	void CreateOrganisms();

	//Creates the next generation of networks, updates the spaceships with them and resets the ships
	void Epoch();


	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

public:
	AMyGameMode();

	//blueprint bindings for the editor
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Actors")
		class TSubclassOf<class ADestructible> m_DestructibleOne;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Actors")
		class TSubclassOf<class AEnemySpaceship> m_EnemySpaceshipOne;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Actors")
		class TSubclassOf<class ANNSpaceShip> m_NNSpaceShips;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Actors")
		class TSubclassOf<class APlayerEndboss> m_PlayerEndboss;
	//UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Actors")
	//	class TSubclassOf<class UParameters> m_ParametersTemplate;

	//the dimensions of the spawn zone. Feature: Don't hardcode, change through spawn object in the editor
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spawn")
		float SpawnAreaLeftY = 0.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spawn")
		float SpawnAreaRightY = 2000.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spawn")
		float SpawnAreaX = 2100.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spawn")
		float WorldZLocation = 0.0f;

	//Spawn functions for enemies
	UFUNCTION(BlueprintCallable, Category = "GamePlay")
		void SpawnDestructibleOnTimer(float DeltaTime);
	UFUNCTION(BlueprintCallable, Category = "GamePlay")
		void SpawnEnemySpaceshipOnTimer(float DeltaTime);

	//Used to reward the training organisms fitness
	UFUNCTION(BlueprintCallable, Category = "GamePlay")
		void AwardFitnessToCurrentPlayer(float fitness);

	//Called if enemy is destroyed
	UFUNCTION(BlueprintCallable, Category = "GamePlay")
		void DestroyedEnemy();

	//functions for the GameInputHandler to call
	void PlayVsBestShipNr(int shipNumber);
	void LetBestShipNrPlay(int shipNumber);
	void ReturnToTraining();

	//info on current player
	int CurrentHealth;
	float CurrentFitness;

	//Information functions on the current player
	UFUNCTION(BlueprintCallable, Category = "CurrentPlayer")
		int GetCurrentPlayerNumber() { return m_iCurrentPlayerID; }
	UFUNCTION(BlueprintCallable, Category = "CurrentPlayer")
		int GetCurrentPlayerHealth() { return CurrentHealth; }
	UFUNCTION(BlueprintCallable, Category = "CurrentPlayer")
		float GetCurrentPlayerFitness() { return CurrentFitness; }

	//Information on status of the GA
	UFUNCTION(BlueprintCallable, Category = "NEAT")
		int GetNumberSpecies();
	UFUNCTION(BlueprintCallable, Category = "NEAT")
		int GetNumberSpaceShips() { return m_NumberSpaceShips; }
	UFUNCTION(BlueprintCallable, Category = "NEAT")
		int GetCurrentGeneration() { return m_iGeneration; }
	UFUNCTION(BlueprintCallable, Category = "NEAT")
		float GetBestFitness() { return m_dBestFitness; }

	UParameters* const GetParameters() { return m_Parameters; }
	FSpawnZone const GetSpawnZone() { return m_SpawnZone; }
};