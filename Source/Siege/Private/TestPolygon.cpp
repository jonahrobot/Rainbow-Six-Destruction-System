#include "Misc/AutomationTest.h"

#include "Polygon.h"

#define TEST_FLAGS EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter

//Unit testing is about testing the public states, behaviors, and interactions of your objects.

// Vertex Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(Vertex_TestEquals, "Polygon.VertexTestEquals", TEST_FLAGS)

bool Vertex_TestEquals::RunTest(const FString& Parameters) {

	Polygon::VertexData v1 = { FVector2D(2, 3) };
	Polygon::VertexData v2 = { FVector2D(2.1, 3.1) };
	Polygon::VertexData v3 = { FVector2D(2, 3), Polygon::ENTRY };

	TestTrue("Vertex: Equals: Idential Vertex should equal", v1 == v1);
	TestTrue("Vertex: Equals: Similar pos vertex should equal", v1 == v2);
	
	v2.pos = FVector2D(2.6, 3.9);

	TestFalse("Vertex: Equals: Not same pos should not equal", v1 == v2);

	TestFalse("Vertex: Equals: Not same type should not equal", v1 == v3);

	FVector2D x = FVector2D(2, 1);

	TestTrue("Vertex: Equals: Test compareAproxPos math", Polygon::VertexData::compareAproxPos(x, x));

	return true;
}

// Iterator Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(Iterator_CheckIterator, "Polygon.CheckIterator", TEST_FLAGS)

bool Iterator_CheckIterator::RunTest(const FString& Parameters) {

	Polygon p1 = Polygon();

	Polygon::Vertex* v1 = p1.Add({ FVector2D(1, 2) });
	Polygon::Vertex* v2 = p1.Add({ FVector2D(2, 0) });
	Polygon::Vertex* v3 = p1.Add({ FVector2D(0, 0) });

	Polygon::PolygonIterator itr = p1.begin();

	TestEqual("Iterator: Start Vertex == first vertex", **itr, *v1);

	itr++;
	itr++;

	TestEqual("Iterator: Check ptr updates", **itr, *v3);

	itr++;

	TestEqual("Iterator: Check ptr loops", **itr, *v1);

	TestEqual("Iterator: Check Loop Count", itr.getLoopCount(), 1);

	Polygon::PolygonIterator itrB = p1.begin();
	itrB++; itrB++; itrB++;

	TestTrue("Iterator: Check itr equals", itr == itrB);

	return true;
}

// Polygon Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestAdd, "Polygon.TestAdd", TEST_FLAGS)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestStringConstructor, "Polygon.TestStringConstructor", TEST_FLAGS)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestCopyConstructor, "Polygon.TestCopyConstructor", TEST_FLAGS)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestInsert, "Polygon.TestInsert", TEST_FLAGS)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestEmpty, "Polygon.TestEmpty", TEST_FLAGS)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestUniqueData, "Polygon.TestUniqueData", TEST_FLAGS)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestInsidePolygon, "Polygon.TestInsidePolygon", TEST_FLAGS)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestRemovePolygon, "Polygon.TestRemovePolygon", TEST_FLAGS)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestTriangulate, "Polygon.TestTriangulate", TEST_FLAGS)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestClockwiseCheck, "Polygon.TestClockwiseCheck", TEST_FLAGS)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestSwapDirection, "Polygon.TestSwapDirection", TEST_FLAGS)


bool TestAdd::RunTest(const FString& Parameters) {

	Polygon p1 = Polygon();

	TestTrue("Polygon: Add: With no nodes Head is Null", p1.HeadNode == nullptr);
	TestTrue("Polygon: Add: With no nodes Tail is Null", p1.TailNode == nullptr);
	TestEqual("Polygon: Add: With no nodes Size is 0", p1.Num(), 0);

	Polygon::Vertex* v1 = p1.Add({ FVector2D(1, 2) });

	TestEqual("Polygon: Add: With one node, head correct", p1.HeadNode->data, v1->data);
	TestEqual("Polygon: Add: With one node, tail correct", p1.TailNode->data, v1->data);
	TestEqual("Polygon: Add: With one node, size Correct", p1.Num(), 1);

	TestTrue("Polygon: Add: With one node, points to self", v1->NextNode->data == v1->data && v1->PrevNode->data == v1->data);

	Polygon::Vertex* v2 = p1.Add({ FVector2D(2, 0) });
	Polygon::Vertex* v3 = p1.Add({ FVector2D(0, 0) });

	// Ensure: [v1] <-> [v2] <-> [v3] <-> [v1]

	TestEqual("Polygon: Add: 3 Nodes, Head Node Correct", p1.HeadNode->data, v1->data);
	TestEqual("Polygon: Add: 3 Nodes, Tail Node Correct", p1.TailNode->data, v3->data);
	TestEqual("Polygon: Add: 3 Nodes, Size Correct", p1.Num(), 3);

	TestTrue("Polygon: Add: 3 Nodes, V1 Properties", v1->NextNode->data == v2->data && v1->PrevNode->data == v3->data);
	TestTrue("Polygon: Add: 3 Nodes, V2 Properties", v2->NextNode->data == v3->data && v2->PrevNode->data == v1->data);
	TestTrue("Polygon: Add: 3 Nodes, V3 Properties", v3->NextNode->data == v1->data && v3->PrevNode->data == v2->data);

	return true;
}

bool TestInsert::RunTest(const FString& Parameters) {

	Polygon p1 = Polygon();
	
	Polygon::Vertex* v1 = p1.Add({ FVector2D(1, 2) });
	Polygon::Vertex* v2 = p1.Add({ FVector2D(2, 0) });
	Polygon::Vertex* v3 = p1.Insert({ FVector2D(0, 0) }, v2);
	Polygon::Vertex* v4 = p1.Insert({ FVector2D(0, 0) }, v3);

	// Ensure: [v1] <-> [v2] <-> [v3] -> [v4] <-> [v1]

	TestEqual("Polygon: Insert: Head Node Correct", p1.HeadNode->data, v1->data);
	TestEqual("Polygon: Insert: Tail Node Correct", p1.TailNode->data, v4->data);
	TestEqual("Polygon: Insert: Size Correct", p1.Num(), 4);

	TestTrue("Polygon: Insert: V1 Properties", v1->NextNode->data == v2->data && v1->PrevNode->data == v4->data);
	TestTrue("Polygon: Insert: V2 Properties", v2->NextNode->data == v3->data && v2->PrevNode->data == v1->data);
	TestTrue("Polygon: Insert: V3 Properties", v3->NextNode->data == v4->data && v3->PrevNode->data == v2->data);
	TestTrue("Polygon: Insert: V4 Properties", v4->NextNode->data == v1->data && v4->PrevNode->data == v3->data);

	Polygon p2 = Polygon();

	Polygon::Vertex* left = p2.Add({ FVector2D(1, 2) });
	Polygon::Vertex* right = p2.Add({ FVector2D(2, 0) });
	Polygon::Vertex* middle = p2.Insert({ FVector2D(0, 0) }, left);

	TestTrue("Polygon: Insert Case 2: V1 Properties", left->NextNode->data == middle->data && left->PrevNode->data == right->data);
	TestTrue("Polygon: Insert Case 2: V2 Properties", middle->NextNode->data == right->data && middle->PrevNode->data == left->data);
	TestTrue("Polygon: Insert Case 2: V3 Properties", right->NextNode->data == left->data && right->PrevNode->data == middle->data);

	return true;
}

bool TestStringConstructor::RunTest(const FString& Parameters) {

	Polygon stringBased = Polygon("(2, 2), (0, 2), (0, 1.5, ENTRY), (0, 0), (1, 0, EXIT), (2.3, 0)");

	Polygon base = Polygon();

	base.Add({ FVector2D(2, 2) });
	base.Add({ FVector2D(0, 2) });
	base.Add({ FVector2D(0, 1.5), Polygon::ENTRY });
	base.Add({ FVector2D(0, 0) });
	base.Add({ FVector2D(1, 0), Polygon::EXIT });
	base.Add({ FVector2D(2.3, 0) });

	TestEqual("Polygon: StringConstructor: String creates correct Polygon", stringBased, base);

	return true;
}

bool TestCopyConstructor::RunTest(const FString& Parameters) {

	Polygon pEmpty = Polygon();
	TestEqual("Polygon: Copy: Empty poly has size 0", pEmpty.Num(), 0);

	Polygon pCopyEmpty = pEmpty;
	TestEqual("Polygon: Copy: Copy of empty poly has size 0", pCopyEmpty.Num(), 0);

	Polygon p1 = Polygon();

	p1.Add({ FVector2D(1, 2) });
	p1.Add({ FVector2D(2, 0) });

	// Copy Constructor
	Polygon p2 = p1;
	TestEqual("Polygon: Copy: Copy of non-empty poly has matching size", p2.Num(), 2);

	// Assignment operator
	Polygon p3 = Polygon();
	p3 = p1;
	TestEqual("Polygon: Copy: Assignment copy of poly has matching size", p3.Num(), 2);

	// Assert p1 == p2
	TestEqual("Polygon: Copy: Copy constructor matches source poly",p1, p2);

	// Assert p1 == p3
	TestEqual("Polygon: Copy: Assigment copy matches source poly", p1, p3);

	return true;
}

bool TestEmpty::RunTest(const FString& Parameters) {
	Polygon p1 = Polygon();

	p1.Add({ FVector2D(1, 2) });
	p1.Add({ FVector2D(2, 0) });

	p1.Empty();

	TestTrue("Polygon: Empty: Polygon should have nullptr Head after empty", p1.HeadNode == nullptr);
	TestTrue("Polygon: Empty: Polygon should have nullptr Tail after empty", p1.TailNode == nullptr);

	// Test empty polygon
	Polygon p2 = Polygon();
	p2.Empty();

	TestEqual("Polygon: Empty: Size should be 0 after empty", p1.Num(),0);

	return true;
}

bool TestUniqueData::RunTest(const FString& Parameters) {

	Polygon p1 = Polygon();
	Polygon p2 = Polygon();

	Polygon::VertexData v1 = { FVector2D(0, 0) };

	p1.Add(v1);
	p2.Add(v1);

	Polygon::Vertex* h1 = p1.HeadNode;
	Polygon::Vertex* h2 = p2.HeadNode;

	h1->data.pos = FVector2D(1, 1);

	TestEqual("Polygon: Test no linking data across polygons", h2->data.pos, FVector2D(0,0));

	v1.pos = FVector2D(-1, -1);

	TestEqual("Polygon: Test no data updates from source", h1->data.pos, FVector2D(1, 1));
	TestEqual("Polygon: Test no data updates from source", h2->data.pos, FVector2D(0, 0)); 

	return true;
}

bool TestInsidePolygon::RunTest(const FString& Parameters) {

	Polygon p1 = Polygon("(1,1),(-1,1),(-1,-1),(1,-1)");

	// Test Inside Caught
	TestTrue("Polygon: InsidePolygon: TopCenter",p1.pointInsidePolygon(FVector2D(0,1))); // Fails
	TestTrue("Polygon: InsidePolygon: BottomCenter", p1.pointInsidePolygon(FVector2D(0, -1)));
	TestTrue("Polygon: InsidePolygon: MidLeft", p1.pointInsidePolygon(FVector2D(-1, 0))); // Fails
	TestTrue("Polygon: InsidePolygon: MidRight", p1.pointInsidePolygon(FVector2D(1, 0)));
	TestTrue("Polygon: InsidePolygon: Interior", p1.pointInsidePolygon(FVector2D(0.3, -0.5)));

	// Test Outside Caught
	TestFalse("Polygon: InsidePolygon: Off_TopCenter", p1.pointInsidePolygon(FVector2D(0, 1.1)));
	TestFalse("Polygon: InsidePolygon: Off_BottomCenter", p1.pointInsidePolygon(FVector2D(0, -1.1)));
	TestFalse("Polygon: InsidePolygon: Off_MidLeft", p1.pointInsidePolygon(FVector2D(-1.1, 0)));
	TestFalse("Polygon: InsidePolygon: Off_MidRight", p1.pointInsidePolygon(FVector2D(1.1, 0)));


	Polygon p2 = Polygon("(-250,250),(250,-200),(-250,46)");

	TestFalse("Polygon: InsidePolygon: ClipTest", p2.pointInsidePolygon(FVector2D(-250, -250)));


	Polygon p3 = Polygon("(0,-250,ENTRY),(-250,-250),(0,0)");
	TestFalse("Polygon: InsidePolygon: IDK", p2.pointInsidePolygon(FVector2D(250, 250)));

	return true;
}

bool TestRemovePolygon::RunTest(const FString& Parameters) {

	Polygon p1 = Polygon("(1,1)");

	Polygon::VertexData v1 = { FVector2D(0, 0) };
	Polygon::VertexData v2 = { FVector2D(10, 10) };

	// (1,1),(0,0),(10,10)

	Polygon::Vertex* middleNode = p1.Add(v1);
	Polygon::Vertex* endNode = p1.Add(v2);

	p1.Remove(middleNode);

	// (1,1),(10,10)
	Polygon testA = Polygon("(1,1),(10,10)");

	TestEqual("Polygon: Remove: Middle node removed", p1, testA);
	TestEqual("Polygon: Remove: Size decreased", p1.size, 2);

	p1.Remove(p1.HeadNode);
	p1.Remove(p1.TailNode);

	// Should be empty
	Polygon testB;
	TestEqual("Polygon: Remove: Middle node removed", p1, testB);
	TestEqual("Polygon: Remove: Size decreased", p1.size, 0);

	return true;
}

bool TestTriangulate::RunTest(const FString& Parameters) {

	Polygon poly = Polygon("(2,2),(2,0),(0,0),(0,2)");

	TArray<FVector2D> out_vertices;
	FJsonSerializableArrayInt out_triangles;
	poly.triangulatePolygon(out_vertices, out_triangles);

	TestEqual("Polygon: Found verticies do not match expected values.", out_vertices, { FVector2D(2,2),  FVector2D(2,0), FVector2D(0,0), FVector2D(0,2) });
	TestEqual("Polygon: Found triangles do not match expected values.", out_triangles, { 0,1,3,1,2,3 });

	return true;
}

bool TestClockwiseCheck::RunTest(const FString& Parameters) {

	Polygon A = Polygon("(2,2),(2,0),(0,0),(0,2)");	// Poly is clockwise
	Polygon B = Polygon("(2,2),(0,2),(0,0),(2,0)");	// Poly is not clockwise

	TestTrue("Polygon A should be clockwise", A.isPolygonClockwise());
	TestFalse("Polygon B should Not be clockwise", B.isPolygonClockwise());

	return true;
}

bool TestSwapDirection::RunTest(const FString& Parameters) {

	Polygon Start = Polygon("(2,2),(2,0),(0,0),(0,2)");	
	Polygon End = Polygon("(2,2),(0,2),(0,0),(2,0)");

	Start.flipPolygonVertexOrder();

	TestEqual("Polygon: Swap Dir Test: Start should become End", Start,End);

	return true;
}