#ifdef UNITTEST_DZBRIDGE

#include "UnitTest_DzUnrealDialog.h"
#include "DzUnrealDialog.h"


bool UnitTest_DzUnrealDialog::runUnitTests()
{
	DzUnrealDialog* testObject = new DzUnrealDialog();

	if (!testObject)
	{
		return false;
	}

	return true;
}

#include "moc_UnitTest_DzUnrealDialog.cpp"

#endif