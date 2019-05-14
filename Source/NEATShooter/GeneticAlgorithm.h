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
#include "GameFramework/Info.h"
#include "GeneticAlgorithm.generated.h"


class UGenome;
class USpecies;
class UInnovation;
class UNeuralNet;
class AMyGameMode;
class UParameters;


//The main class for the NEAT-Genetic Algorithm
UCLASS()
class NEATSHOOTER_API UGeneticAlgorithm : public UObject
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
		AMyGameMode* m_GameMode;

	UPROPERTY()
		//parameter object
		UParameters* m_Parameters;

	UPROPERTY()
		//current population of genomes
		TArray<UGenome*> m_Genomes;
	UPROPERTY()
		//all the species
		TArray<USpecies*> m_Species;

	UPROPERTY()
		//list to keep track of innovations
		UInnovation* m_Innovation;

	//current generation
	int m_iGeneration;
	int m_iNextGenomeID;
	int m_iNextSpeciesID;
	int m_iPopSize;


	//adjusted fitness scores
	double m_dTotalAdjustedFitness;
	double m_dAverageAdjustedFitness;

	UPROPERTY()
		//the precalculated split depths. They are used 
		//to calculate a neurons x/y position
		//for calculating the flush depth of the network when a
		//phenotype is working in 'snapshot' mode
		TArray<FSplitDepth> m_FSplitDepths;

	double m_dBestFitnessEver;

	UPROPERTY()
		//best genomes of last generation
		TArray<UGenome*> m_BestGenomes;

	UPROPERTY()
		UGenome* m_BestGenomeEver;

	double m_AvgNumNeuronsLastGen;
	double m_AvgNumLinksLastGen;



	//Checks if the passed list already contains the neuron
	bool AddNeuronID(int neuronID, TArray<int> &vNeurons);

	//Resets some values to ready for the next epoch, kills off all the phenotypes and any poorly performing species
	void ResetAndKill();

	//Separates each individual into its respective species by calculating
	//a compatibility score with every other member of the population and 
	//niching accordingly. The function then adjusts the fitness scores of
	//each individual by species age and by sharing and also determines
	//how many offspring each individual should spawn
	void SpeciateAndCalculateSpawnAmounts();

	//Adjusts the fitness scores depending on the number sharing the species and the age of the species
	void AdjustSpeciesFitnesses();

	//Generate offspring out of two genomes
	UGenome* Crossover(UGenome* motherGenome, UGenome* fatherGenome);

	//Test fitness of genomes from the entire population against each other numTries, select the winner
	UGenome* TournamentSelection(int numTries);

	//Sorts the population into descending fitness, keeps a record of the best genomes and updates any fitness statistics accordingly
	void SortAndRecord();

	//Automatically adjusts the compatibility threshold in an attempt to keep the number of species at a constant value
	void AdjustCompatibilityThreshold();

	//Used to calculate a lookup table of split depths
	TArray<FSplitDepth> Split(double low, double high, int depth);

public:
	UGeneticAlgorithm();
	//Creates a population starting with minimal, fully connected genomes consisting of specified number of inputs and outputs
	void Initialize(int populationSize, int numInputs, int numOutputs, AMyGameMode* gameMode);

	//Main update function of the GeneticAlgorithm module. Creates new generation
	TArray<UNeuralNet*> Epoch(TArray<double> &fitnessScores);

	//Creates pointer to the genotypes of the population
	TArray<UGenome*> GetGenotypes();

	//Stores the best genomes from last population
	void StoreBestGenomes();

	//Returns the best phenotypes from last generation
	TArray<UNeuralNet*> GetLastGenerationsBestPhenotypes();

	//Returns the best phenotype ever found
	UNeuralNet* GetBestPhenotype();

	FString GetGenomeStats();



	int GetNumSpecies()const { return m_Species.Num(); }
	double GetBestEverFitness()const { return m_dBestFitnessEver; }
	int GetGeneration()const { return m_iGeneration; }
	TArray<FSplitDepth> GetFSplitDepthLookupTable() { return m_FSplitDepths; }

	void SetFitness(int genome, double fitness);
};
