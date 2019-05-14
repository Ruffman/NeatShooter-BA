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

#include "Parameters.h"



UParameters::UParameters()
{
	iPopulationSize = 100;
	iNumTriesForSelection = 20;
	iNumBestOrganisms = 5;
	iNumInputLines = 2;
	iNumInputRows = 10;
	//play area input and currently ship position, shot cooldown and life
	//iNumInputLines * iNumInputRows + 3
	iNumInputs = 23;
	iNumOutputs = 3;
	dBiasValue = 1;

	dChanceAddLink = 0.1;
	iNumTriesAddLink = 5;
	dChanceAddNode = 0.05;
	iNumTriesAddNode = 5;

	dWeightMutationRate = 0.7;
	dMaxWeightMutationPower = 0.8;
	dNewWeightChance = 0.1;

	dCompatibilityThreshold = 3;

	dNewSpeciesFitnessBonus = 0.3;
	iNewSpeciesFitnessBonusAgeCutoff = 10;
	iSpeciesFitnessPenaltyAge = 25;
	dOldAgePenalty = 0.15;
	iNumGensAllowedNoImprovement = 25;
	dSurvivalRate = 0.4;

	iMaxPermittedNeurons = 600;

	dCrossoverRate = 0.75;
	iCrossoverTries = 5;

	iSpeciesTarget = 15;

	iNumTriesToggle = 3;
	dToggleLinkRate = 0.1;
	dEnableLinkRate = 0.1;

	dExcessCoeff = 1;
	dDisjointCoeff = 1;
	dMatchingCoeff = 0.4;

	fTimeLeftToPlay = 20.f;
	fFitnessCutoff = -200.f;
	fFitnessPerSecond = 25.f;
	fFitnessPerShot = 50.f;
	fNetMovementRequired = 150.f;
	fMovementReward = 25.f;

	fTimeBetweenShots = 2.f;
	iShipHealth = 3;

	bSimpleMode = false;
	fSpawnTimeDestructible = 2.f;
	fFitnessRewardDestructible = 100.f;
	fSpawnTimeEnemyShip = 3.f;
	fFitnessRewardEnemy = 200.f;
	fFitnessPenaltyOnHit = 200.f;

	fDestValue = 0.3f;
	fEnemyValue = 0.6f;
	fProjValue = 1.f;

	LoadConfig();
	SaveConfig();
}