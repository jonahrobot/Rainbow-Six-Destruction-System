#include "Polygon.h"

Polygon::Polygon()
{
}

Polygon::Polygon(TArray<FVector2D> vertices)
{
	FVector2D a, b;

	// For each vertex...
	for (int i = 0; i < vertices.Num(); i++) {

		// Find edge from vertices[i] -> vertices[i+1]
		a = vertices[i];

		// Handle edge case when at last vertex, loop back to first
		if (i + 1 >= vertices.Num()) {
			b = vertices[0];
		}
		else {
			b = vertices[i + 1];
		}

		// Add the found edge
		this->edges.Add({ a,b });
	}
}

Polygon::~Polygon()
{
}

// Returns true if X is between bound_A and bound_B
bool Polygon::check_in_range(float bound_A, float bound_B, float x) {

	// Find our bounds
	float lowerBound = std::floorf(std::min(bound_A, bound_B));
	float upperBound = std::floorf(std::max(bound_A, bound_B));

	// Check if x in bounds
	return (lowerBound <= std::floorf(x)) && (std::floorf(x) <= upperBound);
}

// Converts Edge to line represented in (ax + by = c)
void Polygon::edge_to_line_standard_form(float& a, float& b, float& c, Edge e) {
	a = -e.end.Y + e.start.Y;
	b = e.end.X - e.start.X;
	c = -(a * e.start.X + b * e.start.Y);
}

// Sets out param to intersection point between edge_a and edge_b. 
bool Polygon::GetIntersection(FVector2D& out, Edge edge_a, Edge edge_b) {

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

	UE_LOG(LogTemp, Log, TEXT("Intersection found!"));

	return true;
}

// https://www.youtube.com/watch?v=RSXM9bgqxJM&list=LL&index=1
bool Polygon::is_point_in_polygon(FVector2D p) {

	int overlaps = 0;

	for (Edge const e : this->edges) {

		float x1 = e.start.X;
		float y1 = e.start.Y;
		float x2 = e.end.X;
		float y2 = e.end.Y;

		if ((p.Y < y1) != (p.Y < y2)) {
			if (p.X < (x1 + ((p.Y - y1) / (y2 - y1)) * (x2 - x1))) {
				overlaps += 1;
			}
		}
	}

	return overlaps % 2 == 1;

}

// Find all intersections between two polygons
TArray<FVector2D> Polygon::find_intersection_points(Polygon other) {

	TArray<FVector2D> intersections;

	// For edge in Vector array
	for (Edge const x : this->edges) {
		for (Edge const y : other.edges) {
			FVector2D out;
			bool f = GetIntersection(out, x, y);

			if (f) intersections.Add(out);
		}
	}

	return intersections;
}

void Polygon::draw_polygon(UWorld* world, FVector center, FRotator worldRotation, float depth, FColor color) {
	for (Edge const x : this->edges) {
		/*FVector startGlobal = LocalToGlobal(x.start, center, worldRotation, depth);
		FVector endGlobal = LocalToGlobal(x.end, center, worldRotation, depth);*/
		//DrawDebugLine(world, startGlobal, endGlobal, color, true, -1.f, 0, 10.0f);
	}
}