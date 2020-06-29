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
  // vq3 movement
  float run_speed = 1280.f; // cm/s
  float max_accel = 1280.f; // cm/s^2
  FVector wish_direction = Acceleration.GetSafeNormal();

  // for analog input
  float wish_magnitude = (Acceleration / MaxAcceleration).Size();

  float current_speed = Velocity | wish_direction;
  float add_speed = run_speed - current_speed;
  add_speed = FMath::Max<float>(FMath::Min(add_speed, max_accel * deltaTime), 0);
  Velocity += wish_direction * add_speed;

#if 0 // Simple floaty movement
  Velocity += Acceleration / MaxAcceleration * CustomBaseAcceleration * deltaTime;
#endif

	FHitResult Hit(1.f);
	SafeMoveUpdatedComponent(Velocity * deltaTime, UpdatedComponent->GetComponentQuat(), true, Hit);
  if (Hit.bBlockingHit)
  {
    float usedTime = deltaTime * (Hit.Time);
    // Make new velocity = old velocity minus the component going directly into the hit
    Velocity -= Velocity.ProjectOnToNormal(-Hit.Normal);
    // Continue moving after sliding off the hit
	  SafeMoveUpdatedComponent(Velocity * (deltaTime - usedTime), UpdatedComponent->GetComponentQuat(), true, Hit);
  }
}
