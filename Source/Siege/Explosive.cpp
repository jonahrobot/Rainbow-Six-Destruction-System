// Fill out your copyright notice in the Description page of Project Settings.


#include "Explosive.h"
#include "Polygon.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UExplosive::UExplosive()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	debugMode = false;

	// ...
}

// Called when the game starts
void UExplosive::BeginPlay()
{
	Super::BeginPlay();

	// Bind Explode function to explode key
	APlayerController* FirstLocalPlayer = UGameplayStatics::GetPlayerController(this, 0);

	if(IsValid(FirstLocalPlayer) && IsValid(FirstLocalPlayer->InputComponent)){
	
		FirstLocalPlayer->InputComponent->BindAction(FName("Explode"), IE_Pressed, this, &UExplosive::Explode);

	}
	
	// Get properties of attached wall
	actorScale = GetOwner()->GetActorScale() / 2 * 100;
	actorOrigin = GetOwner()->GetActorLocation();
	actorRotation = GetOwner()->GetActorRotation();

	// Get corner of wall
	Wall_Vectors.Add(FVector2D(actorScale.Y, actorScale.Z)); // Top Left
	Wall_Vectors.Add(FVector2D(-actorScale.Y, actorScale.Z)); // Top Right
	Wall_Vectors.Add(FVector2D(-actorScale.Y, -actorScale.Z)); // Bottom Right
	Wall_Vectors.Add(FVector2D(actorScale.Y, -actorScale.Z)); // Bottom Left

	// Setup demo cut points
	float const cutSizeOffset = cutOffset + cutSize;
	Cut_Points.Add(FVector2D(cutOffset, cutOffset));
	Cut_Points.Add(FVector2D(cutSizeOffset + 200, cutOffset));
	Cut_Points.Add(FVector2D(cutSizeOffset, cutSizeOffset));
	Cut_Points.Add(FVector2D(cutOffset, cutSizeOffset + 100));

	// Process Edges
	Base_Polygon = Polygon(Wall_Vectors);
	Cut_Polygon = Polygon(Cut_Points);
}

// Convert FVector local to given Aactor to a global
FVector UExplosive::LocalToGlobal(FVector2D LocalVector, FVector ActorOrigin, FRotator ActorRotation, float x) {

	FVector newVector = FVector(x, LocalVector.X, LocalVector.Y);
	return ActorRotation.RotateVector(newVector) + ActorOrigin;
}

// Start explosion system
void UExplosive::Explode() {
	UE_LOG(LogTemp, Log, TEXT("- Explosion Triggered! -"));

	TArray<FVector2D> intersections = Base_Polygon.find_intersection_points(Cut_Polygon);
	UE_LOG(LogTemp, Log, TEXT("Intersections: %f"), intersections.Num());

	if (debugMode) {
		for (FVector2D const x : Wall_Vectors) {
			FVector XGlobal = LocalToGlobal(x, actorOrigin, actorRotation, actorScale.X);
			DrawDebugSphere(GetWorld(), XGlobal, 25, 5, FColor::Blue, true, -1.f);
		}

		for (FVector2D const x : Cut_Points) {
			FVector XGlobal = LocalToGlobal(x, actorOrigin, actorRotation, actorScale.X);
			DrawDebugSphere(GetWorld(), XGlobal, 25, 5, FColor::Green, true, -1.f);
		}

		Base_Polygon.draw_polygon(GetWorld(), actorOrigin, actorRotation, actorScale.X, FColor::Green);
		Cut_Polygon.draw_polygon(GetWorld(), actorOrigin, actorRotation, actorScale.X, FColor::Red);

		for (FVector2D const x : intersections) {

			FVector XGlobal = LocalToGlobal(x, actorOrigin, actorRotation, actorScale.X);
			DrawDebugSphere(GetWorld(), XGlobal, 25, 5, FColor::Orange, true, -1.f);
		}
	}


	// Create system that returns edges
	// Edges are custom struct of two vectexes
	// Function returns array of edges



	// Triangulate

	// Create new Mesh

	// Delete old mesh

}
