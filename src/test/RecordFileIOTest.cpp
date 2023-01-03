/******************************************************************************
*
*  RecordFileIO class tests implementation
*
*  (C) Bolat Basheyev 2022
*
******************************************************************************/

#include "RecordFileIO.h"
#include "RecordFileIOTest.h"

#include <iostream>
#include <sstream>
#include <filesystem>


using namespace Boson;


/*
*  @brief Generate data records in file
*  @param[in] filename - path to file
*  @param[in] recordsCount - total records to generate
*/
bool RecordFileIOTest::generateData(const char* filename, size_t recordsCount) {
	CachedFileIO cachedFile;
	if (!cachedFile.open(filename)) {
		std::cout << "ERROR: Can't open file '" << filename << "' in write mode.\n";
		return false;
	}
	// Wrapper
	RecordFileIO storage(cachedFile);
	std::cout << "[TEST] Generating " << recordsCount << " data records...";
	auto startTime = std::chrono::high_resolution_clock::now();
	uint32_t length;
	for (size_t i = 0; i < recordsCount; i++) {
		std::stringstream ss;
		ss << "Generated record data #" << i << " and " << std::rand();
		if (std::rand() % 2) ss << " with optional";
		length = (uint32_t)ss.str().length() + 16;
		storage.createRecord(ss.str().c_str(), length);
	}
	auto endTime = std::chrono::high_resolution_clock::now();
	std::cout << "OK in " << (endTime - startTime).count() / 1000000000.0 << "s" << std::endl;

	//cachedFile.close();

	return true;
}



bool RecordFileIOTest::readAscending(const char* filename) {
	CachedFileIO cachedFile;
	if (!cachedFile.open(filename)) {
		std::cout << "ERROR: Can't open file '" << filename << "' in write mode.\n";
		return false;
	}

	RecordFileIO db(cachedFile);


	std::cout << "[TEST] Reading " << db.getTotalRecords() << " data records in ASCENDING order...\n";
	std::cout << "-----------------------------------------------------------\n\n";
	auto startTime = std::chrono::high_resolution_clock::now();
	db.first();
	size_t counter = 0;
	size_t prev, next;
	char* buffer = new char[65536];
	do {
		uint32_t length = db.getRecordLength();
		prev = db.getPrevPosition();
		next = db.getNextPosition();		
		db.getRecordData(buffer, length);
		buffer[length] = 0;
	/*	std::cout << "Pos: " << db.getPosition();
		std::cout << " Prev: " << ((prev == NOT_FOUND) ? 0 : prev);
		std::cout << " Next: " << ((next == NOT_FOUND) ? 0 : next);
		std::cout << " Length: " << db.getRecordLength();
		std::cout << "\n";		
		std::cout <<  buffer << "\n\n";		*/		
		counter++;
	} while (db.next());
	auto endTime = std::chrono::high_resolution_clock::now();
	std::cout << "Read in " << (endTime - startTime).count() / 1000000000.0 << "s" << std::endl;
	delete[] buffer;
	std::cout << "TOTAL READ: " << counter << " records\n\n";
	return true;
}



bool RecordFileIOTest::readDescending(const char* filename) {
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
		uint32_t length = db.getRecordLength();
		prev = db.getPrevPosition();
		next = db.getNextPosition();					
		db.getRecordData(buffer, length);
		buffer[length] = 0;
		std::cout << "Pos: " << db.getPosition();
		std::cout << " Prev: " << ((prev == NOT_FOUND) ? 0 : prev);
		std::cout << " Next: " << ((next == NOT_FOUND) ? 0 : next);
		std::cout << " Length: " << db.getRecordLength();
		std::cout << "\n";		
		std::cout  << buffer << "\n\n";
		counter++;
	} while (db.previous());
	delete[] buffer;
	std::cout << "TOTAL READ: " << counter << " records\n\n";
	return true;
}


bool RecordFileIOTest::removeEvenRecords(const char* filename) {
	CachedFileIO cachedFile;
	if (!cachedFile.open(filename)) {
		std::cout << "ERROR: Can't open file '" << filename << "' in write mode.\n";
		return false;
	}
	RecordFileIO db(cachedFile);
	std::cout << "[TEST] Deleting even data records...\n";
	std::cout << "-----------------------------------------------------------\n\n";
	auto startTime = std::chrono::high_resolution_clock::now();
	db.first();
	size_t counter = 0;
	size_t prev, next;
	do {
		uint32_t length = db.getRecordLength();
		prev = db.getPrevPosition();
		next = db.getNextPosition();
		/*std::cout << "Pos: " << db.getPosition();
		std::cout << " Prev: " << ((prev == NOT_FOUND) ? 0 : prev);
		std::cout << " Next: " << ((next == NOT_FOUND) ? 0 : next);
		std::cout << " Length: " << db.getRecordLength();
		std::cout << " - DELETED \n";*/
		db.removeRecord();
		counter++;
	} while (db.next() && db.next());
	auto endTime = std::chrono::high_resolution_clock::now();
	std::cout << "Read in " << (endTime - startTime).count() / 1000000000.0 << "s" << std::endl;
	std::cout << "TOTAL DELETED: " << counter << " records\n\n";
	return true;
}

bool RecordFileIOTest::insertNewRecords(const char* filename, size_t recordsCount) {
	CachedFileIO cachedFile;
	if (!cachedFile.open(filename)) {
		std::cout << "ERROR: Can't open file '" << filename << "' in write mode.\n";
		return false;
	}

	RecordFileIO storage(cachedFile);

	std::cout << "[TEST] Inserting " << recordsCount << " data records...";
	auto startTime = std::chrono::high_resolution_clock::now();
	uint32_t length;
	for (size_t i = 0; i < recordsCount; i++) {
		std::stringstream ss;
		ss << "inserted record data " << i*2 << " and " << std::rand();
		if (std::rand() % 2) ss << " suffix";
		length = (uint32_t)ss.str().length();
		storage.createRecord(ss.str().c_str(), length);
	}
	auto endTime = std::chrono::high_resolution_clock::now();
	std::cout << "OK in " << (endTime - startTime).count() / 1000000000.0 << "s" << std::endl;
//	cachedFile.close();
	return true;
}


void RecordFileIOTest::run(const char* filename) {
	std::filesystem::remove(filename);
	generateData(filename, 10);
	readAscending(filename);
	removeEvenRecords(filename);
	readDescending(filename);
	insertNewRecords(filename, 3);
	readAscending(filename);
}