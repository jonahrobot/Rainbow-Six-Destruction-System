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
	int a = 0;
	for (FVector2D const x : array) {
		FVector XGlobal = LocalToGlobal(x, actorOrigin, actorRotation, actorScale.X);
		DrawDebugSphere(GetWorld(), XGlobal, 25, 5, color, true, -1.f);

		FString Text = FString::Printf(TEXT("%i"), a);
		FVector vector = actorRotation.RotateVector(FVector(100, x.X - 30, x.Y));
		
		DrawDebugString(GetWorld(), vector, Text, GetOwner(), color, -1.f,false, 2.0f);
		a += 1;
	}
}

// Returns true if X is between bound_A and bound_B
bool check_in_range(float bound_A, float bound_B, float x) {

	// Find our bounds
	float lowerBound = std::floorf(std::min(bound_A, bound_B));
	float upperBound = std::floorf(std::max(bound_A, bound_B));

	// Check if x in bounds
	return (lowerBound <= std::floorf(x)) && (std::floorf(x) <= upperBound);
}

// Add a given Vertex in clockwise vertex order
TArray<FVector2D> UExplosive::addVertexInOrder(TArray<FVector2D> array, FVector2D v, bool markLeaving) {

	if (array.Contains(v)) {
		return array;
	}

	// For each vertex...
	for (int i = 0; i < array.Num(); i++) {

		// Get edge from vert[i] -> next vert in order
		FVector2D start = array[i];
		FVector2D end;

		if (i == array.Num() - 1) {
			end = array[0];
		}
		else {
			end = array[i + 1];
		}

		// If v is inbetween current edge, we found its place!
		// Insert v between the start and end verts of the edge
		if (check_in_range(start.X, end.X, v.X) && check_in_range(start.Y, end.Y, v.Y)) {

			if (markLeaving) {

				// Get point one step away from v->end
				FVector2D dir = (end - v);
				dir.Normalize(0.01f);
				dir = v + dir;

				// Also check if intersection is entering wall or leaving wall
				FVector vector = actorRotation.RotateVector(FVector(100, v.X, v.Y - 30));

				// Vertex is leaving if edge ends outside of polygon
				if (!Base_Polygon.is_point_in_polygon(dir)) {
					DrawDebugString(GetWorld(), vector, TEXT("Exit"), GetOwner(), FColor::Red, -1.f, false, 2.0f);
				}
				else {
					DrawDebugString(GetWorld(), vector, TEXT("Enter"), GetOwner(), FColor::Green, -1.f, false, 2.0f);
				}
			}

			array.Insert(v, i+1);
			return array;
		}
	}

	return array;
}

// Start explosion system
void UExplosive::Explode() {
	UE_LOG(LogTemp, Log, TEXT("- Explosion Triggered! -"));

	TArray<FVector2D> intersections = Base_Polygon.find_intersection_points(Cut_Polygon);

	for (FVector2D const i : intersections) {
		//Wall_Vectors = addVertexInOrder(Wall_Vectors, i, false);
		Cut_Points = addVertexInOrder(Cut_Points, i, true);
	}

	UE_LOG(LogTemp, Log, TEXT("Intersections: %f"), intersections.Num());

	if (debugMode) {

		Draw2DArray(Wall_Vectors, FColor::Blue);
		Draw2DArray(Cut_Points, FColor::Green);

		Base_Polygon.draw_polygon(GetWorld(), actorOrigin, actorRotation, actorScale.X, FColor::Green);
		Cut_Polygon.draw_polygon(GetWorld(), actorOrigin, actorRotation, actorScale.X, FColor::Red);

		
		//Draw2DArray(intersections, FColor::Orange);
	}
}