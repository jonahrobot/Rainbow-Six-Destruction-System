#include "Misc/AutomationTest.h"

#include "Polygon.h"

#define TEST_FLAGS EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter

// Vertex Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(Vertex_TestEquals, "Polygon.Vertex.TestEquals", TEST_FLAGS)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(Vertex_TestConstructor, "Polygon.Vertex.TestConstructor", TEST_FLAGS)

bool Vertex_TestEquals::RunTest(const FString& Parameters) {

}

bool Vertex_TestConstructor::RunTest(const FString& Parameters) {

}

// Iterator Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(Iterator_CheckPointerUpdates, "Polygon.Iterator.CheckPointerUpdates", TEST_FLAGS)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(Iterator_CheckLoopCountUpdates, "Polygon.Iterator.CheckLoopCountUpdates", TEST_FLAGS)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(Iterator_TestEquals, "Polygon.Iterator.TestEquals", TEST_FLAGS)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(Iterator_TestPointerMatchesVertex, "Polygon.Iterator.TestPointerMatchesVertex", TEST_FLAGS)

bool Iterator_CheckPointerUpdates::RunTest(const FString& Parameters) {

}

bool Iterator_CheckLoopCountUpdates::RunTest(const FString& Parameters) {

}

bool Iterator_TestEquals::RunTest(const FString& Parameters) {

}

bool Iterator_TestPointerMatchesVertex::RunTest(const FString& Parameters) {

}

// Polygon Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestCircularDLLProperties, "Polygon.TestCircularDLLProperties", TEST_FLAGS)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestCopyConstructor, "Polygon.TestCopyConstructor", TEST_FLAGS)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestAddInsert, "Polygon.TestAddInsert", TEST_FLAGS)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestEmpty, "Polygon.TestEmpty", TEST_FLAGS)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestPointInsidePolygon, "Polygon.TestPointInsidePolygon", TEST_FLAGS)

bool TestCircularDLLProperties::RunTest(const FString& Parameters) {

}

bool TestCopyConstructor::RunTest(const FString& Parameters) {

}

bool TestAddInsert::RunTest(const FString& Parameters) {

}

bool TestEmpty::RunTest(const FString& Parameters) {

}

bool TestPointInsidePolygon::RunTest(const FString& Parameters) {

}




//bool Vertex_TestEquals::RunTest(const FString& Parameters)
//{
//	TestEqual(TEXT("Hi == Hi"), TEXT("Hi"), TEXT("Hi"));
//	return true;
//}
