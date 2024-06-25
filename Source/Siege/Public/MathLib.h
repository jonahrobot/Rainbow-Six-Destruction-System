#pragma once

#include "CoreMinimal.h"

class SIEGE_API MathLib
{
public:
	struct EDGE {
		FVector2D start;
		FVector2D end;
	};

private:
	bool static check_in_range(float bound_A, float bound_B, float x);
	void static edge_to_line_standard_form(float& a, float& b, float& c, EDGE e);

public:
	bool static Find_Intersection(FVector2D& out, EDGE edge_a, EDGE edge_b);
	FVector static LocalToGlobal(FVector2D LocalVector, FVector ActorOrigin, FRotator ActorRotation, float x);
};
