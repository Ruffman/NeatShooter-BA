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

#include "EnemySpaceshipMovComponent.h"
#include "Engine/World.h"
#include "MyGameMode.h"



void UEnemySpaceshipMovComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Make sure that everything is still valid, and that we are allowed to move.
	if (!UpdatedComponent || ShouldSkipUpdate(DeltaTime))
	{
		return;
	}

	FVector CurrentLocation = this->GetActorLocation();
	float LeftY = m_SpawnZone.m_LeftY + 70.f;
	float RightY = m_SpawnZone.m_RightY - 70.f;

	if ((CurrentLocation.Y < LeftY) && (m_HorizSpeed < 0.0f))
	{
		m_HorizSpeed = -m_HorizSpeed;
	}
	else if ((CurrentLocation.Y > RightY) && (m_HorizSpeed > 0.0f))
	{
		m_HorizSpeed = -m_HorizSpeed;
	}

	FVector DesiredMovementThisFrame = FVector(-m_VertSpeed, m_HorizSpeed, 0.0f) * DeltaTime;
	if (!DesiredMovementThisFrame.IsNearlyZero())
	{
		FHitResult Hit;
		SafeMoveUpdatedComponent(DesiredMovementThisFrame, UpdatedComponent->GetComponentRotation(), true, Hit);
	}
}

void UEnemySpaceshipMovComponent::BeginPlay()
{
	Super::BeginPlay();

	m_World = GetWorld();
	m_GameMode = Cast<AMyGameMode>(m_World->GetAuthGameMode());
	if (!m_GameMode)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Couldn't find GameMode in EnemyShipMovComp"));
	}

	m_SpawnZone = m_GameMode->GetSpawnZone();
}

void UEnemySpaceshipMovComponent::SetVertSpeed(float VertSpeed)
{
	m_VertSpeed = VertSpeed;
}

void UEnemySpaceshipMovComponent::SetHorizSpeed(float HorizSpeed)
{
	m_HorizSpeed = HorizSpeed;
}
