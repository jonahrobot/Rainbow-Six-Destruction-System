
#include "Wall_Cutter_Test.h"
#include "Wall_Cutter.h"
#include "Polygon.h"
#include "Mathlib.h"
#include "Kismet/GameplayStatics.h"

UWall_Cutter_Test::UWall_Cutter_Test()
{

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
		FirstLocalPlayer->InputComponent->BindAction(FName("Draw_Cut_Intercepts_Input"), IE_Pressed, this, &UWall_Cutter_Test::Draw_Shrapnel);

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

	if (cutter->wall_shape.Num() == 0) {
		return;
	}

	FVector lastGlobal = MathLib::LocalToGlobal(cutter->wall_shape[cutter->wall_shape.Num()-1], cutter->actor_origin, cutter->actor_rotation, cutter->actor_scale.X);

	for (int i = 0; i < cutter->wall_shape.Num(); i++) {
		FVector2D local = cutter->wall_shape[i];

		FVector global = MathLib::LocalToGlobal(local, cutter->actor_origin, cutter->actor_rotation, cutter->actor_scale.X);
		DrawDebugSphere(GetWorld(), global, 25, 5, FColor::Blue, true, -1.0f);

		DrawDebugLine(GetWorld(), lastGlobal, global, FColor::Red, true, -1.0f, 0, 10.0f);

		lastGlobal = global;

		// Draw vector type
		FString Text = FString::FromInt(i);
		FVector vector = cutter->actor_rotation.RotateVector(FVector(100, local.X, local.Y - 30));

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

	if (cutter->cut_shape.Num() == 0) {
		return;
	}


	FVector lastGlobal = MathLib::LocalToGlobal(cutter->cut_shape[cutter->cut_shape.Num() - 1], cutter->actor_origin, cutter->actor_rotation, cutter->actor_scale.X);

	for (int i = 0; i < cutter->cut_shape.Num(); i++) {
		FVector2D local = cutter->cut_shape[i];

		FVector global = MathLib::LocalToGlobal(local, cutter->actor_origin, cutter->actor_rotation, cutter->actor_scale.X);
		DrawDebugSphere(GetWorld(), global, 25, 5, FColor::Red, true, -1.0f);

		DrawDebugLine(GetWorld(), lastGlobal, global, FColor::Red, true, -1.0f, 0, 10.0f);

		lastGlobal = global;

		//// Draw vector type
		//FString Text = FString::FromInt(i);
		//FVector vector = cutter->actorRotation.RotateVector(FVector(100, local.X, local.Y - 30));

		//DrawDebugString(GetWorld(), vector, Text, GetOwner(), FColor::Red, -1.f, false, 2.0f);
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

	debug_print_polygon(cutter->wall_polygon, cutter->cut_polygon, "Wall Polygon", Polygon::ENTRY);
	debug_print_polygon(cutter->cut_polygon, cutter->wall_polygon, "Cut Polygon", Polygon::EXIT);

	for (int i = 0; i < cutter->wall_polygon.Num(); i++) {

		Polygon::Vertex local = *cutter->wall_polygon.getVertex(i);

		//if (local.type == cutter->DEFAULT) continue;

		FVector global = MathLib::LocalToGlobal(local.pos, cutter->actor_origin, cutter->actor_rotation, cutter->actor_scale.X);
		DrawDebugSphere(GetWorld(), global, 25, 3, FColor::Green, true, -1.0f);

		// Draw vector type
		FString Text = node_type_names[local.type];
		FVector vector = cutter->actor_rotation.RotateVector(FVector(100, local.pos.X, local.pos.Y - 30));

		DrawDebugString(GetWorld(), vector, Text, GetOwner(), FColor::Green, -1.f, false, 2.0f);

		if (local.type == Polygon::ENTRY) {

			if (local.intercept_index == -1) {
				UE_LOG(LogTemp, Warning, TEXT("Intercept not set!"));
				continue;
			}

			FVector2D localTo = cutter->cut_polygon.getVertex(local.intercept_index)->pos;

			FVector globalTo = MathLib::LocalToGlobal(localTo, cutter->actor_origin, cutter->actor_rotation, cutter->actor_scale.X);

			UE_LOG(LogTemp, Warning, TEXT("Drawing: %s"), *cutter->cut_polygon.getVertex(local.intercept_index)->pos.ToString());

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

	for (int i = 0; i < cutter->cut_polygon.Num(); i++) {

		Polygon::Vertex local = *cutter->cut_polygon.getVertex(i);

		if (local.type == Polygon::NONE) continue;

		FVector global = MathLib::LocalToGlobal(local.pos, cutter->actor_origin, cutter->actor_rotation, cutter->actor_scale.X);
		DrawDebugSphere(GetWorld(), global, 25, 3, FColor::Orange, true, -1.0f);

		// Draw vector type
		FString Text = node_type_names[local.type];
		FVector vector = cutter->actor_rotation.RotateVector(FVector(100, local.pos.X, local.pos.Y - 30));

		DrawDebugString(GetWorld(), vector, Text, GetOwner(), FColor::Orange, -1.f, false, 2.0f);

		UE_LOG(LogTemp, Warning, TEXT("Point on Cut Polygon: %s"), *local.pos.ToString());

		if (local.type == Polygon::EXIT) {

			if (local.intercept_index == -1) {
				UE_LOG(LogTemp, Warning, TEXT("Intercept not set!"));
				continue;
			}

			FVector2D localTo = cutter->wall_polygon.getVertex(local.intercept_index)->pos;
			FVector globalTo = MathLib::LocalToGlobal(localTo, cutter->actor_origin, cutter->actor_rotation, cutter->actor_scale.X);

			DrawDebugLine(GetWorld(), global, globalTo, FColor::MakeRandomColor(), true, -1.0f, 0, 10.0f);
		}
	}
}

void UWall_Cutter_Test::Draw_Shrapnel() {
	
	UKismetSystemLibrary::FlushPersistentDebugLines(GetWorld());
	UKismetSystemLibrary::FlushDebugStrings(GetWorld());

	if (GEngine) {
		GEngine->ClearOnScreenDebugMessages();
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Showing: Shrapnel Polygon");
	}

	for (Polygon shrapnel_polgyon : cutter->regions) {

		FVector lastGlobal = MathLib::LocalToGlobal(shrapnel_polgyon.getVertex(shrapnel_polgyon.Num() - 1)->pos, cutter->actor_origin, cutter->actor_rotation, cutter->actor_scale.X);

		int index = 0;

		for (int i = 0; i < shrapnel_polgyon.Num(); i++) {

			FVector2D local = shrapnel_polgyon.getVertex(i)->pos;

			FVector global = MathLib::LocalToGlobal(local, cutter->actor_origin, cutter->actor_rotation, cutter->actor_scale.X);
			DrawDebugSphere(GetWorld(), global, 25, 5, FColor::Black, true, -1.0f);

			DrawDebugLine(GetWorld(), lastGlobal, global, FColor::Black, true, -1.0f, 0, 10.0f);

			lastGlobal = global;
					// Draw vector type
			FString Text = FString::FromInt(index);
			FVector vector = cutter->actor_rotation.RotateVector(FVector(100, local.X, local.Y - 30));

			DrawDebugString(GetWorld(), vector, Text, GetOwner(), FColor::Red, -1.f, false, 2.0f);
			index += 1;
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

	if (cutter->wall_shape.IsEmpty() || cutter->cut_shape.IsEmpty()) {
		return;
	}

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

	Polygon::InterceptTypes intercept_type = cutter->getInterceptType(out, b_end);


	// Draw A line
	FVector g_a_start = MathLib::LocalToGlobal(a_start, cutter->actor_origin, cutter->actor_rotation, cutter->actor_scale.X);
	FVector g_a_end = MathLib::LocalToGlobal(a_end, cutter->actor_origin, cutter->actor_rotation, cutter->actor_scale.X);
	FVector g_b_start = MathLib::LocalToGlobal(b_start, cutter->actor_origin, cutter->actor_rotation, cutter->actor_scale.X);
	FVector g_b_end = MathLib::LocalToGlobal(b_end, cutter->actor_origin, cutter->actor_rotation, cutter->actor_scale.X);
	DrawDebugLine(GetWorld(), g_a_start, g_a_end, FColor::Red, true, -1.0f, 0, 10.0f);
	DrawDebugLine(GetWorld(), g_b_start, g_b_end, FColor::Green, true, -1.0f, 0, 10.0f);

	if (GEngine) {
		GEngine->ClearOnScreenDebugMessages();
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, "X: " + FString::FromInt(step_through_x) + " Y: " + FString::FromInt(step_through_y));

		if (found_intersept) {
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, "Intercept!");

			if (intercept_type == Polygon::ENTRY) {
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, "Entry!");
			}
			if (intercept_type == Polygon::EXIT) {
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
void UWall_Cutter_Test::debug_print_polygon(Polygon poly, Polygon otherPoly, FString name, Polygon::InterceptTypes checkType) {

	FString out;

	out += name + ": ";

	for (int i = 0; i < poly.Num(); i++) {

		Polygon::Vertex x = *poly.getVertex(i);

		if (x.type == checkType) {

			out += "[" + Vector2DToString(x.pos) + "]" + "->" + "[" + Vector2DToString(otherPoly.getVertex(x.intercept_index)->pos) + "]";
		}
		else {
			out += "[" + Vector2DToString(x.pos) + "]";
		}

		out += ", ";
	}

	out.RemoveFromEnd(", ");

	UE_LOG(LogTemp, Warning, TEXT("%s"), *out);

}
