// Fill out your copyright notice in the Description page of Project Settings.


#include "TestPlayer.h"
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

	Camera->bUsePawnControlRotation = true;

    CharacterMovement = GetCharacterMovement();
	DefaultSpeed = CharacterMovement->MaxWalkSpeed;
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

void ATestPlayer::StartSprint() {
	CharacterMovement->MaxWalkSpeed = DefaultSpeed * 4;
}

void ATestPlayer::StopSprint() {
	CharacterMovement->MaxWalkSpeed = DefaultSpeed;
}

void ATestPlayer::SendCutPoint() {

	FVector start = GetActorLocation();
	FVector forward = Camera->GetForwardVector();
	start = FVector(start.X + (forward.X * 100), start.Y + (forward.Y * 100), start.Z + (forward.Z * 100));

	FVector end = start + (forward * 1000); // Raycast length 1000;
	FHitResult hit;

	if (GetWorld()) {
		bool actorHit = GetWorld()->LineTraceSingleByChannel(hit, start, end, ECC_WorldStatic, FCollisionQueryParams(), FCollisionResponseParams());
		DrawDebugLine(GetWorld(), start, end, FColor::Red, true, 2.f, 0.f, 10.f);
		if (actorHit && hit.GetActor()) {
			
			UWall_Cutter* MyComponent = hit.GetActor()->FindComponentByClass<UWall_Cutter>();

			if (MyComponent != nullptr)
			{
				FVector hitPoint = hit.ImpactPoint;

				FRotator3d reverseRotation = hit.GetActor()->GetActorRotation().GetInverse();

				FVector relativePoint = reverseRotation.RotateVector(hitPoint - hit.GetActor()->GetActorLocation());
				
				MyComponent->Add_Cut_Point(FVector2D(relativePoint.Y, relativePoint.Z));

				DrawDebugSphere(GetWorld(), hit.ImpactPoint, 4.0f, 5, FColor::Blue, true, 2.f, 0.f, 10.f);
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, hit.GetActor()->GetFName().ToString());
			}
		}
	}

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
	PlayerInputComponent->BindAction(FName("Sprint"), IE_Pressed, this, &ATestPlayer::StartSprint);
	PlayerInputComponent->BindAction(FName("Sprint"), IE_Released, this, &ATestPlayer::StopSprint);
	PlayerInputComponent->BindAction(FName("Fire"), IE_Pressed, this, &ATestPlayer::SendCutPoint);
}

