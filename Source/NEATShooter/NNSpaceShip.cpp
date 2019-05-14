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

#include "NNSpaceShip.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "CollidingPawnMovementComponent.h"
#include "Projectile.h"
#include "Engine/World.h"
#include "EnemySpaceship.h"
#include "Destructible.h"
#include "Parameters.h"
#include "Genotype.h"
#include "Phenotype.h"
#include "MyGameMode.h"


//sets default values
ANNSpaceShip::ANNSpaceShip() 
{

}

ANNSpaceShip::ANNSpaceShip(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
 	//turned off because we have our own tick function (Update)
	PrimaryActorTick.bCanEverTick = false;

	//our hit box
	CollisionComponent = ObjectInitializer.CreateDefaultSubobject<UBoxComponent>(this, TEXT("RootCollision"));
	RootComponent = CollisionComponent;
	CollisionComponent->InitBoxExtent(FVector(80.f,50.f,20.f));
	CollisionComponent->SetCollisionProfileName(TEXT("Pawn"));
	CollisionComponent->OnComponentHit.AddDynamic(this, &ANNSpaceShip::OnHit);

	//visual for the ship
	ShipVisual = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("VisualRepresentation"));
	ShipVisual->SetupAttachment(RootComponent);
	ShipVisual->AddLocalRotation(FRotator(-90.0f, 0.0f, 0.0f));

	//create an instance of our movement component, and tell it to update the root
	OurMovementComponent = CreateDefaultSubobject<UCollidingPawnMovementComponent>(TEXT("CustomMovementComponent"));
	OurMovementComponent->UpdatedComponent = RootComponent;

	m_Genotype = nullptr;
	m_NeuralNet = nullptr;
	m_dFitness = 0.0;
	m_fNetDistanceMoved = 0.f;
}

//called when the game starts or when spawned
void ANNSpaceShip::BeginPlay()
{
	Super::BeginPlay();

	m_World = GetWorld();
	m_GameMode = Cast<AMyGameMode>(m_World->GetAuthGameMode());

	m_iNumInputs = m_GameMode->GetParameters()->iNumInputs;
	m_iNumOutputs = m_GameMode->GetParameters()->iNumOutputs;
	m_fNetDistanceRequired = m_GameMode->GetParameters()->fNetMovementRequired;
	m_fDistanceReward = m_GameMode->GetParameters()->fMovementReward;
	m_iHealth = m_GameMode->GetParameters()->iShipHealth;
	m_iMaxHealth = m_GameMode->GetParameters()->iShipHealth;
	m_fShotCooldown = m_GameMode->GetParameters()->fTimeBetweenShots;
	m_StandbyPosition = this->GetTransform();
	m_fLastTickYPosition = GetActorLocation().Y;
}

bool ANNSpaceShip::Update(const TArray<double>& vInputs, run_type runType, float deltaTime)
{
	m_InputsThisTick = vInputs;
	//add the life and fire rate inputs
	m_InputsThisTick.Add(CalculateFireRateInput());
	m_InputsThisTick.Add(CalculateLifeInput());

	int NumInputs = m_InputsThisTick.Num();

	if (NumInputs < m_iNumInputs || NumInputs > m_iNumInputs)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Inputs don't match Parameters!")));
	}

	m_OutputsThisTick = m_NeuralNet->Update(m_InputsThisTick, runType);

	if (m_OutputsThisTick.Num() < m_iNumOutputs)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Not enough outputs!"));
		return false;
	}

	//after outputs from the nn are computed above
	double MoveL, MoveR, Shoot;
	MoveL = m_OutputsThisTick[0];
	MoveR = m_OutputsThisTick[1];
	Shoot = m_OutputsThisTick[2];

	//use the highest output this frame
	if (MoveL > Shoot && MoveL > MoveR)
	{
		MoveRight(-1.f);
	}
	else if (MoveR > Shoot && MoveR > MoveL)
	{
		MoveRight(1.f);
	}
	else if (Shoot > MoveR && Shoot > MoveL)
	{
		OnFire();
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("No action performed!"));
	}

	m_fShotCooldown -= deltaTime;
	if (m_fShotCooldown < 0.f)
	{
		m_fShotCooldown = 0.f;
	}

	return true;
}

void ANNSpaceShip::Reset()
{
	m_dFitness = 0.0;
	m_iHealth = m_iMaxHealth;
	m_fShotCooldown = 0.f;
}

void ANNSpaceShip::MoveShipToStandby()
{
	this->SetActorTransform(m_StandbyPosition);
}

void ANNSpaceShip::MoveShipToStart(FVector startLocation)
{
	this->SetActorLocation(startLocation);
}

void ANNSpaceShip::AwardFitness(float fitness)
{
	m_dFitness += double(fitness);
}

float ANNSpaceShip::CalculateFireRateInput()
{
	double FireRateInput = 1 - m_fShotCooldown / m_GameMode->GetParameters()->fTimeBetweenShots;
	return FireRateInput;
}

float ANNSpaceShip::CalculateLifeInput()
{
	int MaxHealth = m_iMaxHealth;
	double LifeInput;

	//next hit means death
	if (m_iHealth == 1)
	{
		LifeInput = 0.f;
	}
	else
	{
		LifeInput = m_iHealth / MaxHealth;
	}
	return LifeInput;
}

UPawnMovementComponent* ANNSpaceShip::GetMovementComponent() const
{
	return OurMovementComponent;
}

void ANNSpaceShip::MoveRight(float AxisValue)
{
	//move through the component
	if (OurMovementComponent && (OurMovementComponent->UpdatedComponent == RootComponent))
	{
		OurMovementComponent->AddInputVector(GetActorRightVector() * AxisValue);
	}
}

void ANNSpaceShip::MovementReward()
{
	//current position
	float NewYPos = GetActorLocation().Y;
	//did ship move?
	float DistanceMovedThisFrame = NewYPos - m_fLastTickYPosition;
	//add it to total move distance, sign doesn't matter since all we want is fixed distance in any direction from the last time this was called
	m_fNetDistanceMoved += DistanceMovedThisFrame;
	//next time this function is called is in the next tick
	m_fLastTickYPosition = NewYPos;

	//we above required distance?
	if (FMath::Abs(m_fNetDistanceMoved) >= m_fNetDistanceRequired)
	{
		AwardFitness(m_fDistanceReward);
		m_fNetDistanceMoved = 0.f;
	}
}

void ANNSpaceShip::OnFire()
{
	if (m_fShotCooldown <= 0.f)
	{
		if (this != nullptr)
		{
			if (ProjectileClass != nullptr)
			{
				if (m_World)
				{
					//spawn the projectile at the muzzle
					FVector ShipLocation = this->GetActorLocation();
					FRotator ShipRotation = this->GetActorRotation();
					ShipLocation.AddBounded(MuzzleOffset);
					AProjectile* Projectile = m_World->SpawnActor<AProjectile>(ProjectileClass, ShipLocation, ShipRotation);
					if (Projectile)
					{
						//find launch direction
						FVector LaunchDir = ShipRotation.Vector();
						Projectile->InitVelocity(LaunchDir);
						m_fShotCooldown = m_GameMode->GetParameters()->fTimeBetweenShots;

						//shooting costs fitness
						m_dFitness -= m_GameMode->GetParameters()->fFitnessPerShot;
					}
				}
			}
		}
	}
}

void ANNSpaceShip::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
	{
		//if any of those hit us we are penalized
		if (Cast<AProjectile>(OtherActor) || Cast<AEnemySpaceship>(OtherActor) || Cast<ADestructible>(OtherActor))
		{
			m_iHealth--;
			m_dFitness -= m_GameMode->GetParameters()->fFitnessPenaltyOnHit;

			OtherActor->Destroy();
		}
	}
}
