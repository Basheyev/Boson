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
	
	CachedFileIO cf;

	cf.open("F:/sign.txt");
	cf.write(0, &Boson::SIGNATURE, sizeof(uint64_t));
	cf.close();

	cout << sizeof Boson::StorageHeader << endl;

	return 0;
}
