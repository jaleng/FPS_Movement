// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacterMovementComponent.h"
#include "GameFramework/Character.h"

void UMyCharacterMovementComponent::SetDefaultMovementMode()
{
  SetMovementMode(MOVE_Custom, 1);
}

void UMyCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
  // Call blueprint-event; default behavior copied from vanilla UCharacterMovementComponent
  if (CharacterOwner)
  {
    CharacterOwner->K2_UpdateCustomMovement(deltaTime);
  }
   
  // Custom physics
  Velocity += Acceleration.GetSafeNormal() * CustomBaseAcceleration * deltaTime;

	FHitResult Hit(1.f);
	SafeMoveUpdatedComponent(Velocity * deltaTime, UpdatedComponent->GetComponentQuat(), true, Hit);
  if (Hit.bBlockingHit)
  {
    Velocity = FVector::ZeroVector;
  }
}
