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
#include "Species.generated.h"


class UGenome;
class AMyGameMode;


//Defines the species for the NEAT algorithm
UCLASS()
class NEATSHOOTER_API USpecies : public UObject
{
	GENERATED_BODY()
	
protected:
	UPROPERTY()
		AMyGameMode* m_GameMode;

	UPROPERTY()
		//highest fitness genome of the species
		UGenome* m_Leader;
	UPROPERTY()
		//all the genomes within this species
		TArray<UGenome*> m_Members;

	UPROPERTY()
		//to identify species
		int m_iSpeciesID;
	UPROPERTY()
		//best fitness found so far by this species
		double m_dBestFitness;
	UPROPERTY()
		//generations since fitness has improved, we can use this info to kill off a species if required
		int m_iGensNoImprovement;
	UPROPERTY()
		//age of species
		int m_iSpeciesAge;
	UPROPERTY()
		//how many of this species should be spawned for the next population
		double m_dSpawnAmount;

public:
	USpecies();
	void Initialize(UGenome *firstGenome, int speciesID, AMyGameMode* gameMode);

	//Boost fitness of new species, penalizes old species and performs fitness sharing over the entire species
	void AdjustFitnessScores();

	//Add a new genome to the species
	void AddMember(UGenome *newGenome);

	//Clears out all member from last generation
	void Purge();

	//Calculates how many offsprings this species is allowed to produce
	void CalculateSpawnAmount();

	//Returns one of the top genomes of the species selected at random
	UGenome* GetTopGenome();

	//So we can sort species by best fitness. Largest first
	friend bool operator<(const USpecies &lhs, const USpecies &rhs)
	{
		return lhs.m_dBestFitness > rhs.m_dBestFitness;
	}



	UGenome* GetLeader() { return m_Leader; }
	double GetNumToSpawn() { return m_dSpawnAmount; }
	int GetNumMembers() { return m_Members.Num(); }
	int GetGensNoImprovement() { return m_iGensNoImprovement; }
	int GetSpeciesID() { return m_iSpeciesID; }
	double GetLeaderFitness();
	double GetBestFitness() { return m_dBestFitness; }
	int GetAge() { return m_iSpeciesAge; }
};
