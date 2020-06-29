// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"

void UMyCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

#ifdef JG_DEBUG
  FString NetRole;
  switch (GetOwner()->GetLocalRole())
  {
  case ROLE_SimulatedProxy:
    NetRole = TEXT("ROLE_SimulatedProxy");
    break;
  case ROLE_AutonomousProxy:
    NetRole = TEXT("ROLE_AutonomousProxy");
    break;
  case ROLE_Authority:
    NetRole = TEXT("ROLE_Authority");
    break;
  default:
    NetRole = TEXT("other role");
    break;
  }
  DrawDebugString(GetWorld(), GetOwner()->GetActorForwardVector() * 100, NetRole, GetOwner(), FColor::White, DeltaTime);

  if (GetPawnOwner()->IsLocallyControlled())
  {
      
    FString DebugString;
    DebugString.Append(FString::Printf(TEXT("Role: %s | "), *NetRole))
      .Append(FString::Printf(TEXT("Velocity: %f, %f, %f"), Velocity.X, Velocity.Y, Velocity.Z));
    GEngine->AddOnScreenDebugMessage(INDEX_NONE, -1, FColor::Blue, DebugString);
  }
#endif // JG_DEBUG
}

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
