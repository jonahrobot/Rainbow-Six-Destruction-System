
#include "Wall_Cutter_Test.h"
#include "Wall_Cutter.h"
#include "Mathlib.h"
#include "Kismet/GameplayStatics.h"

UWall_Cutter_Test::UWall_Cutter_Test()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UWall_Cutter_Test::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* FirstLocalPlayer = UGameplayStatics::GetPlayerController(this, 0);

	cutter = GetOwner()->FindComponentByClass<UWall_Cutter>();

	if (IsValid(FirstLocalPlayer) && IsValid(FirstLocalPlayer->InputComponent)) {

		FirstLocalPlayer->InputComponent->BindAction(FName("Draw_Wall_Poly_Input"), IE_Pressed, this, &UWall_Cutter_Test::Draw_Wall_Poly);
		FirstLocalPlayer->InputComponent->BindAction(FName("Draw_Cut_Poly_Input"), IE_Pressed, this, &UWall_Cutter_Test::Draw_Cut_Poly);
		FirstLocalPlayer->InputComponent->BindAction(FName("Draw_Wall_Intercepts_Input"), IE_Pressed, this, &UWall_Cutter_Test::Draw_Wall_Intercepts);
		FirstLocalPlayer->InputComponent->BindAction(FName("Draw_Cut_Intercepts_Input"), IE_Pressed, this, &UWall_Cutter_Test::Draw_Cut_Intercepts);

		FirstLocalPlayer->InputComponent->BindAction(FName("x_up"), IE_Pressed, this, &UWall_Cutter_Test::Step_X_Up);
		FirstLocalPlayer->InputComponent->BindAction(FName("y_up"), IE_Pressed, this, &UWall_Cutter_Test::Step_Y_Up);
		FirstLocalPlayer->InputComponent->BindAction(FName("x_down"), IE_Pressed, this, &UWall_Cutter_Test::Step_X_Down);
		FirstLocalPlayer->InputComponent->BindAction(FName("y_down"), IE_Pressed, this, &UWall_Cutter_Test::Step_Y_Down);

	}
}

void UWall_Cutter_Test::Draw_Wall_Poly() {

	UKismetSystemLibrary::FlushPersistentDebugLines(GetWorld());
	UKismetSystemLibrary::FlushDebugStrings(GetWorld());

	if (GEngine) {
		GEngine->ClearOnScreenDebugMessages();
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, "Showing: Wall Polygon");
	}

	for (int i = 0; i < cutter->wall_shape.Num(); i++) {
		FVector2D local = cutter->wall_shape[i];

		FVector global = MathLib::LocalToGlobal(local, cutter->actorOrigin, cutter->actorRotation, cutter->actorScale.X);
		DrawDebugSphere(GetWorld(), global, 25, 5, FColor::Blue, true, -1.0f);

		// Draw vector type
		FString Text = FString::FromInt(i);
		FVector vector = cutter->actorRotation.RotateVector(FVector(100, local.X, local.Y - 30));

		DrawDebugString(GetWorld(), vector, Text, GetOwner(), FColor::Blue, -1.f, false, 2.0f);
	}
}

void UWall_Cutter_Test::Draw_Cut_Poly() {

	UKismetSystemLibrary::FlushPersistentDebugLines(GetWorld());
	UKismetSystemLibrary::FlushDebugStrings(GetWorld());

	if (GEngine) {
		GEngine->ClearOnScreenDebugMessages();
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Showing: Cut Polygon");
	}

	for (int i = 0; i < cutter->cut_shape.Num(); i++) {
		FVector2D local = cutter->cut_shape[i];

		FVector global = MathLib::LocalToGlobal(local, cutter->actorOrigin, cutter->actorRotation, cutter->actorScale.X);
		DrawDebugSphere(GetWorld(), global, 25, 5, FColor::Red, true, -1.0f);

		// Draw vector type
		FString Text = FString::FromInt(i);
		FVector vector = cutter->actorRotation.RotateVector(FVector(100, local.X, local.Y - 30));

		DrawDebugString(GetWorld(), vector, Text, GetOwner(), FColor::Red, -1.f, false, 2.0f);
	}
}

void UWall_Cutter_Test::Draw_Wall_Intercepts() {

	// Clear Drawings
	UKismetSystemLibrary::FlushPersistentDebugLines(GetWorld());
	UKismetSystemLibrary::FlushDebugStrings(GetWorld());

	// Show Label
	if (GEngine) {
		GEngine->ClearOnScreenDebugMessages();
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, "Showing: Wall + Intercepts");
	}

	if (cutter->wall_polygon.IsEmpty()) {
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("_____"));
	// Draw Nodes

	debug_print_polygon(cutter->wall_polygon, cutter->cut_polygon, "Wall Polygon", cutter->INTERCEPT_ENTRY);
	debug_print_polygon(cutter->cut_polygon, cutter->wall_polygon, "Cut Polygon", cutter->INTERCEPT_EXIT);

	for (UWall_Cutter::POLYGON_NODE const& local : cutter->wall_polygon) {

		FVector global = MathLib::LocalToGlobal(local.pos, cutter->actorOrigin, cutter->actorRotation, cutter->actorScale.X);
		DrawDebugSphere(GetWorld(), global, 25, 3, FColor::Green, true, -1.0f);

		// Draw vector type
		FString Text = cutter->node_type_names[local.type];
		FVector vector = cutter->actorRotation.RotateVector(FVector(100, local.pos.X, local.pos.Y - 30));

		DrawDebugString(GetWorld(), vector, Text, GetOwner(), FColor::Green, -1.f, false, 2.0f);

		if (local.type == cutter->INTERCEPT_ENTRY) {

			if (local.intercept_index == -1) {
				UE_LOG(LogTemp, Warning, TEXT("Intercept not set!"));
				continue;
			}

			FVector2D localTo = cutter->cut_polygon[local.intercept_index].pos;

			FVector globalTo = MathLib::LocalToGlobal(localTo, cutter->actorOrigin, cutter->actorRotation, cutter->actorScale.X);

			UE_LOG(LogTemp, Warning, TEXT("Drawing: %s"), *cutter->cut_polygon[local.intercept_index].pos.ToString());

			DrawDebugLine(GetWorld(), global, globalTo, FColor::MakeRandomColor(), true, -1.0f, 0, 10.0f);
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("_____"));

}

void UWall_Cutter_Test::Draw_Cut_Intercepts() {

	UKismetSystemLibrary::FlushPersistentDebugLines(GetWorld());
	UKismetSystemLibrary::FlushDebugStrings(GetWorld());

	if (GEngine) {
		GEngine->ClearOnScreenDebugMessages();
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Orange, "Showing: Cut + Intercepts");
	}

	if (cutter->cut_polygon.IsEmpty()) {
		return;
	}

	for (UWall_Cutter::POLYGON_NODE const& local : cutter->cut_polygon) {
		FVector global = MathLib::LocalToGlobal(local.pos, cutter->actorOrigin, cutter->actorRotation, cutter->actorScale.X);
		DrawDebugSphere(GetWorld(), global, 25, 3, FColor::Orange, true, -1.0f);

		// Draw vector type
		FString Text = cutter->node_type_names[local.type];
		FVector vector = cutter->actorRotation.RotateVector(FVector(100, local.pos.X, local.pos.Y - 30));

		DrawDebugString(GetWorld(), vector, Text, GetOwner(), FColor::Orange, -1.f, false, 2.0f);

		UE_LOG(LogTemp, Warning, TEXT("Point on Cut Polygon: %s"), *local.pos.ToString());

		if (local.type == cutter->INTERCEPT_EXIT) {

			if (local.intercept_index == -1) {
				UE_LOG(LogTemp, Warning, TEXT("Intercept not set!"));
				continue;
			}

			FVector2D localTo = cutter->wall_polygon[local.intercept_index].pos;
			FVector globalTo = MathLib::LocalToGlobal(localTo, cutter->actorOrigin, cutter->actorRotation, cutter->actorScale.X);

			DrawDebugLine(GetWorld(), global, globalTo, FColor::MakeRandomColor(), true, -1.0f, 0, 10.0f);
		}
	}
}

int step_through_x = 0;
int step_through_y = 0;

void UWall_Cutter_Test::Step_X_Up() {
	step_through_x++;
	step_through_x = FMath::Abs(FMath::Clamp(step_through_x, 0, cutter->wall_shape.Num() - 1));
	Step_Through_Draw();
}

void UWall_Cutter_Test::Step_X_Down() {
	step_through_x--;
	step_through_x = FMath::Abs(FMath::Clamp(step_through_x, 0, cutter->wall_shape.Num() - 1));
	Step_Through_Draw();
}

void UWall_Cutter_Test::Step_Y_Up() {
	step_through_y++;
	step_through_y = FMath::Abs(FMath::Clamp(step_through_y, 0, cutter->cut_shape.Num() - 1));
	Step_Through_Draw();
}

void UWall_Cutter_Test::Step_Y_Down() {
	step_through_y--;
	step_through_y = FMath::Abs(FMath::Clamp(step_through_y, 0, cutter->cut_shape.Num() - 1));
	Step_Through_Draw();
}

void UWall_Cutter_Test::Step_Through_Draw() {

	UKismetSystemLibrary::FlushPersistentDebugLines(GetWorld());
	UKismetSystemLibrary::FlushDebugStrings(GetWorld());

	int x = step_through_x;
	int y = step_through_y;

	// Find current edge for wall_polygon
	FVector2D a_start = cutter->wall_shape[x];
	FVector2D a_end = cutter->wall_shape[(x + 1) % cutter->wall_shape.Num()];
	MathLib::EDGE a = { a_start, a_end };

	// Find current edge for cut_polygon
	FVector2D b_start = cutter->cut_shape[y];
	FVector2D b_end = cutter->cut_shape[(y + 1) % cutter->cut_shape.Num()];
	MathLib::EDGE b = { b_start, b_end };

	FVector2D out;
	bool found_intersept = MathLib::Find_Intersection(out, a, b);

	UWall_Cutter::node_type intercept_type = cutter->get_intercept_type(out, b_end);


	// Draw A line
	FVector g_a_start = MathLib::LocalToGlobal(a_start, cutter->actorOrigin, cutter->actorRotation, cutter->actorScale.X);
	FVector g_a_end = MathLib::LocalToGlobal(a_end, cutter->actorOrigin, cutter->actorRotation, cutter->actorScale.X);
	FVector g_b_start = MathLib::LocalToGlobal(b_start, cutter->actorOrigin, cutter->actorRotation, cutter->actorScale.X);
	FVector g_b_end = MathLib::LocalToGlobal(b_end, cutter->actorOrigin, cutter->actorRotation, cutter->actorScale.X);
	DrawDebugLine(GetWorld(), g_a_start, g_a_end, FColor::Red, true, -1.0f, 0, 10.0f);
	DrawDebugLine(GetWorld(), g_b_start, g_b_end, FColor::Green, true, -1.0f, 0, 10.0f);

	if (GEngine) {
		GEngine->ClearOnScreenDebugMessages();
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, "X: " + FString::FromInt(step_through_x) + " Y: " + FString::FromInt(step_through_y));

		if (found_intersept) {
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, "Intercept!");

			if (intercept_type == cutter->INTERCEPT_ENTRY) {
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, "Entry!");
			}
			if (intercept_type == cutter->INTERCEPT_EXIT) {
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "EXIT!");
			}

		}
		if (!found_intersept) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Orange, "No intercept..");
	}
}


FString Vector2DToString(FVector2D vector) {

	FString out;

	out.AppendInt((int)vector.X);
	out += ",";
	out.AppendInt((int)vector.Y);

	return out;

}

// Cut Polygon: [0,0]->[0,0], [0,0], [0,0]
void UWall_Cutter_Test::debug_print_polygon(TArray<UWall_Cutter::POLYGON_NODE> poly, TArray<UWall_Cutter::POLYGON_NODE> otherPoly, FString name, UWall_Cutter::node_type checkType) {

	FString out;

	out += name + ": ";

	for (UWall_Cutter::POLYGON_NODE const& x : poly) {

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
