

#include "RecordFileIO.h"
#include "RecordFileIOTest.h"

#include <iostream>
#include <sstream>
#include <filesystem>


using namespace Boson;


bool RecordFileIOTest::generateData(char* filename, size_t recordsCount) {
	CachedFileIO cachedFile;
	if (!cachedFile.open(filename)) {
		std::cout << "ERROR: Can't open file '" << filename << "' in write mode.\n";
		return false;
	}

	RecordFileIO storage(cachedFile);
				
	std::cout << "[TEST] Generating " << recordsCount << " data records...";
	auto startTime = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < recordsCount; i++) {
		std::stringstream ss;
		ss << "Generated record data #" << i << " and " << std::rand();
		if (std::rand() % 2) ss << " with optional";
		storage.createRecord(ss.str().c_str(), (uint32_t) ss.str().length());
	}
	auto endTime = std::chrono::high_resolution_clock::now();
	std::cout << "OK in " << (endTime - startTime).count() / 1000000000.0 << "s" << std::endl;

	//cachedFile.close();

	return true;
}



bool RecordFileIOTest::readAscending(char* filename) {
	CachedFileIO cachedFile;
	if (!cachedFile.open(filename)) {
		std::cout << "ERROR: Can't open file '" << filename << "' in write mode.\n";
		return false;
	}

	RecordFileIO db(cachedFile);


	std::cout << "[TEST] Reading " << db.getTotalRecords() << " data records in ASCENDING order...\n";
	std::cout << "-----------------------------------------------------------\n\n";
	db.first();
	size_t counter = 0;
	size_t prev, next;
	char* buffer = new char[65536];
	do {
		uint32_t length = db.getLength();
		prev = db.getPrevious();
		next = db.getNext();		
		db.getData(buffer, length);
		buffer[length] = 0;
		std::cout << "Pos: " << db.getPosition();
		std::cout << " Prev: " << ((prev == NOT_FOUND) ? 0 : prev);
		std::cout << " Next: " << ((next == NOT_FOUND) ? 0 : next);
		std::cout << " Length: " << db.getLength();
		std::cout << "\n";		
		std::cout <<  buffer << "\n\n";				
		counter++;
	} while (db.next());
	delete[] buffer;
	std::cout << "TOTAL READ: " << counter << " records\n\n";
	return true;
}



bool RecordFileIOTest::readDescending(char* filename) {
	CachedFileIO cachedFile;
	if (!cachedFile.open(filename)) {
		std::cout << "ERROR: Can't open file '" << filename << "' in write mode.\n";
		return false;
	}
	RecordFileIO db(cachedFile);
	std::cout << "[TEST] Reading " << db.getTotalRecords() << " data records in DESCENDING order...\n";
	std::cout << "-----------------------------------------------------------\n\n";
	db.last();
	size_t counter = 0;
	size_t prev, next;
	char* buffer = new char[65536];
	do {
		uint32_t length = db.getLength();
		prev = db.getPrevious();
		next = db.getNext();					
		db.getData(buffer, length);
		buffer[length] = 0;
		std::cout << "Pos: " << db.getPosition();
		std::cout << " Prev: " << ((prev == NOT_FOUND) ? 0 : prev);
		std::cout << " Next: " << ((next == NOT_FOUND) ? 0 : next);
		std::cout << " Length: " << db.getLength();
		std::cout << "\n";		
		std::cout  << buffer << "\n\n";
		counter++;
	} while (db.previous());
	delete[] buffer;
	std::cout << "TOTAL READ: " << counter << " records\n\n";
	return true;
}


bool RecordFileIOTest::removeOddRecords(char* filename) {
	CachedFileIO cachedFile;
	if (!cachedFile.open(filename)) {
		std::cout << "ERROR: Can't open file '" << filename << "' in write mode.\n";
		return false;
	}
	RecordFileIO db(cachedFile);
	std::cout << "[TEST] Deleting every second data records...\n";
	std::cout << "-----------------------------------------------------------\n\n";
	db.first();
	size_t counter = 0;
	size_t prev, next;
	do {
		uint32_t length = db.getLength();
		prev = db.getPrevious();
		next = db.getNext();
		std::cout << "Pos: " << db.getPosition();
		std::cout << " Prev: " << ((prev == NOT_FOUND) ? 0 : prev);
		std::cout << " Next: " << ((next == NOT_FOUND) ? 0 : next);
		std::cout << " Length: " << db.getLength();
		std::cout << " - DELETED \n";
		db.removeRecord();
		counter++;
	} while (db.next() && db.next());
	std::cout << "TOTAL DELETED: " << counter << " records\n\n";
	return true;
}

bool RecordFileIOTest::insertNewRecords(char* filename, size_t recordsCount) {
	CachedFileIO cachedFile;
	if (!cachedFile.open(filename)) {
		std::cout << "ERROR: Can't open file '" << filename << "' in write mode.\n";
		return false;
	}

	RecordFileIO storage(cachedFile);

	std::cout << "[TEST] Inserting " << recordsCount << " data records...";
	auto startTime = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < recordsCount; i++) {
		std::stringstream ss;
		ss << "inserted record data " << i*2 << " and " << std::rand();
		if (std::rand() % 2) ss << " suffix";
		storage.createRecord(ss.str().c_str(), (uint32_t)ss.str().length());
	}
	auto endTime = std::chrono::high_resolution_clock::now();
	std::cout << "OK in " << (endTime - startTime).count() / 1000000000.0 << "s" << std::endl;
//	cachedFile.close();
	return true;
}


void RecordFileIOTest::run(char* filename) {
	std::filesystem::remove(filename);
	generateData(filename, 10);
	readAscending(filename);
	removeOddRecords(filename);
	readDescending(filename);
	insertNewRecords(filename, 3);
	readAscending(filename);
}