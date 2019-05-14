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
#include "Phenotype.generated.h"


class ULink;


//The neuron structure class
UCLASS()
class NEATSHOOTER_API UNeuron : public UObject
{
	GENERATED_BODY()

	//public because architecture of UE4. Need access to modify object directly
public:
	UPROPERTY()
		TArray<ULink*> m_LinksIn;
	UPROPERTY()
		TArray<ULink*> m_LinksOut;

	UPROPERTY()
		int m_iNeuronID;
	UPROPERTY()
		double m_dActivationSum;
	UPROPERTY()
		double m_dOutput;

	UPROPERTY()
		TEnumAsByte<neuron_type> m_NeuronType;

	UPROPERTY()
		int m_iPosX;
	UPROPERTY()
		int m_iPosY;
	UPROPERTY()
		double m_dSplitX;
	UPROPERTY()
		double m_dSplitY;

	UNeuron() {}
	void Initialize(neuron_type type, int id, double splitX, double splitY);
};

//The link structure class
UCLASS()
class NEATSHOOTER_API ULink : public UObject
{
	GENERATED_BODY()

	//public because architecture of UE4. Need access to modify object directly
public:
	UPROPERTY()
		UNeuron* m_InputNeuron;
	UPROPERTY()
		UNeuron* m_OutputNeuron;

	UPROPERTY()
		double m_dLinkWeight;
	UPROPERTY()
		bool m_bIsRecurrent;

	ULink() {}
	void Initialize(UNeuron* input, UNeuron* output, double dWeight, bool bIsRec);
};

//The phenotype for our organisms
UCLASS()
class NEATSHOOTER_API UNeuralNet : public UObject
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
		TArray<UNeuron*> m_Neurons;
	UPROPERTY()
		int m_iDepth;

public:
	UNeuralNet();
	void Initialize(TArray<UNeuron*> neurons, int depth);

	//Ppdate network for this tick
	TArray<double> Update(TArray<double> &vInputs, run_type runType);
};
