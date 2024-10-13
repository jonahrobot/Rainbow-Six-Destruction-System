// Jonah Ryan 2024 Rainbow Six Siege Explosive System

#include "Wall_Cutter.h"
#include "MathLib.h"
#include "Polygon.h"
#include "Engine/StaticMeshActor.h"
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

		FirstLocalPlayer->InputComponent->BindAction(FName("Explode"), IE_Pressed, this, &UWall_Cutter::startInput);

	}


	actor_scale = GetOwner()->GetActorScale() / 2 * 100;
	actor_origin = GetOwner()->GetActorLocation();
	actor_rotation = GetOwner()->GetActorRotation();

	UE_LOG(LogTemp, Warning, TEXT("Scale = %f , %f, %f"),actor_scale.X, actor_scale.Y, actor_scale.Z);
	UE_LOG(LogTemp, Warning, TEXT("Origin = %f , %f, %f"), actor_origin.X, actor_origin.Y, actor_origin.Z);
	UE_LOG(LogTemp, Warning, TEXT("Rotation = %s"),*actor_rotation.ToString());


	mesh = GetOwner()->FindComponentByClass<UProceduralMeshComponent>();

	// Else
	wall_polygon_out.Empty();
	cut_polygon_out.Empty();

	UE_LOG(LogTemp,Warning,TEXT("%f by %f"), actor_scale.Y,actor_scale.Z)

	start_wall_polygon.Add({ FVector2D(actor_scale.Y, actor_scale.Z), Polygon::NONE });
	start_wall_polygon.Add({ FVector2D(-actor_scale.Y, actor_scale.Z),Polygon::NONE });
	start_wall_polygon.Add({ FVector2D(-actor_scale.Y, -actor_scale.Z),Polygon::NONE });
	start_wall_polygon.Add({ FVector2D(actor_scale.Y, -actor_scale.Z),Polygon::NONE });


	//start_cut_polygon.Add({ FVector2D(0, actor_scale.Z+100), Polygon::NONE });

	//start_cut_polygon.Add({ FVector2D(actor_scale.Y + 100,0), Polygon::NONE });
	//start_cut_polygon.Add({ FVector2D(-actor_scale.Y - 100,0), Polygon::NONE });

	//start_cut_polygon.Add({ FVector2D(actor_scale.Y + 20, -actor_scale.Z - 20), Polygon::NONE });
	//start_cut_polygon.Add({ FVector2D(actor_scale.Y + 20, 0), Polygon::NONE });
	//start_cut_polygon.Add({ FVector2D(0, 0), Polygon::NONE });
	//start_cut_polygon.Add({ FVector2D(0, -actor_scale.Z - 20), Polygon::NONE });


	//start_cut_polygon.Add({ FVector2D(actor_scale.Y + 100, -actor_scale.Z + 400), Polygon::NONE });
	//start_cut_polygon.Add({ FVector2D(actor_scale.Y + 100, 0), Polygon::NONE });
	//start_cut_polygon.Add({ FVector2D(0, 0), Polygon::NONE });
	//start_cut_polygon.Add({ FVector2D(0, -actor_scale.Z + 400), Polygon::NONE });

	start_cut_polygon = Polygon("(10.5,62.5),(130,100),(150,0),(270,100),(280,62.5),(160,-62.5),(170,-100),(310,-135),(295,-220),(160,-135),(135,-140),(90,-270),(32.5,-260),(62.5,-135),(58,-95),(-62.5,-90),(-62.5,-50),(58,-40)");

	TArray<FVector2D> renderableVertices;
	FJsonSerializableArrayInt trianglesStart;

	Polygon test = start_wall_polygon;
	test.flipPolygonVertexOrder();

	test.triangulatePolygon(renderableVertices, trianglesStart);

	TArray<FVector> vertices3D;
	FJsonSerializableArrayInt triangles3D = trianglesStart;

	test.extrudePolygon(actor_scale.X, vertices3D, triangles3D, renderableVertices, trianglesStart);

	TArray<FVector> normals;
	TArray<FVector2d> uv0;
	TArray<FColor> vertexColors;
	TArray<FProcMeshTangent> tangents;
	mesh->CreateMeshSection(0, vertices3D, triangles3D, normals, uv0, vertexColors, tangents, true);

}

#pragma endregion Setup

#pragma region Helper Methods

void UWall_Cutter::startInput() {
	if (start_cut_polygon.Num() <= 2) return;

	start_cut_polygon.printPolygon();

	Draw_Polygon(start_wall_polygon, "Show wall", true, true);
	Draw_Polygon(start_cut_polygon, "Show Cut", true, false);
	FTimerHandle UniqueHandle;
	FTimerDelegate DelayDelegate = FTimerDelegate::CreateUObject(this, &UWall_Cutter::HalfOfCut);
	GetWorld()->GetTimerManager().SetTimer(UniqueHandle,DelayDelegate, 2.f, false);
}

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

bool UWall_Cutter::CompareFVector(FVector2D a, FVector2D b){

	// 0.00 precision 
	return	std::roundf(a.X * 100) == std::roundf(b.X * 100) &&
			std::roundf(a.Y * 100) == std::roundf(b.Y * 100);

}

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

			if (CompareFVector(new_point.pos, wall_vertex->data.pos)) continue;
			if (CompareFVector(new_point.pos, cut_vertex->data.pos)) continue;
			if (CompareFVector(new_point.pos, wall_vertex->NextNode->data.pos)) continue;
			if (CompareFVector(new_point.pos, cut_vertex->NextNode->data.pos)) continue;

			// Add to wall!
			Polygon::Vertex* insert_into_wall = wall_polygon.Insert(new_point, wall_vertex);
			Polygon::Vertex* insert_into_cut = cut_polygon.Insert(new_point, cut_vertex);

			// Create intercept link (ENTRY links in wall_polygon) (EXIT links in cut_polygon)
			if (intercept_type == Polygon::ENTRY) insert_into_wall->intercept_link = insert_into_cut;
			if (intercept_type == Polygon::EXIT) insert_into_cut->intercept_link = insert_into_wall;
		}
	}
}


void UWall_Cutter::Draw_Polygon(Polygon poly, FString nameOfDraw, bool drawEdges, bool erasePast = true) {

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
	FVector lastGlobal = MathLib::LocalToGlobal(poly.TailNode->data.pos, actor_origin, actor_rotation, actor_scale.X);

	int index = 0;
	Polygon::Vertex* currentVertex = poly.HeadNode;
	do {

		FVector globalVertexPos = MathLib::LocalToGlobal(currentVertex->data.pos, actor_origin, actor_rotation, actor_scale.X);

		// Draw Vertex
		switch (currentVertex->data.type) {
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
		FVector indexTextPos = this->actor_rotation.RotateVector(FVector(100, currentVertex->data.pos.X, currentVertex->data.pos.Y - 10));
		FString indexText = FString::FromInt(index);
		index++;

		DrawDebugString(GetWorld(), indexTextPos, indexText, GetOwner(), FColor::Blue, -1.f, false, 2.0f);

		currentVertex = currentVertex->NextNode;
	} while (currentVertex != poly.HeadNode);
}

void UWall_Cutter::HalfOfCut() {
	if (start_cut_polygon.Num() <= 2) return;

	wall_polygon_out = start_wall_polygon;
	cut_polygon_out = start_cut_polygon;


	if (cut_polygon_out.isPolygonClockwise() == false) {
		cut_polygon_out.flipPolygonVertexOrder();
	}

	if (wall_polygon_out.isPolygonClockwise() == false) {
		wall_polygon_out.flipPolygonVertexOrder();
	}

	Add_Intercepts(wall_polygon_out, cut_polygon_out);

	Draw_Polygon(wall_polygon_out, "Show wall", true, true);
	Draw_Polygon(cut_polygon_out, "Show Cut", true, false);

	FTimerHandle UniqueHandle;
	FTimerDelegate DelayDelegate = FTimerDelegate::CreateUObject(this, &UWall_Cutter::cutWall, true);
	GetWorld()->GetTimerManager().SetTimer(UniqueHandle, DelayDelegate, 2.f, false);
}

/*
 *	Cut polygon from wall
 *	Updates mesh Wall_Cutter component is attached too
 *	
 *	@cut_shape the polygon we want to cut from the wall
 *		- cut_shape vertices must be ordered clockwise
 */
void UWall_Cutter::AlmostThere(TArray<extrudable> t) {
	for (extrudable x : t) {
		extrudeAndShow(x);
	}

	this->GetOwner()->Destroy();
}

TArray<FColor> possibleColors = { FColor::Red, FColor::Blue, FColor::White, FColor::Green, FColor::Black, FColor::Yellow };
int indexOfColors = 0;

void UWall_Cutter::DisplayCut() {

	TArray<extrudable> test;

	for (Polygon toRender : regions) {

		if (toRender.isPolygonClockwise() == false) {
			toRender.flipPolygonVertexOrder();
		}

		extrudable next;

		next.input = toRender;

		toRender.triangulatePolygon(next.renderableVertices, next.triangles);

		test.Add(next);
	
		// render traigulated data
	}

	int counter = 0;

	FColor triColor = FColor::MakeRandomColor();

	for (extrudable x : test) {

		indexOfColors = 0;

		TArray<FVector> renderableTri;

		for (FVector2D testVec : x.renderableVertices) {
			renderableTri.Add(MathLib::LocalToGlobal(testVec, actor_origin, actor_rotation, actor_scale.X));
		}

		if (x.triangles.Num() == 0) continue;

		//FVector lastGlobal = MathLib::LocalToGlobal(x.renderableVertices[x.triangles[0]], actor_origin, actor_rotation, actor_scale.X);


		// render from renderableTri and 
		for (int i = 0; i < x.triangles.Num(); i += 3) {

			if (i + 2 > x.triangles.Num() - 1) { break; }

			// Create Indicies for tri
			TArray<int> ourMesh;
			
			// LMAO why 6
			ourMesh.Add(x.triangles[i]);
			ourMesh.Add(x.triangles[i+1]);
			ourMesh.Add(x.triangles[i + 2]);
			//ourMesh.Add(x.triangles[i + 3]);
			//ourMesh.Add(x.triangles[i + 4]);
			//ourMesh.Add(x.triangles[i + 5]);

			DrawDebugMesh(GetWorld(), renderableTri,ourMesh,triColor, true, -1.0f, 0);
			indexOfColors += 1;
			if (indexOfColors >= 5) {
				indexOfColors = 0;
			}
			triColor = possibleColors[indexOfColors];
		}

	/*	for (int i = 0; i < x.triangles.Num(); i++) {

	
			FVector current = MathLib::LocalToGlobal(x.renderableVertices[x.triangles[i]], actor_origin, actor_rotation, actor_scale.X);

			DrawDebugLine(GetWorld(), lastGlobal, current, triColor, true, -1.0f, 0, 10.0f);

			lastGlobal = current;
		}*/
	}

	FTimerHandle UniqueHandle;
	FTimerDelegate DelayDelegate = FTimerDelegate::CreateUObject(this, &UWall_Cutter::AlmostThere, test);
	GetWorld()->GetTimerManager().SetTimer(UniqueHandle, DelayDelegate, 2.f, false);
}

void UWall_Cutter::cutWall(bool shouldRenderRegion = true) {

	if (start_cut_polygon.Num() == 0) {
		return;
	}

	regions.Empty();
	TArray<Polygon::VertexData> visited;

	// Start walk
	for (Polygon::Vertex* current_vertex : cut_polygon_out) {

		if (visited.Contains(current_vertex->data) == false && current_vertex->data.type == Polygon::ENTRY) {

			Polygon new_region = walkLoop(visited, current_vertex, 1);

			regions.Add(new_region);
		}
	}

	UKismetSystemLibrary::FlushPersistentDebugLines(GetWorld());
	UKismetSystemLibrary::FlushDebugStrings(GetWorld());

	for (Polygon toRender : regions) {
		Draw_Polygon(toRender, "Show Cut", true, false);
	}

	if (shouldRenderRegion == false) return;


	FTimerHandle UniqueHandle;
	FTimerDelegate DelayDelegate = FTimerDelegate::CreateUObject(this, &UWall_Cutter::DisplayCut);
	GetWorld()->GetTimerManager().SetTimer(UniqueHandle, DelayDelegate, 2.f, false);
}

void UWall_Cutter::extrudeAndShow(extrudable input) {
	
	Polygon regionToRender = input.input;

	TArray<FVector2D> renderableVertices = input.renderableVertices;
	FJsonSerializableArrayInt trianglesStart = input.triangles;
	TArray<FVector> vertices3D;
	FJsonSerializableArrayInt triangles3D = trianglesStart;

	regionToRender.extrudePolygon(actor_scale.X, vertices3D, triangles3D, renderableVertices, trianglesStart);

	TArray<FVector> normals;
	TArray<FVector2d> uv0;
	TArray<FColor> vertexColors;
	TArray<FProcMeshTangent> tangents;

	/**
		*	Create/replace a section for this procedural mesh component.
		*	This function is deprecated for Blueprints because it uses the unsupported 'Color' type. Use new 'Create Mesh Section' function which uses LinearColor instead.
		*	@param	SectionIndex		Index of the section to create or replace.
		*	@param	Vertices			Vertex buffer of all vertex positions to use for this mesh section.
		*	@param	Triangles			Index buffer indicating which vertices make up each triangle. Length must be a multiple of 3.
		*	@param	Normals				Optional array of normal vectors for each vertex. If supplied, must be same length as Vertices array.
		*	@param	UV0					Optional array of texture co-ordinates for each vertex. If supplied, must be same length as Vertices array.
		*	@param	VertexColors		Optional array of colors for each vertex. If supplied, must be same length as Vertices array.
		*	@param	Tangents			Optional array of tangent vector for each vertex. If supplied, must be same length as Vertices array.
		*	@param	bCreateCollision	Indicates whether collision should be created for this section. This adds significant cost.
		*/

		//mesh->CreateMeshSection(0, vertices3D, triangles3D, normals, uv0, vertexColors, tangents, true);

	FActorSpawnParameters spawnParams = FActorSpawnParameters();
	spawnParams.Template = this->GetOwner();
	if (GetWorld()) {
		UE_LOG(LogTemp, Warning, TEXT("Created duplicate"));

		AStaticMeshActor* out = GetWorld()->SpawnActor<AStaticMeshActor>(FVector(0, 0, 0), FRotator(), spawnParams);
		UWall_Cutter* outCut = out->FindComponentByClass<UWall_Cutter>();

		outCut->start_wall_polygon = input.input;
		outCut->wall_polygon_out.Empty();
		outCut->cut_polygon_out.Empty();

		UProceduralMeshComponent* outProc = out->FindComponentByClass<UProceduralMeshComponent>();
		outProc->CreateMeshSection(0, vertices3D, triangles3D, normals, uv0, vertexColors, tangents, true);


		outCut->regions.Empty();
		outCut->start_cut_polygon.Empty();

	}
}

UWall_Cutter::renderOut UWall_Cutter::renderPolygon(Polygon regionToRender, bool testing) {

	Polygon input = regionToRender;

	TArray<FVector2D> renderableVertices;
	FJsonSerializableArrayInt trianglesStart;

	regionToRender.triangulatePolygon(renderableVertices, trianglesStart);

	TArray<FVector> vertices3D;
	FJsonSerializableArrayInt triangles3D = trianglesStart;

	regionToRender.extrudePolygon(actor_scale.X, vertices3D, triangles3D, renderableVertices, trianglesStart);

	if (testing == false) {
		TArray<FVector> normals;
		TArray<FVector2d> uv0;
		TArray<FColor> vertexColors;
		TArray<FProcMeshTangent> tangents;

		/**
		 *	Create/replace a section for this procedural mesh component.
		 *	This function is deprecated for Blueprints because it uses the unsupported 'Color' type. Use new 'Create Mesh Section' function which uses LinearColor instead.
		 *	@param	SectionIndex		Index of the section to create or replace.
		 *	@param	Vertices			Vertex buffer of all vertex positions to use for this mesh section.
		 *	@param	Triangles			Index buffer indicating which vertices make up each triangle. Length must be a multiple of 3.
		 *	@param	Normals				Optional array of normal vectors for each vertex. If supplied, must be same length as Vertices array.
		 *	@param	UV0					Optional array of texture co-ordinates for each vertex. If supplied, must be same length as Vertices array.
		 *	@param	VertexColors		Optional array of colors for each vertex. If supplied, must be same length as Vertices array.
		 *	@param	Tangents			Optional array of tangent vector for each vertex. If supplied, must be same length as Vertices array.
		 *	@param	bCreateCollision	Indicates whether collision should be created for this section. This adds significant cost.
		 */

		 //mesh->CreateMeshSection(0, vertices3D, triangles3D, normals, uv0, vertexColors, tangents, true);

		FActorSpawnParameters spawnParams = FActorSpawnParameters();
		spawnParams.Template = this->GetOwner();
		if (GetWorld()) {
			UE_LOG(LogTemp, Warning, TEXT("Created duplicate"));

			AStaticMeshActor* out = GetWorld()->SpawnActor<AStaticMeshActor>(FVector(0, 0, 0), FRotator(), spawnParams);
			UWall_Cutter* outCut = out->FindComponentByClass<UWall_Cutter>();

			outCut->start_wall_polygon = input;
			outCut->wall_polygon_out.Empty();
			outCut->cut_polygon_out.Empty();

			UProceduralMeshComponent* outProc = out->FindComponentByClass<UProceduralMeshComponent>();
			outProc->CreateMeshSection(0, vertices3D, triangles3D, normals, uv0, vertexColors, tangents, true);

	
			outCut->regions.Empty();
			outCut->start_cut_polygon.Empty();

		}
	}
	


	return { vertices3D,triangles3D };
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