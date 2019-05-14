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
#include "GameFramework/Actor.h"
#include "NNInput.generated.h"


class AMyGameMode;


//Provides the input for the play area and the positional input of the currently playing organism
UCLASS()
class NEATSHOOTER_API UNNInput : public UObject
{
	GENERATED_BODY()

private:
	//2D map coordinate representation
	TArray<TArray<double>> m_InputsForTheNN;

	int m_iNumberOfRows;
	int m_iNumberOfLines;
	int m_iNumberOfInputs;

	float m_fDestValue;
	float m_fEnemyValue;
	float m_fProjValue;

	//calculated from the Nr of rows and lines
	double m_dInputCellWidth;
	double m_dInputCellHeight;

	//Reset the input array to 0.0
	void ResetInputs();

public:	
	UNNInput();
	void Initialize(AMyGameMode* gameMode);

	//dimensions of the play area. Feature: Don't hardcode, set values with object from editor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayArea)
		float m_InputAreaLeftY = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayArea)
		float m_InputAreaRightY = 2000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayArea)
		float m_InputAreaTopX = 2000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayArea)
		float m_InputAreaBottomX = 0.f;

	//Calculates an input array for the organism out of all enemy actor positions and it's current position
	TArray<double> CalculateInputsThisTick(float currentPlayerYValue);

	//Returns true if given location is inside the play area
	bool LocationInInputArea(FVector location);
};
