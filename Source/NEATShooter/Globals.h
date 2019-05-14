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


#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Globals.generated.h"


//Class for functions, structs and enums that are used by different classes. Sigmoid and helper functions are found here
UCLASS()
class NEATSHOOTER_API AGlobals : public AInfo
{
	GENERATED_BODY()

public:

	AGlobals() {}
};

UENUM()
enum neuron_type
{
	bias,
	input,
	hidden,
	output,
	noType
};

//you have to select one of these types when updating the network
//if snapshot is chosen the network depth is used to completely
//flush the inputs through the network. active just updates the
//network each timestep
UENUM()
enum run_type 
{ 
	snapshot, 
	active 
};

//for futer use of printing genomes to file
template<typename T>
static FString EnumToString(const FString& enumName, const T value)
{
	UEnum* pEnum = FindObject<UEnum>(ANY_PACKAGE, *enumName);
	return *(pEnum ? pEnum->GetNameStringByIndex(static_cast<uint8>(value)) : "null");
}

FORCEINLINE const char * const BoolToString(bool b)
{
	return b ? "true" : "false";
}

FORCEINLINE float Sigmoid(float input)
{
	return 1 / (1 + exp(-4.9 * input));
}

//rounds a double up or down depending on its value
FORCEINLINE int Round(double value)
{
	int integral = (int)value;
	double mantissa = value - integral;

	if (mantissa < 0.5)
	{
		return integral;
	}

	else
	{
		return integral + 1;
	}
}

//struct to calculate net depth
USTRUCT()
struct FSplitDepth
{
	GENERATED_BODY()

	UPROPERTY()
		double Value;
	UPROPERTY()
		int Depth;

	FSplitDepth() { Value = 0.0, Depth = -1; }
	FSplitDepth(double value, int depth) :Value(value), Depth(depth) {}
};

//not used anymore
USTRUCT()
struct FSpawnZone
{
	GENERATED_BODY()

	UPROPERTY()
		float m_UpX;
	UPROPERTY()
		float m_DownX;
	UPROPERTY()
		float m_LeftY;
	UPROPERTY()
		float m_RightY;

	FSpawnZone() : m_UpX(0.f), m_DownX(0.f), m_LeftY(0.f), m_RightY(0.f) {}
	FSpawnZone(float UpX, float DownX, float LeftY, float RightY) : m_UpX(UpX), m_DownX(DownX), m_LeftY(LeftY), m_RightY(RightY) {}
};

//returns a random integer between x and y
FORCEINLINE int RandInt(int x, int y) { return rand() % (y - x + 1) + x; }

//returns a random float in [0;1[ or [min, max[
FORCEINLINE double RandFloat(float min = 0, float max = 1)
{
	if (min > max)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Globals.h RandFloat() min > max"));
	}
	float random = ((float)rand()) / ((float)RAND_MAX + 1.0f);
	float range = max - min;
	return (random * range) + min;
}

//returns the bigger number
FORCEINLINE int BiggerInt(int var1, int var2)
{
	if (var1 < var2)
	{
		return var2;
	}
	else if (var1 > var2)
	{
		return var1;
	}
	else if (var1 == var2)
	{
		return var1;
	}
	else
	{
		return -1;
	}
}

//returns a random bool
FORCEINLINE bool RandBool()
{
	int x = RandInt(1, 10);
	if (x <= 5)
		return true;

	else
		return false;
}

//returns a random float in the range -1 < n < 1
FORCEINLINE double RandomClamped() { return RandFloat() - RandFloat(); }
