// Jonah Ryan 2024 Rainbow Six Siege Explosive System

#include "Wall_Cutter.h"
#include "MathLib.h"
#include "Polygon.h"
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

		FirstLocalPlayer->InputComponent->BindAction(FName("Explode"), IE_Pressed, this, &UWall_Cutter::cutWall);

	}

	actor_scale = GetOwner()->GetActorScale() / 2 * 100;
	actor_origin = GetOwner()->GetActorLocation();
	actor_rotation = GetOwner()->GetActorRotation();

	wall_polygon.Empty();
	cut_polygon.Empty();

	wall_shape.Add(FVector2D(actor_scale.Y, actor_scale.Z));
	wall_shape.Add(FVector2D(-actor_scale.Y, actor_scale.Z));
	wall_shape.Add(FVector2D(-actor_scale.Y, -actor_scale.Z));
	wall_shape.Add(FVector2D(actor_scale.Y, -actor_scale.Z));
}

#pragma endregion Setup

#pragma region Helper Methods

void UWall_Cutter::addCutPoint(FVector2D const& PointToAdd) {
	cut_shape.Add(PointToAdd);
}


// Get intercept type, ENTER or EXIT
// @return ENTER if intercept is entering wall_polyogn 
// @return EXIT if intercept leaving wall_polygon
Polygon::InterceptTypes UWall_Cutter::getInterceptType(FVector2D const& intercept_point, FVector2D const& next_point) {

	// Get point 1 unit on path from intercept -> next point on cut_polygon
	FVector2D dir = (next_point - intercept_point);
	dir.Normalize(0.01f);
	dir = intercept_point + dir;

	FVector global = MathLib::LocalToGlobal(dir, actor_origin, actor_rotation, actor_scale.X);
	DrawDebugSphere(GetWorld(), global, 15, 10, FColor::Orange, true, -1.0f,-1);

	bool nextPointInWall = wall_polygon.pointInsidePolygon(dir);

	if (nextPointInWall) {
		return Polygon::ENTRY;
	}
	
	return Polygon::EXIT;
}

#pragma endregion Helper Methods

/*
 *	Cut polygon from wall
 *	Updates mesh Wall_Cutter component is attached too
 *	
 *	@cut_shape the polygon we want to cut from the wall
 *		- cut_shape vertices must be ordered clockwise
 */
void UWall_Cutter::cutWall() {

	// Follows Weiler-Atherton polygon clipping algorithm

	// Create polygon lists

	wall_polygon.Empty(); cut_polygon.Empty();

	for (FVector2D const x : wall_shape) {
		Polygon::Vertex current = { x,Polygon::NONE,NULL };
		wall_polygon.Add(current);
	}

	for (FVector2D const x : cut_shape) {
		Polygon::Vertex current = { x,Polygon::NONE,NULL };
		cut_polygon.Add(current);
	}

	// Find intersections between the two polygons
	// Then add them to the new wall_polygon and cut_polygon

	Polygon wall_polygon_saved = wall_polygon;
	Polygon cut_polygon_saved = cut_polygon;

	int total_added_to_wall = 0;
	int total_added_to_cut = 0;

	for (int x = 0; x < wall_polygon_saved.Num(); x++) {
		for (int y = 0; y < cut_polygon_saved.Num(); y++) {

			// Find current edge for wall_polygon
			FVector2D a_start = wall_polygon_saved.getVertex(x)->pos;
			FVector2D a_end = wall_polygon_saved.getVertex((x + 1) % wall_polygon_saved.Num())->pos;
			MathLib::EDGE a = { a_start, a_end };

			// Find current edge for cut_polygon
			FVector2D b_start = cut_polygon_saved.getVertex(y)->pos;
			FVector2D b_end = cut_polygon_saved.getVertex((y + 1) % cut_polygon_saved.Num())->pos;
			MathLib::EDGE b = { b_start, b_end };

			FVector2D out;
			bool found_intersept = MathLib::Find_Intersection(out, a, b);

			if (!found_intersept) continue;

			// If intercept found -

			Polygon::Vertex add_to_wall;
			Polygon::Vertex add_to_cut;

			add_to_wall.pos = out;
			add_to_cut.pos = out;

			// Check if Intercept is ENTRY or EXIT
			Polygon::InterceptTypes intercept_type = getInterceptType(out, b_end);

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
			if (intercept_type == Polygon::ENTRY) {
				wall_polygon.getVertex(x + total_added_to_wall)->intercept_index = y + total_added_to_cut;
			}

			// (EXIT links cut_polygon -> wall_polygon) 
			if (intercept_type == Polygon::EXIT) {
				cut_polygon.getVertex(y + total_added_to_cut)->intercept_index = x + total_added_to_wall;
			}
		}
	}

	// At this point we have a wall_polygon and cut_polygon, with intercepts in correct order, pointed and labeled!

	// Find our resulting destruction piece polygons

	regions.Empty();
	TArray<Polygon::Vertex> visited;
	
	bool clockwise = true;

	// Find Direction
	for (int i = 0; i < cut_polygon.Num(); i++) {
		Polygon::Vertex x = *cut_polygon.getVertex(i);
		if (visited.Contains(x) == false && x.type == Polygon::EXIT) {
			int trash;
			Polygon::Vertex firstStepClockwise = getNextNode(trash, x.intercept_index, false, true);

			if (cut_polygon.pointInsidePolygon(firstStepClockwise.pos)) {
				clockwise = false;
			}
			break;
		}
	}

	for (int i = 0; i < cut_polygon.Num(); i++) {
		Polygon::Vertex x = *cut_polygon.getVertex(i);

		if (visited.Contains(x) == false && x.type == Polygon::ENTRY) {

			Polygon new_region = walkLoop(visited, x, i, clockwise);

			regions.Add(new_region);
		}
	}
}

Polygon UWall_Cutter::walkLoop(TArray<Polygon::Vertex> &OUT_visited, Polygon::Vertex const&  start, int indexOfVertex, bool clockwise) {

	Polygon loop;
	loop.Add(start);
	OUT_visited.Add(start);

	int currentIndex = indexOfVertex;
	bool in_cut_polygon = true;
	Polygon::Vertex x = getNextNode(currentIndex, currentIndex, true, clockwise);

	while (x.equals(start) == false){
		loop.Add(x);

		if (OUT_visited.Contains(x) && (x.type == Polygon::ENTRY || x.type == Polygon::EXIT)) {
			UE_LOG(LogTemp, Warning, TEXT("%s vs %s"), *x.pos.ToString(), *start.pos.ToString());
			UE_LOG(LogTemp, Error, TEXT("Infinite loop found when walking loop! %s vs %s"), *x.pos.ToString(), *start.pos.ToString());
			return loop;
		}

		OUT_visited.Add(x);

		if (x.intercept_index != -1 && (x.type == Polygon::ENTRY || x.type == Polygon::EXIT)) {

			currentIndex = x.intercept_index;

			if (in_cut_polygon) {
				int trash;
				Polygon::Vertex firstStepClockwise = getNextNode(trash, x.intercept_index, false, true);

				if (cut_polygon.pointInsidePolygon(firstStepClockwise.pos)) {
					clockwise = false;
				}
				x = *wall_polygon.getVertex(currentIndex);
			}
			else {
				x = *cut_polygon.getVertex(currentIndex);
			}

			in_cut_polygon = !in_cut_polygon;
		}

		x = getNextNode(currentIndex, currentIndex, in_cut_polygon, clockwise);
	}

	return loop;
}

Polygon::Vertex UWall_Cutter::getNextNode(int& OUT_new_index, int currentIndex, bool in_cut_polygon, bool goClockwise) {

	int indexChange = 1;
	if (goClockwise == false) indexChange = -1;

	if (in_cut_polygon) {

		if (cut_polygon.IsValidIndex(currentIndex + 1)) {
			OUT_new_index = currentIndex + 1;
			return *cut_polygon.getVertex(currentIndex + 1); // Last changed thing
		}
		else {
			OUT_new_index = 0;
			return *cut_polygon.getVertex(0);
		}
	}
	else {

		if (wall_polygon.IsValidIndex(currentIndex + indexChange)) {
			OUT_new_index = currentIndex + indexChange;
			return *wall_polygon.getVertex(currentIndex + indexChange);
		}
		else {
			if (goClockwise) {
				OUT_new_index = 0;
				return *wall_polygon.getVertex(0);
			}
			else {
				OUT_new_index = wall_polygon.Num()-1;
				return *wall_polygon.getVertex(wall_polygon.Num() - 1);
			}
		}
	}

}
