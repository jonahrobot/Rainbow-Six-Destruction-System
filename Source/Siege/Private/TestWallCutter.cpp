#include "Misc/AutomationTest.h"
#include "Wall_Cutter.h"
#include "Polygon.h"
#include "Tests/AutomationEditorCommon.h"

#define TEST_FLAGS EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
IMPLEMENT_SIMPLE_AUTOMATION_TEST(WallCutter_TwoSquaresOverlap, "WallCutter.TwoSquaresOverlap", TEST_FLAGS)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(WallCutter_RectangleSplitTest, "WallCutter.RectangleSplitTest", TEST_FLAGS)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(WallCutter_TriangulateTest, "WallCutter.TriangulateTest", TEST_FLAGS)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(WallCutter_ExtrudeTest, "WallCutter.ExtrudeTest", TEST_FLAGS)



class TestWallCutter {

public:

	struct CutTestCase {
		FString testName;
		FString in_cut_poly;
		FString out_wall_poly;
		FString out_cut_poly;
		TArray<FString> out_regions;
	};

	bool static TestEqual(FString errorString, Polygon a, Polygon b) {
		if (a != b) {
			UE_LOG(LogTemp, Warning, TEXT("%s"),*errorString);
			return false;
		}
		return true;
	}

	bool static ProcessTestCase(CutTestCase x) {

		// Init
		UWall_Cutter* core = NewObject<UWall_Cutter>();

		core->actor_scale = FVector::OneVector;
		core->actor_origin = FVector::ZeroVector;
		core->actor_rotation = FRotator::ZeroRotator;
		core->start_wall_polygon = Polygon("(1,1),(-1,1),(-1,-1),(1,-1)");

		int testsPassed = 0;

		// Init test case
		core->start_cut_polygon = Polygon(x.in_cut_poly);

		Polygon expected_wall_poly = Polygon(x.out_wall_poly);
		Polygon expected_cut_poly = Polygon(x.out_cut_poly);

		// Preform cut
		core->cutWall(false);

		// Test Wall
		testsPassed += TestEqual("WallCutter:" + x.testName + " Check wall polygon correct failed", core->wall_polygon_out, expected_wall_poly);
		testsPassed += TestEqual("WallCutter:" + x.testName + " Check cut polygon correct failed", core->cut_polygon_out, expected_cut_poly);

		// Test Regions
		for (int i = 0; i < x.out_regions.Num(); i++) {

			// Check if matching region exists
			if (i >= core->regions.Num()) {
				UE_LOG(LogTemp, Warning, TEXT("WallCutter: %s Expected %d regions, got %d."), *(x.testName), x.out_regions.Num(), core->regions.Num());
			}
			else {

				// Test if Regions match
				Polygon expected_region = Polygon(x.out_regions[i]);
				testsPassed += TestEqual("WallCutter:" + x.testName + " Out region " + FString::FromInt(i) + " does not match expected region.", core->regions[i], expected_region);
			}
		}
		// Test both regions + Wall Polygon and Cut Polygon ended up correct
		return testsPassed == 2 + x.out_regions.Num();
	}
};


bool WallCutter_TwoSquaresOverlap::RunTest(const FString& Parameters) {
	
	TestWallCutter::CutTestCase x{
		"Two Squares Overlap",											// Name
		"(2,2),(0,2),(0,0),(2,0)",										// Cut Wall in
		"(1,1),(0,1,ENTRY),(-1,1),(-1,-1),(1,-1),(1,0,EXIT)",			// Wall Out
		"(2,2),(0,2),(0,1,ENTRY),(0,0),(1,0,EXIT),(2,0)",				// Cut Out
		{																// Regions out
			"(0,1,ENTRY),(0,0),(1,0,EXIT),(1,-1),(-1,-1),(-1,1)"
		}
	};
	
	return TestWallCutter::ProcessTestCase(x);
}

bool WallCutter_RectangleSplitTest::RunTest(const FString& Parameters) {

	TestWallCutter::CutTestCase x{
		"Rectangle Split Test",																				// Name
		"(0.5,2),(-0.5,2),(-0.5,-2),(0.5,-2)",																// Cut Wall in
		"(1,1),(0.5,1,EXIT),(-0.5,1,ENTRY),(-1,1),(-1,-1),(-0.5,-1,EXIT),(0.5,-1,ENTRY),(1,-1)",			// Wall Out
		"(0.5,2),(-0.5,2),(-0.5,1,ENTRY),(-0.5,-1,EXIT),(-0.5,-2),(0.5,-2),(0.5,-1,ENTRY),(0.5,1,EXIT)",	// Cut Out
		{																									// Regions out
			"(-0.5,1,ENTRY),(-0.5,-1,EXIT),(-1,-1),(-1,1)",
			"(0.5,-1,ENTRY),(0.5,1,EXIT),(1,1),(1,-1)"
		}
	};

	return TestWallCutter::ProcessTestCase(x);
}

bool WallCutter_TriangulateTest::RunTest(const FString& Parameters) {

	Polygon poly = Polygon("(2,2),(2,0),(0,0),(0,2)");

	UWall_Cutter* core = NewObject<UWall_Cutter>();

	TArray<FVector2D> out_vertices;
	FJsonSerializableArrayInt out_triangles;
	core->triangulatePolygon(out_vertices, out_triangles, poly);

	TestEqual("WallCutter: Found verticies do not match expected values.", out_vertices, { FVector2D(2,2),  FVector2D(2,0), FVector2D(0,0), FVector2D(0,2)});
	TestEqual("WallCutter: Found triangles do not match expected values.", out_triangles, { 0,1,3,1,2,3 });

	return true;
}

bool WallCutter_ExtrudeTest::RunTest(const FString& Parameters) {

	Polygon input = Polygon(R"((-249.999985, -16.478102, ENTRY), (-195.448657, -29.212806, NONE), (-138.992821, -34.614438, NONE), "
		"(-97.101171, -35.850874, NONE), (-54.033271, -34.226103, NONE), (-25.803016, -37.322383, NONE), (-17.744017, -36.969197, NONE), (-9.676088, -36.608087, NONE), "
		"(-5.638574, -36.424547, NONE), (45.26098, 59.993483, NONE), (45.26098, 59.993483, NONE), (45.26098, 59.993483, NONE), (49.389202, 60.249123, NONE),"
		"(77.889357, 62.090876, NONE), (100.854706, 72.786052, NONE), (122.908255, 81.310403, NONE), (150.800836, 96.783303, NONE), (172.653196, 117.147103, NONE),"
		"(182.121048, 147.558851, NONE), (182.584432, 191.621302, NONE), (139.556278, 233.011042, NONE), (127.548462, 250.000015, EXIT), (250, 250, NONE), (250, -250, NONE), (-250, -250, NONE))");

	FJsonSerializableArrayInt expectedTriangles = {0,1,24,1,2,24,2,3,24,3,4,24,4,5,24,5,6,24,6,7,
		24,7,8,24,14,15,13,20,21,19,21,22,19,22,23,19,24,8,23,8,9,23,19,23,18,18,23,17,17,23,16,16,
		23,15,15,23,13,13,23,12,23,9,12,9,9,12,9,9,12,49,26,25,49,27,26,49,28,27,49,29,28,49,30,29,
		49,31,30,49,32,31,49,33,32,38,40,39,44,46,45,44,47,46,44,48,47,48,33,49,48,34,33,43,48,44,42,
		48,43,41,48,42,40,48,41,38,48,40,37,48,38,37,34,48,37,34,34,37,34,34,24,25,0,25,24,49,0,26,1,26,
		0,25,1,27,2,27,1,26,2,28,3,28,2,27,3,29,4,29,3,28,4,30,5,30,4,29,5,31,6,31,5,30,6,32,7,32,6,31,7,
		33,8,33,7,32,8,34,9,34,8,33,9,35,10,35,9,34,10,36,11,36,10,35,11,37,12,37,11,36,12,38,13,38,12,37,
		13,39,14,39,13,38,14,40,15,40,14,39,15,41,16,41,15,40,16,42,17,42,16,41,17,43,18,43,17,42,18,44,19,
		44,18,43,19,45,20,45,19,44,20,46,21,46,20,45,21,47,22,47,21,46,22,48,23,48,22,47,23,49,24,49,23,48};


	TArray<FVector> castedVertices = {
		FVector(100.000,-250.000,-16.478),
		FVector(100.000,-195.449,-29.213),
		FVector(100.000, -138.993, -34.614),
		FVector(100.000, -97.101, -35.851),
		FVector(100.000, -54.033, -34.226),
		FVector(100.000, -25.803, -37.322),
		FVector(100.000, -17.744, -36.969),
		FVector(100.000, -9.676, -36.608),
		FVector(100.000, -5.639, -36.425),
		FVector(100.000, 45.261, 59.993),
		FVector(100.000, 45.261, 59.993),
		FVector(100.000, 45.261, 59.993),
		FVector(100.000, 49.389, 60.249),
		FVector(100.000, 77.889, 62.091),
		FVector(100.000, 100.855, 72.786),
		FVector(100.000, 122.908, 81.310),
		FVector(100.000, 150.801, 96.783),
		FVector(100.000, 172.653, 117.147),
		FVector(100.000, 182.121, 147.559),
		FVector(100.000, 182.584, 191.621),
		FVector(100.000, 139.556, 233.011),
		FVector(100.000, 127.548, 250.000),
		FVector(100.000, 250.000, 250.000),
		FVector(100.000, 250.000, -250.000),
		FVector(100.000, -250.000, -250.000),
		FVector(-100.000, -250.000, -16.478),
		FVector(-100.000, -195.449, -29.213),
		FVector(-100.000, -138.993, -34.614),
		FVector(-100.000, -97.101, -35.851),
		FVector(-100.000, -54.033, -34.226),
		FVector(-100.000, -25.803, -37.322),
		FVector(-100.000, -17.744, -36.969),
		FVector(-100.000, -9.676, -36.608),
		FVector(-100.000, -5.639, -36.425),
		FVector(-100.000, 45.261, 59.993),
		FVector(-100.000, 45.261, 59.993),
		FVector(-100.000, 45.261, 59.993),
		FVector(-100.000, 49.389, 60.249),
		FVector(-100.000, 77.889, 62.091),
		FVector(-100.000, 100.855, 72.786),
		FVector(-100.000, 122.908, 81.310),
		FVector(-100.000, 150.801, 96.783),
		FVector(-100.000, 172.653, 117.147),
		FVector(-100.000, 182.121, 147.559),
		FVector(-100.000, 182.584, 191.621),
		FVector(-100.000, 139.556, 233.011),
		FVector(-100.000, 127.548, 250.000),
		FVector(-100.000, 250.000, 250.000),
		FVector(-100.000, 250.000, -250.000),
		FVector(-100.000, -250.000, -250.000)
	};

	UWall_Cutter* core = NewObject<UWall_Cutter>();

	UWall_Cutter::renderOut output = core->startRenderProcess(input, true);

	bool vertexEqual = true;

	if (output.renderableVertices.Num() != castedVertices.Num()) {
		vertexEqual = false;
	}

	if (vertexEqual) {
		for (int i = 0; i < castedVertices.Num(); ++i) {

			FVector A = castedVertices[i];
			FVector B = output.renderableVertices[i];

			if( std::roundf(A.X) != std::roundf(B.X) ||
				std::roundf(A.Y) != std::roundf(B.Y) ||
				std::roundf(A.Z) != std::roundf(B.Z) ){ 
					vertexEqual = false;
					break;
			}
		}
	}

	TestEqual("WallCutter: Test Extrude: Vertex Check ", vertexEqual, true);
	TestEqual("WallCutter: Test Extrude: Tri Check ", output.triangles, expectedTriangles);

	return true;
}
