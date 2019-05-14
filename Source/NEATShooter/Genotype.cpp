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

#include "Genotype.h"
#include "Phenotype.h"
#include "Innovation.h"
#include "Parameters.h"
#include "Engine/World.h"
#include "MyGameMode.h"
#include <algorithm>



UGenome::UGenome()
{
	m_Phenotype = nullptr;
	m_GenomeID = 0;
	m_dFitness = 0;
	m_dSpeciesFitness = 0;
	m_iNumInputs = 0;
	m_iNumOutputs = 0;
	m_dSpawnAmount = 0;
}

void UGenome::InitializeStandard(int id, int numInputs, int numOutputs, AMyGameMode* gameMode)
{
	m_Phenotype = nullptr;
	m_GenomeID = id;
	m_dFitness = 0;
	m_dSpeciesFitness = 0;
	m_iNumInputs = numInputs;
	m_iNumOutputs = numOutputs;
	m_dSpawnAmount = 0;

	//determine grid size
	double InputRowSlice = 0.8 / double(numInputs);

	//create the input neurons with IDs from 0 to numInputs
	for (int i = 0; i < m_iNumInputs; i++)
	{
		m_Neurons.Add(FSNeuronGene(input, i, 0.1 + i * InputRowSlice, 0.0));
	}

	//create the bias with ID of numInputs
	m_Neurons.Add(FSNeuronGene(bias, m_iNumInputs, 1.0, 0.0));

	double OutputRowSlice = 1 / (double)(numOutputs + 1);

	//create the output neurons with ID of numInput +1 for the bias to numOutputs
	for (int i = 0; i < m_iNumOutputs; i++)
	{
		m_Neurons.Add(FSNeuronGene(output, i + m_iNumInputs + 1, (i + 1) * OutputRowSlice, 1.0));
	}

	//Used so that the links are listed after all nodes in the innovation list
	int iNextLinkNumber = 1;
	//create the link genes, connect each input neuron to each output neuron and assign a random weight -1 < w < 1
	//first iterate over all inputs +1 for the bias
	for (int i = 0; i < m_iNumInputs + 1; ++i)
	{
		for (int j = 0; j < m_iNumOutputs; ++j)
		{
			//toNeuron has +1 for the bias
			m_Links.Add(FSLinkGene(m_Neurons[i].iID, m_Neurons[m_iNumInputs + j + 1].iID, RandomClamped(), true, m_iNumInputs + m_iNumOutputs + 1 + iNextLinkNumber));
			++iNextLinkNumber;
		}
	}

	m_GameMode = gameMode;
}

void UGenome::InitializeCustom(int id, TArray<FSNeuronGene> neurons, TArray<FSLinkGene> genes, int nrInputs, int nrOutputs, AMyGameMode* gameMode)
{
	m_GenomeID = id;
	m_Phenotype = NULL;
	m_Links = genes;
	m_Neurons = neurons;
	m_dSpawnAmount = 0;
	m_dFitness = 0;
	m_dSpeciesFitness = 0;
	m_iNumInputs = nrInputs;
	m_iNumOutputs = nrOutputs;
	m_GameMode = gameMode;
}

void UGenome::InitializeWeights()
{
	for (FSLinkGene curLink : m_Links)
	{
		curLink.dWeight = RandFloat(-1.0, 1.0);
	}
}

void UGenome::ToggleLinkGenes(double toggleChance, int numTries)
{
	TArray<FSLinkGene>::TIterator checkgene = m_Links.CreateIterator();

	while (numTries > 0)
	{
		if (RandFloat() < toggleChance)
		{
			int RandomLinkGene = RandFloat(0, m_Links.Num());
			bool bGeneStatus = m_Links[RandomLinkGene].bEnabled;

			if (bGeneStatus == true)
			{
				//we need to make sure that another gene connects out of the in-node because if not a section of the network will break off and become isolated
				while (checkgene && (((checkgene->FromNeuron) != m_Links[RandomLinkGene].FromNeuron) || checkgene->bEnabled == false ||
					(checkgene->iInnovationID == m_Links[RandomLinkGene].iInnovationID)))
				{
					++checkgene;
				}
				if (!checkgene)
				{
					m_Links[RandomLinkGene].bEnabled = false;
				}
			}
			else if (bGeneStatus == false)
			{
				m_Links[RandomLinkGene].bEnabled = true;
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Genotype ToggleLinkGene error; LinkEnabled neiter true / false"));
			}
		}
		--numTries;
	}
}

void UGenome::ReenableLinkGenes(double enableChance)
{
	for (FSLinkGene &curLink : m_Links)
	{
		if (curLink.bEnabled == false)
		{
			if (RandFloat() < enableChance)
			{
				curLink.bEnabled = true;
			}
		}
	}
}

void UGenome::MutateWeights(double maxMutationPower, double mutationChance, double newWeightChance)
{
	for (FSLinkGene &curLink : m_Links)
	{
		if (RandFloat() < mutationChance)
		{
			if (RandFloat() < newWeightChance)
			{
				curLink.dWeight = RandomClamped();
			}
			else
			{
				double weight = curLink.dWeight;
				weight += RandFloat(-maxMutationPower, maxMutationPower);
				curLink.dWeight = weight;
			}
		}
	}
}

void UGenome::MutateAddNode(UInnovation & innovationList, double mutationChance, int numTries)
{
	if (RandFloat() > mutationChance)
	{
		return;
	}

	bool bFoundLink = false;

	FSLinkGene *SelectedLink = new FSLinkGene();

	//select random link to split numTries times
	while (numTries > 0)
	{

		int AlreadyTriedThisLink = -1;
		int RandLinkID = RandInt(0, m_Links.Num() - 1);

		//start new if we already tried with this link
		if (RandLinkID == AlreadyTriedThisLink)
		{
			continue;
		}

		*SelectedLink = m_Links[RandLinkID];

		//search new link if selected link is disabled or has a bias neuron as its input
		if ((SelectedLink->bEnabled == false) || (m_Neurons[GetNeuronPosFromID(SelectedLink->FromNeuron)].NeuronType == bias))
		{
			AlreadyTriedThisLink = RandLinkID;
			--numTries;
			continue;
		}

		numTries = 0;
		bFoundLink = true;
	}

	//couldn't find link in numTries
	if (bFoundLink == false)
	{
		return;
	}

	SelectedLink->bEnabled = false;

	//new link leading into the new node has its weight set to 1, the link leading out gets the old weight
	double NewLinkWeight = SelectedLink->dWeight;

	int FromNeuronID = SelectedLink->FromNeuron;
	int ToNeuronID = SelectedLink->ToNeuron;

	double NewDepth = (m_Neurons[GetNeuronPosFromID(FromNeuronID)].dSplitY + m_Neurons[GetNeuronPosFromID(ToNeuronID)].dSplitY) / 2;
	double NewWidth = (m_Neurons[GetNeuronPosFromID(FromNeuronID)].dSplitX + m_Neurons[GetNeuronPosFromID(ToNeuronID)].dSplitX) / 2;

	int InnovationID = innovationList.CheckForInnovation(FromNeuronID, ToNeuronID, new_neuron);

	//If innovation already exists check if the neuronID of that innovation is already used by the genome
	if (InnovationID >= 0)
	{
		int NeuronID = innovationList.GetNeuronID(InnovationID);

		if (GenomeAlreadyHasNeuronID(NeuronID))
		{
			//if yes we need a new innovation
			InnovationID = -1;
		}
	}

	if (InnovationID < 0)
	{
		//create new gene for the new neuron
		int NewNeuronID = innovationList.GetNextNeuronID();
		FSNeuronGene NewNeuronGene = FSNeuronGene(hidden, NewNeuronID, NewWidth, NewDepth);
		m_Neurons.Add(NewNeuronGene);

		//then register it in the innovation list
		innovationList.CreateNewNeuronInnovation(NewNeuronGene, FromNeuronID, ToNeuronID);

		//create new link1 with weight of 1
		int LinkOneID = innovationList.GetNextInnovationID();
		FSLinkGene NewLinkGeneOne = FSLinkGene(FromNeuronID, NewNeuronID, 1, true, LinkOneID);
		m_Links.Add(NewLinkGeneOne);
		innovationList.CreateNewLinkInnovation(FromNeuronID, NewNeuronID);

		//create new link2 with old weight
		int LinkTwoID = innovationList.GetNextInnovationID();
		FSLinkGene NewLinkGeneTwo = FSLinkGene(NewNeuronID, ToNeuronID, NewLinkWeight, true, LinkTwoID);
		m_Links.Add(NewLinkGeneTwo);
		innovationList.CreateNewLinkInnovation(NewNeuronID, ToNeuronID);
	}

	//the innovation already exists and its neuronID is not in use
	else
	{
		//create new gene for the new neuron
		int NewNeuronID = innovationList.GetNeuronID(InnovationID);
		FSNeuronGene NewNeuronGene = FSNeuronGene(hidden, NewNeuronID, NewWidth, NewDepth);
		m_Neurons.Add(NewNeuronGene);

		//since the neuron innovation already took place we should also have the 2 link innovations
		int LinkOneID = innovationList.CheckForInnovation(FromNeuronID, NewNeuronID, new_link);
		int LinkTwoID = innovationList.CheckForInnovation(NewNeuronID, ToNeuronID, new_link);

		if ((LinkOneID < 0) || (LinkTwoID < 0))
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Genotype MutateAddNeuron error"));
		}

		//create new link1 with old weight
		FSLinkGene NewLinkGene1 = FSLinkGene(FromNeuronID, NewNeuronID, NewLinkWeight, true, LinkOneID);
		m_Links.Add(NewLinkGene1);

		//create new link2 with weight of 1
		FSLinkGene NewLinkGene2 = FSLinkGene(NewNeuronID, ToNeuronID, 1, true, LinkTwoID);
		m_Links.Add(NewLinkGene2);
	}
}

void UGenome::MutateAddLink(UInnovation & innovationList, double mutationChance, int numTries)
{
	if (RandFloat() > mutationChance)
	{
		return;
	}

	//IDs of valid neurons we want to link are stored here
	int Neuron1ID = -1;
	int Neuron2ID = -1;

	//for a reccurent link
	bool bRecurrent = false;

	//try to find two neurons with no existing link
	while (numTries > 0)
	{
		//rand select first neuron
		Neuron1ID = m_Neurons[RandInt(0, m_Neurons.Num() - 1)].iID;

		//second neuron that's no an input or a bias
		Neuron2ID = m_Neurons[RandInt(m_iNumInputs + 1, m_Neurons.Num() - 1)].iID;

		//check if they are the same
		if (Neuron1ID == Neuron2ID)
		{
			Neuron1ID = -1;
			Neuron2ID = -1;
			--numTries;
			continue;
		}

		//check if there is a link between the two neurons
		if (DuplicateLink(Neuron1ID, Neuron2ID))
		{
			Neuron1ID = -1;
			Neuron2ID = -1;
			--numTries;
			continue;
		}

		numTries = 0;
	}

	//couldn't find a possible link
	if ((Neuron1ID < 0) || (Neuron2ID < 0))
	{
		return;
	}

	//check to see if we have already created this innovation
	int InnovationID = innovationList.CheckForInnovation(Neuron1ID, Neuron2ID, new_link);

	//check if this link is a recurrent link
	if (m_Neurons[GetNeuronPosFromID(Neuron1ID)].dSplitY > m_Neurons[GetNeuronPosFromID(Neuron2ID)].dSplitY)
	{
		bRecurrent = true;
	}

	if (InnovationID < 0)
	{
		//create new gene
		int NewInnovID = innovationList.GetNextInnovationID();
		FSLinkGene NewGene = FSLinkGene(Neuron1ID, Neuron2ID, RandomClamped(), true, NewInnovID, bRecurrent);
		m_Links.Add(NewGene);

		//then register it in the innovation list
		innovationList.CreateNewLinkInnovation(Neuron1ID, Neuron2ID);
	}

	else
	{
		//the innovation already exists, so we create the new gene with the existing innovation ID
		FSLinkGene NewGene = FSLinkGene(Neuron1ID, Neuron2ID, RandomClamped(), true, InnovationID, bRecurrent);
		m_Links.Add(NewGene);
	}
}

bool UGenome::GenomeAlreadyHasNeuronID(int neuronID)
{
	for (FSNeuronGene curNeuron : m_Neurons)
	{
		if (curNeuron.iID == neuronID)
		{
			return true;
		}
	}
	return false;
}

double UGenome::GetCompatibilityScore(UGenome* otherGenome)
{
	//count all disjointed, excess and matching genes
	double	NumDisjoint = 0;
	double	NumExcess = 0;
	double	NumMatching = 0;

	//total weight difference of matching genes
	double	WeightDifference = 0;

	int NumLinkGenesOfBigGenome = BiggerInt(m_Links.Num(), otherGenome->m_Links.Num());

	TArray<FSLinkGene>::TIterator CurrentGene1 = m_Links.CreateIterator();
	TArray<FSLinkGene>::TIterator CurrentGene2 = otherGenome->m_Links.CreateIterator();

	//continue until both iterators reached the end
	while (CurrentGene1 || CurrentGene2)
	{
		//if we reached the end of genome1 count the excess of genome2
		if (!CurrentGene1)
		{
			++CurrentGene2;
			++NumExcess;
			continue;
		}

		//if we reached the end of genome2 count the excess of genome1
		if (!CurrentGene2)
		{
			++CurrentGene1;
			++NumExcess;
			continue;
		}

		//get innovation IDs
		int InnovationID1 = CurrentGene1->iInnovationID;
		int InnovationID2 = CurrentGene2->iInnovationID;

		//matching gene found
		if (InnovationID1 == InnovationID2)
		{
			//calculate weight difference
			WeightDifference += fabs(CurrentGene1->dWeight - CurrentGene2->dWeight);

			++CurrentGene1;
			++CurrentGene2;
			++NumMatching;
			continue;
		}

		//disjoint genes
		if (InnovationID1 < InnovationID2)
		{
			++NumDisjoint;
			++CurrentGene1;
			continue;
		}

		if (InnovationID2 < InnovationID1)
		{
			++NumDisjoint;
			++CurrentGene2;
			continue;
		}
	}//end while

	 //calculate score
	double CompatibilityScore =
		(m_GameMode->GetParameters()->dExcessCoeff * (NumExcess / NumLinkGenesOfBigGenome)) +
		(m_GameMode->GetParameters()->dDisjointCoeff * (NumDisjoint / NumLinkGenesOfBigGenome)) +
		(m_GameMode->GetParameters()->dMatchingCoeff * (WeightDifference / NumMatching));

	return CompatibilityScore;
}

UNeuralNet* UGenome::CreatePhenotype()
{
	//make sure there is no existing phenotype for this genome
	DeletePhenotype();

	//all the neurons required for the phenotype
	TArray<UNeuron*> vNeurons;

	//create all the required neurons
	for (FSNeuronGene curNeuron : m_Neurons)
	{
		UNeuron* NewNeuron = NewObject<UNeuron>();
		NewNeuron->Initialize(curNeuron.NeuronType, curNeuron.iID, curNeuron.dSplitX, curNeuron.dSplitY);
		vNeurons.Add(NewNeuron);
	}

	//create the links. 
	for (FSLinkGene curLink : m_Links)
	{
		//make sure the link gene is enabled before the connection is created
		if (curLink.bEnabled)
		{
			//get the pointers to the relevant neurons
			int element = GetNeuronPosFromID(curLink.FromNeuron);
			UNeuron* FromNeuron = vNeurons[element];

			element = GetNeuronPosFromID(curLink.ToNeuron);
			UNeuron* ToNeuron = vNeurons[element];

			//create a link between those two neurons and assign the weight stored in the gene
			ULink* NewLink = NewObject<ULink>();
			NewLink->Initialize(FromNeuron, ToNeuron, curLink.dWeight, curLink.bRecurrent);

			//add new links to neuron
			FromNeuron->m_LinksOut.Add(NewLink);
			ToNeuron->m_LinksIn.Add(NewLink);
		}
	}

	//create neural net from all neurons
	m_Phenotype = NewObject<UNeuralNet>();
	m_Phenotype->Initialize(vNeurons, m_iDepth);

	return m_Phenotype;
}

bool UGenome::DuplicateLink(int NeuronIn, int NeuronOut)
{
	for (FSLinkGene curLink : m_Links)
	{
		if ((curLink.FromNeuron == NeuronIn) && (curLink.ToNeuron == NeuronOut))
		{
			//we already have this link
			return true;
		}
	}
	return false;
}

int UGenome::GetNeuronPosFromID(int neuronID)
{
	for (int i = 0; i < m_Neurons.Num(); ++i)
	{
		if (m_Neurons[i].iID == neuronID)
		{
			return i;
		}
	}
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Genotype GetNeuronPosFromID Can't find neuron ID in neuron list in genome"));

	return -1;
}

void UGenome::SortGenes()
{
	m_Links.Sort();
}

void UGenome::DeletePhenotype() 
{
	m_Phenotype = nullptr;
}

void UGenome::CalculateNetDepth(TArray<FSplitDepth> FSplitDepths)
{
	int MaxSoFar = 0;

	for (FSNeuronGene curNeuron : m_Neurons)
	{
		for (int j = 0; j < FSplitDepths.Num(); ++j)
		{
			if ((curNeuron.dSplitY == FSplitDepths[j].Value) && (FSplitDepths[j].Depth > MaxSoFar))
			{
				MaxSoFar = FSplitDepths[j].Depth;
			}
		}
	}

	//+2 because of input and output layer
	SetDepth(MaxSoFar + 2);
}
