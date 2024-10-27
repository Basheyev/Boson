/*
* ==========================================================================
* Boson Database
* ==========================================================================
* - NoSQL database engine.
* - Single database file.
* - Standard Key/Value store.
* - Document store (JSON).
* - Support cursors for linear records traversal.
* - Support for on-disk as well in-memory databases.
* - Support Terabyte sized databases.
* ==========================================================================
* (C) Bolat Basheyev 2022
*/

#include "Boson.h"
#include "BalancedIndexTest.h"


using namespace Boson;
using namespace std;


int main()
{
	char* filename = "D:/bptree.bin";
	BalancedIndexTest bit(filename);
	bit.run();

	return 0;
}
