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

	TestTrue("Test Vertices Equals", v1 == v1);
	TestTrue("Test Vertices Equals", v1 == v2);
	
	v2.pos = FVector2D(2.6, 3.9);

	TestFalse("Test Vertices Don't Equal", v1 == v2);

	TestFalse("Test Vertices Don't Equal", v1 == v3);

	FVector2D x = FVector2D(2, 1);

	TestTrue("Test compareAproxPos", Polygon::VertexData::compareAproxPos(x, x));

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

	TestEqual("Iterator: Check Start Vertex Correct", **itr, *v1);

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
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestCopyConstructor, "Polygon.TestCopyConstructor", TEST_FLAGS)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestInsert, "Polygon.TestInsert", TEST_FLAGS)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestEmpty, "Polygon.TestEmpty", TEST_FLAGS)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestUniqueData, "Polygon.TestUniqueData", TEST_FLAGS)

bool TestAdd::RunTest(const FString& Parameters) {

	Polygon p1 = Polygon();

	Polygon::Vertex* v1 = p1.Add({ FVector2D(1, 2) });
	Polygon::Vertex* v2 = p1.Add({ FVector2D(2, 0) });
	Polygon::Vertex* v3 = p1.Add({ FVector2D(0, 0) });

	// Ensure: [v1] <-> [v2] <-> [v3] <-> [v1]

	TestEqual("Polygon: Head Node Correct", p1.HeadNode->data, v1->data);
	TestEqual("Polygon: Tail Node Correct", p1.TailNode->data, v3->data);
	TestEqual("Polygon: Size Correct", p1.Num(), 3);

	TestTrue("Polygon: V1 Properties", v1->NextNode->data == v2->data && v1->PrevNode->data == v3->data);
	TestTrue("Polygon: V2 Properties", v2->NextNode->data == v3->data && v2->PrevNode->data == v1->data);
	TestTrue("Polygon: V3 Properties", v3->NextNode->data == v1->data && v3->PrevNode->data == v2->data);

	return true;
}

bool TestInsert::RunTest(const FString& Parameters) {

	Polygon p1 = Polygon();
	
	Polygon::Vertex* v1 = p1.Add({ FVector2D(1, 2) });
	Polygon::Vertex* v2 = p1.Add({ FVector2D(2, 0) });
	Polygon::Vertex* v3 = p1.Insert({ FVector2D(0, 0) }, v2);
	Polygon::Vertex* v4 = p1.Insert({ FVector2D(0, 0) }, v3);

	// Ensure: [v1] <-> [v2] <-> [v3] -> [v4] <-> [v1]

	TestEqual("Polygon: Head Node Correct", p1.HeadNode->data, v1->data);
	TestEqual("Polygon: Tail Node Correct", p1.TailNode->data, v4->data);
	TestEqual("Polygon: Size Correct", p1.Num(), 4);

	TestTrue("Polygon: V1 Properties", v1->NextNode->data == v2->data && v1->PrevNode->data == v4->data);
	TestTrue("Polygon: V2 Properties", v2->NextNode->data == v3->data && v2->PrevNode->data == v1->data);
	TestTrue("Polygon: V3 Properties", v3->NextNode->data == v4->data && v3->PrevNode->data == v2->data);
	TestTrue("Polygon: V4 Properties", v4->NextNode->data == v1->data && v4->PrevNode->data == v3->data);

	return true;
}

bool TestCopyConstructor::RunTest(const FString& Parameters) {

	Polygon p1 = Polygon();

	Polygon::VertexData v1 = { FVector2D(1, 2) };
	Polygon::VertexData v2 = { FVector2D(2, 0) };
	p1.Add(v1);
	p1.Add(v2);

	// Copy Constructor
	Polygon p2 = p1;

	// Assignment operator
	Polygon p3 = Polygon();
	p3 = p1;

	// Assert p1 == p2
	TestEqual("Polygon: Test Copy Constructor",p1, p2);

	// Assert p1 == p3
	TestEqual("Polygon: Test Assignment operator", p1, p3);

	return true;
}

bool TestEmpty::RunTest(const FString& Parameters) {
	Polygon p1 = Polygon();

	Polygon::VertexData v1 = { FVector2D(1, 2) };
	Polygon::VertexData v2 = { FVector2D(2, 0) };

	p1.Add(v1);
	p1.Add(v2);

	p1.Empty();

	TestEqual("Polygon: Check empty", p1.Num(),0);

	return true;
}

bool TestUniqueData::RunTest(const FString& Parameters) {

	Polygon p1 = Polygon();
	Polygon p2 = Polygon();

	Polygon::VertexData v1 = { FVector2D(0, 0) };

	p1.Add(v1);
	p2.Add(v1);

	// If we modify v1 shouldn't modify p1 or p2s v1
	// If we modify v1 in p1 shouldn't modify p1

	Polygon::Vertex* h1 = p1.HeadNode;
	Polygon::Vertex* h2 = p2.HeadNode;

	h1->data.pos = FVector2D(1, 1);

	TestEqual("Polygon: Test no linking data across polygons", h2->data.pos, FVector2D(0,0));

	v1.pos = FVector2D(-1, -1);

	TestEqual("Polygon: Test no data updates from source", h1->data.pos, FVector2D(1, 1));
	TestEqual("Polygon: Test no data updates from source", h2->data.pos, FVector2D(0, 0)); 

	return true;
}