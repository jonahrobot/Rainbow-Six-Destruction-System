
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

	currentX = cutter->wall_polygon_out.HeadNode;
	currentY = cutter->cut_polygon_out.HeadNode;
}

void UWall_Cutter_Test::Step_Up() {
	if (currentY == nullptr || currentX == nullptr) {
		currentX = cutter->wall_polygon_out.HeadNode;
		currentY = cutter->cut_polygon_out.HeadNode;
	}

	if (currentY == nullptr || currentX == nullptr) return;

	currentY = currentY->NextNode;
	Step_Through_Draw();
}
void UWall_Cutter_Test::Step_Down() {
	if (currentY == nullptr || currentX == nullptr) {
		currentX = cutter->wall_polygon_out.HeadNode;
		currentY = cutter->cut_polygon_out.HeadNode;
	}

	if (currentY == nullptr || currentX == nullptr) return;

	currentY = currentY->PrevNode;
	Step_Through_Draw();
}
void UWall_Cutter_Test::Step_Left() {
	if (currentY == nullptr || currentX == nullptr) {
		currentX = cutter->wall_polygon_out.HeadNode;
		currentY = cutter->cut_polygon_out.HeadNode;
	}

	if (currentY == nullptr || currentX == nullptr) return;

	currentX = currentX->PrevNode;
	Step_Through_Draw();
}
void UWall_Cutter_Test::Step_Right() {
	if (currentY == nullptr || currentX == nullptr) {
		currentX = cutter->wall_polygon_out.HeadNode;
		currentY = cutter->cut_polygon_out.HeadNode;
	}

	if (currentY == nullptr || currentX == nullptr) return;

	currentX = currentX->NextNode;
	Step_Through_Draw();
}

void UWall_Cutter_Test::Draw_Polygon(Polygon poly, FString nameOfDraw, bool drawEdges, bool erasePast = true) {

	// Clear past draws
	if (erasePast) {
		UKismetSystemLibrary::FlushPersistentDebugLines(GetWorld());
		UKismetSystemLibrary::FlushDebugStrings(GetWorld());
	}

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
	FVector lastGlobal = MathLib::LocalToGlobal(poly.TailNode->data.pos, origin, rotation, scale.X);

	int index = 0;
	Polygon::Vertex* currentVertex = poly.HeadNode;
	do{
	
		FVector globalVertexPos = MathLib::LocalToGlobal(currentVertex->data.pos, origin, rotation, scale.X);

		// Draw Vertex
		switch(currentVertex->data.type){
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
		FVector indexTextPos = cutter->actor_rotation.RotateVector(FVector(100, currentVertex->data.pos.X, currentVertex->data.pos.Y - 10));
		FString indexText = FString::FromInt(index);
		index++;

		DrawDebugString(GetWorld(), indexTextPos, indexText, GetOwner(), FColor::Blue, -1.f, false, 2.0f);

		currentVertex = currentVertex->NextNode;
	}while(currentVertex != poly.HeadNode);
}



void UWall_Cutter_Test::Draw_Wall_Poly() {
	Draw_Polygon(cutter->start_wall_polygon, "Showing: Wall Polygon",true);
}

void UWall_Cutter_Test::Draw_Cut_Poly() {
	Draw_Polygon(cutter->start_cut_polygon,  "Showing: Cut Polygon",true);
}

void UWall_Cutter_Test::Draw_Wall_Intercepts() {
	Draw_Polygon(cutter->cut_polygon_out, "Wall Polygon: Intercepts",true);
}

void UWall_Cutter_Test::Draw_Shrapnel() {
	
	UKismetSystemLibrary::FlushPersistentDebugLines(GetWorld());
	UKismetSystemLibrary::FlushDebugStrings(GetWorld());

	for (Polygon shrapnel_polgyon : cutter->regions) {
		Draw_Polygon(shrapnel_polgyon, "Showing: Shrapnel Polygon", true,false);
	}
}

void UWall_Cutter_Test::Step_Through_Draw() {

	if (cutter->start_wall_polygon.IsEmpty() || cutter->start_cut_polygon.IsEmpty()) {
		return;
	}

	UKismetSystemLibrary::FlushPersistentDebugLines(GetWorld());
	UKismetSystemLibrary::FlushDebugStrings(GetWorld());

	// Find current edge for wall_polygon
	FVector2D a_start = currentX->data.pos;
	FVector2D a_end = currentX->NextNode->data.pos;
	MathLib::EDGE a = { a_start, a_end };

	// Find current edge for cut_polygon
	FVector2D b_start = currentY->data.pos;
	FVector2D b_end = currentY->NextNode->data.pos;
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

	Polygon::Vertex* x = poly.HeadNode;
	while(x->NextNode->data != poly.HeadNode->data){

		if (x->data.type == checkType) {

			out += "[" + Vector2DToString(x->data.pos) + "]" + "->" + "[" + Vector2DToString(x->intercept_link->data.pos) + "]";
		}
		else {
			out += "[" + Vector2DToString(x->data.pos) + "]";
		}

		out += ", ";

		x = x->NextNode;
	}

	out.RemoveFromEnd(", ");

	UE_LOG(LogTemp, Warning, TEXT("%s"), *out);

}
