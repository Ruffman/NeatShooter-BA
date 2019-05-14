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


#include "Species.h"
#include "Genotype.h"
#include "Parameters.h"
#include "Globals.h"
#include "MyGameMode.h"



USpecies::USpecies()
{
}

void USpecies::Initialize(UGenome *firstGenome, int speciesID, AMyGameMode* gameMode)
{
	m_Members.Add(firstGenome);
	m_Leader = firstGenome;
	m_iSpeciesID = speciesID;
	m_dBestFitness = firstGenome->GetFitness();
	m_iGensNoImprovement = 0;
	m_iSpeciesAge = 0;
	m_dSpawnAmount = 0;

	m_GameMode = gameMode;
}

void USpecies::AdjustFitnessScores()
{
	for (UGenome* curGenome : m_Members)
	{
		double fitness = curGenome->GetFitness();

		//boost the fitness scores if the species is young
		if (m_iSpeciesAge < m_GameMode->GetParameters()->iNewSpeciesFitnessBonusAgeCutoff)
		{
			fitness += (fitness * m_GameMode->GetParameters()->dNewSpeciesFitnessBonus);
		}
		//punish older species
		else if (m_iSpeciesAge > m_GameMode->GetParameters()->iSpeciesFitnessPenaltyAge)
		{
			fitness -= (fitness * m_GameMode->GetParameters()->dOldAgePenalty);
		}

		//share fitness with species
		double SpeciatedFitness = fitness / m_Members.Num();
		curGenome->SetSpeciesFitness(SpeciatedFitness);
	}
}

void USpecies::AddMember(UGenome *newGenome)
{
	if (newGenome->GetFitness() > m_dBestFitness)
	{
		m_dBestFitness = newGenome->GetFitness();
		m_iGensNoImprovement = 0;
		m_Leader = newGenome;
	}
	newGenome->SetSpecies(m_iSpeciesID);
	m_Members.Add(newGenome);
	m_Members.Sort();
}

void USpecies::Purge()
{
	m_Members.Empty();
	++m_iSpeciesAge;
	++m_iGensNoImprovement;
	m_dSpawnAmount = 0;
}

void USpecies::CalculateSpawnAmount()
{
	for (UGenome* curGenome : m_Members)
	{
		m_dSpawnAmount += curGenome->GetSpawnAmount();
	}
}

UGenome* USpecies::GetTopGenome()
{
	UGenome* Genome;

	if (m_Members.Num() == 1)
	{
		Genome = m_Members[0];
	}
	else
	{
		int Max = (int)(m_GameMode->GetParameters()->dSurvivalRate * m_Members.Num()) + 1;
		int ChosenOne = RandInt(0, Max);
		Genome = m_Members[ChosenOne];
	}

	return Genome;
}

double USpecies::GetLeaderFitness()
{
	return m_Leader->GetFitness();
}
