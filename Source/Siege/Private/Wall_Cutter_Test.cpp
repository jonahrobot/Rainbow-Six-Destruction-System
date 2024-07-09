
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
		FirstLocalPlayer->InputComponent->BindAction(FName("Draw_Wall_Intercepts_Input"), IE_Pressed, this,&UWall_Cutter_Test::Draw_Wall_Intercepts);
		FirstLocalPlayer->InputComponent->BindAction(FName("Draw_Cut_Intercepts_Input"), IE_Pressed, this, &UWall_Cutter_Test::Draw_Shrapnel);

		FirstLocalPlayer->InputComponent->BindAction(FName("x_up"), IE_Pressed, this, &UWall_Cutter_Test::Step_Right);
		FirstLocalPlayer->InputComponent->BindAction(FName("y_up"), IE_Pressed, this, &UWall_Cutter_Test::Step_Up);
		FirstLocalPlayer->InputComponent->BindAction(FName("x_down"), IE_Pressed, this, &UWall_Cutter_Test::Step_Left);
		FirstLocalPlayer->InputComponent->BindAction(FName("y_down"), IE_Pressed, this, &UWall_Cutter_Test::Step_Down);

	}

	origin = cutter->actor_origin;
	rotation = cutter->actor_rotation;
	scale = cutter->actor_scale;
}


void UWall_Cutter_Test::Step_Up() {
	Step(0, 1);
}
void UWall_Cutter_Test::Step_Down() {
	Step(0, -1);
}
void UWall_Cutter_Test::Step_Left() {
	Step(-1, 0);
}
void UWall_Cutter_Test::Step_Right() {
	Step(1, 0);
}

void UWall_Cutter_Test::Draw_Polygon(Polygon poly, FString nameOfDraw, bool drawEdges) {

	// Clear past draws
	UKismetSystemLibrary::FlushPersistentDebugLines(GetWorld());
	UKismetSystemLibrary::FlushDebugStrings(GetWorld());

	// Log drawing
	if (GEngine) {
		GEngine->ClearOnScreenDebugMessages();
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, nameOfDraw);
	}

	// Pre-Conditions
	if (poly.IsEmpty()) {
		return;
	}

	// Get last vertex pos, to create lines from
	FVector lastGlobal = MathLib::LocalToGlobal(poly.getVertex(poly.Num() - 1)->pos, origin, rotation, scale.X);

	for (int i = 0; i < poly.Num(); i++) {

		Polygon::Vertex currentVertex = *poly.getVertex(i);
	
		FVector globalVertexPos = MathLib::LocalToGlobal(currentVertex.pos, origin, rotation, scale.X);

		// Draw Vertex
		switch(currentVertex.type){
			case Polygon::NONE:
				DrawDebugSphere(GetWorld(), globalVertexPos, 25, 5, FColor::Black, true, -1.0f);
				break;
			case Polygon::ENTRY:
				DrawDebugSphere(GetWorld(), globalVertexPos, 25, 5, FColor::Green, true, -1.0f);
				break;
			case Polygon::EXIT:
				DrawDebugSphere(GetWorld(), globalVertexPos, 25, 5, FColor::Red, true, -1.0f);
				break;
		}

		// Draw edge
		if (drawEdges) {
			DrawDebugLine(GetWorld(), lastGlobal, globalVertexPos, FColor::Black, true, -1.0f, 0, 10.0f);
		}

		// Update vertex pos
		lastGlobal = globalVertexPos;

		// Draw vector type
		FVector indexTextPos = cutter->actor_rotation.RotateVector(FVector(100, currentVertex.pos.X, currentVertex.pos.Y - 10));
		FString indexText = FString::FromInt(i);

		DrawDebugString(GetWorld(), indexTextPos, indexText, GetOwner(), FColor::Blue, -1.f, false, 2.0f);
	}
}



void UWall_Cutter_Test::Draw_Wall_Poly() {
	Draw_Polygon(cutter->start_wall_polygon, "Showing: Wall Polygon",true);
}

void UWall_Cutter_Test::Draw_Cut_Poly() {
	Draw_Polygon(cutter->start_cut_polygon,  "Showing: Cut Polygon",true);
}

void UWall_Cutter_Test::Draw_Wall_Intercepts() {
	Draw_Polygon(cutter->wall_polygon, "Wall Polygon: Intercepts",true);
}

void UWall_Cutter_Test::Draw_Shrapnel() {
	for (Polygon shrapnel_polgyon : cutter->regions) {
		Draw_Polygon(shrapnel_polgyon, "Showing: Shrapnel Polygon", true);
	}
}

void UWall_Cutter_Test::Step(float x, float y) {
	step_through_x += x;
	step_through_y += y;

	step_through_x = FMath::Abs(FMath::Clamp(step_through_x, 0, cutter->start_wall_polygon.Num() - 1));
	step_through_y = FMath::Abs(FMath::Clamp(step_through_y, 0, cutter->start_cut_polygon.Num() - 1));
	Step_Through_Draw();
}

void UWall_Cutter_Test::Step_Through_Draw() {

	if (cutter->start_wall_polygon.IsEmpty() || cutter->start_cut_polygon.IsEmpty()) {
		return;
	}

	UKismetSystemLibrary::FlushPersistentDebugLines(GetWorld());
	UKismetSystemLibrary::FlushDebugStrings(GetWorld());

	int x = step_through_x;
	int y = step_through_y;

	// Find current edge for wall_polygon
	FVector2D a_start = cutter->start_wall_polygon.getVertex(x)->pos;
	FVector2D a_end = cutter->start_wall_polygon.getVertex((x + 1) % cutter->start_wall_polygon.Num())->pos;
	MathLib::EDGE a = { a_start, a_end };

	// Find current edge for cut_polygon
	FVector2D b_start = cutter->start_cut_polygon.getVertex(y)->pos;
	FVector2D b_end = cutter->start_cut_polygon.getVertex((y + 1) % cutter->start_cut_polygon.Num())->pos;
	MathLib::EDGE b = { b_start, b_end };

	FVector2D out;
	bool found_intersept = MathLib::Find_Intersection(out, a, b);

	Polygon::InterceptTypes intercept_type = cutter->getInterceptType(out, b_end);

	// Draw A line
	FVector g_a_start = MathLib::LocalToGlobal(a_start, origin, rotation, scale.X);
	FVector g_a_end = MathLib::LocalToGlobal(a_end, origin, rotation, scale.X);
	FVector g_b_start = MathLib::LocalToGlobal(b_start, origin, rotation, scale.X);
	FVector g_b_end = MathLib::LocalToGlobal(b_end, origin, rotation, scale.X);
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
