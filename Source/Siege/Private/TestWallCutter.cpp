#include "Misc/AutomationTest.h"
#include "Wall_Cutter.h"
#include "Polygon.h"
#include "Tests/AutomationEditorCommon.h"

#define TEST_FLAGS EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
IMPLEMENT_SIMPLE_AUTOMATION_TEST(WallCutter_TwoSquaresOverlap, "WallCutter.TwoSquaresOverlap", TEST_FLAGS)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(WallCutter_RectangleSplitTest, "WallCutter.RectangleSplitTest", TEST_FLAGS)


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

		bool passedAllTests = true;

		// Init test case
		core->start_cut_polygon = Polygon(x.in_cut_poly);

		Polygon expected_wall_poly = Polygon(x.out_wall_poly);
		Polygon expected_cut_poly = Polygon(x.out_cut_poly);

		// Preform cut
		core->cutWall();

		// Test Wall
		passedAllTests = TestEqual("WallCutter:" + x.testName + " Check wall polygon correct failed", core->wall_polygon_out, expected_wall_poly);
		passedAllTests = TestEqual("WallCutter:" + x.testName + " Check cut polygon correct failed", core->cut_polygon_out, expected_cut_poly);

		// Test Regions
		for (int i = 0; i < x.out_regions.Num(); i++) {

			// Check if matching region exists
			if (i >= core->regions.Num()) {
				UE_LOG(LogTemp, Warning, TEXT("WallCutter: %s Expected %d regions, got %d."), *(x.testName), x.out_regions.Num(), core->regions.Num());
				passedAllTests = false;
			}
			else {

				// Test if Regions match
				Polygon expected_region = Polygon(x.out_regions[i]);
				passedAllTests = TestEqual("WallCutter:" + x.testName + " Out region " + FString::FromInt(i) + " does not match expected region.", core->regions[i], expected_region);
			}
		}

		return passedAllTests;
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