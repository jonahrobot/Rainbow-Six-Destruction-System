// Fill out your copyright notice in the Description page of Project Settings.


#include "TestPlayer.h"
#include "Camera/CameraComponent.h"

// Sets default values
ATestPlayer::ATestPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create Camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Player Camera"));

	// Attach Camera
	Camera->SetupAttachment(RootComponent);

	Camera->bUsePawnControlRotation = true;

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

// Called to bind functionality to input
void ATestPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(FName("Jump"), IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAxis(FName("MoveForward"), this, &ATestPlayer::MoveForward);
	PlayerInputComponent->BindAxis(FName("MoveRight"), this, &ATestPlayer::MoveRight);
	PlayerInputComponent->BindAxis(FName("TurnCamera"), this, &ATestPlayer::TurnCamera);
	PlayerInputComponent->BindAxis(FName("LookUp"), this, &ATestPlayer::LookUp);
}

