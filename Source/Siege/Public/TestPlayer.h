// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Wall_Cutter.h"
#include "TestPlayer.generated.h"

UCLASS()
class SIEGE_API ATestPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATestPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere)
	float rayLength = 10000;
	
protected:

	DECLARE_DELEGATE_OneParam(FBoolDelegate, bool);

	UPROPERTY(EditAnywhere)
	class UCameraComponent* Camera;

	UCharacterMovementComponent* CharacterMovement;

	void MoveForward(float input);
	void MoveRight(float input);
	void TurnCamera(float input);
	void LookUp(float input);
	void SetSprintStatus(bool newState);
	void StopSprint();
	void SendCutPoint();
	void SetLaserCut(bool newState);

	float DefaultSpeed;
	FHitResult currentTargetRaycastData;
	FCollisionQueryParams collisionParams;
	UWall_Cutter* currentTarget;
	bool hasTarget = false;
	FVector startCameraRotation;

private:
	bool laserCutting = false;
};
