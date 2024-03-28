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
	
	// Get properties of wall
	actorScale = GetOwner()->GetActorScale() / 2 * 100;
	actorOrigin = GetOwner()->GetActorLocation();
	actorRotation = GetOwner()->GetActorRotation();

	// Create polygon size of wall surface
	Wall_Vectors.Add(FVector2D(actorScale.Y, actorScale.Z)); // Top Left
	Wall_Vectors.Add(FVector2D(-actorScale.Y, actorScale.Z)); // Top Right
	Wall_Vectors.Add(FVector2D(-actorScale.Y, -actorScale.Z)); // Bottom Right
	Wall_Vectors.Add(FVector2D(actorScale.Y, -actorScale.Z)); // Bottom Left

	Base_Polygon = Polygon(Wall_Vectors);

	// Turn cut points into polygon
	Cut_Polygon = Polygon(Cut_Points);
}

// Convert FVector local to given Aactor to a global
FVector UExplosive::LocalToGlobal(FVector2D LocalVector, FVector ActorOrigin, FRotator ActorRotation, float x) {

	FVector newVector = FVector(x, LocalVector.X, LocalVector.Y);
	return ActorRotation.RotateVector(newVector) + ActorOrigin;
}

void UExplosive::Draw2DArray(TArray<FVector2D> array, FColor color) {
	for (FVector2D const x : array) {
		FVector XGlobal = LocalToGlobal(x, actorOrigin, actorRotation, actorScale.X);
		DrawDebugSphere(GetWorld(), XGlobal, 25, 5, color, true, -1.f);
	}
}

// Start explosion system
void UExplosive::Explode() {
	UE_LOG(LogTemp, Log, TEXT("- Explosion Triggered! -"));

	TArray<FVector2D> intersections = Base_Polygon.find_intersection_points(Cut_Polygon);
	UE_LOG(LogTemp, Log, TEXT("Intersections: %f"), intersections.Num());

	if (debugMode) {

		Draw2DArray(Wall_Vectors, FColor::Blue);
		Draw2DArray(Cut_Points, FColor::Green);

		Base_Polygon.draw_polygon(GetWorld(), actorOrigin, actorRotation, actorScale.X, FColor::Green);
		Cut_Polygon.draw_polygon(GetWorld(), actorOrigin, actorRotation, actorScale.X, FColor::Red);

		Draw2DArray(intersections, FColor::Orange);
	}


	// Create system that returns edges
	// Edges are custom struct of two vectexes
	// Function returns array of edges



	// Triangulate

	// Create new Mesh

	// Delete old mesh

}
