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
#include "GameFramework/PawnMovementComponent.h"
#include "DestructibleMovementComponent.generated.h"


//Used by Destructible for movement each tick
UCLASS()
class NEATSHOOTER_API UDestructibleMovementComponent : public UNavMovementComponent
{
	GENERATED_BODY()

public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
		float m_VertSpeed = 400.0f;

	void SetVertSpeed(float VertSpeed);
};
