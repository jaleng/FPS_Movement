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

  class FSavedMove_My : public FSavedMove_Character
  {
  public:

    typedef FSavedMove_Character Super;

    virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override;
    virtual void Clear() override;
    virtual uint8 GetCompressedFlags() const override;
    virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;
    virtual void PrepMoveFor(class ACharacter* Character) override;

    //Walk Speed Update
    uint8 bSavedRequestMaxWalkSpeedChange : 1;

    //Dodge
    FVector SavedMoveDirection;
    uint8 bSavedWantsToDodge : 1;
  };

  class FNetworkPredictionData_Client_My : public FNetworkPredictionData_Client_Character
  {
  public:
    FNetworkPredictionData_Client_My(const UCharacterMovementComponent& ClientMovement);

    typedef FNetworkPredictionData_Client_Character Super;

    virtual FSavedMovePtr AllocateNewMove() override;
  };

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	/** Set movement mode to the default based on the current physics volume. */
	virtual void SetDefaultMovementMode() override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

public:

  virtual void UpdateFromCompressedFlags(uint8 Flags) override;
  virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;
  void OnMovementUpdated(float DeltaTime, const FVector& OldLocation, const FVector& OldVelocity);

  /// Movement parameters
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement Parameters")
  float RunSpeed = 1280.f; // cm/s

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement Parameters")
  float MaxAccelAir = 1280.f; // cm/s^2

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement Parameters")
  float MaxAccelGround = 5000.f; // cm/s^2

private:

  /**
   * Calculate horizontal velocity to add using quake algorithm that allows strafe-jumping
   * @param deltaTime - timeframe to simulate over
   * @return The calculated velocity that should be added to the total velocity for Q3 movement
   */
  FVector GetQ3HorizontalAddVelocity(float deltaTime);

  /**
   * Move the updated component using Velocity and deltatime. Collisions cancel all velocity normal to the hit.
   */
  void RepositionUsingVelocity(float deltaTime, int32 Iterations);

  /**
   * Apply ground friction if on the ground.
   */
  void ApplyGroundFriction(float deltaTime);

  /**
   * Apply gravity if updated component is not on the floor.
   */
  void ApplyGravityToVelocity(float deltaTime);

  /**
   * Will change vertical velocity to jumping velocity if the player is pressing jump and the character is on the floor
   */
  void ApplyJumpStateToVelocity();

public:
  //Set Max Walk Speed
  uint8 bRequestMaxWalkSpeedChange : 1;

  UFUNCTION(Unreliable, Server, WithValidation)
    void Server_SetMaxWalkSpeed(const float NewMaxWalkSpeed);

  float MyNewMaxWalkSpeed;

  //Set Max Walk Speed (Called from the owning client)
  UFUNCTION(BlueprintCallable, Category = "Max Walk Speed")
    void SetMaxWalkSpeed(float NewMaxWalkSpeed);

  //Dodge
  UPROPERTY(EditAnywhere, Category = "Dodge")
    float DodgeStrength;

  UFUNCTION(Unreliable, Server, WithValidation)
    void Server_MoveDirection(const FVector& MoveDir);

  //Trigger the dodge ability (Called from the owning client)
  UFUNCTION(BlueprintCallable, Category = "Dodge")
    void Dodge();

  FVector MoveDirection;
  uint8 bWantsToDodge : 1;


};
