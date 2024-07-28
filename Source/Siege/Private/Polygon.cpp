// Fill out your copyright notice in the Description page of Project Settings.


#include "Polygon.h"

Polygon::~Polygon()
{
	Empty();
}

void Polygon::printPolygon() const{
	Vertex* currentVertex = HeadNode;
	FString out;

	out.AppendInt(Num());

	do{
		out += currentVertex->ToString() + "<->";
		currentVertex = currentVertex->NextNode;
	} while (currentVertex != HeadNode);

	UE_LOG(LogTemp, Warning, TEXT("%s"), *out);
}

bool Polygon::pointInsidePolygon(FVector2D const& point) const{

	// Follows algorithm presented below, finds it point is inside polygon
	// https://www.youtube.com/watch?v=RSXM9bgqxJM&list=LL&index=1

	int overlaps = 0;
	
	Vertex* currentVertex = HeadNode;

	while(currentVertex->NextNode != HeadNode){

		// Find current edge for wall_polygon
		FVector2D a_start = currentVertex->data.pos;
		FVector2D a_end = currentVertex->NextNode->data.pos;

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

Polygon::Vertex* Polygon::Insert(VertexData x, Vertex* nodeBeforex){

	if (nodeBeforex == nullptr) return nullptr;

	if (nodeBeforex == TailNode) {
		return Add(x);
	}
	else {

		Vertex* newVertex = new Vertex(x);

		// Insert new node

		// Hook up new node
		newVertex->PrevNode = nodeBeforex;
		newVertex->NextNode = nodeBeforex->NextNode;

		// Hook up neighbors
		nodeBeforex->NextNode = newVertex;
		nodeBeforex->NextNode->PrevNode = newVertex;

		size++;
		return newVertex;
	}
}

// Append to end
Polygon::Vertex* Polygon::Add(VertexData x) {

	Vertex* newVertex = new Vertex(x);

	if (size == 0) {
		HeadNode = TailNode = newVertex;
	}
	else {
		// Add new Tail

		// Hook up new node
		newVertex->NextNode = HeadNode;
		newVertex->PrevNode = TailNode;

		// Connect neighbors
		TailNode->NextNode = newVertex;
		HeadNode->PrevNode = newVertex;

		// Change Tail
		TailNode = newVertex;
	}

	size++;
	return newVertex;
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