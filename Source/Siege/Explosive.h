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
		TArray<FVector2D> Cut_Points;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool debugMode;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void Explode();

	TArray<FVector2D> Wall_Vectors;

	Polygon Base_Polygon;
	Polygon Cut_Polygon;
	FVector actorScale;
	FVector actorOrigin;
	FRotator actorRotation;

	FVector LocalToGlobal(FVector2D LocalVector, FVector ActorOrigin, FRotator ActorRotation, float x);

	void Draw2DArray(TArray<FVector2D> array, FColor color);
		
};
