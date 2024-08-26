// Jonah Ryan 2024 Rainbow Six Siege Explosive System

#include "Wall_Cutter.h"
#include "MathLib.h"
#include "Polygon.h"
#include "Kismet/GameplayStatics.h"

#pragma region Setup

// Sets default values for this component's properties
UWall_Cutter::UWall_Cutter()
{

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

	wall_polygon_out.Empty();
	cut_polygon_out.Empty();

	start_wall_polygon.Add({ FVector2D(actor_scale.Y, actor_scale.Z), Polygon::NONE });
	start_wall_polygon.Add({ FVector2D(-actor_scale.Y, actor_scale.Z),Polygon::NONE });
	start_wall_polygon.Add({ FVector2D(-actor_scale.Y, -actor_scale.Z),Polygon::NONE });
	start_wall_polygon.Add({ FVector2D(actor_scale.Y, -actor_scale.Z),Polygon::NONE });

	//start_cut_polygon.Add({ FVector2D(actor_scale.Y + 20, -actor_scale.Z - 20), Polygon::NONE });
	//start_cut_polygon.Add({ FVector2D(actor_scale.Y + 20, 0), Polygon::NONE });
	//start_cut_polygon.Add({ FVector2D(0, 0), Polygon::NONE });
	//start_cut_polygon.Add({ FVector2D(0, -actor_scale.Z - 20), Polygon::NONE });
}

#pragma endregion Setup

#pragma region Helper Methods

void UWall_Cutter::addCutPoint(FVector2D const& PointToAdd) {
	start_cut_polygon.Add({ PointToAdd, Polygon::NONE });
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

	bool nextPointInWall = wall_polygon_out.pointInsidePolygon(dir);

	if (nextPointInWall) {
		return Polygon::ENTRY;
	}
	
	return Polygon::EXIT;
}

#pragma endregion Helper Methods

void UWall_Cutter::Add_Intercepts(Polygon& wall_polygon, Polygon& cut_polygon) {

	for (Polygon::Vertex* wall_vertex : wall_polygon) {

		for (Polygon::Vertex* cut_vertex : cut_polygon) {
			
			// Find current edge for wall_polygon
			MathLib::EDGE a = { wall_vertex->data.pos, wall_vertex->NextNode->data.pos };

			// Find current edge for cut_polygon
			MathLib::EDGE b = { cut_vertex->data.pos, cut_vertex->NextNode->data.pos };

			FVector2D out;
			bool found_intersept = MathLib::Find_Intersection(out, a, b);

			if (!found_intersept) {
				continue;
			}

			// If intercept found -
			
			// Get new point
			Polygon::InterceptTypes intercept_type = getInterceptType(out, b.end);
			Polygon::VertexData new_point = { out,intercept_type };

			// Check for repeat
			if (new_point == wall_vertex->data || new_point == cut_vertex->data) continue;

			// Add to wall!
			Polygon::Vertex* insert_into_wall = wall_polygon.Insert(new_point, wall_vertex);
			Polygon::Vertex* insert_into_cut = cut_polygon.Insert(new_point, cut_vertex);

			// Create intercept link (ENTRY links in wall_polygon) (EXIT links in cut_polygon)
			if (intercept_type == Polygon::ENTRY) insert_into_wall->intercept_link = insert_into_cut;
			if (intercept_type == Polygon::EXIT) insert_into_cut->intercept_link = insert_into_wall;
		}
	}
}

/*
 *	Cut polygon from wall
 *	Updates mesh Wall_Cutter component is attached too
 *	
 *	@cut_shape the polygon we want to cut from the wall
 *		- cut_shape vertices must be ordered clockwise
 */
void UWall_Cutter::cutWall() {

	if (start_cut_polygon.Num() <= 2) return;

	wall_polygon_out = start_wall_polygon;
	cut_polygon_out = start_cut_polygon;

    Add_Intercepts(wall_polygon_out, cut_polygon_out);

	regions.Empty();
	TArray<Polygon::VertexData> visited;

	// Start walk
	for (Polygon::Vertex* current_vertex : cut_polygon_out) {

		if (visited.Contains(current_vertex->data) == false && current_vertex->data.type == Polygon::ENTRY) {

			Polygon new_region = walkLoop(visited, current_vertex, 1);

			regions.Add(new_region);
		}
	}
}

Polygon UWall_Cutter::walkLoop(TArray<Polygon::VertexData> &OUT_visited, Polygon::Vertex*  start, int direction) {

	Polygon loop = Polygon();
	loop.Add(start->data);
	OUT_visited.Add(start->data);

	bool in_cut_polygon = true;
	Polygon::Vertex* x = start->NextNode;

	while (x->data == start->data == false){
		loop.Add(x->data);

		if (OUT_visited.Contains(x->data) && (x->data.type == Polygon::ENTRY || x->data.type == Polygon::EXIT)) {
			UE_LOG(LogTemp, Warning, TEXT("%s vs %s"), *x->data.pos.ToString(), *start->data.pos.ToString());
			UE_LOG(LogTemp, Error, TEXT("Infinite loop found when walking loop! %s vs %s"), *x->data.pos.ToString(), *start->data.pos.ToString());
			return loop;
		}

		OUT_visited.Add(x->data);

		if (x->intercept_link != nullptr && (x->data.type == Polygon::ENTRY || x->data.type == Polygon::EXIT)) {

			x = x->intercept_link;

			if (in_cut_polygon) {
				if (cut_polygon_out.pointInsidePolygon(x->NextNode->data.pos)) {
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