// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Wall_Cutter.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SIEGE_API UWall_Cutter : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWall_Cutter();
	/*
	 *	Cut polygon from wall
	 *	Updates mesh Wall_Cutter component is attached too
	 *
	 *	@cut_polygon the polygon we want to cut from the wall
	 */
	void cutWall();

	// Replicate input from user for testing
	// Calls Cut_Wall with pre-defined shape
	void startCut();

	void addCutPoint(FVector2D const& PointToAdd);

	enum InterceptTypes { NONE, ENTRY, EXIT };

	struct Vertex {
		FVector2D pos;
		InterceptTypes type;
		bool visited = false;

		// Marks where W-A (Weiler-Atherton) Algorithm goes for intercepts
		int intercept_index = -1;

		bool compareAproxPos(FVector2D A, FVector2D B) const {
			return (std::roundf(A.X) == std::roundf(B.X) &&
				std::roundf(A.Y) == std::roundf(B.Y));
		}

		bool equals(const Vertex& other)
		{
			return (compareAproxPos(pos,other.pos) && type == other.type);
		}

		bool operator==(const Vertex& other) const
		{
			return (compareAproxPos(pos, other.pos) && type == other.type);
		}
	};

	typedef TArray<UWall_Cutter::Vertex> Polygon;


private:
	Polygon wall_polygon;
	Polygon cut_polygon;
	TArray<Polygon> regions;

	TArray<FVector2D> wall_shape;
	TArray<FVector2D> cut_shape;

	FVector actor_scale;
	FVector actor_origin;
	FRotator actor_rotation;

	bool pointInsidePolygon(FVector2D const& point, Polygon polygon);

	Polygon walkLoop(TArray<Vertex>& OUT_visited, Vertex const& start, int indexOfVertex, bool clockwise);

	Vertex getNextNode(int& OUT_new_index, int currentIndex, bool in_cut_polygon, bool goClockwise);

	InterceptTypes getInterceptType(FVector2D const& intercept_point, FVector2D const& next_point);


protected:
	virtual void BeginPlay() override;


public:	
	friend class UWall_Cutter_Test;
};
