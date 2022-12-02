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

#include <fstream>
#include <filesystem>

using namespace Boson;


namespace fs = std::filesystem;

int main()
{
	CachedFileIOTest cft("database.db");

	cft.generateFileData();
		
	

	return 0;
}
