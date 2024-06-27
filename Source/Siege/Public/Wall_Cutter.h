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
	void Cut_Wall();

	// Replicate input from user for testing
	// Calls Cut_Wall with pre-defined shape
	void Start_Cut();

	enum node_type { DEFAULT, INTERCEPT_ENTRY, INTERCEPT_EXIT };

	struct POLYGON_NODE {
		FVector2D pos;
		node_type type;
		bool visited = false;

		// Marks where W-A (Weiler-Atherton) Algorithm goes for intercepts
		int intercept_index = -1;

		bool equals(const POLYGON_NODE& other)
		{
			return (pos == other.pos && type == other.type);
		}

		bool operator==(const POLYGON_NODE& other) const
		{
			return (pos == other.pos && type == other.type);
		}
	};

	typedef TArray<UWall_Cutter::POLYGON_NODE> Polygon;


private:
	Polygon wall_polygon;
	Polygon cut_polygon;
	TArray<Polygon> regions;

	TArray<FVector2D> wall_shape;
	TArray<FVector2D> cut_shape;

	FVector actorScale;
	FVector actorOrigin;
	FRotator actorRotation;

	Polygon walk_loop(TArray<POLYGON_NODE>& OUT_visited, POLYGON_NODE start, int indexOfVertex);

	POLYGON_NODE get_next_node(int& OUT_new_index, int currentIndex, bool in_cut_polygon);

	node_type get_intercept_type(FVector2D intercept_point, FVector2D next_point);


protected:
	virtual void BeginPlay() override;


public:	
	friend class UWall_Cutter_Test;
};
