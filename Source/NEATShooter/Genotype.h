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
#include "Genotype.generated.h"


class UInnovation;
class UGenome;
class UNeuralNet;
class AMyGameMode;
struct FSplitDepth;
struct FSNeuronGene;
struct FSLinkGene;


//NeuronGene for the genotype
USTRUCT()
struct FSNeuronGene
{
	GENERATED_BODY()

	UPROPERTY()
		int iID;
	UPROPERTY()
		TEnumAsByte<neuron_type> NeuronType;
	UPROPERTY()
		double dSplitX;
	UPROPERTY()
		double dSplitY;

	FSNeuronGene() { iID = -1; NeuronType = noType; dSplitX = -1.0; dSplitY = -1.0; }
	FSNeuronGene(neuron_type type, int id, double splitX, double splitY, double activation = 1.0) : NeuronType(type), iID(id), dSplitX(splitX), dSplitY(splitY) {}

	//FString WriteToFile()
	//{
	//	FString out = "\nNeuron: " + FString::FromInt(iID) + "  Type: " + EnumToString("neuron_type", NeuronType)
	//		+ "  SplitX: " + FString::SanitizeFloat(dSplitX) + "  SplitY: " + FString::SanitizeFloat(dSplitY);

	//	return out;
	//}
};

//LinkGene for the genotype
USTRUCT()
struct FSLinkGene
{
	GENERATED_BODY()

	UPROPERTY()
		int FromNeuron;
	UPROPERTY()
		int ToNeuron;
	UPROPERTY()
		double dWeight;
	UPROPERTY()
		bool bEnabled;
	UPROPERTY()
		int iInnovationID;
	UPROPERTY()
		bool bRecurrent;

	FSLinkGene() { FromNeuron = -1; ToNeuron = -1; dWeight = -1; bEnabled = false; iInnovationID = -1; bRecurrent = false; }
	FSLinkGene(int fromNeuron, int toNeuron, double weight, bool enabled, int innovationID, bool rec = false) : FromNeuron(fromNeuron), ToNeuron(toNeuron), dWeight(weight), bEnabled(enabled), iInnovationID(innovationID), bRecurrent(rec) {}

	//for sorting depending on innovation ID
	friend bool operator<(const FSLinkGene& lhs, const FSLinkGene& rhs)
	{
		return (lhs.iInnovationID < rhs.iInnovationID);
	}

	//FString WriteToFile()
	//{
	//	FString out = "\nInnovID: " + FString::FromInt(iInnovationID) + "  From: " + FString::FromInt(FromNeuron)
	//		+ "  To: " + FString::FromInt(ToNeuron) + "  Enabled: " + BoolToString(bEnabled)
	//		+ "  Recurrent: " + BoolToString(bRecurrent) + "  Weight: " + FString::SanitizeFloat(dWeight);

	//	return out;
	//}
};

//This class stores the genetic information (genotype) of the organisms (NNSpaceShip). Used to create the phenotype and to mutate itself
UCLASS()
class NEATSHOOTER_API UGenome : public UObject
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
		AMyGameMode* m_GameMode;

	UPROPERTY()
	int m_GenomeID;

	UPROPERTY()
		//the vector of neurons has the input neurons first from 0 to numInputs, then one bias neuron, then output neurons, then hidden neurons
		TArray<FSNeuronGene> m_Neurons;
	UPROPERTY()
		TArray<FSLinkGene> m_Links;

	UPROPERTY()
		UNeuralNet* m_Phenotype;

	UPROPERTY()
		//depth of the produced net
		int m_iDepth;

	UPROPERTY()
		double m_dFitness;
	UPROPERTY()
		//fitness adjusted for species
		double m_dSpeciesFitness;

	UPROPERTY()
		int m_iNumInputs;
	UPROPERTY()
		int m_iNumOutputs;

	UPROPERTY()
		//ID of the species it belongs to
		int m_iSpecies;

	UPROPERTY()
		//number of offspring to be produced by this genome
		double m_dSpawnAmount;



	//returns true if both neurons are already linked
	bool DuplicateLink(int NeuronIn, int NeuronOut);

	//given a neuron ID this function finds its position in the neuron list of the genome
	int GetNeuronPosFromID(int neuronID);

	//overload '<' used for sorting from fittest to poorest
	friend bool operator<(const UGenome& lhs, const UGenome& rhs)
	{
		return (lhs.m_dFitness > rhs.m_dFitness);
	}

public:
	UGenome();
	//Creates a network where all inputs are connected with outputs
	void InitializeStandard(int id, int nrInputs, int nrOutputs, AMyGameMode* gameMode);
	void InitializeCustom(int id, TArray<FSNeuronGene> neurons, TArray<FSLinkGene> genes, int nrInputs, int nrOutputs, AMyGameMode* gameMode);

	//Create phenotype from genome and return its pointer
	UNeuralNet*	CreatePhenotype();

	void DeletePhenotype();

	//Returns true if the genome already has a specific neuron
	bool GenomeAlreadyHasNeuronID(int neuronID);

	//Calculates and returns the compatibility score with another genome
	double GetCompatibilityScore(UGenome* otherGenome);

	//Initializes all the link weights to random values in ]-1,1[
	void InitializeWeights();

	//Sorts genes
	void SortGenes();

	//Determines Net-Depth by comparing SplitY-values of neurons to those in the depth-table 
	void CalculateNetDepth(TArray<FSplitDepth> FSplitDepths);

	//----------------Mutator functions---------------------//
	//Toggle links on or off 
	void ToggleLinkGenes(double toggleChance, int numTries);

	//Enables disabled linkGenes
	void ReenableLinkGenes(double enableChance);

	//Mutate the genome by altering the connection weights
	void MutateWeights(double maxMutationPower, double mutationChance, double newWeightChance);

	//Mutate the genome by adding a neural node 
	void MutateAddNode(UInnovation &innovationList, double mutationChance, int numTries);

	//Mutate the genome by adding a new link between 2 random neural nodes
	void MutateAddLink(UInnovation &innovationList, double mutationChance, int numTries);



	int GetID() { return m_GenomeID; }
	void SetID(int id) { m_GenomeID = id; }

	int GetDepth() { return m_iDepth; }
	void SetDepth(int depth) { m_iDepth = depth; }

	int GetNumLinkGenes() { return m_Links.Num(); }
	int GetNumNeuronGenes() { return m_Neurons.Num(); }
	int GetNumInputs() { return m_iNumInputs; }
	int GetNumOutputs() { return m_iNumOutputs; }

	double GetSpawnAmount() { return m_dSpawnAmount; }
	void SetSpawnAmount(double spawnAmount) { m_dSpawnAmount = spawnAmount; }

	void SetFitness(double fitness) { m_dFitness = fitness; }
	void SetSpeciesFitness(double fitness) { m_dSpeciesFitness = fitness; }
	double GetFitness() { return m_dFitness; }
	double GetSpeciesFitness() { return m_dSpeciesFitness; }

	double GetSplitY(int id) { return m_Neurons[id].dSplitY; }

	int GetSpecies() { return m_iSpecies; }
	void SetSpecies(int species) { m_iSpecies = species; }

	UNeuralNet* GetPhenotype() { return m_Phenotype; }

	TArray<FSLinkGene> GetLinkGenesList() { return m_Links; }
	TArray<FSNeuronGene> GetNeuronGenesList() { return m_Neurons; }
};
