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

#include "EnemySpaceship.h"
#include "EnemySpaceshipMovComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Projectile.h"
#include "MyGameMode.h"
#include "Parameters.h"


// Sets default values
AEnemySpaceship::AEnemySpaceship()
{
}

AEnemySpaceship::AEnemySpaceship(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Our root component will be a sphere that reacts to physics
	CollisionComponent = ObjectInitializer.CreateDefaultSubobject<UBoxComponent>(this, TEXT("RootCollision"));
	RootComponent = CollisionComponent;
	CollisionComponent->InitBoxExtent(FVector(80.f, 50.f, 20.f));
	CollisionComponent->SetCollisionProfileName(TEXT("Pawn"));
	CollisionComponent->OnComponentHit.AddDynamic(this, &AEnemySpaceship::OnHit);

	// Create and position a mesh component so we can see where our sphere is
	ShipVisual = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("VisualRepresentation"));
	ShipVisual->SetupAttachment(RootComponent);
	ShipVisual->AddLocalRotation(FRotator(90.0f, 0.0f, 0.0f));

	// Create an instance of our movement component, and tell it to update the root.
	OurMovementComponent = CreateDefaultSubobject<UEnemySpaceshipMovComponent>(TEXT("CustomMovementComponent"));
	OurMovementComponent->UpdatedComponent = RootComponent;

	this->SetActorLocation(FVector(0.0f, 0.0f, 100.0f));
	m_TimeTillNextShot = FireRate;
}


// Called when the game starts or when spawned
void AEnemySpaceship::BeginPlay()
{
	Super::BeginPlay();
	
	m_GameMode = Cast<AMyGameMode>(GetWorld()->GetAuthGameMode());
	m_fFitnessForKill = m_GameMode->GetParameters()->fFitnessRewardEnemy;
}

// Called every frame
void AEnemySpaceship::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	m_TimeTillNextShot -= DeltaTime;

	if (m_TimeTillNextShot < 0.0f)
	{
		OnFire();
		m_TimeTillNextShot = FireRate;
	}
}

void AEnemySpaceship::OnFire()
{
	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		UWorld* const World = GetWorld();
		if (World)
		{
			// spawn the projectile at the muzzle
			FVector ShipLocation = this->GetActorLocation();
			FRotator ShipRotation = this->GetActorRotation();
			ShipRotation.Add(-180.0f, 0.0f, 0.0f);
			ShipLocation.AddBounded(MuzzleOffset);
			AProjectile* Projectile = World->SpawnActor<AProjectile>(ProjectileClass, ShipLocation, ShipRotation);
			if (Projectile)
			{
				// find launch direction
				FVector LaunchDir = ShipRotation.Vector();
				Projectile->InitVelocity(LaunchDir);
			}
		}
	}
}

void AEnemySpaceship::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
	{
		if (Cast<AProjectile>(OtherActor))
		{
			Destroy();
			OtherActor->Destroy();
			m_GameMode->AwardFitnessToCurrentPlayer(m_fFitnessForKill);
		}
	}
}
