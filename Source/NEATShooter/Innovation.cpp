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

#include "Innovation.h"




UInnovation::UInnovation()
{
}

void UInnovation::Initialize(TArray<FSLinkGene> startLinks, TArray<FSNeuronGene> startNeurons)
{
	m_NextNeuronID = 0;
	m_NextInnovationID = 0;

	//add the neurons
	for (FSNeuronGene curNeuron : startNeurons)
	{
		CreateNewNeuronInnovation(curNeuron);
	}

	//add the links
	for (FSLinkGene curLink : startLinks)
	{
		CreateNewLinkInnovation(curLink.FromNeuron, curLink.ToNeuron);
	}
}

int UInnovation::CheckForInnovation(int fromNeuron, int toNeuron, innovation_type type)
{
	for (FSInnovation curInnovation : m_Innovations)
	{
		if (curInnovation.InnovationType == type)
		{
			if (curInnovation.ToNeuron == toNeuron)
			{
				if (curInnovation.FromNeuron == fromNeuron)
				{
					return curInnovation.InnovationID;
				}
			}
		}
	}
	return -1;
}

int UInnovation::CreateNewLinkInnovation(int fromNeuron, int toNeuron)
{
	FSInnovation NewInnov = FSInnovation(fromNeuron, toNeuron, m_NextInnovationID);
	m_Innovations.Add(NewInnov);

	return m_NextInnovationID++;
}

int UInnovation::CreateNewNeuronInnovation(FSNeuronGene &newNeuronGene, int fromNeuron, int toNeuron)
{
	FSInnovation NewInnov = FSInnovation(newNeuronGene, m_NextInnovationID, fromNeuron, toNeuron);
	m_Innovations.Add(NewInnov);
	++m_NextNeuronID;

	return m_NextInnovationID++;
}

FSNeuronGene UInnovation::CreateNeuronFromID(int neuronID)
{
	FSNeuronGene NeuronGene = FSNeuronGene();

	for (FSInnovation curInnovation : m_Innovations)
	{
		if (curInnovation.InnovationType == new_neuron)
		{
			if (curInnovation.NeuronID == neuronID)
			{
				NeuronGene = FSNeuronGene(curInnovation.NeuronType, neuronID, curInnovation.dSplitX, curInnovation.dSplitY);
				return NeuronGene;
			}
		}
	}
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Innovation CreateNeuronFromID Couldn't create neuron from innovation list"));
	return NeuronGene;
}
