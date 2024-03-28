// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Polygon.h"
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
		
};
