#include "MathLib.h"

// Returns true if X is between bound_A and bound_B
bool MathLib::check_in_range(float bound_A, float bound_B, float x) {

	// Find our bounds
	float lowerBound = std::floorf(std::min(bound_A, bound_B));
	float upperBound = std::floorf(std::max(bound_A, bound_B));

	// Check if x in bounds
	return (lowerBound <= std::floorf(x)) && (std::floorf(x) <= upperBound);
}

// Converts Edge to line represented in (ax + by = c)
void MathLib::edge_to_line_standard_form(float& a, float& b, float& c, EDGE e) {
	a = -e.end.Y + e.start.Y;
	b = e.end.X - e.start.X;
	c = -(a * e.start.X + b * e.start.Y);
}

// Find Intersection between two edges
// @return true if intersection found, passed in out
// @return false if no intersection found
bool MathLib::Find_Intersection(FVector2D& out, EDGE edge_a, EDGE edge_b) {

	float a1, b1, c1;
	edge_to_line_standard_form(a1, b1, c1, edge_a);

	float a2, b2, c2;
	edge_to_line_standard_form(a2, b2, c2, edge_b);

	float d = (a1 * b2 - a2 * b1);

	// Return false, impossible to find intersection
	if (d == 0) return false;

	// Find intersection point
	// https://www.geeksforgeeks.org/point-of-intersection-of-two-lines-formula/
	float x = (b1 * c2 - b2 * c1) / d;
	float y = (c1 * a2 - c2 * a1) / d;

	// Check if found point is in both lines
	if (!check_in_range(edge_a.start.X, edge_a.end.X, x)) return false;
	if (!check_in_range(edge_a.start.Y, edge_a.end.Y, y)) return false;

	if (!check_in_range(edge_b.start.X, edge_b.end.X, x)) return false;
	if (!check_in_range(edge_b.start.Y, edge_b.end.Y, y)) return false;

	// Return success!
	out = FVector2D(x, y);
	UE_LOG(LogTemp, Warning, TEXT("Found Intercept"));
	return true;
}

FVector MathLib::LocalToGlobal(FVector2D LocalVector, FVector ActorOrigin, FRotator ActorRotation, float x) {

	FVector newVector = FVector(x, LocalVector.X, LocalVector.Y);
	return ActorRotation.RotateVector(newVector) + ActorOrigin;
}