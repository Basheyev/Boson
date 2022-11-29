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

	char* filename = "write.db";

	size_t bufferSize = 8096; // average JSON size
	
	
	//double stdTime = cf.stdioRandomRead(filename, 0, bufferSize);
	
	//double stdTime = cf.stdioSequencialRead(filename, bufferSize);

	cf.open(filename);
	//cf.sequencialWriteTest(0);
	cf.randomWriteTest();
	//double cachedTime = cf.randomReadTest(0, bufferSize);
	//double cachedTime = cf.sequencialReadTest(bufferSize);



	cf.close();

	//std::cout << "Cached File IO is: " << stdTime / cachedTime * 100.0 << "% of STDIO\n";

	return 0;
}
