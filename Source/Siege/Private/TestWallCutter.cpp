#include "Misc/AutomationTest.h"
#include "Wall_Cutter.h"
#include "Polygon.h"
#include "Tests/AutomationEditorCommon.h"

#define TEST_FLAGS EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
IMPLEMENT_SIMPLE_AUTOMATION_TEST(WallCutter_TwoSquaresOverlap, "WallCutter.TwoSquaresOverlap", TEST_FLAGS)

bool WallCutter_TwoSquaresOverlap::RunTest(const FString& Parameters) {

	// Initalize our wall_cutter

	UWall_Cutter* core = NewObject<UWall_Cutter>();

	core->actor_scale = FVector::OneVector;
	core->actor_origin = FVector::ZeroVector;
	core->actor_rotation = FRotator::ZeroRotator;

	// Add our input polygons to test
	core->start_wall_polygon = Polygon("(1,1),(-1,1),(-1,-1),(1,-1)");
	core->start_cut_polygon = Polygon("(2,2),(0,2),(0,0),(2,0)");

	// Preform cut
	core->cutWall();

	Polygon wall_polygon_out = Polygon("(1,1),(0,1,ENTRY),(-1,1),(-1,-1),(1,-1),(1,0,EXIT)");
	Polygon cut_polygon_out = Polygon("(2,2),(0,2),(0,1,ENTRY),(0,0),(1,0,EXIT),(2,0)");

	Polygon cut_region_a = Polygon("(0,1,ENTRY),(-1,1),(-1,-1),(1,-1),(1,0,EXIT),(0,0)");
	
	//// Print walls
	//core->wall_polygon_out.printPolygon("Test result wall:");
	//wall_polygon_out.printPolygon("Expected wall:");

	//// Print cuts
	//core->cut_polygon_out.printPolygon("Test result cut:");
	//cut_polygon_out.printPolygon("Expected cut:");

	TestEqual("WallCutter: WallCutter_TwoSquaresOverlap: Check wall polygon out correct", core->wall_polygon_out, wall_polygon_out);

	TestEqual("WallCutter: WallCutter_TwoSquaresOverlap: Check cut polygon out correct", core->cut_polygon_out, cut_polygon_out);

	TestEqual("WallCutter: WallCutter_TwoSquaresOverlap: Only one cut shape returned", core->regions.Num(), 1);
	if (core->regions.IsEmpty() == false) {
		TestEqual("WallCutter: WallCutter_TwoSquaresOverlap: Check out cut shape 1 correct", core->regions[0], cut_region_a);
	}
	else {
		return false;
	}

	return true;
}
