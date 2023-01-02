/******************************************************************************
*
*  RecordFileIO class header
*
*  RecordFileIO is designed for seamless storage of binary records of
*  arbitary size (max record size limited to 4Gb), accessing records as
*  linked list and reuse space of deleted records. RecordFileIO uses
*  CachedFileIO to cache frequently accessed data and win IO performance.
*
*  Features:
*    - create/read/update/delete records of arbitrary size (up to 4Gb)
*    - navigate records: first, last, next, previous, exact position
*    - reuse space of deleted records
*    - data consistency check (checksum)
*
*  (C) Boson Database, Bolat Basheyev 2022
*
******************************************************************************/

#include "RecordFileIO.h"


#include <algorithm>
#include <chrono>
#include <iostream>

using namespace Boson;

/*
* 
* @brief RecordFileIO constructor and initializations
* @param[in] cachedFile - reference to cached file object
* @param[in] freeDepth - free record lookup maximum iterations (unlim by default)
* 
*/
RecordFileIO::RecordFileIO(CachedFileIO& cachedFile, size_t freeDepth) : cachedFile(cachedFile) {
	// Check if file is open
	if (!cachedFile.isOpen()) {
		const char* msg = "ERROR: Can't operate on closed file.\n";
		std::cerr << msg;
		throw std::runtime_error(msg);
	}
	// Initialize internal data structures and variables to zero
	memset(&storageHeader, 0, sizeof StorageHeader);
	memset(&recordHeader, 0, sizeof RecordHeader);
	currentPosition = NOT_FOUND;
	freeLookupDepth = freeDepth;
	// If file is empty and write is permitted, then write storage header
	if (cachedFile.getFileSize() == 0 && !cachedFile.isReadOnly()) {
		initStorageHeader();
	}
	// Try to load storage header
	if (!loadStorageHeader()) {
		const char* msg = "ERROR: Storage file header is invalid or corrupt.\n";
		std::cerr << msg;
		throw std::runtime_error(msg);
	}
}



/*
*
* @brief RecordFileIO destructor and finalizations
*
*/
RecordFileIO::~RecordFileIO() {
	if (!cachedFile.isOpen()) return;	
	saveStorageHeader();
	cachedFile.flush();
}



/*
*
* @brief Get total number of records in storage
* @return total number of records
*
*/
uint64_t RecordFileIO::getTotalRecords() {
	return storageHeader.totalRecords;
}


/*
* 
* @biref Get total number of free (released) records
* @return total number of free records
* 
*/
uint64_t RecordFileIO::getTotalFreeRecords() {
	return storageHeader.totalFreeRecords;
}



/*
*
* @brief Set cursor position
* @param[in] offset - offset from file beginning 
* @return true - if offset points to consistent record, false - otherwise
*
*/
bool RecordFileIO::setPosition(uint64_t offset) {
	if (!cachedFile.isOpen()) return false;	
	// Try to read record header
	RecordHeader header;
	if (getRecordHeader(offset, header)==NOT_FOUND) return false;
	// If everything is ok - copy to internal buffer
	memcpy(&recordHeader, &header, sizeof RecordHeader);
	currentPosition = offset;
	return true;
}



/*
*
* @brief Get cursor position
* @return current cursor position in database
*
*/
uint64_t RecordFileIO::getPosition() {
	if (!cachedFile.isOpen()) return NOT_FOUND;
	return currentPosition;
}



/*
*
* @brief Moves cursor to first record in database
* @return true - if offset points to consistent record, false - otherwise
*
*/
bool RecordFileIO::first() {
	if (!cachedFile.isOpen()) return false;
	if (storageHeader.firstDataRecord == NOT_FOUND) return false;
	return setPosition(storageHeader.firstDataRecord);
}



/*
*
* @brief Moves cursor to last record in database
* @return true - if offset points to consistent record, false - otherwise
*
*/
bool RecordFileIO::last() {
	if (!cachedFile.isOpen()) return false;
	if (storageHeader.lastDataRecord == NOT_FOUND) return false;
	return setPosition(storageHeader.lastDataRecord);
}



/*
*
* @brief Moves cursor to the next record in database
* @return true - if next record exists, false - otherwise
*
*/
bool RecordFileIO::next() {
	if (!cachedFile.isOpen() || currentPosition==NOT_FOUND) return false;
	if (recordHeader.next == NOT_FOUND) return false;
	return setPosition(recordHeader.next);
}



/*
*
* @brief Moves cursor to the previois record in database
* @return true - if previous record exists, false - otherwise
* 
*/
bool RecordFileIO::previous() {
	if (!cachedFile.isOpen() || currentPosition == NOT_FOUND) return false;
	if (recordHeader.previous == NOT_FOUND) return false;
	return setPosition(recordHeader.previous);
}



/*
*
* @brief Creates new record in the storage
* 
* @param[in] data - pointer to data
* @param[in] length - length of data in bytes
* @param[out] result - created record header information
* 
* @return returns offset of the new record or NOT_FOUND if fails
*
*/
uint64_t RecordFileIO::createRecord(const void* data, uint32_t length, uint32_t type) {
	if (!cachedFile.isOpen() || cachedFile.isReadOnly()) return false;
	// find free record of required length
	RecordHeader newRecordHeader;
	uint64_t offset = allocateRecord(length, newRecordHeader);
	// Fill record header fields and link to previous record
	newRecordHeader.next = NOT_FOUND;                               	
	newRecordHeader.length = length;
	newRecordHeader.type = type;
	newRecordHeader.checksum = checksum((uint8_t*) data, length);
	// Write record header and data to the storage file
	constexpr uint64_t HEADER_SIZE = sizeof RecordHeader;
	cachedFile.write(offset, &newRecordHeader, HEADER_SIZE);
	cachedFile.write(offset + HEADER_SIZE, data, length);
	// copy to working record
	memcpy(&recordHeader, &newRecordHeader, HEADER_SIZE);
	currentPosition = offset;
	// Return offset of new record
	return offset;
}



/*
*
* @brief Delete record in current position
*
* @return returns offset of the next sibling or NOT_FOUND if it was last one
*
*/
uint64_t RecordFileIO::removeRecord() {
	if (!cachedFile.isOpen() || cachedFile.isReadOnly() || currentPosition == NOT_FOUND) return false;
	// check siblings
	uint64_t leftSiblingOffset = recordHeader.previous;
	uint64_t rightSiblingOffset = recordHeader.next;
	bool leftSiblingExists = (leftSiblingOffset != NOT_FOUND);
	bool rightSiblingExists = (rightSiblingOffset != NOT_FOUND);
	RecordHeader leftSiblingHeader;
	RecordHeader rightSiblingHeader;
	uint64_t returnOffset;
	if (leftSiblingExists && rightSiblingExists) {  // removing record in the middle
		getRecordHeader(recordHeader.previous, leftSiblingHeader);
		getRecordHeader(recordHeader.next, rightSiblingHeader);
		leftSiblingHeader.next = rightSiblingOffset;
		rightSiblingHeader.previous = leftSiblingOffset;
		putRecordHeader(leftSiblingOffset, leftSiblingHeader);
		putRecordHeader(rightSiblingOffset, rightSiblingHeader);
		putToFreeList(currentPosition);
		returnOffset = rightSiblingOffset;
	} else if (leftSiblingExists) {		             // removing last record
		getRecordHeader(recordHeader.previous, leftSiblingHeader);
		leftSiblingHeader.next = NOT_FOUND;
		putRecordHeader(leftSiblingOffset, leftSiblingHeader);
		putToFreeList(currentPosition);
		storageHeader.lastDataRecord = leftSiblingOffset;
		returnOffset = leftSiblingOffset;
	} else if (rightSiblingExists) {		         // removing first record
		getRecordHeader(recordHeader.next, rightSiblingHeader);
		rightSiblingHeader.previous = NOT_FOUND;
		putRecordHeader(rightSiblingOffset, rightSiblingHeader);
		putToFreeList(currentPosition);
		storageHeader.firstDataRecord = rightSiblingOffset;
		returnOffset = rightSiblingOffset;
	} else {                                         // removing the only record
		putToFreeList(currentPosition);
		storageHeader.firstDataRecord = NOT_FOUND;
		storageHeader.lastDataRecord = NOT_FOUND;
		returnOffset = NOT_FOUND;
	}
	// Update storage header information about total records number
	storageHeader.totalRecords--;
	saveStorageHeader();
	return returnOffset;
}



/*
*
* @brief Set user defined record type
* @param[in] recordType - user defined unsigned 32-bit integer
* @return offset of current record or NOT_FOUND if failed
*
*/
uint64_t RecordFileIO::setRecordType(uint32_t recordType) {
	if (!cachedFile.isOpen() || currentPosition == NOT_FOUND) return NOT_FOUND;
	recordHeader.type = recordType;
	return currentPosition;
}



/*
*
* @brief Get user defined record type
* @return value of type field of record header or zero if failed
*
*/
uint32_t RecordFileIO::getRecordType() {
	if (!cachedFile.isOpen() || currentPosition == NOT_FOUND) return 0;
	return recordHeader.type;
}



/*
*
* @brief Get actual data payload length in bytes of current record
* @return returns data payload length in bytes or zero if fails
*
*/
uint32_t RecordFileIO::getRecordLength() {
	if (!cachedFile.isOpen() || currentPosition == NOT_FOUND) return 0;
	return recordHeader.length;
}



/*
*
* @brief Get maximum capacity in bytes of current record
* @return returns maximum capacity in bytes or zero if fails
*
*/
uint32_t RecordFileIO::getRecordCapacity() {
	if (!cachedFile.isOpen() || currentPosition == NOT_FOUND) return 0;
	return recordHeader.capacity;
}



/*
*
* @brief Get current record's next neighbour
* @return returns offset of next neighbour or NOT_FOUND if fails
*
*/
uint64_t RecordFileIO::getNextPosition() {
	if (!cachedFile.isOpen() || currentPosition == NOT_FOUND) return NOT_FOUND;
	return recordHeader.next;
}



/*
*
* @brief Get current record's previous neighbour
* @return returns offset of previous neighbour or NOT_FOUND if fails
*
*/
uint64_t RecordFileIO::getPrevPosition() {
	if (!cachedFile.isOpen() || currentPosition == NOT_FOUND) return NOT_FOUND;
	return recordHeader.previous;
}



/*
*
* @brief Reads record data in current position and checks consistency
*
* @param[out] data - pointer to the user buffer
* @param[in]  length - bytes to read to the user buffer
*
* @return returns offset of the record or NOT_FOUND if data corrupted
*
*/
uint64_t RecordFileIO::getRecordData(void* data, uint32_t length) {
	if (!cachedFile.isOpen() || currentPosition == NOT_FOUND || length==0) return NOT_FOUND;
	uint64_t bytesToRead = std::min(recordHeader.length, length);
	uint64_t dataOffset = currentPosition + sizeof RecordHeader;
	cachedFile.read(dataOffset, data, bytesToRead);
	// check data consistency by checksum
	if (checksum((uint8_t*) data, bytesToRead) != recordHeader.checksum) return NOT_FOUND;
	return currentPosition;
}



/*
*
* @brief Updates record's data in current position
*
* @param[in] data - pointer to new data
* @param[in] length - length of data in bytes
* @param[out] result - updated record header information
*
* @return returns offset of the new record or NOT_FOUND if fails
*
*/
uint64_t RecordFileIO::setRecordData(const void* data, uint32_t length) {
	if (!cachedFile.isOpen() || cachedFile.isReadOnly() || currentPosition == NOT_FOUND) return false;
	// if there is enough capacity in record
	if (length < recordHeader.capacity) {
		// Update header data length info without affecting ID
		recordHeader.length = length;
		// Write record header and data to the storage file
		constexpr uint64_t HEADER_SIZE = sizeof RecordHeader;
		cachedFile.write(currentPosition, &recordHeader, HEADER_SIZE);
		cachedFile.write(currentPosition + HEADER_SIZE, data, length);
		return currentPosition;
	}
	// if there is not enough record capacity, then move record		
	RecordHeader newRecordHeader;
	// find free record of required length
	uint64_t offset = allocateRecord(length, newRecordHeader);
	if (offset == NOT_FOUND) return NOT_FOUND;
	// Copy record header fields, update data length and checksum
	newRecordHeader.next = recordHeader.next;
	newRecordHeader.previous = recordHeader.previous;
	newRecordHeader.length = length;
	newRecordHeader.checksum = checksum((uint8_t*)data, length);
	// Delete old record and add it to the free records list
	if (!putToFreeList(currentPosition)) return NOT_FOUND;
	// if this is first record, then update storage header
	if (recordHeader.previous == NOT_FOUND) {
		storageHeader.firstDataRecord = offset;
		saveStorageHeader();
	};
	// Write record header and data to the storage file
	constexpr uint64_t HEADER_SIZE = sizeof RecordHeader;
	cachedFile.write(offset, &newRecordHeader, HEADER_SIZE);
	cachedFile.write(offset + HEADER_SIZE, data, length);
	// Update current record in memory
	memcpy(&recordHeader, &newRecordHeader, HEADER_SIZE);
	// Set cursor to new updated position
	return currentPosition = offset;

}



//=============================================================================
// 
// 
//                       Private Methods
// 
// 
//=============================================================================


/*
* @brief Initialize in memory storage header for new database
*/
void RecordFileIO::initStorageHeader() {
	
	storageHeader.signature = BOSONDB_SIGNATURE;
	storageHeader.version = BOSONDB_VERSION;
	storageHeader.endOfFile = sizeof StorageHeader;

	storageHeader.totalRecords = 0;
	storageHeader.firstDataRecord = NOT_FOUND;
	storageHeader.lastDataRecord = NOT_FOUND;

	storageHeader.totalFreeRecords = 0;
	storageHeader.firstFreeRecord = NOT_FOUND;
	storageHeader.lastFreeRecord = NOT_FOUND;

	saveStorageHeader();

}



/*
*  @brief Saves in memory storage header to the file storage
*  @return true - if succeeded, false - if failed
*/
bool RecordFileIO::saveStorageHeader() {
	if (!cachedFile.isOpen()) return false;
	uint64_t bytesWritten = cachedFile.write(0, &storageHeader, sizeof StorageHeader);
	// check read success
	if (bytesWritten != sizeof StorageHeader) return false;
	return true;
}



/*
*  @brief Loads file storage header to memory storage header
*  @return true - if succeeded, false - if failed
*/
bool RecordFileIO::loadStorageHeader() {
	if (!cachedFile.isOpen()) return false;
	StorageHeader sh;
	uint64_t bytesRead = cachedFile.read(0, &sh, sizeof StorageHeader);
	// check read success
	if (bytesRead != sizeof StorageHeader) return false;  
	// check signature and version
	if (sh.signature != BOSONDB_SIGNATURE) return false;
	if (sh.version != BOSONDB_VERSION) return false;
	// Copy header data to working structure
	memcpy(&storageHeader, &sh, sizeof StorageHeader);
	return true;
}



/**
*  @brief Read record header at the given file position
*  @param[in] offset - record position in the file
*  @param[out] result - user buffer to load record header
*  @return record offset in file or NOT_FOUND if can't read
*/
uint64_t RecordFileIO::getRecordHeader(uint64_t offset, RecordHeader& result) {
	uint64_t bytesRead = cachedFile.read(offset, &result, sizeof RecordHeader);
	if (bytesRead != sizeof RecordHeader) return NOT_FOUND;
	return offset;
}



/**
*  @brief Write record header at the given file position
*  @param[in] offset - record position in the file
*  @param[in] result - user buffer to load record header
*  @return record offset in file or NOT_FOUND if can't write
*/
uint64_t RecordFileIO::putRecordHeader(uint64_t offset, const RecordHeader& header) {
	uint64_t bytesWritten = cachedFile.write(offset, &header, sizeof RecordHeader);
	if (bytesWritten != sizeof RecordHeader) return NOT_FOUND;
	return offset;
}


/*
* 
*  @brief Allocates new record from free records list or appends to the ond of file
*  @param[in] capacity - requested capacity of record
*  @param[out] result  - record header of created new record
*  @return offset of record in the storage file
*/
uint64_t RecordFileIO::allocateRecord(uint32_t capacity, RecordHeader& result) {

	// if there is no free records yet
	if (storageHeader.firstFreeRecord == NOT_FOUND && storageHeader.lastDataRecord == NOT_FOUND) {
		// if there is no records at all create first record
		return createFirstRecord(capacity, result);
	} else {
		// look up free list for record of suitable capacity
		uint64_t offset = getFromFreeList(capacity, result);
		// if found, then just return it
		if (offset != NOT_FOUND) return offset;
	}

	// if there is no free records, append to the end of file	
	return appendNewRecord(capacity, result);

}


/*
*
*  @brief Creates first record in database
*  @param[in] capacity - requested capacity of record
*  @param[out] result  - record header of created new record
*  @return offset of record in the storage file
*/
uint64_t RecordFileIO::createFirstRecord(uint32_t capacity, RecordHeader& result) {
	// clear record header
	memset(&result, 0, sizeof RecordHeader);
	// set value to capacity
	result.next = NOT_FOUND;
	result.previous = NOT_FOUND;
	result.capacity = capacity;
	result.length = 0;
	result.type = 0;
	// calculate offset right after Storage header
	uint64_t offset = sizeof StorageHeader;
	storageHeader.firstDataRecord = offset;
    storageHeader.lastDataRecord = offset;
	storageHeader.endOfFile += sizeof(RecordHeader) + capacity;
	storageHeader.totalRecords++;
	saveStorageHeader();
	
	return offset;
}



/*
*
*  @brief Creates record in the end of storage file
*  @param[in] capacity - requested capacity of record
*  @param[out] result  - record header of created new record
*  @return offset of record in the storage file
*/
uint64_t RecordFileIO::appendNewRecord(uint32_t capacity, RecordHeader& result) {

	if (capacity == 0) return NOT_FOUND;
		
	// update previous free record
	RecordHeader lastRecord;
	uint64_t freeRecordOffset;

	getRecordHeader(storageHeader.lastDataRecord, lastRecord);
	lastRecord.next = freeRecordOffset = storageHeader.endOfFile;
	putRecordHeader(storageHeader.lastDataRecord, lastRecord);

	result.next = NOT_FOUND;
	result.previous = storageHeader.lastDataRecord;
	result.capacity = capacity;
	result.length = 0;
	result.type = 0;

	storageHeader.lastDataRecord = freeRecordOffset;
	storageHeader.endOfFile += sizeof(RecordHeader) + capacity;
	storageHeader.totalRecords++;
	saveStorageHeader();

	return freeRecordOffset;
}


/*
*
*  @brief Creates record from the free list (previously deleted records)
*  @param[in] capacity - requested capacity of record
*  @param[out] result  - record header of created new record
*  @return offset of record in the storage file
*/
uint64_t RecordFileIO::getFromFreeList(uint32_t capacity, RecordHeader& result) {
	// If there are free records
	RecordHeader freeRecord;
	
	freeRecord.next = storageHeader.firstFreeRecord;
	uint64_t offset = freeRecord.next;
	uint64_t maximumIterations = std::min(storageHeader.totalFreeRecords, freeLookupDepth);
	uint64_t iterationCounter = 0;
	// iterate through free list and check iterations counter
	while (freeRecord.next != NOT_FOUND && iterationCounter < maximumIterations) {
		// Read next record header
		getRecordHeader(offset, freeRecord);
		// if record with requested capacity found
		if (freeRecord.capacity >= capacity) {
			// Remove free record from the free list
			removeFromFreeList(freeRecord);
			RecordHeader lastRecord;
			// update last record to point to new record
			getRecordHeader(storageHeader.lastDataRecord, lastRecord);
			lastRecord.next = offset;			
			putRecordHeader(storageHeader.lastDataRecord, lastRecord);
			// connect new record with previous
			result.next = NOT_FOUND;
			result.previous = storageHeader.lastDataRecord;
			result.capacity = freeRecord.capacity;
			result.length = 0;
			result.type = 0;
			// update storage header last record to new record
			storageHeader.lastDataRecord = offset;
			storageHeader.totalRecords++;
			saveStorageHeader();
			return offset;
		}
		offset = freeRecord.next;
		iterationCounter++;
	}
	return NOT_FOUND;
}



/*
*  @brief Put record to the free list
*  @return true - if record added to the free list, false - if not found
*/
bool RecordFileIO::putToFreeList(uint64_t offset) {
	RecordHeader freeRecord;
	if (getRecordHeader(offset, freeRecord) == NOT_FOUND) return false;
	freeRecord.next = NOT_FOUND;
	freeRecord.previous = storageHeader.lastFreeRecord;
	freeRecord.length = 0;
	freeRecord.checksum = 0;
	putRecordHeader(offset, freeRecord);
	// if its first free record, save its offset to the storage header	
	if (storageHeader.firstFreeRecord == NOT_FOUND) {
		storageHeader.firstFreeRecord = offset;
	}
	// save it as last added free record
	storageHeader.lastFreeRecord = offset;
	storageHeader.totalFreeRecords++;
	// save storage header
	saveStorageHeader();
	return true;
}


/*
*  @brief Remove record from free list and update siblings interlinks
*  @param[in] freeRecord - header of record to remove from free list
*/
void RecordFileIO::removeFromFreeList(RecordHeader& freeRecord) {
	uint64_t leftSiblingOffset = freeRecord.previous;
	uint64_t rightSiblingOffset = freeRecord.next;
	bool leftSiblingExists = (leftSiblingOffset != NOT_FOUND);
	bool rightSiblingExists = (rightSiblingOffset != NOT_FOUND);
	RecordHeader leftSiblingHeader;
	RecordHeader rightSiblingHeader;
	if (leftSiblingExists && rightSiblingExists) {		               // If removing in the middle
		getRecordHeader(freeRecord.previous, leftSiblingHeader);
		getRecordHeader(freeRecord.next, rightSiblingHeader);
		leftSiblingHeader.next = rightSiblingOffset;
		rightSiblingHeader.previous = leftSiblingOffset;
		putRecordHeader(leftSiblingOffset, leftSiblingHeader);
		putRecordHeader(rightSiblingOffset, rightSiblingHeader);
		putToFreeList(currentPosition);
	} else if (leftSiblingExists) {                                    // if removing back free record
		getRecordHeader(freeRecord.previous, leftSiblingHeader);
		leftSiblingHeader.next = NOT_FOUND;
		putRecordHeader(leftSiblingOffset, leftSiblingHeader);
		putToFreeList(currentPosition);
		storageHeader.lastDataRecord = leftSiblingOffset;
	} else if (rightSiblingExists) {                                   // if removing front free record
		getRecordHeader(freeRecord.next, rightSiblingHeader);
		rightSiblingHeader.previous = NOT_FOUND;
		putRecordHeader(rightSiblingOffset, rightSiblingHeader);
		putToFreeList(currentPosition);
		storageHeader.firstDataRecord = rightSiblingOffset;
	} else {                                                           // If removing last free record
		storageHeader.firstFreeRecord = NOT_FOUND;
		storageHeader.lastFreeRecord = NOT_FOUND;
	}
	storageHeader.totalFreeRecords--;
	saveStorageHeader();
}



/**
*  @brief Adler-32 checksum algoritm
*  @param[in] data - byte array of data to be checksummed
*  @param[in] length - length of data in bytes
*  @return 32-bit checksum of given data
*/
uint32_t RecordFileIO::checksum(const uint8_t* data, uint64_t length) {
	
	// FIXME: inefficient, but straightforward algorithm

	const uint32_t MOD_ADLER = 65521;
	uint32_t a = 1, b = 0;
	uint64_t index;
	// Process each byte of the data in order
	for (index = 0; index < length; ++index)
	{
		a = (a + data[index]) % MOD_ADLER;
		b = (b + a) % MOD_ADLER;
	}
	return (b << 16) | a;
}