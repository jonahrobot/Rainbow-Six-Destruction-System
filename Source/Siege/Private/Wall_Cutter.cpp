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

	start_wall_polygon.Add(new Polygon::Vertex(FVector2D(actor_scale.Y, actor_scale.Z),Polygon::NONE));
	start_wall_polygon.Add(new Polygon::Vertex(FVector2D(-actor_scale.Y, actor_scale.Z),Polygon::NONE));
	start_wall_polygon.Add(new Polygon::Vertex(FVector2D(-actor_scale.Y, -actor_scale.Z),Polygon::NONE));
	start_wall_polygon.Add(new Polygon::Vertex(FVector2D(actor_scale.Y, -actor_scale.Z),Polygon::NONE));
}

#pragma endregion Setup

#pragma region Helper Methods

void UWall_Cutter::addCutPoint(FVector2D const& PointToAdd) {
	start_cut_polygon.Add(new Polygon::Vertex(PointToAdd, Polygon::NONE));
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

	wall_polygon = start_wall_polygon;
	cut_polygon = start_cut_polygon;

	// Find intersections between the two polygons
	// Then add them to the new wall_polygon and cut_polygon

	int total_added_to_wall = 0;
	int total_added_to_cut = 0;

	Polygon::Vertex* wall_vertex = wall_polygon.HeadNode;
	Polygon::Vertex* cut_vertex = cut_polygon.HeadNode;
	

	while(wall_vertex->NextNode != wall_polygon.HeadNode){

		while (cut_vertex->NextNode != cut_polygon.HeadNode) {


			// Find current edge for wall_polygon
			MathLib::EDGE a = { wall_vertex->pos, wall_vertex->NextNode->pos };

			// Find current edge for cut_polygon
			MathLib::EDGE b = { cut_vertex->pos, cut_vertex->NextNode->pos };

			FVector2D out;
			bool found_intersept = MathLib::Find_Intersection(out, a, b);

			if (!found_intersept) {
				cut_vertex = cut_vertex->NextNode;
				continue;
			}

			// If intercept found -

			Polygon::InterceptTypes intercept_type = getInterceptType(out, b.end);

			Polygon::Vertex* add_to_wall = new Polygon::Vertex(out,intercept_type);
			Polygon::Vertex* add_to_cut = new Polygon::Vertex(out, intercept_type);

			// Create intercept link (ENTRY links in wall_polygon) (EXIT links in cut_polygon)
			if(intercept_type == Polygon::ENTRY) add_to_wall->intercept_link = add_to_cut;
			if (intercept_type == Polygon::EXIT) add_to_cut->intercept_link = add_to_wall;

			// Add intercept to each polygon
			wall_polygon.Insert(add_to_wall, wall_vertex);
			cut_polygon.Insert(add_to_cut, cut_vertex);

			cut_vertex = cut_vertex->NextNode;
			cut_vertex = cut_vertex->NextNode;
		}
		wall_vertex = wall_vertex->NextNode;
	}

	// At this point we have a wall_polygon and cut_polygon, with intercepts in correct order, pointed and labeled!

	// Find our resulting destruction piece polygons

	regions.Empty();
	TArray<Polygon::Vertex> visited;
	
	int clockwise = 1;

	// Find Direction

	Polygon::Vertex* currentVertex = cut_polygon.HeadNode;

	while (currentVertex->NextNode != cut_polygon.HeadNode) {

		if (visited.Contains(*currentVertex) == false && currentVertex->type == Polygon::ENTRY) {

			Polygon new_region = walkLoop(visited, currentVertex, clockwise);

			regions.Add(new_region);
		}
	}
}

Polygon UWall_Cutter::walkLoop(TArray<Polygon::Vertex> &OUT_visited, Polygon::Vertex*  start, int direction) {

	Polygon loop;
	loop.Add(start);
	OUT_visited.Add(*start);

	bool in_cut_polygon = true;
	Polygon::Vertex* x = start->NextNode;

	while (x->equals(*start) == false){
		loop.Add(x);

		if (OUT_visited.Contains(*x) && (x->type == Polygon::ENTRY || x->type == Polygon::EXIT)) {
			UE_LOG(LogTemp, Warning, TEXT("%s vs %s"), *x->pos.ToString(), *start->pos.ToString());
			UE_LOG(LogTemp, Error, TEXT("Infinite loop found when walking loop! %s vs %s"), *x->pos.ToString(), *start->pos.ToString());
			return loop;
		}

		OUT_visited.Add(*x);

		if (x->intercept_link != nullptr && (x->type == Polygon::ENTRY || x->type == Polygon::EXIT)) {

			x = x->intercept_link;

			if (in_cut_polygon) {
				if (cut_polygon.pointInsidePolygon(x->NextNode->pos)) {
					direction = -1;
				}
			}

			in_cut_polygon = !in_cut_polygon;
		}
		if (direction == -1) {
			x = x->PrevNode;
		}
		else {
			x = x->NextNode;
		}
	}

	return loop;
}