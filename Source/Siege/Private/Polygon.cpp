// Fill out your copyright notice in the Description page of Project Settings.


#include "Polygon.h"

Polygon::~Polygon()
{
	Empty();
}

void Polygon::initalizeWithString(FString polygonString) {

	polygonString.RemoveSpacesInline();

	FRegexPattern Pattern("\\((-?\\d+(?:.\\d+)?),(-?\\d+(?:.\\d+)?)(?:,([^)]+))?\\)");
	FRegexMatcher Matcher(Pattern, polygonString);

	while (Matcher.FindNext()) {

		float x = FCString::Atof(*Matcher.GetCaptureGroup(1));
		float y = FCString::Atof(*Matcher.GetCaptureGroup(2));

		FString type = Matcher.GetCaptureGroup(3);

		VertexData newVertex = { FVector2D(x, y) };

		if (type == "ENTRY") {
			newVertex.type = ENTRY;
		}
		if (type == "EXIT") {
			newVertex.type = EXIT;
		}

		Add(newVertex);
	}
}


void Polygon::printPolygon(FString title) const{
	Vertex* currentVertex = HeadNode;
	FString out;

	out += title + " Size:";
	out.AppendInt(Num());
	out += " ";

	do{
		out += currentVertex->ToString() + ",";
		currentVertex = currentVertex->NextNode;
	} while (currentVertex != HeadNode);

	UE_LOG(LogTemp, Warning, TEXT("%s"), *out);
}

bool Polygon::pointInsidePolygon(FVector2D const& point){

	// Follows algorithm presented below, finds it point is inside polygon
	// https://www.youtube.com/watch?v=RSXM9bgqxJM&list=LL&index=1

	int overlaps = 0;

	for (PolygonIterator itr = begin(); itr != end(); itr++) {
		Vertex* currentVertex = *itr;

		// Find current edge for wall_polygon
		FVector2D a_start = currentVertex->data.pos;
		FVector2D a_end = currentVertex->NextNode->data.pos;

		float x1 = a_start.X;
		float y1 = a_start.Y;
		float x2 = a_end.X;
		float y2 = a_end.Y;

		// Check if point is on line edge
		FVector2D directionOfLine = a_end - a_start;
		FVector2D pointProjectionToLine = a_start + ((FVector2D::DotProduct(point - a_start, directionOfLine) / (directionOfLine.SizeSquared()) * directionOfLine));
		if (FVector2D::Distance(pointProjectionToLine,point) < 0.001) {
			return true;
		}

		// This is magic idk what is going on here
		if ((point.Y < y1) != (point.Y < y2)) {
			if (point.X < (x1 + ((point.Y - y1) / (y2 - y1)) * (x2 - x1))) {
				overlaps += 1;
			}
		}
	}

	return overlaps % 2 == 1;
}

bool Polygon::extrudePolygon(float length, TArray<FVector>& out_vertices, FJsonSerializableArrayInt& out_triangles, TArray<FVector2D>& in_vertices, FJsonSerializableArrayInt& in_triangles) {
	
	// Set up
	out_triangles = in_triangles;

	// Create 3D Points
	for (FVector2D y : in_vertices) {
		out_vertices.Add(FVector(length, y.X, y.Y));
	}

	// Add other face
	FJsonSerializableArrayInt tri;

	for (int32 y : in_triangles) {
		tri.Add(y + in_vertices.Num());
		if (tri.Num() == 3) {
			for (int i = tri.Num() - 1; i >= 0; i--) {
				out_triangles.Add(tri[i]);
			}
			tri.Empty();
		}
	}
	
	for (FVector2D y : in_vertices) {
		out_vertices.Add(FVector(-length, y.X, y.Y));
	}

	// Add connecting faces
	int x = in_vertices.Num() - 1;
	for (int y = 0; y < in_vertices.Num(); y++) {

		out_triangles.Add(x);
		out_triangles.Add(y + in_vertices.Num());
		out_triangles.Add(y);

		out_triangles.Add(y + in_vertices.Num());
		out_triangles.Add(x);
		out_triangles.Add(x + in_vertices.Num());

		x = y;
	}

	return true;
}

bool Polygon::triangulatePolygon(TArray<FVector2D>& out_vertices, FJsonSerializableArrayInt& out_triangles) {

	Polygon::Vertex* currentNode = HeadNode;

	// Convert polygon vertices 
	for (Polygon::Vertex* current_vertex : *this) {
		out_vertices.Add(current_vertex->data.pos);
	}

	// Create tris
	int failCase = 0;

	while (Num() >= 3 && failCase < 100) {

		failCase++;

		Polygon::Vertex* next = currentNode->NextNode;

		Polygon triangle;

		triangle.Add(currentNode->data);
		triangle.Add(currentNode->NextNode->data);
		triangle.Add(currentNode->PrevNode->data);

		// Check if Current Node is in or out of region
		FVector2D center_to_left = currentNode->NextNode->data.pos - currentNode->data.pos;
		FVector2D center_to_right = currentNode->PrevNode->data.pos - currentNode->data.pos;

		if (FVector2D::CrossProduct(center_to_left, center_to_right) > 0) {
			currentNode = next;
			continue;
		}

		bool isValidTriangle = true;

		// Check if any point is inside the triangle
		for (Polygon::Vertex* other : *this) {
			FVector2D checkingPos = other->data.pos;
			if (checkingPos != currentNode->data.pos && checkingPos != currentNode->NextNode->data.pos && checkingPos != currentNode->PrevNode->data.pos) {
				if (triangle.pointInsidePolygon(other->data.pos)) {
					isValidTriangle = false;
					break;
				}
			}
		}

		// If triangle -> Convert to Int array structure
		if (isValidTriangle) {
			// Remove current node from region
			Remove(currentNode);

			// Add triangle to out_triangles encoding it with our out_verticies

			for (Polygon::Vertex* vertexInTri : triangle) {

				// Find matching 
				int index = out_vertices.IndexOfByKey(vertexInTri->data.pos);

				if (index != INDEX_NONE) {
					out_triangles.Add(index);
				}
				else {
					UE_LOG(LogTemp, Warning, TEXT("Failed to add Tri"));
				}
			}
		}

		// Go to next vertex
		currentNode = next;
	}

	return failCase < 100;
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
		nodeBeforex->NextNode->PrevNode = newVertex; // Needs to move up one
		nodeBeforex->NextNode = newVertex;

		size++;
		return newVertex;
	}
}

// Append to end
Polygon::Vertex* Polygon::Add(VertexData x) {

	Vertex* newVertex = new Vertex(x);

	if (size == 0) {
		HeadNode = TailNode = newVertex;
		newVertex->NextNode = newVertex;
		newVertex->PrevNode = newVertex;
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

	while (Current != nullptr && Current != EndTarget)
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


void Polygon::Remove(Vertex* x) {

	if (size == 1 || size == 0) {
		Empty(); return;
	}

	// Hook up neighbors
	x->PrevNode->NextNode = x->NextNode;
	x->NextNode->PrevNode = x->PrevNode;

	// Hook up new Head or Tail
	if (x == TailNode) {
		TailNode = x->PrevNode;
	}

	if (x == HeadNode) {
		HeadNode = x->NextNode;
	}

	size -= 1;

	delete x;
}
