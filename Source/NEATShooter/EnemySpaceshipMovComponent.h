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
#include "CollidingPawnMovementComponent.h"
#include "EnemySpaceshipMovComponent.generated.h"


class AMyGameMode;


//Used by EnemySpaceship for movement each tick
UCLASS()
class NEATSHOOTER_API UEnemySpaceshipMovComponent : public UNavMovementComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY()
		UWorld* m_World;
	UPROPERTY()
		AMyGameMode* m_GameMode;
	UPROPERTY()
		FSpawnZone m_SpawnZone;

public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
		float m_VertSpeed = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
		float m_HorizSpeed = 400.0f;

	void SetVertSpeed(float VertSpeed);
	void SetHorizSpeed(float HorizSpeed);
};
