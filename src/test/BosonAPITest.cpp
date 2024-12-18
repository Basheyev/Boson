
#include "BosonAPITest.h"

using namespace Boson;



BosonAPITest::BosonAPITest(char* path) {
	//std::remove(path);
	db.open(path, false);
}


BosonAPITest::~BosonAPITest() {
	db.close();
}



void BosonAPITest::insertData() {
	std::cout << "============================================================================================" << std::endl;
	std::cout << "INSERTING\n";
	std::cout << "============================================================================================" << std::endl;

	int i;
	for (i = 0; i < 5; i++) {
		db.insert("Anyways all project will be completed.");
		db.insert("Beniki told me that message length is just a marker of records in the storage file.");
		db.insert("Syely lovely touches his hairs.");
		db.insert("Vereniki died trying to get rich.");
		db.insert("Thats what happened.");
		db.insert("Anyways all project will be completed.");
		db.insert("Beniki told me that message length is just a marker of records in the storage file.");
		db.insert("Unlikely that it would happen");
		db.insert("But still believa I can do it");
		db.insert("lets try again to create more nodes");
		db.insert("And test overflow cases");
	}

	std::cout << "Entries inserted: " << i * 11 << std::endl;
}


void BosonAPITest::eraseData() {

	std::cout << "============================================================================================" << std::endl;
	std::cout << "DELETING\n";
	std::cout << "============================================================================================" << std::endl;

	
	// Get all IDs
	std::vector<uint64_t> allRecords;
	uint64_t totalRecords = db.size();
	auto pair = db.first();
	while (pair.second != nullptr) {	
		allRecords.push_back(pair.first);
		pair = db.next();
	}

	// Delete by IDs	
	for (uint64_t i = 0; i < totalRecords; i++) {
		db.erase(allRecords[i]);
	}

	std::cout << "Entries deleted: " << totalRecords << std::endl;
	std::cout << "Cache hits: " << db.getCacheHits() << "%" << std::endl;

}


void BosonAPITest::traverseEntries(bool descendingOrder) {


	std::cout << "============================================================================================" << std::endl;
	std::cout << (descendingOrder ? "REVERSE " : "") << "TRAVERSING ENTRIES\n";
	std::cout << "============================================================================================" << std::endl;

	uint64_t key = 0;

	auto pair = descendingOrder ? db.last() : db.first();
	auto value = pair.second;

	// traverse entries
	if (value != nullptr) {
		do {
			std::cout << pair.first << " = '" << *pair.second << "'" << std::endl;
			pair = descendingOrder ? db.previous() : db.next();
		} while (pair.second != nullptr);
	}

	std::cout << "--------------------------------------------\n";
	std::cout << "ENTRIES: " << db.size() << "\n";
	std::cout << "CACHE HITS: " << db.getCacheHits() << "%\n";
}


void BosonAPITest::run() {

	insertData();
	//db.printTreeState();
	traverseEntries();
	eraseData();
	//db.printTreeState();
	insertData(); 

	db.printTreeState();
	//traverseEntries(true);
	eraseData();
	
	//db.printTreeState();
}
