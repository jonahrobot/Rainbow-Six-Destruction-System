// Fill out your copyright notice in the Description page of Project Settings.

#include "MathLib.h"
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
		// For A -> B need to do b-a
		// Project our point onto our edge
		FVector2D a = point - a_start; // Start to point
		FVector2D b = a_end - a_start; // Start to end

		float x = FVector2D::DotProduct(a, b) / b.Length();

		b.Normalize();

		FVector2D projectedPoint = (b * x) + a_start;

		double space = FVector2D::Distance(projectedPoint, point);

		if (space < 0.001 && x > 0) {
			return true;
		}

		//// Is point Y between the two line ends?
		if ((point.Y < y1) != (point.Y < y2)) {

			// Is point X to the left of the line
			if (point.X <= (x1 + ((point.Y - y1) / (y2 - y1)) * (x2 - x1))) {
				overlaps += 1;
			}
		}
	}

	//UE_LOG(LogTemp, Warning, TEXT("Overlaps: %d"), overlaps);

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

	Polygon oldRef = *this;

	//UE_LOG(LogTemp, Warning, TEXT("Triangulated Polygon"));

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

		//FVector test_scale = FVector(100, 250, 250);
		//FVector test_origin = FVector(-1360, 1210, 300);
		//FRotator test_rotation = FRotator(0, 0, 0);

		//FVector a = MathLib::LocalToGlobal(currentNode->data.pos, test_origin, test_rotation, test_scale.X);
		//FVector b = MathLib::LocalToGlobal(currentNode->NextNode->data.pos, test_origin, test_rotation, test_scale.X);
		//FVector c = MathLib::LocalToGlobal(currentNode->PrevNode->data.pos, test_origin, test_rotation, test_scale.X);

		//DrawDebugLine(GWorld, a, b, FColor::Orange, true, -1.0f, 0, 10.0f);
		//DrawDebugLine(GWorld, b, c, FColor::Orange, true, -1.0f, 0, 10.0f);
		//DrawDebugLine(GWorld, c, a, FColor::Orange, true, -1.0f, 0, 10.0f);

		triangle.Add(currentNode->data);
		triangle.Add(currentNode->NextNode->data);
		triangle.Add(currentNode->PrevNode->data);

		FVector2D start = currentNode->NextNode->data.pos;
		FVector2D end = currentNode->PrevNode->data.pos;

		FVector2D midPoint = ((start.X + end.X) / 2, (start.Y + end.Y) / 2);
		
		if (oldRef.pointInsidePolygon(midPoint) == false) {
			currentNode = next;
			continue;
		}

		// Check if Current Node is in or out of region
		FVector2D center_to_left = currentNode->NextNode->data.pos - currentNode->data.pos;
		FVector2D center_to_right = currentNode->PrevNode->data.pos - currentNode->data.pos;

		if (triangle.isPolygonClockwise() == false) {
			triangle.flipPolygonVertexOrder();
		}

		/*if (FVector2D::CrossProduct(center_to_left, center_to_right) > 0) {
			currentNode = next;
			continue;
		}*/

		bool isValidTriangle = true;

		// Check if any point is inside the triangle
		for (Polygon::Vertex* other : *this) {
			FVector2D checkingPos = other->data.pos;
			if (checkingPos != currentNode->data.pos && checkingPos != currentNode->NextNode->data.pos && checkingPos != currentNode->PrevNode->data.pos) { // failing because close data
				if (triangle.pointInsidePolygon(other->data.pos)) {
					UE_LOG(LogTemp, Warning, TEXT("Point was inside triangle -------"));
					triangle.printPolygon("Triangle in question");
					UE_LOG(LogTemp, Warning, TEXT("Point found inside: X=%f, Y=%f"), checkingPos.X, checkingPos.Y);
					UE_LOG(LogTemp, Warning, TEXT("-------"));

					isValidTriangle = false;
					break;
				}
			}
		}

		// If triangle -> Convert to Int array structure
		if (isValidTriangle) {

			//UE_LOG(LogTemp, Warning, TEXT("Triangle added to render"));

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

bool Polygon::isPolygonClockwise() {

	float area = 0.0f;

	for (Vertex* other : *this) {

		FVector2d a = other->data.pos;
		FVector2d b = other->NextNode->data.pos;

		float width = b.X - a.X;	
		float height = (b.Y + a.Y) / 2.0f;

		area += width * height;

	};

	return area >= 0;
}

void Polygon::flipPolygonVertexOrder() {

	Vertex* start = HeadNode;
	Vertex* appendAfter = TailNode;

	for (Vertex* currentVertex : *this) {
	
		if (currentVertex->data == start->data) continue; // Skip start
		if (currentVertex->data == appendAfter->data) return; // End at appendAfter node

		VertexData caughtData = currentVertex->data;
		Remove(currentVertex);

		Insert(caughtData, appendAfter);
	}
}
