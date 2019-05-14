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

#include "NNInput.h"
#include "Projectile.h"
#include "Destructible.h"
#include "EnemySpaceship.h"
#include "UObjectIterator.h"
#include "math.h"
#include "MyGameMode.h"
#include "Parameters.h"


UNNInput::UNNInput()
{
}

void UNNInput::Initialize(AMyGameMode* gameMode)
{
	m_iNumberOfRows = gameMode->GetParameters()->iNumInputRows;
	m_iNumberOfLines = gameMode->GetParameters()->iNumInputLines;

	//calculate cell dimensions
	m_iNumberOfInputs = m_iNumberOfRows * m_iNumberOfLines;
	m_dInputCellHeight = (m_InputAreaTopX - m_InputAreaBottomX) / m_iNumberOfLines;
	m_dInputCellWidth = (m_InputAreaRightY - m_InputAreaLeftY) / m_iNumberOfRows;

	//create the 2D array
	for (int i = 0; i < m_iNumberOfLines; ++i)
	{
		TArray<double> Lines;
		for (int j = 0; j < m_iNumberOfRows; j++)
		{
			Lines.Add(0.0);
		}
		m_InputsForTheNN.Add(Lines);
	}

	m_fDestValue = gameMode->GetParameters()->fDestValue;
	m_fEnemyValue = gameMode->GetParameters()->fEnemyValue;
	m_fProjValue = gameMode->GetParameters()->fProjValue;
}

TArray<double> UNNInput::CalculateInputsThisTick(float currentPlayerYValue)
{
	ResetInputs();

	for (TObjectIterator<ADestructible> DestructIter; DestructIter; ++DestructIter)
	{
		if (DestructIter->ActorHasTag("Destructible"))
		{
			FVector CurrentLocation = DestructIter->GetActorLocation();
			if (LocationInInputArea(CurrentLocation))
			{
				int X = FMath::TruncToInt(CurrentLocation.X / m_dInputCellHeight);
				int Y = FMath::TruncToInt(CurrentLocation.Y / m_dInputCellWidth);

				m_InputsForTheNN[X][Y] = m_fDestValue;
			}
		}
	}

	for (TObjectIterator<AEnemySpaceship> EnemyIter; EnemyIter; ++EnemyIter)
	{
		if (EnemyIter->ActorHasTag("EnemySpaceship"))
		{
			FVector CurrentLocation = EnemyIter->GetActorLocation();
			if (LocationInInputArea(CurrentLocation))
			{
				int X = FMath::TruncToInt(CurrentLocation.X / m_dInputCellHeight);
				int Y = FMath::TruncToInt(CurrentLocation.Y / m_dInputCellWidth);

				m_InputsForTheNN[X][Y] = m_fEnemyValue;
			}
		}
	}

	for (TObjectIterator<AProjectile> ProjIter; ProjIter; ++ProjIter)
	{
		if (ProjIter->ActorHasTag("EnemyProjectile"))
		{
			FVector CurrentLocation = ProjIter->GetActorLocation();
			if (LocationInInputArea(CurrentLocation))
			{
				int X = FMath::TruncToInt(CurrentLocation.X / m_dInputCellHeight);
				int Y = FMath::TruncToInt(CurrentLocation.Y / m_dInputCellWidth);

				m_InputsForTheNN[X][Y] = m_fProjValue;
			}
		}
	}

	//convert the vector representing cells into a one dimensional so the NN can use it
	TArray<double> InputsIn1D;
	for (int i = 0; i < m_iNumberOfLines; ++i)
	{
		for (int j = 0; j < m_iNumberOfRows; j++)
		{
			InputsIn1D.Add(m_InputsForTheNN[i][j]);
		}
	}

	//where the ship is positioned on the input area in [0;1]
	double InputAreaSpan = FMath::Abs(m_InputAreaLeftY - m_InputAreaRightY);
	double RelativePlayerLocation = FMath::Abs(m_InputAreaLeftY - currentPlayerYValue);
	double LocationInput = RelativePlayerLocation / InputAreaSpan;
	InputsIn1D.Add(LocationInput);

	return InputsIn1D;
}

bool UNNInput::LocationInInputArea(FVector location)
{
	float X = location.X;
	float Y = location.Y;
	if (X < m_InputAreaBottomX || X > m_InputAreaTopX)
	{
		return false;
	}
	if (Y < m_InputAreaLeftY || Y > m_InputAreaRightY)
	{
		return false;
	}
	return true;
}

void UNNInput::ResetInputs()
{
	for (int i = 0; i < m_iNumberOfLines; ++i)
	{
		for (int j = 0; j < m_iNumberOfRows; j++)
		{
			m_InputsForTheNN[i][j] = 0.0;
		}
	}
}
