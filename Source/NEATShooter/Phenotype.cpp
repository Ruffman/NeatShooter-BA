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

#include "Phenotype.h"




UNeuralNet::UNeuralNet()
{
	m_Neurons.Empty();
	m_iDepth = 0;
}

void UNeuralNet::Initialize(TArray<UNeuron*> neurons, int depth)
{
	m_Neurons = neurons;
	m_iDepth = depth;
}

TArray<double> UNeuralNet::Update(TArray<double>& vInputs, run_type runType)
{
	TArray<double> outputs;

	int IterationCount = 0;

	if (runType == snapshot)
	{
		IterationCount = m_iDepth;
	}
	else
	{
		IterationCount = 1;
	}

	for (int i = 0; i < IterationCount; ++i)
	{
		outputs.Empty();
		int CurrentNeuron = 0;

		//set output of input-neurons to inputs from the input list
		for (int i = CurrentNeuron; i < m_Neurons.Num(); ++i)
		{
			if (m_Neurons[i]->m_NeuronType == input)
			{
				m_Neurons[i]->m_dOutput = vInputs[CurrentNeuron];
				++CurrentNeuron;
			}

			//set output of bias neuron
			if (m_Neurons[i]->m_NeuronType == bias)
			{
				m_Neurons[i]->m_dOutput = 1;
				++CurrentNeuron;
			}
		}

		//now outputs and hidden neurons are calculated
		while (CurrentNeuron < m_Neurons.Num())
		{
			double sum = 0;
			//calculate sum by going through all incomming links
			for (int i = 0; i < m_Neurons[CurrentNeuron]->m_LinksIn.Num(); ++i)
			{
				double LinkWeight = m_Neurons[CurrentNeuron]->m_LinksIn[i]->m_dLinkWeight;
				double NeuronOutput = m_Neurons[CurrentNeuron]->m_LinksIn[i]->m_InputNeuron->m_dOutput;
				sum += LinkWeight * NeuronOutput;
			}

			//assign outputs
			m_Neurons[CurrentNeuron]->m_dOutput = Sigmoid(sum);

			if (m_Neurons[CurrentNeuron]->m_NeuronType == output)
			{
				outputs.Add(m_Neurons[CurrentNeuron]->m_dOutput);
			}

			++CurrentNeuron;
		}
	}//next iteration through network


	 //the network needs to be flushed if this type of update is performed otherwise
	 //it is possible for dependencies to be built on the order the training data is
	 //presented
	if (runType == snapshot)
	{
		for (UNeuron* curNeuron : m_Neurons)
		{
			curNeuron->m_dOutput = 0;
		}
	}

	return outputs;
}

void ULink::Initialize(UNeuron * input, UNeuron * output, double dWeight, bool bIsRec)
{
	m_InputNeuron = input;
	m_OutputNeuron = output;
	m_dLinkWeight = dWeight;
	m_bIsRecurrent = bIsRec;
}

void UNeuron::Initialize(neuron_type type, int id, double splitX, double splitY)
{
	m_NeuronType = type;
	m_iNeuronID = id;
	m_dActivationSum = 0;
	m_dOutput = 0;
	m_iPosX = 0;
	m_iPosY = 0;
	m_dSplitX = splitX;
	m_dSplitY = splitY;
}
