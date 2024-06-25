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
	void Test_Input_Triggered();

	enum node_type { DEFAULT, INTERCEPT_ENTRY, INTERCEPT_EXIT };

	struct POLYGON_NODE {
		FVector2D pos;
		node_type type;

		// Marks where W-A (Weiler-Atherton) Algorithm goes for intercepts
		int intercept_index = -1;
	};

private:
	TArray<FString> node_type_names{ "-", "Entry", "Exit" };

	TArray<UWall_Cutter::POLYGON_NODE> wall_polygon;
	TArray<UWall_Cutter::POLYGON_NODE> cut_polygon;

	TArray<FVector2D> wall_shape;
	TArray<FVector2D> cut_shape;

	FVector actorScale;
	FVector actorOrigin;
	FRotator actorRotation;

	node_type get_intercept_type(FVector2D intercept_point, FVector2D next_point);
	void debug_print_polygon(TArray<POLYGON_NODE> poly, TArray<POLYGON_NODE> otherPoly, FString name, node_type checkType);


protected:
	virtual void BeginPlay() override;


public:	
	friend class UWall_Cutter_Test;
};
