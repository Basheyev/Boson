/******************************************************************************
*
*  Boson Database
*
*  Features:
*  - NoSQL database engine.
*  - Single database file.
*  - Standard Key/Value store.
*  - Document store (JSON).
*  - Support cursors for linear records traversal.
*  - Support for on-disk as well in-memory databases.
*  - Support Terabyte sized databases.
*
*  (C) Boson Database, Bolat Basheyev 2022-2024
*
******************************************************************************/

#include "api/BosonAPI.h"
#include "BalancedIndexTest.h"


using namespace Boson;
using namespace std;


int main()
{
	//char* filename = "D:/bptree.bin";
	//BalancedIndexTest bit(filename);
	//bit.run(true);


	BosonAPI db;

	db.open("D:/mydatabase.bin", true);
	
	for (int i = 0; i < 150; i++) {
		db.insert("Anyways all project will be completed.");
		db.insert("Beniki told me that message length is just a marker of records in the storage file.");
		db.insert("Syely lovely touches his hairs.");
		db.insert("Vereniki died trying to get rich.");
		db.insert("Thats what happened.");
		db.insert("Anyways all project will be completed.");
		db.insert("Beniki told me that message length is just a marker of records in the storage file.");		
	}	
	
	//auto pair = db.last();

	uint64_t key = 4311;
	uint64_t lastKey = 4349;
	auto value = db.get(key);
	auto pair = std::make_pair(key, value);

	// traverse entries
	if (value != nullptr) {		
		do {
			std::cout << pair.first << " = '" << *pair.second << "'" << std::endl;
			pair = db.next();
		} while (pair.second != nullptr && pair.first <= lastKey);
	}
	
	std::cout << "--------------------------------------------\n";
	std::cout << "ENTRIES: " << db.size() << "\n";
	std::cout << "CACHE HITS: " << db.getCacheHits() << "%\n";

	db.close();

	return 0;
}
