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
#include "GameFramework/Pawn.h"
#include "NNSpaceShip.generated.h"


class UBoxComponent;
class AProjectile;
class UNeuralNet;
class UGenome;
class AMyGameMode;
enum run_type;


//The organism class of NEAT. Physical actor in the game world to play the game. Controlled by its neural net
UCLASS()
class NEATSHOOTER_API ANNSpaceShip : public APawn
{
	GENERATED_BODY()

private:
	UPROPERTY()
		UWorld* m_World;
	UPROPERTY()
		AMyGameMode* m_GameMode;

	UPROPERTY()
		UGenome* m_Genotype;
	UPROPERTY()
		UNeuralNet* m_NeuralNet;

	UPROPERTY()
		double m_dFitness;

	UPROPERTY()
		int m_iNumInputs;
	UPROPERTY()
		int m_iNumOutputs;

	UPROPERTY()
		TArray<double> m_InputsThisTick;
	UPROPERTY()
		TArray<double> m_OutputsThisTick;

	UPROPERTY()
		//located here if not playing
		FTransform m_StandbyPosition;

	UPROPERTY()
		float m_fShotCooldown;
	UPROPERTY()
		int m_iHealth;
	UPROPERTY()
		int m_iMaxHealth;

	//fitness for moving enough distance is handled here. Feature: Let the game mode handle this
	UPROPERTY()
		float m_fNetDistanceMoved;
	UPROPERTY()
		float m_fNetDistanceRequired;
	UPROPERTY()
		float m_fDistanceReward;
	UPROPERTY()
		float m_fLastTickYPosition;

	//Moves the ship left/right
	void MoveRight(float AxisValue);
	//Fires projectile from muzzle
	void OnFire();

	//Calculates an input for the NN from its shot cooldown scaled in [0;1]; 0 means max cooldown; 1 means it is able to shoot
	float CalculateFireRateInput();
	//Used so the NN can know how much Health it has left scaled in [0;1]; 0 means next hit is death; 1 represents full life
	float CalculateLifeInput();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	ANNSpaceShip();
	ANNSpaceShip(const FObjectInitializer& ObjectInitializer);

	virtual UPawnMovementComponent* GetMovementComponent() const override;
	
	//components for hitbox etc
	UPROPERTY(EditAnywhere)
		UBoxComponent *CollisionComponent;
	UPROPERTY(EditAnywhere)
		UStaticMeshComponent *ShipVisual;
	UPROPERTY(EditAnywhere)
		class UCollidingPawnMovementComponent* OurMovementComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		FVector MuzzleOffset;

	//blueprint binding for editor
	UPROPERTY(EditAnywhere, Category = Projectile)
		TSubclassOf<class AProjectile> ProjectileClass;

	void Reset();
	void MoveShipToStandby();
	void MoveShipToStart(FVector startLocation);
	void AwardFitness(float fitness);

	//Checks if ship moved enough to get reward and awards fitness accordingly
	void MovementReward();

	UFUNCTION()
		//Handles hit events
		void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	//Called every Tick, calculates outputs of the net and executes an action (shoot, move left / right)
	bool Update(const TArray<double>& vInputs, run_type runType, float deltaTime);



	double GetFitness()const { return m_dFitness; }
	int GetCurrentHealth() { return m_iHealth; }
	void AssignNeuralNet(UNeuralNet* neuralNet) { m_NeuralNet = neuralNet; }
	void AssignGenotype(UGenome* genotype) { m_Genotype = genotype; }
	UGenome* GetGenotype() { return m_Genotype; }
};
