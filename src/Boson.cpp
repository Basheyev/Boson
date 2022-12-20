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


uint64_t callStack1, callStack2;


void generateData(char* filename) {
	StorageIO storage;
	vector<string> myData;	
	
	for (int i = 0; i < 10; i++) {
		stringstream ss;		
		ss << "Database record message #";		
		ss << i;
		ss << " and " << (int64_t(std::rand()) * 100);
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
	int var1;
	callStack1 = (uint64_t) &var1;
	char buf[1024] = { 0 };
	char* fileName = "f:/records.bin";
	generateData(fileName);


	
	StorageIO db;
	db.open(fileName);
	db.first();
	
	int counter = 0;
	size_t prev, next;
	cout << "------------------------------------------ ASCENDING \n";
	do {
		uint32_t length = db.getLength();
		prev = db.getPreviousPosition();
		next = db.getNextPosition();
		if (prev == NOT_FOUND) prev = 0;
		if (next == NOT_FOUND) next = 0;
		cout << "Offset: " << db.getPosition() << " Length: " << length << " ID: " << db.getID();
		cout << " Previous: " << prev << " Next:" << next;
		db.getData(buf, length);
		buf[length] = 0;
		cout << " - " << buf << "\n\n";
		if (counter == 5) {
			cout << "Record #5 - DELETED\n\n";
			db.remove();
		}
		counter++;
	} while (db.next());

	db.insert("INSERTED RECORD XXX", 19);
	cout << "Record inserted\n\n";
	
	//db.insert("INSERTED RECORD 2", 17); // NEED BUG DIX

	db.last();
	cout << "------------------------------------------ DESCENDING\n";


	do  {
		uint32_t length = db.getLength();
		prev = db.getPreviousPosition();
		next = db.getNextPosition();
		if (prev == NOT_FOUND) prev = 0;
		if (next == NOT_FOUND) next = 0;
		cout << "Offset: " << db.getPosition() << " Length: " << length << " ID: " << db.getID();
		cout << " Previous: " << prev << " Next:" << next;
		db.getData(buf, length);
		buf[length] = 0;
		cout << " - " << buf << "\n\n";
	} while (db.previous());



	cout << "------------------------------------------ FIRST\n";
	{
		db.first();
		uint32_t length = db.getLength();
		cout << "Offset: " << db.getPosition() << " Length: " << length << " ID: " << db.getID() << endl;
		db.getData(buf, length);
		buf[length] = 0;
		cout << buf << "\n\n";
	}
	cout << "------------------------------------------ LAST\n";
	{
		db.last();
		uint32_t length = db.getLength();
		cout << "Offset: " << db.getPosition() << " Length: " << length << " ID: " << db.getID() << endl;
		db.getData(buf, length);
		buf[length] = 0;
		cout << buf << "\n\n";
	}
	db.close();

}
