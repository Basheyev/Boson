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



void standardIORead(char* filename) {
	
}


int main()
{
	CachedFileIOTest cf;

	char* filename = "write.db";

	size_t bufferSize = 1565; // average JSON size

	cf.open(filename);
	cf.sequencialWriteTest(0);
	cf.sequencialReadTest(bufferSize);
	cf.close();

	//cf.stdioSequencialRead(filename, bufferSize);

	return 0;
}
