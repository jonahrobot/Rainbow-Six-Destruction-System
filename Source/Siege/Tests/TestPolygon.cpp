#include "Misc/AutomationTest.h"

#include "Polygon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFoo, "Polygon.Basic", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FFoo::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Hi == Hi"), TEXT("Hi"), TEXT("Hi"));
	return true;
}