// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Wall_Cutter.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Wall_Cutter_Test.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SIEGE_API UWall_Cutter_Test : public UActorComponent
{
	GENERATED_BODY()

public:	
	UWall_Cutter_Test();

private:
	UWall_Cutter* current_cutter;

	void Draw_Wall_Poly();

	void Draw_Cut_Poly();

	void Draw_Wall_Intercepts();

	void Draw_Cut_Intercepts();

	void Step_Through_Draw();
	void Step_X_Up();
	void Step_X_Down();
	void Step_Y_Up();
	void Step_Y_Down();

protected:
	virtual void BeginPlay() override;

public:	
	
};
