// Jonah Ryan 2024 Rainbow Six Siege Explosive System



#include "Wall_Cutter.h"
#include "Kismet/GameplayStatics.h"


enum node_type { DEFAULT, INTERCEPT_ENTRY, INTERCEPT_EXIT };

TArray<FString> node_type_names {"-", "Entry", "Exit"};

struct POLYGON_NODE {
	FVector2D pos;
	node_type type;

	// Marks where W-A (Weiler-Atherton) Algorithm goes for intercepts
	POLYGON_NODE* intercept_pointer;
};

struct EDGE {
	FVector2D start;
	FVector2D end;
};

TArray<POLYGON_NODE> wall_polygon;
TArray<POLYGON_NODE> cut_polygon;


// Sets default values for this component's properties
UWall_Cutter::UWall_Cutter()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UWall_Cutter::BeginPlay()
{
	Super::BeginPlay();

	// -- For testing purposes --

	APlayerController* FirstLocalPlayer = UGameplayStatics::GetPlayerController(this, 0);

	if (IsValid(FirstLocalPlayer) && IsValid(FirstLocalPlayer->InputComponent)) {

		FirstLocalPlayer->InputComponent->BindAction(FName("Explode"), IE_Pressed, this, &UWall_Cutter::Test_Input_Triggered);


		FirstLocalPlayer->InputComponent->BindAction(FName("Draw_Wall_Poly_Input"), IE_Pressed, this, &UWall_Cutter::Draw_Wall_Poly);
		FirstLocalPlayer->InputComponent->BindAction(FName("Draw_Cut_Poly_Input"), IE_Pressed, this, &UWall_Cutter::Draw_Cut_Poly);
		FirstLocalPlayer->InputComponent->BindAction(FName("Draw_Wall_Intercepts_Input"), IE_Pressed, this, &UWall_Cutter::Draw_Wall_Intercepts);
		FirstLocalPlayer->InputComponent->BindAction(FName("Draw_Cut_Intercepts_Input"), IE_Pressed, this, &UWall_Cutter::Draw_Cut_Intercepts);

	}

	actorScale = GetOwner()->GetActorScale() / 2 * 100;
	actorOrigin = GetOwner()->GetActorLocation();
	actorRotation = GetOwner()->GetActorRotation();
}


#pragma region Helper Methods

void UWall_Cutter::Test_Input_Triggered() {

	// -- For testing purposes --

	// Create polygon size of wall surface
	wall_shape.Add(FVector2D(actorScale.Y, actorScale.Z));
	wall_shape.Add(FVector2D(-actorScale.Y, actorScale.Z));
	wall_shape.Add(FVector2D(-actorScale.Y, -actorScale.Z));
	wall_shape.Add(FVector2D(actorScale.Y, -actorScale.Z));

	// Create polygon that we will cut from wall
	cut_shape.Add(FVector2D(-300, 0));
	cut_shape.Add(FVector2D(0, -300));
	cut_shape.Add(FVector2D(300, 0));
	cut_shape.Add(FVector2D(0, 300));

	Cut_Wall();
}

// Returns true if X is between bound_A and bound_B
bool check_in_range(float bound_A, float bound_B, float x) {

	// Find our bounds
	float lowerBound = std::floorf(std::min(bound_A, bound_B));
	float upperBound = std::floorf(std::max(bound_A, bound_B));

	// Check if x in bounds
	return (lowerBound <= std::floorf(x)) && (std::floorf(x) <= upperBound);
}

// Converts Edge to line represented in (ax + by = c)
void edge_to_line_standard_form(float& a, float& b, float& c, EDGE e) {
	a = -e.end.Y + e.start.Y;
	b = e.end.X - e.start.X;
	c = -(a * e.start.X + b * e.start.Y);
}

// Find Intersection between two edges
// @return true if intersection found, passed in out
// @return false if no intersection found
bool Find_Intersection(FVector2D& out, EDGE edge_a, EDGE edge_b) {

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
	printf("Found intercept");
	return true;
}

// Get intercept type, ENTER or EXIT
// @return ENTER if intercept is entering wall_polyogn
// @return EXIT if intercept leaving wall_polygon
node_type get_intercept_type(FVector2D intercept_point, FVector2D next_point) {

	// Follows algorithm presented below, finds it point is inside polygon
	// https://www.youtube.com/watch?v=RSXM9bgqxJM&list=LL&index=1

	// Get point 1 unit on path from intercept -> next point on cut_polygon
	FVector2D dir = (next_point - intercept_point);
	dir.Normalize(0.01f);
	dir = intercept_point + dir;

	int overlaps = 0;

	for (int x = 0; x < wall_polygon.Num(); x++) {
		
		// Find current edge for wall_polygon
		FVector2D a_start = wall_polygon[x].pos;
		FVector2D a_end = wall_polygon[(x + 1) % wall_polygon.Num()].pos;

		float x1 = a_start.X;
		float y1 = a_start.Y;
		float x2 = a_end.X;
		float y2 = a_end.Y;

		// This is magic idk what is going on here
		if ((intercept_point.Y < y1) != (intercept_point.Y < y2)) {
			if (intercept_point.X < (x1 + ((intercept_point.Y - y1) / (y2 - y1)) * (x2 - x1))) {
				overlaps += 1;
			}
		}
	}

	// Check if overlaps is odd number, in that case, point is inside polygon
	// If point is inside polygon, intercept is entering
	if (overlaps % 2 == 1) {
		return INTERCEPT_ENTRY;
	}
	
	// Otherwise we are leaving the polygon
	return INTERCEPT_EXIT;
}

FVector LocalToGlobal(FVector2D LocalVector, FVector ActorOrigin, FRotator ActorRotation, float x) {

	FVector newVector = FVector(x, LocalVector.X, LocalVector.Y);
	return ActorRotation.RotateVector(newVector) + ActorOrigin;
}

#pragma endregion Helper Methods

#pragma region Debug Prints

void UWall_Cutter::Draw_Wall_Poly() {

	UKismetSystemLibrary::FlushPersistentDebugLines(GetWorld());
	UKismetSystemLibrary::FlushDebugStrings(GetWorld());

	if (GEngine) {
		GEngine->ClearOnScreenDebugMessages();
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, "Showing: Wall Polygon");	
	}

	for (FVector2D const &local : wall_shape) {
		FVector global = LocalToGlobal(local, actorOrigin, actorRotation, actorScale.X);
		DrawDebugSphere(GetWorld(), global, 25, 5, FColor::Blue, true, -1.0f);
	}
}

void UWall_Cutter::Draw_Cut_Poly() {

	UKismetSystemLibrary::FlushPersistentDebugLines(GetWorld());
	UKismetSystemLibrary::FlushDebugStrings(GetWorld());

	if (GEngine) {
		GEngine->ClearOnScreenDebugMessages();
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Showing: Cut Polygon");
	}

	for (FVector2D const &local : cut_shape) {
		FVector global = LocalToGlobal(local, actorOrigin, actorRotation, actorScale.X);
		DrawDebugSphere(GetWorld(), global, 25, 5, FColor::Red, true, -1.0f);
	}
}

void UWall_Cutter::Draw_Wall_Intercepts() {

	// Clear Drawings
	UKismetSystemLibrary::FlushPersistentDebugLines(GetWorld());
	UKismetSystemLibrary::FlushDebugStrings(GetWorld());

	// Show Label
	if (GEngine) {
		GEngine->ClearOnScreenDebugMessages();
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, "Showing: Wall + Intercepts");
	}

	// Draw Nodes
	for (POLYGON_NODE const &local : wall_polygon) {

		FVector global = LocalToGlobal(local.pos, actorOrigin, actorRotation, actorScale.X);
		DrawDebugSphere(GetWorld(), global, 25, 3, FColor::Green, true, -1.0f);

		// Draw vector type
		FString Text = node_type_names[local.type];
		FVector vector = actorRotation.RotateVector(FVector(100, local.pos.X, local.pos.Y - 30));

		DrawDebugString(GetWorld(), vector, Text, GetOwner(), FColor::Green, -1.f, false, 2.0f);
	}

}

void UWall_Cutter::Draw_Cut_Intercepts() {

	UKismetSystemLibrary::FlushPersistentDebugLines(GetWorld());
	UKismetSystemLibrary::FlushDebugStrings(GetWorld());

	if (GEngine) {
		GEngine->ClearOnScreenDebugMessages();
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Orange, "Showing: Cut + Intercepts");
	}

	for (POLYGON_NODE const &local : cut_polygon) {
		FVector global = LocalToGlobal(local.pos, actorOrigin, actorRotation, actorScale.X);
		DrawDebugSphere(GetWorld(), global, 25, 3, FColor::Orange, true, -1.0f);

		// Draw vector type
		FString Text = node_type_names[local.type];
		FVector vector = actorRotation.RotateVector(FVector(100, local.pos.X, local.pos.Y - 30));

		DrawDebugString(GetWorld(), vector, Text, GetOwner(), FColor::Orange, -1.f, false, 2.0f);
	}
}

#pragma endregion Debug Prints

/*
 *	Cut polygon from wall
 *	Updates mesh Wall_Cutter component is attached too
 *	
 *	@cut_shape the polygon we want to cut from the wall
 *		- cut_shape vertices must be ordered clockwise
 */
void UWall_Cutter::Cut_Wall() {

	// Follows Weiler-Atherton polygon clipping algorithm
	
	// Create polygon lists

	wall_polygon.Empty(); cut_polygon.Empty();

	for (FVector2D const x : wall_shape) {
		POLYGON_NODE current = {x,DEFAULT,NULL};
		wall_polygon.Add(current);
	}

	for (FVector2D const x : cut_shape) {
		POLYGON_NODE current = { x,DEFAULT,NULL };
		cut_polygon.Add(current);
	}

	// Find intersections between the two polygons
	// Then add them to the new wall_polygon and cut_polygon

	TArray<POLYGON_NODE> wall_polygon_intersections = wall_polygon;
	TArray<POLYGON_NODE> cut_polygon_intersections = cut_polygon;

	for (int x = 0; x < wall_polygon.Num(); x++) {
		for (int y = 0; y < cut_polygon.Num(); y++) {

			// Find current edge for wall_polygon
			FVector2D a_start = wall_polygon[x].pos;
			FVector2D a_end = wall_polygon[(x + 1) % wall_polygon.Num()].pos;
			EDGE a = { a_start, a_end };

			// Find current edge for cut_polygon
			FVector2D b_start = cut_polygon[y].pos;
			FVector2D b_end = cut_polygon[(y + 1) % cut_polygon.Num()].pos;
			EDGE b = { b_start, b_end };

			FVector2D out;
			bool found_intersept = Find_Intersection(out, a, b);

			if (!found_intersept) break;

			// If intercept found -

			POLYGON_NODE add_to_wall;
			POLYGON_NODE add_to_cut;

			add_to_wall.pos = out;
			add_to_cut.pos = out;

			// Check if Intercept is ENTRY or EXIT
			node_type intercept_type = get_intercept_type(out, b_end);

			add_to_wall.type = intercept_type;
			add_to_cut.type = intercept_type;

			// Create intercept link (ENTRY links in wall_polygon) (EXIT links in cut_polygon)

			// (ENTRY links wall_polygon -> cut_polygon) 
			if (intercept_type == INTERCEPT_ENTRY) {
				add_to_wall.intercept_pointer = &add_to_cut;
			}

			// (EXIT links cut_polygon -> wall_polygon) 
			if (intercept_type == INTERCEPT_EXIT) {
				add_to_cut.intercept_pointer = &add_to_wall;
			}

			// Add intercept to each polygon

			wall_polygon_intersections.Insert(add_to_wall, x + 1);
			cut_polygon_intersections.Insert(add_to_cut, y + 1);
		}
	}

	wall_polygon = wall_polygon_intersections;
	cut_polygon = cut_polygon_intersections;

	// At this point we have a wall_polygon and cut_polygon, with intercepts in correct order, pointed and labeled!
}