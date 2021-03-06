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
#include "Destructible.generated.h"


class UBoxComponent;
class AMyGameMode;


//Spawnable enemy
UCLASS()
class NEATSHOOTER_API ADestructible : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADestructible();
	ADestructible(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY()
		AMyGameMode* m_GameMode;
	UPROPERTY()
		float m_fFitnessForKill;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
		UBoxComponent *CollisionComponent;

	UPROPERTY(EditAnywhere)
		UStaticMeshComponent *DestructibleVisual;

	UPROPERTY(EditAnywhere)
		class UDestructibleMovementComponent* OurMovementComponent;

	UFUNCTION()
		void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UDestructibleMovementComponent* GetMovementComponent();
};
