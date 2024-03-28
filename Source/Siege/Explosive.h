// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Explosive.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SIEGE_API UExplosive : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UExplosive();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool debugMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float cutSize;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float cutOffset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int connectionToDraw;

	struct Edge {
		FVector2D start;
		FVector2D end;
	};

	struct Polygon {
		TArray<Edge> edges;
	};


protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void Explode();

	TArray<FVector2D> Wall_Vectors;
	TArray<FVector2D> Cut_Points;
	Polygon Base_Polygon;
	Polygon Cut_Polygon;
	FVector actorScale;
	FVector actorOrigin;
	FRotator actorRotation;

	FVector LocalToGlobal(FVector2D LocalVector, FVector ActorOrigin, FRotator ActorRotation, float x);
	TArray<Edge> GetEdges(TArray<FVector2D> vectors);
	bool check_in_range(float bound_A, float bound_B, float x);
	bool GetIntersection(FVector2D& out, Edge edge_a, Edge edge_b);
	void edge_to_line_standard_form(float& a, float& b, float& c, Edge e);
	TArray<FVector2D> find_all_intersections(UExplosive::Polygon polygon_a, UExplosive::Polygon polygon_b);
		
};
