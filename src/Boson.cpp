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

#include <chrono>
#include <iostream>
#include <string>

using namespace Boson;




int main()
{
	CachedFileIOTest cf;

	char* filename = "datafile.db"; // "write.db";

	size_t bufferSize = 1565; // average JSON size

	//cf.stdioSequencialRead(filename, bufferSize);
	

	cf.open(filename);
	//cf.sequencialWriteTest(0);
	cf.randomReadTest(1000, bufferSize);
	//cf.sequencialReadTest(bufferSize);
	cf.close();

	
	cf.stdioRandomRead(filename, 1000, bufferSize);


	return 0;
}
