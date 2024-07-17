// Fill out your copyright notice in the Description page of Project Settings.


#include "Polygon.h"

Polygon::~Polygon()
{
	Empty();
}

bool Polygon::pointInsidePolygon(FVector2D const& point) const{

	// Follows algorithm presented below, finds it point is inside polygon
	// https://www.youtube.com/watch?v=RSXM9bgqxJM&list=LL&index=1

	int overlaps = 0;
	
	Vertex* currentVertex = HeadNode;

	while(currentVertex->NextNode != HeadNode){

		// Find current edge for wall_polygon
		FVector2D a_start = currentVertex->pos;
		FVector2D a_end = currentVertex->NextNode->pos;

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

		currentVertex = currentVertex->NextNode;
	}

	return overlaps % 2 == 1;
}

int Polygon::Num() const{
	return size;
}

void Polygon::Insert(Vertex* x, Vertex* nodeBeforex){

	if (x == nullptr || nodeBeforex == nullptr) return;

	if (nodeBeforex == TailNode) {
		Add(x);
	}
	else {
		// Insert new node

		// Hook up new node
		x->PrevNode = nodeBeforex;
		x->NextNode = nodeBeforex->NextNode;

		// Hook up neighbors
		nodeBeforex->NextNode = x;
		x->NextNode->PrevNode = x;
	}

	size++;
}

// Append to end
void Polygon::Add(Vertex* x) {

	if (x == nullptr) return;

	if (size == 0) {
		HeadNode = TailNode = x;
	}
	else {
		// Add new Tail

		// Hook up new node
		x->NextNode = HeadNode;
		x->PrevNode = TailNode;

		// Connect neighbors
		TailNode->NextNode = x;
		HeadNode->PrevNode = x;

		// Change Tail
		TailNode = x;
	}

	size++;
}

void Polygon::Empty() {

	if (size == 0) return;

	Vertex* Current = HeadNode;
	Vertex* EndTarget = TailNode;

	while (Current != EndTarget)
	{
		Vertex* DeleteNode = Current;
		Current = Current->NextNode;
		delete DeleteNode;
	}

	// Handle Tail node
	delete TailNode;

	HeadNode = TailNode = nullptr;
	size = 0;
}

bool Polygon::IsEmpty() const{
	return size == 0;
}