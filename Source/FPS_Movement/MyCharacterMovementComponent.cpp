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
  //DrawDebugString(GetWorld(), GetOwner()->GetActorForwardVector() * 100, NetRole, GetOwner(), FColor::White, DeltaTime);
  //DrawDebugString(GetWorld(), GetOwner()->GetActorForwardVector() * 100, 
  //  //FString::Printf(TEXT("bPressedJump = %s"), CharacterOwner->bPressedJump ? TEXT("true") : TEXT("false")),
  //  FString::Printf(TEXT("floor blocking hit = %s"), CurrentFloor.bBlockingHit ? TEXT("true") : TEXT("false")),
  //  GetOwner(), FColor::White, DeltaTime);

  if (GetPawnOwner()->IsLocallyControlled())
  {
      
    FString DebugString;
    DebugString.Append(FString::Printf(TEXT("Role: %s | "), *NetRole))
      .Append(FString::Printf(TEXT("Speed: %f"), Velocity.Size()));
    GEngine->AddOnScreenDebugMessage(INDEX_NONE, -1, FColor::Blue, DebugString);
  }
#endif // JG_DEBUG
}

void UMyCharacterMovementComponent::SetDefaultMovementMode()
{
  SetMovementMode(MOVE_Custom, 1);
}

FVector UMyCharacterMovementComponent::GetQ3HorizontalAddVelocity(float deltaTime)
{
  FVector wish_direction = Acceleration.GetSafeNormal();
  bool useCpmAirAccel = (!CurrentFloor.bBlockingHit || CharacterOwner->bPressedJump)
                     && FMath::Abs((UpdatedComponent->GetForwardVector() | wish_direction)) < 0.2f;
  UE_LOG(LogTemp, Warning, TEXT("useCpmAirAccel: %d"), (int)useCpmAirAccel);

  // for analog input
  float wish_magnitude = (Acceleration / MaxAcceleration).Size();

  float current_speed = Velocity | wish_direction;
  float run_speed = useCpmAirAccel ? RunSpeed / 10.f : RunSpeed;
  float add_speed = run_speed - current_speed;
  float air_accel = useCpmAirAccel ? MaxAccelAir * 70 : MaxAccelAir;
  float max_accel = (!CurrentFloor.bBlockingHit || CharacterOwner->bPressedJump) ? air_accel : MaxAccelGround;
  add_speed = FMath::Max<float>(FMath::Min(add_speed, max_accel * deltaTime), 0);
  return wish_direction * add_speed;
}

void UMyCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
  // Call blueprint-event; default behavior copied from vanilla UCharacterMovementComponent
  if (CharacterOwner)
  {
    CharacterOwner->K2_UpdateCustomMovement(deltaTime);
  }
   
  Velocity += GetQ3HorizontalAddVelocity(deltaTime);
  FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, Velocity.IsZero(), NULL);
  ApplyJumpStateToVelocity();
  ApplyGravityToVelocity(deltaTime);
  ApplyGroundFriction(deltaTime);
  RepositionUsingVelocity(deltaTime, Iterations);
}

void UMyCharacterMovementComponent::RepositionUsingVelocity(float deltaTime, int32 Iterations)
{
  FHitResult Hit(1.f);
  SafeMoveUpdatedComponent(Velocity * deltaTime, UpdatedComponent->GetComponentQuat(), true, Hit);

  // If hit something (eg wall), cancel out the velocity-component going into the wall and continue moving.
  // The effect is sliding against the wall.
  for (float time_left = deltaTime; Hit.bBlockingHit && Iterations < MaxSimulationIterations; ++Iterations)
  {
    time_left -= time_left * Hit.Time;
    // Make new velocity = old velocity minus the component going directly into the hit
    Velocity -= Velocity.ProjectOnToNormal(-Hit.Normal);
    // Continue moving after sliding off the hit
    SafeMoveUpdatedComponent(Velocity * time_left, UpdatedComponent->GetComponentQuat(), true, Hit);
  }
}

void UMyCharacterMovementComponent::ApplyGroundFriction(float deltaTime)
{
  // add ground friction unless midair or jumping
  if (CurrentFloor.bBlockingHit && CurrentFloor.FloorDist < 1 && !CharacterOwner->bPressedJump)
  {
    float ground_friction_accel = 3000.f; // TODO(jg): make data member and blueprint-editable
                                         // consider making ground friction more realistic (a force affected by character mass and gravity)
    float adjusted_speed = Velocity.Size() - ground_friction_accel * deltaTime;
    adjusted_speed = FMath::Max(adjusted_speed, 0.f);
    Velocity = Velocity.GetSafeNormal() * adjusted_speed;
  }
}

void UMyCharacterMovementComponent::ApplyGravityToVelocity(float deltaTime)
{
  if (!CurrentFloor.bBlockingHit || CurrentFloor.FloorDist > KINDA_SMALL_NUMBER)
  {
    Velocity.Z += GetGravityZ() * deltaTime;
  }
}

void UMyCharacterMovementComponent::ApplyJumpStateToVelocity()
{
  if (CharacterOwner->bPressedJump && CurrentFloor.bBlockingHit)
  {
    Velocity.Z = JumpZVelocity;
  }
}

void UMyCharacterMovementComponent::OnMovementUpdated(float DeltaTime, const FVector& OldLocation, const FVector& OldVelocity)
{
  Super::OnMovementUpdated(DeltaTime, OldLocation, OldVelocity);

  if (!CharacterOwner)
  {
    return;
  }

  //Set Max Walk Speed
  if (bRequestMaxWalkSpeedChange)
  {
    bRequestMaxWalkSpeedChange = false;
    MaxWalkSpeed = MyNewMaxWalkSpeed;
  }

  //Dodge
  if (bWantsToDodge)
  {
    bWantsToDodge = false;

    //Only dodge if on the ground (in the air causes problems trying to get the two modes to line up due to friction)
    if (IsMovingOnGround())
    {
      MoveDirection.Normalize();
      FVector DodgeVelocity = MoveDirection * DodgeStrength;
      //Set Z component to zero so we don't go up
      DodgeVelocity.Z = 0.0f;

      Launch(DodgeVelocity);
    }
  }
}

void UMyCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)//Client only
{
  Super::UpdateFromCompressedFlags(Flags);

  bRequestMaxWalkSpeedChange = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
  bWantsToDodge = (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;
}

class FNetworkPredictionData_Client* UMyCharacterMovementComponent::GetPredictionData_Client() const
{
  check(PawnOwner != NULL);
  //check(PawnOwner->GetLocalRole() < ROLE_Authority);

  if (!ClientPredictionData)
  {
    UMyCharacterMovementComponent* MutableThis = const_cast<UMyCharacterMovementComponent*>(this);

    MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_My(*this);
    MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
    MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
  }

  return ClientPredictionData;
}

void UMyCharacterMovementComponent::FSavedMove_My::Clear()
{
  Super::Clear();

  bSavedRequestMaxWalkSpeedChange = false;
  bSavedWantsToDodge = false;
  SavedMoveDirection = FVector::ZeroVector;
}

uint8 UMyCharacterMovementComponent::FSavedMove_My::GetCompressedFlags() const
{
  uint8 Result = Super::GetCompressedFlags();

  if (bSavedRequestMaxWalkSpeedChange)
  {
    Result |= FLAG_Custom_0;
  }

  if (bSavedWantsToDodge)
  {
    Result |= FLAG_Custom_1;
  }

  return Result;
}

bool UMyCharacterMovementComponent::FSavedMove_My::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const
{
  //Set which moves can be combined together. This will depend on the bit flags that are used.	
  if (bSavedRequestMaxWalkSpeedChange != ((FSavedMove_My*)&NewMove)->bSavedRequestMaxWalkSpeedChange)
  {
    return false;
  }
  if (bSavedWantsToDodge != ((FSavedMove_My*)&NewMove)->bSavedWantsToDodge)
  {
    return false;
  }
  if (SavedMoveDirection != ((FSavedMove_My*)&NewMove)->SavedMoveDirection)
  {
    return false;
  }

  return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

void UMyCharacterMovementComponent::FSavedMove_My::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData)
{
  Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

  UMyCharacterMovementComponent* CharacterMovement = Cast<UMyCharacterMovementComponent>(Character->GetCharacterMovement());
  if (CharacterMovement)
  {
    bSavedRequestMaxWalkSpeedChange = CharacterMovement->bRequestMaxWalkSpeedChange;
    bSavedWantsToDodge = CharacterMovement->bWantsToDodge;
    SavedMoveDirection = CharacterMovement->MoveDirection;
  }
}

void UMyCharacterMovementComponent::FSavedMove_My::PrepMoveFor(class ACharacter* Character)
{
  Super::PrepMoveFor(Character);

  UMyCharacterMovementComponent* CharacterMovement = Cast<UMyCharacterMovementComponent>(Character->GetCharacterMovement());
  if (CharacterMovement)
  {
    CharacterMovement->MoveDirection = SavedMoveDirection;
  }
}

UMyCharacterMovementComponent::FNetworkPredictionData_Client_My::FNetworkPredictionData_Client_My(const UCharacterMovementComponent& ClientMovement)
  : Super(ClientMovement)
{

}

FSavedMovePtr UMyCharacterMovementComponent::FNetworkPredictionData_Client_My::AllocateNewMove()
{
  return FSavedMovePtr(new FSavedMove_My());
}


//Set Max Walk Speed RPC to transfer the current Max Walk Speed from the Owning Client to the Server
bool UMyCharacterMovementComponent::Server_SetMaxWalkSpeed_Validate(const float NewMaxWalkSpeed)
{
  if (NewMaxWalkSpeed < 0.f || NewMaxWalkSpeed > 2000.f)
    return false;
  else
    return true;
}

void UMyCharacterMovementComponent::Server_SetMaxWalkSpeed_Implementation(const float NewMaxWalkSpeed)
{
  MyNewMaxWalkSpeed = NewMaxWalkSpeed;
}

void UMyCharacterMovementComponent::SetMaxWalkSpeed(float NewMaxWalkSpeed)
{
  if (PawnOwner->IsLocallyControlled())
  {
    MyNewMaxWalkSpeed = NewMaxWalkSpeed;
    Server_SetMaxWalkSpeed(NewMaxWalkSpeed);
  }

  bRequestMaxWalkSpeedChange = true;
}


//Dodge RPC to transfer the current Move Direction from the Owning Client to the Server
bool UMyCharacterMovementComponent::Server_MoveDirection_Validate(const FVector& MoveDir)
{
  return true;
}

void UMyCharacterMovementComponent::Server_MoveDirection_Implementation(const FVector& MoveDir)
{
  MoveDirection = MoveDir;
}

//Trigger the Dodge ability on the Owning Client
void UMyCharacterMovementComponent::Dodge()
{
  if (PawnOwner->IsLocallyControlled())
  {
    MoveDirection = PawnOwner->GetLastMovementInputVector();
    Server_MoveDirection(MoveDirection);
  }

  bWantsToDodge = true;
}

