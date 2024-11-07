/******************************************************************************
*
*  Boson Database
*
*  Features:
*  - NoSQL database engine.
*  - Single database file.
*  - Standard Key/Value store.
*  - Document store (JSON).
*  - Support cursors for linear records traversal.
*  - Support for on-disk as well in-memory databases.
*  - Support Terabyte sized databases.
*
*  (C) Boson Database, Bolat Basheyev 2022-2024
*
******************************************************************************/

#include "api/BosonAPI.h"
#include "BalancedIndexTest.h"
#include "RecordFileIOTest.h"
#include "BosonAPITest.h"


using namespace Boson;
using namespace std;





int main()
{
	BosonAPITest bapit("D:/data.bin");
	bapit.run();  // BUG: RecordFileIO free recrods list

	return 0;
}
