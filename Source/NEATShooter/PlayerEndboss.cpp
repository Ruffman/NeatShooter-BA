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

#include "PlayerEndboss.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "CollidingPawnMovementComponent.h"
#include "Projectile.h"
#include "Engine/World.h"
#include "EnemySpaceship.h"
#include "Destructible.h"
#include "Components/InputComponent.h"
#include "MyGameMode.h"




APlayerEndboss::APlayerEndboss(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Set this pawn to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;

	// Our root component will be a sphere that reacts to physics
	CollisionComponent = ObjectInitializer.CreateDefaultSubobject<UBoxComponent>(this, TEXT("RootCollision"));
	RootComponent = CollisionComponent;
	CollisionComponent->InitBoxExtent(FVector(80.f, 120.f, 20.f));
	CollisionComponent->SetCollisionProfileName(TEXT("Pawn"));
	CollisionComponent->OnComponentHit.AddDynamic(this, &APlayerEndboss::OnHit);

	// Create and position a mesh component
	VisualComponent = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("VisualRepresentation"));
	VisualComponent->SetupAttachment(RootComponent);
	VisualComponent->AddLocalRotation(FRotator(90.0f, 0.0f, 0.0f));

	// Create an instance of our movement component, and tell it to update the root.
	OurMovementComponent = CreateDefaultSubobject<UCollidingPawnMovementComponent>(TEXT("CustomMovementComponent"));
	OurMovementComponent->UpdatedComponent = RootComponent;
}

// Called when the game starts or when spawned
void APlayerEndboss::BeginPlay()
{
	Super::BeginPlay();

	m_World = GetWorld();
	m_GameMode = Cast<AMyGameMode>(m_World->GetAuthGameMode());
	if (!m_GameMode)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Couldn't find GameMode in PlayerEndboss"));
	}

	m_iHealth = Health;
	m_fShotCooldown = 0.f;
	m_StandbyPosition = this->GetTransform();
}

// Called every frame
void APlayerEndboss::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	m_fShotCooldown -= DeltaTime;
	if (m_fShotCooldown < 0.f)
	{
		m_fShotCooldown = 0.f;
	}
}

// Called to bind functionality to input
void APlayerEndboss::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	InputComponent->BindAction("AccelerateGame", IE_Pressed, this, &APlayerEndboss::AccelerateGame);
	InputComponent->BindAction("ResetGameSpeed", IE_Pressed, this, &APlayerEndboss::ResetGameSpeed);
	InputComponent->BindAction("FastForward", IE_Pressed, this, &APlayerEndboss::FastForward);
	InputComponent->BindAction("ReturnToTraining", IE_Pressed, this, &APlayerEndboss::ReturnToTraining);

	InputComponent->BindAction("PlayerVsShip1", IE_Pressed, this, &APlayerEndboss::PlayVsShip1);
	InputComponent->BindAction("PlayerVsShip2", IE_Pressed, this, &APlayerEndboss::PlayVsShip2);
	InputComponent->BindAction("PlayerVsShip3", IE_Pressed, this, &APlayerEndboss::PlayVsShip3);
	InputComponent->BindAction("PlayerVsShip4", IE_Pressed, this, &APlayerEndboss::PlayVsShip4);
	InputComponent->BindAction("PlayerVsShip5", IE_Pressed, this, &APlayerEndboss::PlayVsShip5);

	InputComponent->BindAction("PlayShip1", IE_Pressed, this, &APlayerEndboss::LetShip1Play);
	InputComponent->BindAction("PlayShip2", IE_Pressed, this, &APlayerEndboss::LetShip2Play);
	InputComponent->BindAction("PlayShip3", IE_Pressed, this, &APlayerEndboss::LetShip3Play);
	InputComponent->BindAction("PlayShip4", IE_Pressed, this, &APlayerEndboss::LetShip4Play);
	InputComponent->BindAction("PlayShip5", IE_Pressed, this, &APlayerEndboss::LetShip5Play);

	InputComponent->BindAction("FireProjectile", IE_Pressed, this, &APlayerEndboss::OnFire);
	InputComponent->BindAxis("MoveRight", this, &APlayerEndboss::MoveRight);
}

void APlayerEndboss::MoveRight(float AxisValue)
{
	if (OurMovementComponent && (OurMovementComponent->UpdatedComponent == RootComponent))
	{
		OurMovementComponent->AddInputVector(GetActorRightVector() * AxisValue);
	}
}

void APlayerEndboss::MoveShipToStandby()
{
	this->SetActorTransform(m_StandbyPosition);
}

void APlayerEndboss::MoveShipToStart(FVector startLocation)
{
	this->SetActorLocation(startLocation);
}

void APlayerEndboss::OnFire()
{
	if (m_fShotCooldown <= 0.f)
	{
		if (this != nullptr)
		{
			if (ProjectileClass != nullptr)
			{
				if (m_World)
				{
					FVector ShipLocation = this->GetActorLocation();
					FRotator ShipRotation = this->GetActorRotation();
					FRotator MuzzleRotation1 = ShipRotation + FRotator(-180.0f, 0.0f, 0.0f);

					ShipLocation.AddBounded(MuzzleOffset1);
					AProjectile* Projectile1 = m_World->SpawnActor<AProjectile>(ProjectileClass, ShipLocation, MuzzleRotation1);

					ShipLocation = this->GetActorLocation();
					FRotator MuzzleRotation2 = ShipRotation + FRotator(-180.0f, -18.0f, 0.0f);
					ShipLocation.AddBounded(MuzzleOffset2);
					AProjectile* Projectile2 = m_World->SpawnActor<AProjectile>(ProjectileClass, ShipLocation, MuzzleRotation2);

					ShipLocation = this->GetActorLocation();
					FRotator MuzzleRotation3 = ShipRotation + FRotator(-180.0f, 18.0f, 0.0f);
					ShipLocation.AddBounded(MuzzleOffset3);
					AProjectile* Projectile3 = m_World->SpawnActor<AProjectile>(ProjectileClass, ShipLocation, MuzzleRotation3);
					if (Projectile1 && Projectile2 && Projectile3)
					{
						//find launch direction
						FVector LaunchDir1 = MuzzleRotation1.Vector();
						FVector LaunchDir2 = MuzzleRotation2.Vector();
						FVector LaunchDir3 = MuzzleRotation3.Vector();
						Projectile1->InitVelocity(LaunchDir1);
						Projectile2->InitVelocity(LaunchDir2);
						Projectile3->InitVelocity(LaunchDir3);
						m_fShotCooldown = TimeBetweenShots;
					}
				}
			}
		}
	}
}

void APlayerEndboss::OnHit(UPrimitiveComponent * HitComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit)
{
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
	{
		if (Cast<AProjectile>(OtherActor) || Cast<AEnemySpaceship>(OtherActor) || Cast<ADestructible>(OtherActor))
		{
			--m_iHealth;

			OtherActor->Destroy();
		}
	}
}

UPawnMovementComponent* APlayerEndboss::GetMovementComponent() const
{
	return OurMovementComponent;
}