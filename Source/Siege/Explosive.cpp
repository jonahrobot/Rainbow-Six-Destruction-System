// Fill out your copyright notice in the Description page of Project Settings.


#include "Explosive.h"
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
	Base_Polygon.edges = GetEdges(Wall_Vectors);
	Cut_Polygon.edges = GetEdges(Cut_Points);
}

// Convert FVector local to given Aactor to a global
FVector UExplosive::LocalToGlobal(FVector2D LocalVector, FVector ActorOrigin, FRotator ActorRotation, float x) {

	FVector newVector = FVector(x, LocalVector.X, LocalVector.Y);
	return ActorRotation.RotateVector(newVector) + ActorOrigin;
}

TArray<UExplosive::Edge> UExplosive::GetEdges(TArray<FVector2D> vectors) {
	
	TArray<UExplosive::Edge> edges;

	FVector2D a, b;

	for (int i = 0; i < vectors.Num(); i++) {

		// Get current Edge
		a = vectors[i];
		if (i + 1 >= vectors.Num()) {
			b = vectors[0];
		}
		else {
			b = vectors[i + 1];
		}

		edges.Add({ a,b });
	}
	
	return edges;
}

// Could refactor most of this code into a Polygon .cpp and .h

// Returns true if X is between bound_A and bound_B
bool UExplosive::check_in_range(float bound_A, float bound_B, float x) {

	// Find our bounds
	float lowerBound = std::floorf(std::min(bound_A, bound_B));
	float upperBound = std::floorf(std::max(bound_A, bound_B));

	// Check if x in bounds
	return (lowerBound <= std::floorf(x)) && (std::floorf(x) <= upperBound);
}

// Converts Edge to line represented in (ax + by = c)
void UExplosive::edge_to_line_standard_form(float& a, float& b, float& c, Edge e) {
	a = -e.end.Y + e.start.Y;
	b = e.end.X - e.start.X;
	c = -(a * e.start.X + b * e.start.Y);
}

// Sets out param to intersection point between edge_a and edge_b. 
bool UExplosive::GetIntersection(FVector2D &out, Edge edge_a, Edge edge_b) {

	float a1, b1, c1;
	edge_to_line_standard_form(a1, b1, c1, edge_a);
	
	float a2, b2, c2;
	edge_to_line_standard_form(a2, b2, c2, edge_b);

	float d = (a1 * b2 - a2 * b1);

	// Return false, impossible to find intersection
	if (d == 0) return false;
	
	// Find intersection point
	// https://www.geeksforgeeks.org/point-of-intersection-of-two-lines-formula/
	float x = (b1 * c2 - b2 * c1) / d;
	float y = (c1 * a2 - c2 * a1) / d;

	// Check if found point is in both lines
	if (!check_in_range(edge_a.start.X, edge_a.end.X, x)) return false;
	if (!check_in_range(edge_a.start.Y, edge_a.end.Y, y)) return false;

	if (!check_in_range(edge_b.start.X, edge_b.end.X, x)) return false;
	if (!check_in_range(edge_b.start.Y, edge_b.end.Y, y)) return false;

	// Return success!
	out = FVector2D(x, y);

	UE_LOG(LogTemp, Log, TEXT("Intersection found!"));

	return true;
}

// Find all intersections between two polygons
TArray<FVector2D> UExplosive::find_all_intersections(UExplosive::Polygon polygon_a, UExplosive::Polygon polygon_b) {
	
	TArray<FVector2D> intersections;

	// For edge in Vector array
	for (Edge const x : polygon_a.edges) {
		for (Edge const y : polygon_b.edges) {
			FVector2D out;
			bool f = GetIntersection(out, x, y);

			if (f) intersections.Add(out);
		}
	}

	return intersections;
}


// Start explosion system
void UExplosive::Explode() {
	UE_LOG(LogTemp, Log, TEXT("- Explosion Triggered! -"));

	TArray<FVector2D> intersections = find_all_intersections(Base_Polygon, Cut_Polygon);
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

		for (Edge const x : Base_Polygon.edges) {
			FVector startGlobal = LocalToGlobal(x.start, actorOrigin, actorRotation, actorScale.X);
			FVector endGlobal = LocalToGlobal(x.end, actorOrigin, actorRotation, actorScale.X);
			DrawDebugLine(GetWorld(), startGlobal, endGlobal, FColor::Green, true, -1.f, 0, 10.0f);
		}

		for (Edge const x : Cut_Polygon.edges) {
			FVector startGlobal = LocalToGlobal(x.start, actorOrigin, actorRotation, actorScale.X);
			FVector endGlobal = LocalToGlobal(x.end, actorOrigin, actorRotation, actorScale.X);
			DrawDebugLine(GetWorld(), startGlobal, endGlobal, FColor::Blue, true, -1.f, 0, 10.0f);
		}

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
