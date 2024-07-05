// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Wall_Cutter.h"
#include "CoreMinimal.h"
#include "Polygon.h"
#include "Components/ActorComponent.h"
#include "Wall_Cutter_Test.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SIEGE_API UWall_Cutter_Test : public UActorComponent
{
	GENERATED_BODY()

public:
	UWall_Cutter_Test();

private:
	TArray<FString> node_type_names{ "-", "Entry", "Exit" };
	UWall_Cutter* cutter;

	void Draw_Wall_Poly();

	void Draw_Cut_Poly();

	void Draw_Wall_Intercepts();


	void Step_Up();
	void Step_Down();
	void Step_Left();
	void Step_Right();
	void Step_Through_Draw();
	void Draw_Shrapnel();

	void Draw_Polygon_Intercepts(Polygon poly, Polygon linkedPolygon, FColor color, FString nameOfDraw);

	FVector origin;
	FRotator rotation;
	FVector scale;

	int step_through_x = 0;
	int step_through_y = 0;

	void Step(float x, float y);

	void debug_print_polygon(Polygon poly, Polygon otherPoly, FString name, Polygon::InterceptTypes checkType);

protected:
	virtual void BeginPlay() override;

public:	
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
};
