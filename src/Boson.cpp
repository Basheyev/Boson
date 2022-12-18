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
#include <sstream>

using namespace Boson;
using namespace std;

void generateData(char* filename) {
	StorageIO storage;
	vector<string> myData;
	stringstream ss;

	for (int i = 0; i < 10; i++) {
		ss.clear();
		ss << "This is data record under number = ";
		ss << i;
		myData.push_back(ss.str());
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


int main()
{
	char buf[1024] = { 0 };
	char* fileName = "f:/records.bin";
	generateData(fileName);

	StorageIO db;
	db.open(fileName);
	db.first();
	while (db.next()) {
		cout << " Offset: " << db.getPosition() << " Length: " << db.getLength() << endl;
	}
	db.close();

}
