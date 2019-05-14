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

#include "Destructible.h"
#include "DestructibleMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Projectile.h"
#include "MyGameMode.h"
#include "Parameters.h"


// Sets default values
ADestructible::ADestructible()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

ADestructible::ADestructible(const FObjectInitializer & ObjectInitializer) : Super(ObjectInitializer)
{
	CollisionComponent = ObjectInitializer.CreateDefaultSubobject<UBoxComponent>(this, TEXT("CollComponent"));
	RootComponent = CollisionComponent;
	CollisionComponent->InitBoxExtent(FVector(80.f, 80.f, 20.f));
	CollisionComponent->SetCollisionProfileName(TEXT("Destructible"));
	CollisionComponent->OnComponentHit.AddDynamic(this, &ADestructible::OnHit);

	DestructibleVisual = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("DestructVisual"));
	DestructibleVisual->SetupAttachment(RootComponent);

	OurMovementComponent = ObjectInitializer.CreateDefaultSubobject<UDestructibleMovementComponent>(this, TEXT("MovementComponent"));
	OurMovementComponent->UpdatedComponent = RootComponent;

	this->SetActorLocation(FVector(0.0f, 0.0f, 100.0f));
}

// Called when the game starts or when spawned
void ADestructible::BeginPlay()
{
	Super::BeginPlay();
	
	m_GameMode = Cast<AMyGameMode>(GetWorld()->GetAuthGameMode());
	m_fFitnessForKill = m_GameMode->GetParameters()->fFitnessRewardDestructible;
}

// Called every frame
void ADestructible::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

UDestructibleMovementComponent* ADestructible::GetMovementComponent()
{
	return OurMovementComponent;
}

void ADestructible::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
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
