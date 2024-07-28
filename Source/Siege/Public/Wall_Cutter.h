// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Polygon.h"
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

private:
	Polygon wall_polygon;
	Polygon cut_polygon;
	TArray<Polygon> regions;

	Polygon start_wall_polygon;
	Polygon start_cut_polygon;

	FVector actor_scale;
	FVector actor_origin;
	FRotator actor_rotation;

	Polygon walkLoop(TArray<Polygon::VertexData>& OUT_visited, Polygon::Vertex* start, int direction);

	Polygon::InterceptTypes getInterceptType(FVector2D const& intercept_point, FVector2D const& next_point);


protected:
	virtual void BeginPlay() override;


public:	
	friend class UWall_Cutter_Test;
};
