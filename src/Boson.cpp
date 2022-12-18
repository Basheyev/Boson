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

	StorageIO storage;

	vector<string> myData;
		
	for (int i = 0; i < 3; i++) {
		myData.push_back(string("This is data record."));
	}

	filesystem::remove("f:/records.bin");

	if (storage.open("f:/records.bin")) {
		for (string& str : myData) {
			storage.insert(str.c_str(), (uint32_t)str.length());
		}
		storage.close();
	}
	else {
		cout << "can't open file." << endl;
	}


}
