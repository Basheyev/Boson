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

#include "io/CachedFileIO.h"
#include "test/CachedFileIOTest.h"

#include "core/BalancedTreeIndex.h"
#include "BalancedTreeTest.h"

#include <iostream>
#include <ctime>

using namespace Boson;


int main()
{
	using namespace std;
	
	CachedFileIOTest cft("F://database.bin");
	cft.run(1000000,384,0.05,0.05);

	/*for (size_t i = 1; i <= 3; i++) {
		cout << "\n======================================================================\n\n";
		cft.run(i * 300000);
	}*/

	return 0;
}
