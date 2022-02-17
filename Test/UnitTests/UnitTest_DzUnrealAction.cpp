#ifdef UNITTEST_DZBRIDGE

#include "UnitTest_DzUnrealAction.h"
#include "DzUnrealAction.h"


bool UnitTest_DzUnrealAction::runUnitTests()
{
	DzUnrealAction* testObject = new DzUnrealAction();

	if (!testObject)
	{
		return false;
	}

	return true;
}


#include "moc_UnitTest_DzUnrealAction.cpp"

#endif