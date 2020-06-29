// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MyCharacterMovementComponent.generated.h"

// enable debug printing
#define JG_DEBUG
/**
 * 
 */
UCLASS()
class FPS_MOVEMENT_API UMyCharacterMovementComponent : public UCharacterMovementComponent
{
  GENERATED_BODY()

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	/** Set movement mode to the default based on the current physics volume. */
	virtual void SetDefaultMovementMode() override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

public:
  UPROPERTY(BlueprintReadWrite, EditAnywhere)
  float CustomBaseAcceleration = 100.f;
};
