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

#include "MyGameMode.h"
#include "Destructible.h"
#include "DestructibleMovementComponent.h"
#include "EnemySpaceship.h"
#include "EnemySpaceshipMovComponent.h"
#include "Projectile.h"
#include "Engine/World.h"
#include "UnrealMathUtility.h"
#include "GeneticAlgorithm.h"
#include "NNInput.h"
#include "NNSpaceShip.h"
#include "Phenotype.h"
#include "Genotype.h"
#include "UObjectIterator.h"
#include "Parameters.h"
#include "PlayerEndboss.h"
#include "Kismet/GameplayStatics.h"



AMyGameMode::AMyGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMyGameMode::BeginPlay()
{
	Super::BeginPlay();

	m_World = GetWorld();

	m_Parameters = NewObject<UParameters>(this);

	m_InputProvider = NewObject<UNNInput>(this);
	m_InputProvider->Initialize(this);

	m_fTimeTillNextSpawnDestructible = 0.5f;
	m_fTimeTillNextSpawnEnemy = 1.f;

	m_SpawnZone = FSpawnZone(SpawnAreaX, SpawnAreaX, SpawnAreaLeftY, SpawnAreaRightY);

	//TODO DONT HARDCODE
	m_PlayerStartPosition = FVector(100.f, 900.f, 100.f);
	m_EndbossStartPosition = FVector(1900.f, 1000.f, 100.f);
	//TODO

	m_Endboss = m_World->SpawnActor<APlayerEndboss>(m_PlayerEndboss, FVector(100.f, -300.f, 100.f), FRotator(0.f, 0.f, 0.f));
	
	//NEATController
	//create the population
	m_Population = NewObject<UGeneticAlgorithm>(this);
	m_Population->Initialize(m_Parameters->iPopulationSize, m_Parameters->iNumInputs, m_Parameters->iNumOutputs, this);
	m_iGeneration = m_Population->GetGeneration();

	//create the organisms
	CreateOrganisms();

	//get genotypes of the population
	TArray<UGenome*> PopulationGenotypes = m_Population->GetGenotypes();

	TArray<FSplitDepth> FSplitDepthTable = m_Population->GetFSplitDepthLookupTable();

	//assign the genotypes to the organisms and then create their respective neural net from their genotype
	for (int i = 0; i < m_NumberSpaceShips; ++i)
	{
		m_SpaceShips[i]->AssignGenotype(PopulationGenotypes[i]);
		m_SpaceShips[i]->GetGenotype()->CalculateNetDepth(FSplitDepthTable);
		m_SpaceShips[i]->AssignNeuralNet(m_SpaceShips[i]->GetGenotype()->CreatePhenotype());
	}

	m_fTimeTillNextSpawnDestructible = 0.5f;
	m_fTimeTillNextSpawnEnemy = 1.5f;
	m_fTimePlayed = 0.f;
	m_iCurrentPlayerID = 0;
	m_bGameReadyForCurrentPlayer = false;
	m_bAllPlayed = false;
	m_iGameState = -1;
	m_iTrainingsStage = -1;
	m_iNumEnemies = 0;
	m_bTraining = true;
	m_bFirstEnemySpawned = false;
	m_SimID = FDateTime::Now().ToString();
}

FTransform AMyGameMode::GenerateSpawnLocation()
{
	//Feature: Edit in Editor, don't hardcode
	int SpawnPoints = 10;
	int PointNr = FMath::RandRange(1, SpawnPoints);
	float Range = SpawnAreaRightY - SpawnAreaLeftY;
	float PointWidth = Range / SpawnPoints;
	float RandomYCoord = PointNr * PointWidth - PointWidth * 0.5;

	FVector Location = FVector(SpawnAreaX, RandomYCoord, WorldZLocation);
	FRotator Rotation = FRotator(0, 0, 0);

	FTransform MyTransform = FTransform(Rotation, Location);
	return MyTransform;
}

void AMyGameMode::SpawnDestructibleOnTimer(float DeltaTime)
{
	m_fTimeTillNextSpawnDestructible -= DeltaTime;

	if (m_fTimeTillNextSpawnDestructible < 0.0f)
	{
		if (m_DestructibleOne != nullptr)
		{
			if (m_World)
			{
				bool bSpawned = false;
				while (bSpawned == false)
				{
					FTransform SpawnTransform = GenerateSpawnLocation();
					ADestructible* Destructible = m_World->SpawnActor<ADestructible>(m_DestructibleOne, SpawnTransform);

					if (Destructible != nullptr)
					{
						float RandomVertSpeed = FMath::RandRange(200.0f, 500.0f);
						Destructible->OurMovementComponent->SetVertSpeed(RandomVertSpeed);

						m_fTimeTillNextSpawnDestructible = m_Parameters->fSpawnTimeDestructible;
						bSpawned = true;
						++m_iNumEnemies;
					}
				}
			}
		}
	}
}

void AMyGameMode::SpawnEnemySpaceshipOnTimer(float DeltaTime)
{
	m_fTimeTillNextSpawnEnemy -= DeltaTime;

	if (m_fTimeTillNextSpawnEnemy < 0.0f)
	{
		if (m_EnemySpaceshipOne != nullptr)
		{
			if (m_World)
			{
				bool bSpawned = false;
				while (bSpawned == false)
				{
					FTransform SpawnTransform = GenerateSpawnLocation();
					AEnemySpaceship* EnemyShip = m_World->SpawnActor<AEnemySpaceship>(m_EnemySpaceshipOne, SpawnTransform);

					if (EnemyShip != nullptr)
					{
						float RandomVertSpeed = FMath::RandRange(100.0f, 300.0f);
						EnemyShip->OurMovementComponent->SetVertSpeed(RandomVertSpeed);

						float sign = FMath::RandRange(0.0f, 2.0f);
						float RandomHorizSpeed = FMath::RandRange(100.0f, 300.0f);
						if (sign < 1.0f)
						{
							RandomHorizSpeed *= -1.0f;
							EnemyShip->OurMovementComponent->SetHorizSpeed(RandomHorizSpeed);
						}
						EnemyShip->OurMovementComponent->SetHorizSpeed(RandomHorizSpeed);

						m_fTimeTillNextSpawnEnemy = m_Parameters->fSpawnTimeEnemyShip;
						bSpawned = true;
					}
				}
			}
		}
	}
}

void AMyGameMode::AwardFitnessToCurrentPlayer(float fitness)
{
	m_SpaceShips[m_iCurrentPlayerID]->AwardFitness(fitness);
}

void AMyGameMode::DestroyedEnemy()
{
	--m_iNumEnemies;
}

void AMyGameMode::CreateOrganisms()
{
	//spawn everything
	m_NumberSpaceShips = m_Parameters->iPopulationSize;
	FVector SpawnLocation = FVector(2000.f, -500.f, 0.f);
	FRotator SpawnRotation = FRotator(0.f, 0.f, 0.f);

	for (int i = 0; i < m_NumberSpaceShips; ++i)
	{
		m_SpaceShips.Add(m_World->SpawnActor<ANNSpaceShip>(m_NNSpaceShips, SpawnLocation, SpawnRotation));

		//give them offsets so they dont bump into each other
		SpawnLocation.X -= 200.f;
		if (SpawnLocation.X < 500.f)
		{
			SpawnLocation.X = 2000.f;
			SpawnLocation.Y -= 150.f;
		}
	}

	SpawnLocation.X -= 300.f;

	for (int i = 0; i < m_Parameters->iNumBestOrganisms; ++i)
	{
		m_BestSpaceShips.Add(m_World->SpawnActor<ANNSpaceShip>(m_NNSpaceShips, SpawnLocation, SpawnRotation));

		SpawnLocation.X -= 200.f;
		if (SpawnLocation.X < 500.f)
		{
			SpawnLocation.X = 2000.f;
			SpawnLocation.Y -= 150.f;
		}
	}

	SpawnLocation.X -= 300.f;
	m_BestSpaceShip = m_World->SpawnActor<ANNSpaceShip>(m_NNSpaceShips, SpawnLocation, SpawnRotation);
}

void AMyGameMode::ResetGameForCurrentPlayer()
{
	//just move all best ships back... maybe Feature: identify last ship as best or regular etc pp
	for (ANNSpaceShip* curBestShip : m_BestSpaceShips)
	{
		curBestShip->MoveShipToStandby();
	}
	m_BestSpaceShip->MoveShipToStandby();
	m_Endboss->MoveShipToStandby();

	if (m_bAllPlayed)
	{
		//move last player to standby
		m_SpaceShips[m_iCurrentPlayerID]->MoveShipToStandby();
		//game needs to be reset for first ship from next generation
		m_bGameReadyForCurrentPlayer = false;
	}
	else if (m_iCurrentPlayerID == 0)
	{
		ResetGame();
		//move first player in position
		m_SpaceShips[m_iCurrentPlayerID]->MoveShipToStart(m_PlayerStartPosition);
		m_bGameReadyForCurrentPlayer = true;
	}
	else
	{
		ResetGame();
		//move previous player to standby
		m_SpaceShips[m_iCurrentPlayerID - 1]->MoveShipToStandby();
		m_SpaceShips[m_iCurrentPlayerID]->MoveShipToStart(m_PlayerStartPosition);
		m_bGameReadyForCurrentPlayer = true;
	}
}

void AMyGameMode::ResetGameForBestPlayer(int shipIndex)
{
	ResetGame();
	//move current player to standby
	//just move all best ships back... maybe Feature: identify last ship as best or regular etc pp
	for (ANNSpaceShip* curBestShip : m_BestSpaceShips)
	{
		curBestShip->MoveShipToStandby();
	}
	m_BestSpaceShip->MoveShipToStandby();
	m_Endboss->MoveShipToStandby();
	m_SpaceShips[m_iCurrentPlayerID]->MoveShipToStandby();
	//move best ship in position
	if (shipIndex == -1)
	{
		m_BestSpaceShip->MoveShipToStart(m_PlayerStartPosition);
	}
	else
	{
		m_BestSpaceShips[shipIndex]->MoveShipToStart(m_PlayerStartPosition);
	}
	m_bGameReadyForCurrentPlayer = false;
}

int AMyGameMode::GetNumberSpecies()
{
	int Number = m_Population->GetNumSpecies();
	return Number;
}

float AMyGameMode::GetCurrentPlayerYValue()
{
	FVector Position;

	if (m_iGameState == -1)
	{
		Position = m_SpaceShips[m_iCurrentPlayerID]->GetActorLocation();
	}
	else if (m_iGameState == 0)
	{
		Position = m_BestSpaceShip->GetActorLocation();
	}
	else
	{
		Position = m_BestSpaceShips[m_iGameState - 1]->GetActorLocation();
	}

	return Position.Y;
}

// Called every frame
void AMyGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*if (m_bTraining) //Trainingsmode... didn't work out
	{
	switch (m_iTrainingsStage)
	{
	case -1:
	if (m_bFirstEnemySpawned)
	{
	if (m_iNumEnemies < 3)
	{
	SpawnDestructibleOnTimer(DeltaTime);
	}
	}
	else
	{
	bool bSpawned = false;
	while (bSpawned == false)
	{
	FTransform SpawnTransform = FTransform(FRotator(0.f, 0.f, 0.f), FVector(m_SpawnAreaX, m_PlayerStartPosition.Y, m_WorldZLocation));
	ADestructible* Destructible = m_World->SpawnActor<ADestructible>(m_DestructibleOne, SpawnTransform);

	if (Destructible != nullptr)
	{
	float RandomVertSpeed = FMath::RandRange(200.0f, 500.0f);
	Destructible->OurMovementComponent->SetVertSpeed(RandomVertSpeed);

	m_TimeTillNextSpawnDestructible = m_SpawnTimeDestructibles;
	bSpawned = true;
	m_bFirstEnemySpawned = true;
	++m_iNumEnemies;
	}
	}
	}
	break;
	case 0:
	SpawnEnemySpaceshipOnTimer(DeltaTime);
	break;
	default:
	break;
	}
	}*/
	if (m_bTraining)
	{
		SpawnDestructibleOnTimer(DeltaTime);
		if (m_Parameters->bSimpleMode == false)
		{
			SpawnEnemySpaceshipOnTimer(DeltaTime);
		}
	}

	if (!UpdateNN(active, DeltaTime))
	{
		printf("Error Updating NEAT");
	}
}

//you may ask why... because I'm still pretty bad at programming
bool AMyGameMode::UpdateNN(run_type runType, float DeltaTime)
{
	switch (m_iGameState)
	{
	case -1:
		if (m_bGameReadyForCurrentPlayer)
		{
			if (UpdateTraining(runType, DeltaTime))
			{
				return true;
			}
		}
		else
		{
			ResetGameForCurrentPlayer();
			if (UpdateTraining(runType, DeltaTime))
			{
				return true;
			}
		}
		return false;
	case 0:
		if (UpdateTheBest(runType, DeltaTime))
		{
			return true;
		}
		return false;
	case 1:
		if (UpdateTopPlayer(runType, DeltaTime, 0))
		{
			return true;
		}
		return false;
	case 2:
		if (UpdateTopPlayer(runType, DeltaTime, 1))
		{
			return true;
		}
		return false;
	case 3:
		if (UpdateTopPlayer(runType, DeltaTime, 2))
		{
			return true;
		}
		return false;
	case 4:
		if (UpdateTopPlayer(runType, DeltaTime, 3))
		{
			return true;
		}
		return false;
	case 5:
		if (UpdateTopPlayer(runType, DeltaTime, 4))
		{
			return true;
		}
		return false;
	default:
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Switch-case in UpdateNEAT no valid case"));
		return false;
	}
}

bool AMyGameMode::UpdateTraining(run_type runType, float DeltaTime)
{
	if (m_bAllPlayed == false)
	{
		m_InputsForTheNN = m_InputProvider->CalculateInputsThisTick(GetCurrentPlayerYValue());

		if (!m_SpaceShips[m_iCurrentPlayerID]->Update(m_InputsForTheNN, runType, DeltaTime))
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("SS2GMB ExecuteUpdate Error updating spaceships"));
			return false;
		}

		m_fTimePlayed += DeltaTime;

		AwardFitnessToCurrentPlayer(DeltaTime * m_Parameters->fFitnessPerSecond);
		m_SpaceShips[m_iCurrentPlayerID]->MovementReward();

		CurrentHealth = m_SpaceShips[m_iCurrentPlayerID]->GetCurrentHealth();
		CurrentFitness = float(m_SpaceShips[m_iCurrentPlayerID]->GetFitness());

		//this NN is done playing
		if (m_fTimePlayed > m_Parameters->fTimeLeftToPlay || CurrentHealth < 1 || CurrentFitness <= m_Parameters->fFitnessCutoff)
		{
			//reset time played
			m_fTimePlayed = 0;
			//next player
			++m_iCurrentPlayerID;
			//need to ready game for next player
			m_bGameReadyForCurrentPlayer = false;

			if (CurrentFitness > m_dBestFitness)
			{
				m_dBestFitness = CurrentFitness;
			}

			//time for next generation?
			if (m_iCurrentPlayerID > (m_NumberSpaceShips - 1))
			{
				m_bAllPlayed = true;
				//we are still at the last ship
				--m_iCurrentPlayerID;
			}
		}
	}
	else if (m_bAllPlayed == true)
	{
		Epoch();

		//update generation
		m_iGeneration = m_Population->GetGeneration();

		//reset player
		m_iCurrentPlayerID = 0;
		m_bAllPlayed = false;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("SS2GMB Update m_bAllPlayed error"));
		return false;
	}
	return true;
}

bool AMyGameMode::UpdateTopPlayer(run_type runType, float DeltaTime, int bestPlayerIndex)
{
	m_InputsForTheNN = m_InputProvider->CalculateInputsThisTick(GetCurrentPlayerYValue());

	if (!m_BestSpaceShips[bestPlayerIndex]->Update(m_InputsForTheNN, runType, DeltaTime))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("SS2GMB ExecuteUpdate Error updating best spaceship"));
		return false;
	}

	CurrentHealth = m_BestSpaceShips[bestPlayerIndex]->GetCurrentHealth();
	//instead of letting the next ship play just continue with this guy
	if (CurrentHealth < 1)
	{
		m_BestSpaceShips[bestPlayerIndex]->Reset();
	}
	CurrentFitness = bestPlayerIndex + 1;

	return true;
}

bool AMyGameMode::UpdateTheBest(run_type runType, float DeltaTime)
{
	m_InputsForTheNN = m_InputProvider->CalculateInputsThisTick(GetCurrentPlayerYValue());

	if (!m_BestSpaceShip->Update(m_InputsForTheNN, runType, DeltaTime))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("SS2GMB ExecuteUpdate Error updating best spaceship"));
		return false;
	}

	CurrentHealth = m_BestSpaceShip->GetCurrentHealth();
	//instead of letting the next ship play just continue with this guy
	if (CurrentHealth < 1)
	{
		m_BestSpaceShip->Reset();
	}
	CurrentFitness = 9001;

	return true;
}

void AMyGameMode::Epoch()
{
	//get the fitness of all ships
	for (ANNSpaceShip* curSpaceShip : m_SpaceShips)
	{
		m_GenotypeFitness.Add(curSpaceShip->GetFitness());
	}

	TArray<UNeuralNet*> NewNetworks = m_Population->Epoch(m_GenotypeFitness);

	//log experiment data to file; uses m_GenotypeFitness so called here before values are reset
	LogDataToFile(m_GenotypeFitness);

	m_GenotypeFitness.Empty();

	//assign the new networks to the spaceships and reset
	for (int i = 0; i < m_NumberSpaceShips; ++i)
	{
		m_SpaceShips[i]->AssignNeuralNet(NewNetworks[i]);
		m_SpaceShips[i]->Reset();
	}

	//get the NN of the best performer form last generation
	TArray<UNeuralNet*> BestNetworks = m_Population->GetLastGenerationsBestPhenotypes();

	//record them and reset
	for (int i = 0; i < m_BestSpaceShips.Num(); ++i)
	{
		m_BestSpaceShips[i]->AssignNeuralNet(BestNetworks[i]);
		m_BestSpaceShips[i]->Reset();
	}

	//do the same for the best ship
	m_BestSpaceShip->AssignNeuralNet(m_Population->GetBestPhenotype());
	m_BestSpaceShip->Reset();
}

void AMyGameMode::PlayVsBestShipNr(int shipNumber)
{
	//for our switch case in UpdateNEAT
	m_iGameState = shipNumber;
	ResetGameForBestPlayer(shipNumber - 1);
	m_bTraining = false;
	m_Endboss->MoveShipToStart(m_EndbossStartPosition);
	APlayerController* m_BossController = UGameplayStatics::GetPlayerController(m_World, 0);
	m_BossController->Possess(m_Endboss);
}

void AMyGameMode::LetBestShipNrPlay(int shipNumber)
{
	//for our switch case in UpdateNEAT
	m_iGameState = shipNumber;
	m_bTraining = true;
	ResetGameForBestPlayer(shipNumber - 1);
}

void AMyGameMode::ReturnToTraining()
{
	m_iGameState = -1;
	m_bTraining = true;
	ResetGameForCurrentPlayer();
	//need to reset the current player completely, he starts new
	m_fTimePlayed = 0.f;
	m_SpaceShips[m_iCurrentPlayerID]->Reset();
}

void AMyGameMode::ResetGame()
{
	for (TObjectIterator<AProjectile> ProjIter; ProjIter; ++ProjIter)
	{
		ProjIter->Destroy();
	}

	for (TObjectIterator<ADestructible> DestructIter; DestructIter; ++DestructIter)
	{
		DestructIter->Destroy();
	}

	for (TObjectIterator<AEnemySpaceship> EnemyIter; EnemyIter; ++EnemyIter)
	{
		EnemyIter->Destroy();
	}

	m_iNumEnemies = 0;
	m_bFirstEnemySpawned = false;
}

void AMyGameMode::LogDataToFile(const TArray<double> &genotypeFitness)
{
	double avgFitness = 0.0;
	double bestFitness = 0.0;

	for (double fit : genotypeFitness)
	{
		avgFitness += fit;
		if (fit > bestFitness)
		{
			bestFitness = fit;
		}
	}

	avgFitness /= genotypeFitness.Num();

	FString savePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir());
	FString configPath = savePath;
	//don't rename the folder
	configPath.Append("Config/WindowsNoEditor/Parameters.ini");

	FString expName = savePath;
	expName.Append("Experiment");
	expName.Append(m_SimID);
	expName.Append(".txt");

	FString confName = savePath;
	confName.Append("ConfigSettings");
	confName.Append(m_SimID);
	confName.Append(".txt");

	FString config = "";
	FFileHelper::LoadFileToString(config, *configPath);

	FString log = "";

	if (m_iGeneration == 1)
	{
		log = "Generation;";
		log.Append("avgFitness;");
		log.Append("bestFitness;");
		log.Append("numSpecies;");
		log.Append("avgLinks;");
		log.Append("avgNeurons");
		log += LINE_TERMINATOR;
		FFileHelper::SaveStringToFile(log, *expName);
		log = "";

		FFileHelper::SaveStringToFile(config, *confName);
	}

	log += FString::FromInt(m_iGeneration) + ";" + FString::FromInt(int(avgFitness)) + ";" + FString::FromInt(int(bestFitness)) + ";" + FString::FromInt(m_Population->GetNumSpecies()) +
		";" + m_Population->GetGenomeStats() + LINE_TERMINATOR;

	FFileHelper::SaveStringToFile(log, *expName, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), 0x08);
}
