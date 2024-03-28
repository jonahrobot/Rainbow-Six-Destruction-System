#pragma once

#include "CoreMinimal.h"

class SIEGE_API Polygon
{
public:
	Polygon();
	Polygon(TArray<FVector2D> vertices);
	~Polygon();

	struct Edge {
		FVector2D start;
		FVector2D end;
	};

	// Functionality
	TArray<FVector2D> find_intersection_points(Polygon other);

	void draw_polygon(UWorld* world, FVector center, FRotator worldRotation, float depth, FColor color);

private:

	TArray<Edge> edges;

	// Helper functions for find_intersection_points
	bool check_in_range(float bound_A, float bound_B, float x);
	bool GetIntersection(FVector2D& out, Edge edge_a, Edge edge_b);
	void edge_to_line_standard_form(float& a, float& b, float& c, Edge e);
};
