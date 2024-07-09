// Fill out your copyright notice in the Description page of Project Settings.


#include "Polygon.h"

Polygon::Polygon()
{
}

Polygon::~Polygon()
{

}

bool Polygon::pointInsidePolygon(FVector2D const& point) const{

	// Follows algorithm presented below, finds it point is inside polygon
	// https://www.youtube.com/watch?v=RSXM9bgqxJM&list=LL&index=1

	int overlaps = 0;

	for (int x = 0; x < Num(); x++) {

		// Find current edge for wall_polygon
		FVector2D a_start = vertices[x].pos;
		FVector2D a_end = vertices[(x + 1) % Num()].pos;

		float x1 = a_start.X;
		float y1 = a_start.Y;
		float x2 = a_end.X;
		float y2 = a_end.Y;

		// This is magic idk what is going on here
		if ((point.Y < y1) != (point.Y < y2)) {
			if (point.X < (x1 + ((point.Y - y1) / (y2 - y1)) * (x2 - x1))) {
				overlaps += 1;
			}
		}
	}

	return overlaps % 2 == 1;
}

Polygon::Vertex* Polygon::getVertex(int index){
	verify(vertices.IsValidIndex(index));
	return &vertices[index];
}

int Polygon::Num() const{
	return vertices.Num();
}

int Polygon::Insert(Vertex x, int index) {
	return vertices.Insert(x, index);
}

void Polygon::Add(Vertex x) {
	vertices.Add(x);
}

void Polygon::Empty() {
	vertices.Empty();
}

bool Polygon::IsValidIndex(int index) const{
	return vertices.IsValidIndex(index);
}

bool Polygon::IsEmpty() const{
	return vertices.IsEmpty();
}