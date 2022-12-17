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

#include "CachedFileIO.h"
#include "CachedFileIOTest.h"

#include "StorageIO.h"

#include <iostream>

using namespace Boson;

int main()
{
	using namespace std;
	
	CachedFileIOTest cft("F://database.bin");
	cft.run();


	//cout << sizeof std::list<CachePage*>::iterator << endl;

	return 0;
}
