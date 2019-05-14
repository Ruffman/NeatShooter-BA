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
#include "Genotype.h"

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Innovation.generated.h"


UENUM()
enum innovation_type
{
	new_neuron,
	new_link
};

//Defines the structure of the innovations
USTRUCT()
struct FSInnovation
{
	GENERATED_BODY()

	UPROPERTY()
		int InnovationID;

	UPROPERTY()
		TEnumAsByte<innovation_type> InnovationType;

	//if the innovation is a link
	UPROPERTY()
		int FromNeuron;
	UPROPERTY()
		int ToNeuron;

	//if the innovation is a neuron (then 2 new link-innovations are created as well)
	UPROPERTY()
		int NeuronID;
	UPROPERTY()
		TEnumAsByte<neuron_type> NeuronType;
	UPROPERTY()
		double dSplitX;
	UPROPERTY()
		double dSplitY;

	FSInnovation() { FromNeuron = -1; ToNeuron = -1; NeuronID = -1; NeuronType = noType; dSplitX = -1; dSplitY = -1; }

	//Innovation of a new link. No new neuron is created so NewNeuronID is set to -1 and NewNeuronType to noType
	FSInnovation(int fromNeuron, int toNeuron, int id) : FromNeuron(fromNeuron), ToNeuron(toNeuron), InnovationType(new_link),
		InnovationID(id), NeuronID(-1), NeuronType(noType), dSplitX(0), dSplitY(0) {}

	//Innovation of a new neuron
	FSInnovation(FSNeuronGene newNeuronGene, int id, int fromNeuron, int toNeuron) : NeuronID(newNeuronGene.iID), NeuronType(newNeuronGene.NeuronType),
		InnovationType(new_neuron), InnovationID(id), FromNeuron(fromNeuron), ToNeuron(toNeuron), dSplitX(newNeuronGene.dSplitX), dSplitY(newNeuronGene.dSplitY) {}

};

//The Innovation List Class. Keeps track of all innovations created during the evolution of the population
UCLASS()
class NEATSHOOTER_API UInnovation : public UObject
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
		//list of all the innovations, first are the startneurons, then the startlinks
		TArray<FSInnovation> m_Innovations;

	//keeps track of the IDs new innovations need
	int m_NextNeuronID;
	int m_NextInnovationID;

public:
	UInnovation();
	void Initialize(TArray<FSLinkGene> startLinks, TArray<FSNeuronGene> startNeurons);

	//Returns innovationID if it already exists, else returns -1
	int CheckForInnovation(int fromNeuron, int toNeuron, innovation_type type);

	//Returns the ID of the new innovation
	int CreateNewLinkInnovation(int fromNeuron, int toNeuron);

	//Returns the ID of the new innovation TODO
	//From and ToNeuron IDs of Input neurons are set to - 1 as another way to identify those
	int CreateNewNeuronInnovation(FSNeuronGene &neuronGene, int fromNeuron = -1, int toNeuron = -1);

	//Looks for the passed neuronID in the innovation list then creates and returns a copy of this neuron
	FSNeuronGene CreateNeuronFromID(int neuronID);



	int GetNeuronID(int innovationID) const { return m_Innovations[innovationID].NeuronID; }
	void Clear() { m_Innovations.Empty(); return; }
	int GetNextInnovationID() { return m_NextInnovationID; }
	int GetNextNeuronID() { return m_NextNeuronID; }
};
