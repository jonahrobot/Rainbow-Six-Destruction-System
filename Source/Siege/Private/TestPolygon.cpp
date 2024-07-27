#include "Misc/AutomationTest.h"

#include "Polygon.h"

#define TEST_FLAGS EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter

//Unit testing is about testing the public states, behaviors, and interactions of your objects.

// Vertex Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(Vertex_TestEquals, "Polygon.VertexTestEquals", TEST_FLAGS)

bool Vertex_TestEquals::RunTest(const FString& Parameters) {

	Polygon::Vertex v1 = Polygon::Vertex(FVector2D(2, 3), Polygon::NONE);
	Polygon::Vertex v2 = Polygon::Vertex(FVector2D(2.1, 3.1), Polygon::NONE);
	Polygon::Vertex v3 = Polygon::Vertex(FVector2D(2, 3), Polygon::ENTRY);

	TestTrue("Test Vertices Equals", v1 == v1);
	TestTrue("Test Vertices Equals", v1.equals(v2));
	TestTrue("Test Vertices Equals", v1 == v2);
	
	v2.pos = FVector2D(2.6, 3.9);

	TestFalse("Test Vertices Don't Equal", v1.equals(v2));

	TestFalse("Test Vertices Don't Equal", v1.equals(v3));

	FVector2D x = FVector2D(2, 1);

	TestTrue("Test compareAproxPos", Polygon::Vertex::compareAproxPos(x, x));

	return true;
}

// Iterator Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(Iterator_CheckIterator, "Polygon.CheckIterator", TEST_FLAGS)

bool Iterator_CheckIterator::RunTest(const FString& Parameters) {

	Polygon p1 = Polygon();

	Polygon::Vertex* v1 = new Polygon::Vertex(FVector2D(1, 2), Polygon::NONE);
	Polygon::Vertex* v2 = new Polygon::Vertex(FVector2D(2, 0), Polygon::NONE);
	Polygon::Vertex* v3 = new Polygon::Vertex(FVector2D(0, 0), Polygon::NONE);
	p1.Add(v1);
	p1.Add(v2);
	p1.Add(v3);

	Polygon::PolygonIterator itr = p1.begin();

	TestEqual("Iterator: Check Start Vertex Correct", *itr, *v1);

	itr++;
	itr++;

	TestEqual("Iterator: Check ptr updates", *itr, *v3);

	itr++;

	TestEqual("Iterator: Check ptr loops", *itr, *v1);

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

	Polygon::Vertex* v1 = new Polygon::Vertex(FVector2D(1, 2), Polygon::NONE);
	Polygon::Vertex* v2 = new Polygon::Vertex(FVector2D(2, 0), Polygon::NONE);
	Polygon::Vertex* v3 = new Polygon::Vertex(FVector2D(0, 0), Polygon::NONE);
	p1.Add(v1);
	p1.Add(v2);
	p1.Add(v3);

	// Ensure: [v1] <-> [v2] <-> [v3] <-> [v1]

	TestEqual("Polygon: Head Node Correct", *p1.HeadNode, *v1);
	TestEqual("Polygon: Tail Node Correct", *p1.TailNode, *v3);
	TestEqual("Polygon: Size Correct", p1.Num(), 3);

	TestTrue("Polygon: V1 Properties", v1->NextNode->equals(*v2) && v1->PrevNode->equals(*v3));
	TestTrue("Polygon: V2 Properties", v2->NextNode->equals(*v3) && v2->PrevNode->equals(*v1));
	TestTrue("Polygon: V3 Properties", v3->NextNode->equals(*v1) && v3->PrevNode->equals(*v2));

	return true;
}

bool TestInsert::RunTest(const FString& Parameters) {

	Polygon p1 = Polygon();

	Polygon::Vertex* v1 = new Polygon::Vertex(FVector2D(1, 2), Polygon::NONE);
	Polygon::Vertex* v2 = new Polygon::Vertex(FVector2D(2, 0), Polygon::NONE);
	p1.Add(v1);
	p1.Add(v2);

	Polygon::Vertex* ivA = new Polygon::Vertex(FVector2D(0, 0), Polygon::NONE);
	Polygon::Vertex* ivB = new Polygon::Vertex(FVector2D(0, 0), Polygon::NONE);

	p1.Insert(ivA, v2);
	p1.Insert(ivB, ivA);

	// Ensure: [v1] <-> [v2] <-> [ivA] -> [ivB] <-> [v1]

	TestEqual("Polygon: Head Node Correct", *p1.HeadNode, *v1);
	TestEqual("Polygon: Tail Node Correct", *p1.TailNode, *ivB);
	TestEqual("Polygon: Size Correct", p1.Num(), 4);

	TestTrue("Polygon: V1 Properties", v1->NextNode->equals(*v2) && v1->PrevNode->equals(*ivB));
	TestTrue("Polygon: V2 Properties", v2->NextNode->equals(*ivA) && v2->PrevNode->equals(*v1));
	TestTrue("Polygon: ivA Properties", ivA->NextNode->equals(*ivB) && ivA->PrevNode->equals(*v2));
	TestTrue("Polygon: ivB Properties", ivB->NextNode->equals(*v1) && ivB->PrevNode->equals(*ivA));

	return true;
}

bool TestCopyConstructor::RunTest(const FString& Parameters) {

	Polygon p1 = Polygon();

	Polygon::Vertex* v1 = new Polygon::Vertex(FVector2D(1, 2), Polygon::NONE);
	Polygon::Vertex* v2 = new Polygon::Vertex(FVector2D(2, 0), Polygon::NONE);
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

// What happens if I do
// Vertex v1
// p1.Add(v1) and p2.Add(v1) they would update each of their internal stuff
// Fucking up the data structure

bool TestEmpty::RunTest(const FString& Parameters) {
	Polygon p1 = Polygon();

	Polygon::Vertex* v1 = new Polygon::Vertex(FVector2D(1, 2), Polygon::NONE);
	Polygon::Vertex* v2 = new Polygon::Vertex(FVector2D(2, 0), Polygon::NONE);

	/*
	
	Best use case

	- Creating vertex, adding to polygon, refrenceing vertex later

	Public access to vertex is critical, wrapper could be used for vertex usage like Unreal
	Wrapper not public avalible. 

	Vertex v = Vertex(FVector2D(2,2), NONE); // Will be deleted once out of scope!

	Polygon p1 = Polygon();

	p1.Add(v); // Adds copy of v into polygon.


	doing (v->pos = new value) is invalid as v is not referencing the correct node
	To inforce this behavior we can either make those values const or private and have specific getters only friends can modify


	Polygons in use case should be add to and editable!
	
	*/

	
	p1.Add(v1);
	p1.Add(v2);

	p1.Empty();

	TestEqual("Polygon: Check empty", p1.Num(),0);

	return true;
}

bool TestUniqueData::RunTest(const FString& Parameters) {

	Polygon p1 = Polygon();
	Polygon p2 = Polygon();

	Polygon::Vertex* v1 = new Polygon::Vertex(FVector2D(0, 0), Polygon::NONE);

	p1.Add(v1);
	p2.Add(v1);

	// If we modify v1 shouldn't modify p1 or p2s v1
	// If we modify v1 in p1 shouldn't modify p1

	Polygon::Vertex* h1 = p1.HeadNode;
	Polygon::Vertex* h2 = p2.HeadNode;

	h1->pos = FVector2D(1, 1);

	TestEqual("Polygon: Test no linking data across polygons", h2->pos, FVector2D(0,0));

	v1->pos = FVector2D(-1, -1);

	TestEqual("Polygon: Test no data updates from source", h1->pos, FVector2D(1, 1));
	TestEqual("Polygon: Test no data updates from source", h2->pos, FVector2D(0, 0)); 

	return true;
}

// Also don't check vertex is deleted
