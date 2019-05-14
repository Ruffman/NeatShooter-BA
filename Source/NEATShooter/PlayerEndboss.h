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
#include "GameInputHandler.h"
#include "PlayerEndboss.generated.h"


class UBoxComponent;

UCLASS()
class NEATSHOOTER_API APlayerEndboss : public AGameInputHandler
{
	GENERATED_BODY()

private:
	float m_fShotCooldown;
	int m_iHealth;

	UPROPERTY()
		FTransform m_StandbyPosition;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	APlayerEndboss(const FObjectInitializer& ObjectInitializer);

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual UPawnMovementComponent* GetMovementComponent() const override;

	//components for hit box etc.
	UPROPERTY(EditAnywhere)
		UBoxComponent *CollisionComponent;
	UPROPERTY(EditAnywhere)
		UStaticMeshComponent *VisualComponent;
	UPROPERTY(EditAnywhere)
		class UCollidingPawnMovementComponent* OurMovementComponent;

	//locations where the projectiles are fired from
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		FVector MuzzleOffset1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		FVector MuzzleOffset2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		FVector MuzzleOffset3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		float TimeBetweenShots = 1.f;
	//not used atm
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		int Health = 3;

	//blueprint binding
	UPROPERTY(EditAnywhere, Category = Projectile)
		TSubclassOf<class AProjectile> ProjectileClass;



	void MoveShipToStandby();
	void MoveShipToStart(FVector startLocation);
	void MoveRight(float AxisValue);
	void OnFire();

	UFUNCTION()
		void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
