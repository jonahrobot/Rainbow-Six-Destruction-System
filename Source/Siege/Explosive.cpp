// Fill out your copyright notice in the Description page of Project Settings.


#include "Explosive.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UExplosive::UExplosive()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UExplosive::BeginPlay()
{
	Super::BeginPlay();

	// Link binding
	APlayerController* FirstLocalPlayer = UGameplayStatics::GetPlayerController(this, 0);

	if(IsValid(FirstLocalPlayer) && IsValid(FirstLocalPlayer->InputComponent)){
	
		FirstLocalPlayer->InputComponent->BindAction(FName("Explode"), IE_Pressed, this, &UExplosive::Explode);

	}
	
}

// Convert FVector local to given Aactor to a global
FVector LocalToGlobal(FVector2D LocalVector, FVector ActorOrigin, FRotator ActorRotation, float x) {

	FVector newVector = FVector(x, LocalVector.X, LocalVector.Y);
	return ActorRotation.RotateVector(newVector) + ActorOrigin;
}

struct Edge {
	FVector2D start;
	FVector2D end;
};

TArray<Edge> GetEdges(TArray<FVector2D> vectors) {
	
	TArray<Edge> edges;

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

// True if intersection found
// https://www.geeksforgeeks.org/point-of-intersection-of-two-lines-formula/
bool GetIntersection(FVector2D &out, Edge edge_a, Edge edge_b, int drew, int connectionToDraw) {

	float a1 = -edge_a.end.Y + edge_a.start.Y;
	float b1 = edge_a.end.X - edge_a.start.X;
	float c1 = -(a1 * edge_a.start.X + b1 * edge_a.start.Y);

	float a2 = -edge_b.end.Y + edge_b.start.Y;
	float b2 = edge_b.end.X - edge_b.start.X;
	float c2 = -(a2 * edge_b.start.X + b2 * edge_b.start.Y);

	if ((a1 * b2 - a2 * b1) == 0) return false;

	float x = (b1 * c2 - b2 * c1) / (a1 * b2 - a2 * b1);
	float y = (c1 * a2 - c2 * a1) / (a1 * b2 - a2 * b1);

	if (drew == connectionToDraw) {
		UE_LOG(LogTemp, Log, TEXT("A1: %f, B1: %f, C1: %f"), a1, b1, c1);
		UE_LOG(LogTemp, Log, TEXT("A2: %f, B2: %f, C2: %f"), a2, b2, c2);
		UE_LOG(LogTemp, Log, TEXT("X: %f, Y: %f"), x, y);
	}

	float floorX = std::floorf(x);
	float floorY = std::floorf(y);

	if ((std::floorf(std::min(edge_a.start.X, edge_a.end.X))) > floorX || floorX > std::floorf(std::max(edge_a.start.X, edge_a.end.X))) {
		if (drew == connectionToDraw) {
			UE_LOG(LogTemp, Log, TEXT("Min1: %f, Max: %f"), std::floorf(std::min(edge_a.start.X, edge_a.end.X)), std::floorf(std::max(edge_a.start.X, edge_a.end.X)));
			UE_LOG(LogTemp, Log, TEXT("Failed A."));
		}
		return false;
	}
	if (std::floorf(std::min(edge_a.start.Y, edge_a.end.Y)) > floorY || floorY > std::floorf(std::max(edge_a.start.Y, edge_a.end.Y))) {
		if (drew == connectionToDraw) {
			UE_LOG(LogTemp, Log, TEXT("Failed B."));
		}
		return false;
	}

	if (std::floorf(std::min(edge_b.start.X, edge_b.end.X)) > floorX || floorX > std::floorf(std::max(edge_b.start.X, edge_b.end.X))) {
		if (drew == connectionToDraw) {
			UE_LOG(LogTemp, Log, TEXT("Failed C."));
		}
		return false;
	}
	if (std::floorf(std::min(edge_b.start.Y, edge_b.end.Y)) > floorY || floorY > std::floorf(std::max(edge_b.start.Y, edge_b.end.Y))) {
		if (drew == connectionToDraw) {
			UE_LOG(LogTemp, Log, TEXT("Failed D."));
		}
		return false;
	}

	out = FVector2D(x, y);

	UE_LOG(LogTemp, Log, TEXT("Intersection found!"));


	return true;
}

void UExplosive::Explode() {
	UE_LOG(LogTemp, Log, TEXT("Key Pressed"));


	FVector actorScale = GetOwner()->GetActorScale() / 2 * 100;
	FVector actorOrigin = GetOwner()->GetActorLocation();
	FRotator actorRotation = GetOwner()->GetActorRotation();

	// Get Corners of Wall

	TArray<FVector2D> Wall_Vectors;

	Wall_Vectors.Add(FVector2D(actorScale.Y,  actorScale.Z)); // Top Left
	Wall_Vectors.Add(FVector2D(-actorScale.Y, actorScale.Z)); // Top Right
	Wall_Vectors.Add(FVector2D( -actorScale.Y, -actorScale.Z)); // Bottom Right
	Wall_Vectors.Add(FVector2D( actorScale.Y, -actorScale.Z)); // Bottom Left
	

	for (FVector2D const x : Wall_Vectors) {
		FVector XGlobal = LocalToGlobal(x, actorOrigin, actorRotation, actorScale.X);
		DrawDebugSphere(GetWorld(), XGlobal, 25,5,FColor::Blue,true,-1.f);
	}

	// Get our cut points

	TArray<FVector2D> Cut_Points;

	float const cutSizeOffset = cutOffset + cutSize;
	Cut_Points.Add(FVector2D(cutOffset, cutOffset));
	Cut_Points.Add(FVector2D(cutSizeOffset+200, cutOffset));
	Cut_Points.Add(FVector2D(cutSizeOffset, cutSizeOffset));
	Cut_Points.Add(FVector2D(cutOffset, cutSizeOffset+100));
	

	for (FVector2D const x : Cut_Points) {
		FVector XGlobal = LocalToGlobal(x, actorOrigin, actorRotation, actorScale.X);
		DrawDebugSphere(GetWorld(), XGlobal, 25, 5, FColor::Green, true, -1.f);
	}

	TArray<Edge> test = GetEdges(Wall_Vectors);
	TArray<Edge> other = GetEdges(Cut_Points);
	
	for (Edge const x : test) {
		FVector startGlobal = LocalToGlobal(x.start, actorOrigin, actorRotation, actorScale.X);
		FVector endGlobal = LocalToGlobal(x.end, actorOrigin, actorRotation, actorScale.X);
		DrawDebugLine(GetWorld(), startGlobal, endGlobal, FColor::Green, true, -1.f,0,10.0f);
	}

	for (Edge const x : other) {
		FVector startGlobal = LocalToGlobal(x.start, actorOrigin, actorRotation, actorScale.X);
		FVector endGlobal = LocalToGlobal(x.end, actorOrigin, actorRotation, actorScale.X);
		DrawDebugLine(GetWorld(), startGlobal, endGlobal, FColor::Blue, true, -1.f, 0, 10.0f);
	}

	int drew = 0;

	// For edge in Vector array
	for (Edge const x : test) {
		for (Edge const y : other) {
			FVector2D out;
			bool f = GetIntersection(out, x, y, drew, connectionToDraw);

			if(f) UE_LOG(LogTemp, Log, TEXT("Logged: %i"), drew);

			if (f) {
				
				FVector xa = LocalToGlobal(y.start, actorOrigin, actorRotation, actorScale.X);
				FVector xb = LocalToGlobal(y.end, actorOrigin, actorRotation, actorScale.X);
				DrawDebugLine(GetWorld(), xa, xb, FColor::Orange, true, -1.f, 0, 10.0f);

				FVector startGlobal = LocalToGlobal(x.start, actorOrigin, actorRotation, actorScale.X);
				FVector endGlobal = LocalToGlobal(x.end, actorOrigin, actorRotation, actorScale.X);
				DrawDebugLine(GetWorld(), startGlobal, endGlobal, FColor::Orange, true, -1.f, 0, 10.0f);

				FVector XGlobal = LocalToGlobal(out, actorOrigin, actorRotation, actorScale.X);
				DrawDebugSphere(GetWorld(), XGlobal, 25, 5, FColor::Orange, true, -1.f);
			}
			drew += 1;
		}
	}


	// Create system that returns edges
	// Edges are custom struct of two vectexes
	// Function returns array of edges



	// Triangulate

	// Create new Mesh

	// Delete old mesh

}

// Called every frame
void UExplosive::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

