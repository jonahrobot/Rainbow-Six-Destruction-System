// Jonah Ryan 2024 Rainbow Six Siege Explosive System

#include "MathLib.h"
#include "Wall_Cutter.h"
#include "Kismet/GameplayStatics.h"

#pragma region Setup

TArray<FString> node_type_names {"-", "Entry", "Exit"};


TArray<UWall_Cutter::POLYGON_NODE> wall_polygon;
TArray<UWall_Cutter::POLYGON_NODE> cut_polygon;


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

		FirstLocalPlayer->InputComponent->BindAction(FName("x_up"), IE_Pressed, this, &UWall_Cutter::Step_X_Up);
		FirstLocalPlayer->InputComponent->BindAction(FName("y_up"), IE_Pressed, this, &UWall_Cutter::Step_Y_Up);
		FirstLocalPlayer->InputComponent->BindAction(FName("x_down"), IE_Pressed, this, &UWall_Cutter::Step_X_Down);
		FirstLocalPlayer->InputComponent->BindAction(FName("y_down"), IE_Pressed, this, &UWall_Cutter::Step_Y_Down);


	}

	actorScale = GetOwner()->GetActorScale() / 2 * 100;
	actorOrigin = GetOwner()->GetActorLocation();
	actorRotation = GetOwner()->GetActorRotation();

	wall_polygon.Empty();
	cut_polygon.Empty();
}

#pragma endregion Setup

#pragma region Helper Methods

void UWall_Cutter::Test_Input_Triggered() {

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

	// Create polygon that we will cut from wall
	cut_shape.Add(FVector2D(0, 300));
	cut_shape.Add(FVector2D(-300, 0));
	cut_shape.Add(FVector2D(300, 0));

	Cut_Wall();
}

FVector LocalToGlobal(FVector2D LocalVector, FVector ActorOrigin, FRotator ActorRotation, float x) {

	FVector newVector = FVector(x, LocalVector.X, LocalVector.Y);
	return ActorRotation.RotateVector(newVector) + ActorOrigin;
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

	FVector global = LocalToGlobal(dir, actorOrigin, actorRotation, actorScale.X);
	DrawDebugSphere(GetWorld(), global, 25, 5, FColor::Red, true, -1.0f);


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

#pragma region Debug Prints

void UWall_Cutter::Draw_Wall_Poly() {

	UKismetSystemLibrary::FlushPersistentDebugLines(GetWorld());
	UKismetSystemLibrary::FlushDebugStrings(GetWorld());

	if (GEngine) {
		GEngine->ClearOnScreenDebugMessages();
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, "Showing: Wall Polygon");	
	}

	for(int i = 0; i < wall_shape.Num(); i++){
		FVector2D local = wall_shape[i];

		FVector global = LocalToGlobal(local, actorOrigin, actorRotation, actorScale.X);
		DrawDebugSphere(GetWorld(), global, 25, 5, FColor::Blue, true, -1.0f);

		// Draw vector type
		FString Text = FString::FromInt(i);
		FVector vector = actorRotation.RotateVector(FVector(100, local.X, local.Y - 30));

		DrawDebugString(GetWorld(), vector, Text, GetOwner(), FColor::Blue, -1.f, false, 2.0f);
	}
}

void UWall_Cutter::Draw_Cut_Poly() {

	UKismetSystemLibrary::FlushPersistentDebugLines(GetWorld());
	UKismetSystemLibrary::FlushDebugStrings(GetWorld());

	if (GEngine) {
		GEngine->ClearOnScreenDebugMessages();
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Showing: Cut Polygon");
	}

	for (int i = 0; i < cut_shape.Num(); i++) {
		FVector2D local = cut_shape[i];

		FVector global = LocalToGlobal(local, actorOrigin, actorRotation, actorScale.X);
		DrawDebugSphere(GetWorld(), global, 25, 5, FColor::Red, true, -1.0f);

		// Draw vector type
		FString Text = FString::FromInt(i);
		FVector vector = actorRotation.RotateVector(FVector(100, local.X, local.Y - 30));

		DrawDebugString(GetWorld(), vector, Text, GetOwner(), FColor::Red, -1.f, false, 2.0f);
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

	if (wall_polygon.IsEmpty()) {
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("_____"));
	// Draw Nodes

	debug_print_polygon(wall_polygon, cut_polygon, "Wall Polygon", INTERCEPT_ENTRY);
	debug_print_polygon(cut_polygon, wall_polygon, "Cut Polygon", INTERCEPT_EXIT);

	for (POLYGON_NODE const &local : wall_polygon) {

		FVector global = LocalToGlobal(local.pos, actorOrigin, actorRotation, actorScale.X);
		DrawDebugSphere(GetWorld(), global, 25, 3, FColor::Green, true, -1.0f);

		// Draw vector type
		FString Text = node_type_names[local.type];
		FVector vector = actorRotation.RotateVector(FVector(100, local.pos.X, local.pos.Y - 30));

		DrawDebugString(GetWorld(), vector, Text, GetOwner(), FColor::Green, -1.f, false, 2.0f);

		if (local.type == INTERCEPT_ENTRY) {

			if (local.intercept_index == -1) {
				UE_LOG(LogTemp, Warning, TEXT("Intercept not set!"));
				continue;
			}

			FVector2D localTo = cut_polygon[local.intercept_index].pos;

			FVector globalTo = LocalToGlobal(localTo, actorOrigin, actorRotation, actorScale.X);

			UE_LOG(LogTemp, Warning, TEXT("Drawing: %s"), *cut_polygon[local.intercept_index].pos.ToString());

			DrawDebugLine(GetWorld(), global, globalTo, FColor::MakeRandomColor(), true, -1.0f, 0, 10.0f);
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("_____"));

}

void UWall_Cutter::Draw_Cut_Intercepts() {

	UKismetSystemLibrary::FlushPersistentDebugLines(GetWorld());
	UKismetSystemLibrary::FlushDebugStrings(GetWorld());

	if (GEngine) {
		GEngine->ClearOnScreenDebugMessages();
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Orange, "Showing: Cut + Intercepts");
	}

	if (cut_polygon.IsEmpty()) {
		return;
	}

	for (POLYGON_NODE const &local : cut_polygon) {
		FVector global = LocalToGlobal(local.pos, actorOrigin, actorRotation, actorScale.X);
		DrawDebugSphere(GetWorld(), global, 25, 3, FColor::Orange, true, -1.0f);

		// Draw vector type
		FString Text = node_type_names[local.type];
		FVector vector = actorRotation.RotateVector(FVector(100, local.pos.X, local.pos.Y - 30));

		DrawDebugString(GetWorld(), vector, Text, GetOwner(), FColor::Orange, -1.f, false, 2.0f);

		UE_LOG(LogTemp, Warning, TEXT("Point on Cut Polygon: %s"), *local.pos.ToString());
		
		if (local.type == INTERCEPT_EXIT) {

			if (local.intercept_index == -1) {
				UE_LOG(LogTemp, Warning, TEXT("Intercept not set!"));
				continue;
			}

			FVector2D localTo = wall_polygon[local.intercept_index].pos;
			FVector globalTo = LocalToGlobal(localTo, actorOrigin, actorRotation, actorScale.X);

			DrawDebugLine(GetWorld(), global, globalTo, FColor::MakeRandomColor(), true, -1.0f,0,10.0f);
		}
	}
}

int step_through_x = 0;
int step_through_y = 0;

void UWall_Cutter::Step_X_Up() {
	step_through_x++;
	step_through_x = FMath::Abs(FMath::Clamp(step_through_x, 0, wall_shape.Num() - 1));
	Step_Through_Draw();
}

void UWall_Cutter::Step_X_Down() {
	step_through_x--;
	step_through_x = FMath::Abs(FMath::Clamp(step_through_x, 0, wall_shape.Num() - 1));
	Step_Through_Draw();
}

void UWall_Cutter::Step_Y_Up() {
	step_through_y++;
	step_through_y = FMath::Abs(FMath::Clamp(step_through_y, 0, cut_shape.Num() - 1));
	Step_Through_Draw();
}

void UWall_Cutter::Step_Y_Down() {
	step_through_y--;
	step_through_y = FMath::Abs(FMath::Clamp(step_through_y, 0, cut_shape.Num() - 1));
	Step_Through_Draw();
}

void UWall_Cutter::Step_Through_Draw() {

	UKismetSystemLibrary::FlushPersistentDebugLines(GetWorld());
	UKismetSystemLibrary::FlushDebugStrings(GetWorld());

	int x = step_through_x;
	int y = step_through_y;

	// Find current edge for wall_polygon
	FVector2D a_start = wall_shape[x];
	FVector2D a_end = wall_shape[(x + 1) % wall_shape.Num()];
	MathLib::EDGE a = { a_start, a_end };

	// Find current edge for cut_polygon
	FVector2D b_start = cut_shape[y];
	FVector2D b_end = cut_shape[(y + 1) % cut_shape.Num()];
	MathLib::EDGE b = { b_start, b_end };

	FVector2D out;
	bool found_intersept = MathLib::Find_Intersection(out, a, b);

	node_type intercept_type = get_intercept_type(out, b_end);


	// Draw A line
	FVector g_a_start = LocalToGlobal(a_start, actorOrigin, actorRotation, actorScale.X);
	FVector g_a_end = LocalToGlobal(a_end, actorOrigin, actorRotation, actorScale.X);
	FVector g_b_start = LocalToGlobal(b_start, actorOrigin, actorRotation, actorScale.X);
	FVector g_b_end = LocalToGlobal(b_end, actorOrigin, actorRotation, actorScale.X);
	DrawDebugLine(GetWorld(), g_a_start, g_a_end, FColor::Red, true, -1.0f, 0, 10.0f);
	DrawDebugLine(GetWorld(), g_b_start, g_b_end, FColor::Green, true, -1.0f, 0, 10.0f);

	if (GEngine) {
		GEngine->ClearOnScreenDebugMessages();
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, "X: " + FString::FromInt(step_through_x) + " Y: " + FString::FromInt(step_through_y));

		if (found_intersept) {
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, "Intercept!");

			if (intercept_type == INTERCEPT_ENTRY) {
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, "Entry!");
			}
			if (intercept_type == INTERCEPT_EXIT) {
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "EXIT!");
			}

		}
		if(!found_intersept) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Orange, "No intercept..");
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

	// Debug testing
	debug_print_polygon(wall_polygon, cut_polygon,"Wall Polygon", INTERCEPT_ENTRY);
	debug_print_polygon(cut_polygon, wall_polygon,"Cut Polygon", INTERCEPT_EXIT);

	// At this point we have a wall_polygon and cut_polygon, with intercepts in correct order, pointed and labeled!
}

FString Vector2DToString(FVector2D vector) {

	FString out;

	out.AppendInt((int)vector.X);
	out += ",";
	out.AppendInt((int)vector.Y);

	return out;

}

// Cut Polygon: [0,0]->[0,0], [0,0], [0,0]
void UWall_Cutter::debug_print_polygon(TArray<POLYGON_NODE> poly, TArray<POLYGON_NODE> otherPoly, FString name, node_type checkType){

	FString out;

	out += name + ": ";

	for (POLYGON_NODE const& x : poly) {

		if (x.type == checkType) {

			out += "[" + Vector2DToString(x.pos) + "]" + "->" + "[" + Vector2DToString(otherPoly[x.intercept_index].pos) + "]";
		}
		else {
			out += "[" + Vector2DToString(x.pos) + "]";
		}

		out += ", ";
	}

	out.RemoveFromEnd(", ");

	UE_LOG(LogTemp, Warning, TEXT("%s"), *out);

}
