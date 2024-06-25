// Jonah Ryan 2024 Rainbow Six Siege Explosive System

#include "Wall_Cutter.h"
#include "MathLib.h"
#include "Kismet/GameplayStatics.h"

#pragma region Setup

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

		FirstLocalPlayer->InputComponent->BindAction(FName("Explode"), IE_Pressed, this, &UWall_Cutter::Start_Cut);

	}

	actorScale = GetOwner()->GetActorScale() / 2 * 100;
	actorOrigin = GetOwner()->GetActorLocation();
	actorRotation = GetOwner()->GetActorRotation();

	wall_polygon.Empty();
	cut_polygon.Empty();
}

#pragma endregion Setup

#pragma region Helper Methods

void UWall_Cutter::Start_Cut() {

	// -- For testing purposes --

	wall_shape.Empty();
	cut_shape.Empty();
	wall_polygon.Empty();
	cut_polygon.Empty();

	// Create polygon size of wall surface
	wall_shape.Add(FVector2D(actorScale.Y, actorScale.Z));
	wall_shape.Add(FVector2D(-actorScale.Y, actorScale.Z));
	wall_shape.Add(FVector2D(-actorScale.Y, -actorScale.Z));
	wall_shape.Add(FVector2D(actorScale.Y, -actorScale.Z));

	// Polygon points to add
	cut_shape = {
		FVector2D(119,169),
		FVector2D(13,177),
		FVector2D(-59,176),
		FVector2D(-43,104),
		FVector2D(-110,113),
		FVector2D(-180,6),
		FVector2D(-110,-24),
		FVector2D(-80,9),
		FVector2D(-78,-92),
		FVector2D(-226,-158),
		FVector2D(-118,-192),
		FVector2D(-39,-107),
		FVector2D(27,-123),
		FVector2D(68,-200),
		FVector2D(92,-60),
		FVector2D(193,25),
		FVector2D(76,45), // Problem chilkd
		FVector2D(174,82) //173 - 174 
	};

	Cut_Wall();
}

// Get intercept type, ENTER or EXIT
// @return ENTER if intercept is entering wall_polyogn
// @return EXIT if intercept leaving wall_polygon
UWall_Cutter::node_type UWall_Cutter::get_intercept_type(FVector2D intercept_point, FVector2D next_point) {

	// Follows algorithm presented below, finds it point is inside polygon
	// https://www.youtube.com/watch?v=RSXM9bgqxJM&list=LL&index=1

	// Get point 1 unit on path from intercept -> next point on cut_polygon
	FVector2D dir = (next_point - intercept_point);
	dir.Normalize(0.01f);
	dir = intercept_point + dir;

	FVector global = MathLib::LocalToGlobal(dir, actorOrigin, actorRotation, actorScale.X);
	DrawDebugSphere(GetWorld(), global, 15, 10, FColor::Orange, true, -1.0f,-1);


	int overlaps = 0;

	for (int x = 0; x < wall_shape.Num(); x++) {
		
		// Find current edge for wall_polygon
		FVector2D a_start = wall_shape[x];
		FVector2D a_end = wall_shape[(x + 1) % wall_shape.Num()];

		float x1 = a_start.X;
		float y1 = a_start.Y;
		float x2 = a_end.X;
		float y2 = a_end.Y;

		// This is magic idk what is going on here
		if ((dir.Y < y1) != (dir.Y < y2)) {
			if (dir.X < (x1 + ((dir.Y - y1) / (y2 - y1)) * (x2 - x1))) {
				overlaps += 1;
			}
		}
	}
	UE_LOG(LogTemp, Log, TEXT("OVerlaps: %d"), overlaps);

	// Check if overlaps is odd number, in that case, point is inside polygon
	// If point is inside polygon, intercept is entering
	if (overlaps % 2 == 1) {
		return INTERCEPT_ENTRY;
	}
	
	// Otherwise we are leaving the polygon
	return INTERCEPT_EXIT;
}

#pragma endregion Helper Methods

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

	TArray<POLYGON_NODE> wall_polygon_saved = wall_polygon;
	TArray<POLYGON_NODE> cut_polygon_saved = cut_polygon;

	int total_added_to_wall = 0;
	int total_added_to_cut = 0;

	for (int x = 0; x < wall_polygon_saved.Num(); x++) {
		for (int y = 0; y < cut_polygon_saved.Num(); y++) {

			// Find current edge for wall_polygon
			FVector2D a_start = wall_polygon_saved[x].pos;
			FVector2D a_end = wall_polygon_saved[(x + 1) % wall_polygon_saved.Num()].pos;
			MathLib::EDGE a = { a_start, a_end };

			// Find current edge for cut_polygon
			FVector2D b_start = cut_polygon_saved[y].pos;
			FVector2D b_end = cut_polygon_saved[(y + 1) % cut_polygon_saved.Num()].pos;
			MathLib::EDGE b = { b_start, b_end };

			FVector2D out;
			bool found_intersept = MathLib::Find_Intersection(out, a, b);

			if (!found_intersept) continue;

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

			// Add intercept to each polygon

			int indexWall = wall_polygon.Insert(add_to_wall, x + 1 + total_added_to_wall);
			total_added_to_wall++;
			int indexCut = cut_polygon.Insert(add_to_cut, y + 1 + total_added_to_cut);
			total_added_to_cut++;

			// Link up!

			// (ENTRY links wall_polygon -> cut_polygon) 
			if (intercept_type == INTERCEPT_ENTRY) {
				wall_polygon[x + total_added_to_wall].intercept_index = y + total_added_to_cut;
			}

			// (EXIT links cut_polygon -> wall_polygon) 
			if (intercept_type == INTERCEPT_EXIT) {
				cut_polygon[y + total_added_to_cut].intercept_index = x + total_added_to_wall;
			}
		}
	}

	// At this point we have a wall_polygon and cut_polygon, with intercepts in correct order, pointed and labeled!
}