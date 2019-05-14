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
#include "GameFramework/Info.h"
#include "Parameters.generated.h"



//Stores all parameters for the simulation. Edit in /Config/DefaultParameters.ini
UCLASS(Config=Parameters, Blueprintable)
class NEATSHOOTER_API UParameters : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Config, EditAnywhere)
		//number of organisms
		int iPopulationSize;
	UPROPERTY(VisibleAnywhere)
		//game logic has 5 atm
		int iNumBestOrganisms;

	UPROPERTY(Config, EditAnywhere)
		int iNumInputRows;
	UPROPERTY(Config, EditAnywhere)
		int iNumInputLines;
	UPROPERTY(Config, EditAnywhere)
		//calculated form rows * lines (game area) + location + fire rate + life
		int iNumInputs;
	UPROPERTY(Config, EditAnywhere)
		//currently 3 actions: fire, move left / right
		//if changed need to change code in NNSpaceShip::Update()
		int iNumOutputs;

	UPROPERTY(Config, EditAnywhere)
		double dBiasValue;

	UPROPERTY(Config, EditAnywhere)
		//the chance, each epoch, that a link will be added to the genome
		double dChanceAddLink;
	UPROPERTY(Config, EditAnywhere)
		int iNumTriesAddLink;
	UPROPERTY(Config, EditAnywhere)
		//the chance, each epoch, that a neuron will be added to the genome
		double dChanceAddNode;
	UPROPERTY(Config, EditAnywhere)
		int iNumTriesAddNode;

	UPROPERTY(Config, EditAnywhere)
		//mutation probabilities for mutating weights
		double dWeightMutationRate;
	UPROPERTY(Config, EditAnywhere)
		double dMaxWeightMutationPower;
	UPROPERTY(Config, EditAnywhere)
		double dNewWeightChance;

	UPROPERTY(Config, EditAnywhere)
		//chances of toggling or enabling linkGenes
		int iNumTriesToggle;
	UPROPERTY(Config, EditAnywhere)
		double dToggleLinkRate;
	UPROPERTY(Config, EditAnywhere)
		double dEnableLinkRate;

	UPROPERTY(Config, EditAnywhere)
		//indicates how similar genomes need to be to belong to the same species. Smaller means more species will be created
		double dCompatibilityThreshold;

	UPROPERTY(Config, EditAnywhere)
		//used in calculating the compatibility score
		double dExcessCoeff;
	UPROPERTY(Config, EditAnywhere)
		double dDisjointCoeff;
	UPROPERTY(Config, EditAnywhere)
		double dMatchingCoeff;

	UPROPERTY(Config, EditAnywhere)
		//during fitness adjustment this is how much the fitnesses of young species are boosted (e.g. 0.3 = 30%)
		double dNewSpeciesFitnessBonus;
	UPROPERTY(Config, EditAnywhere)
		//if the species are below this age their fitnesses are boosted
		int iNewSpeciesFitnessBonusAgeCutoff;
	UPROPERTY(Config, EditAnywhere)
		//if the species is above this age their fitness gets penalized
		int iSpeciesFitnessPenaltyAge;
	UPROPERTY(Config, EditAnywhere)
		//by this much (e.g. 0.2 = 20%)
		double dOldAgePenalty;

	UPROPERTY(Config, EditAnywhere)
		//percentage from which the member of a species get selected for the next generation. (0.2 = 20%) Don't put this above 80% or array index invalid
		double dSurvivalRate;

	UPROPERTY(Config, EditAnywhere)
		//how long we allow a species to exist without any improvement
		int iNumGensAllowedNoImprovement;

	UPROPERTY(Config, EditAnywhere)
		//maximum number of neurons permitted in the network
		int iMaxPermittedNeurons;

	UPROPERTY(Config, EditAnywhere)
		//chance at crossover
		double dCrossoverRate;
	UPROPERTY(Config, EditAnywhere)
		//amount of tries to find different crossover partner
		int iCrossoverTries;

	UPROPERTY(Config, EditAnywhere)
		//max desired amount of species. set to 0 to disable threshold adjustment feature
		int iSpeciesTarget;

	UPROPERTY(Config, EditAnywhere)
		//amount of tries for tournament selection after which the winner is selected out of the entire population. Change depending on population size
		int iNumTriesForSelection;

	UPROPERTY(Config, EditAnywhere)
		//time each player gets to play
		float fTimeLeftToPlay;

	UPROPERTY(Config, EditAnywhere)
		//below this fitness value the current player gets his run terminated
		float fFitnessCutoff;

	UPROPERTY(Config, EditAnywhere)
		//how much fitness is rewarded for each survived second
		float fFitnessPerSecond;

	UPROPERTY(Config, EditAnywhere)
		//fitness cost for shooting the gun
		float fFitnessPerShot;

	UPROPERTY(Config, EditAnywhere)
		//the distance a ship has to move in one direction before it gets awarded fitness
		float fNetMovementRequired;

	UPROPERTY(Config, EditAnywhere)
		//fitness rewarded after moving fMovementRequired distance
		float fMovementReward;

	UPROPERTY(Config, EditAnywhere)
		float fTimeBetweenShots;
	UPROPERTY(Config, EditAnywhere)
		int iShipHealth;

	UPROPERTY(Config, EditAnywhere)
		bool bSimpleMode;

	UPROPERTY(Config, EditAnywhere)
		float fSpawnTimeDestructible;
	UPROPERTY(Config, EditAnywhere)
		float fFitnessRewardDestructible;
	UPROPERTY(Config, EditAnywhere)
		float fSpawnTimeEnemyShip;
	UPROPERTY(Config, EditAnywhere)
		float fFitnessRewardEnemy;
	UPROPERTY(Config, EditAnywhere)
		float fFitnessPenaltyOnHit;

	UPROPERTY(Config, EditAnywhere)
	float fDestValue;
	UPROPERTY(Config, EditAnywhere)
	float fEnemyValue;
	UPROPERTY(Config, EditAnywhere)
	float fProjValue;

	UParameters();
};
