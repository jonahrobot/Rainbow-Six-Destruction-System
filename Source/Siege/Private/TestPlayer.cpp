// Fill out your copyright notice in the Description page of Project Settings.


#include "TestPlayer.h"
#include "MathLib.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Wall_Cutter.h"

// Sets default values
ATestPlayer::ATestPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create Camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Player Camera"));

	// Attach Camera
	Camera->SetupAttachment(RootComponent);

	// Enable camera rotation
	Camera->bUsePawnControlRotation = true;

	// Link Character Movement
    CharacterMovement = GetCharacterMovement();
	DefaultSpeed = CharacterMovement->MaxWalkSpeed;

	// Setup Raycast Params
	collisionParams = FCollisionQueryParams();
	collisionParams.AddIgnoredActor(this);
}

// Called when the game starts or when spawned
void ATestPlayer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATestPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (laserCutting) {
		timeSinceLastCut += DeltaTime;

		if (timeSinceLastCut >= 0.05f) {
			SendCutPoint();
			timeSinceLastCut = 0.0f;
		}
	}

}

void ATestPlayer::MoveForward(float input) {
	FVector ForwardDir = GetActorForwardVector();
	AddMovementInput(ForwardDir, input);
}

void ATestPlayer::MoveRight(float input) {
	FVector RightDir = GetActorRightVector();
	AddMovementInput(RightDir, input);
}

void ATestPlayer::TurnCamera(float input) {
	AddControllerYawInput(input);
}
void ATestPlayer::LookUp(float input) {
	AddControllerPitchInput(input);
}

void  ATestPlayer::SetSprintStatus(bool newState) {
	if (newState) {
		CharacterMovement->MaxWalkSpeed = DefaultSpeed * 4;
	}
	else {
		CharacterMovement->MaxWalkSpeed = DefaultSpeed;
	}
}

void ATestPlayer::SetLaserCut(bool newState) {
	laserCutting = newState;

	if (laserCutting == false) {
		hasTarget = false;
	}
}

void ATestPlayer::SendCutPoint() {

	if (GetWorld() == false) return;

	// If no target, raycast until target is found
	if (hasTarget == false) {

		FHitResult hit;
		FVector rayStart = GetActorLocation() + Camera->GetRelativeLocation();
		FVector rayEnd = rayStart + (Camera->GetForwardVector() * rayLength);

		bool actorHit = GetWorld()->LineTraceSingleByChannel(hit, rayStart, rayEnd, ECC_WorldStatic, collisionParams, FCollisionResponseParams());

		if (actorHit && hit.GetActor()) {
			currentTarget = hit.GetActor()->FindComponentByClass<UWall_Cutter>();

			if (currentTarget != nullptr) {
				hasTarget = true;
				currentTargetRaycastData = hit;
				startCameraRotation = GetActorLocation() + Camera->GetForwardVector();
				DrawDebugSphere(GetWorld(), hit.ImpactPoint, 4.0f, 5, FColor::Orange, true, 2.f, 0.f, 10.f);
			}
		}
	}

	// Once target found, lock to target and stop raycasts
	if (hasTarget == false) return;

	// Define wall plane
	FVector wallPos = currentTargetRaycastData.ImpactPoint;
	FVector wallNormal = currentTargetRaycastData.ImpactNormal;

	FVector rayStart = GetActorLocation() + Camera->GetRelativeLocation();
	FVector rayEnd = rayStart + (Camera->GetForwardVector() * rayLength);

	// FInd point on wall plane
	FVector pointOnWall = FMath::LinePlaneIntersection(rayStart, rayEnd, wallPos, wallNormal);

	DrawDebugSphere(GetWorld(), pointOnWall, 4.0f, 5, FColor::Blue, true, 2.f, 0.f, 10.f);

	// Add our point
	FVector2D pointToAdd = MathLib::GlobalToLocal(pointOnWall, currentTargetRaycastData.GetActor());

	currentTarget->addCutPoint(pointToAdd);
}

// Called to bind functionality to input
void ATestPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(FName("Jump"), IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAxis(FName("MoveForward"), this, &ATestPlayer::MoveForward);
	PlayerInputComponent->BindAxis(FName("MoveRight"), this, &ATestPlayer::MoveRight);
	PlayerInputComponent->BindAxis(FName("TurnCamera"), this, &ATestPlayer::TurnCamera);
	PlayerInputComponent->BindAxis(FName("LookUp"), this, &ATestPlayer::LookUp);

	PlayerInputComponent->BindAction<FBoolDelegate>(FName("Sprint"), IE_Pressed, this, &ATestPlayer::SetSprintStatus, true);
	PlayerInputComponent->BindAction<FBoolDelegate>(FName("Sprint"), IE_Released, this, &ATestPlayer::SetSprintStatus,false);
	PlayerInputComponent->BindAction<FBoolDelegate>(FName("Fire"), IE_Pressed, this, &ATestPlayer::SetLaserCut,true);
	PlayerInputComponent->BindAction<FBoolDelegate>(FName("Fire"), IE_Released, this, &ATestPlayer::SetLaserCut, false);
}

