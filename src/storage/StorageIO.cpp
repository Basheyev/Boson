

#include "StorageIO.h"


#include <algorithm>
#include <chrono>
#include <iostream>

using namespace Boson;

/*
* 
* @brief StorageIO constructor and initializations
* 
*/
StorageIO::StorageIO() {

	memset(&storageHeader, 0, sizeof(storageHeader));
	cursorOffset = NOT_FOUND;

	// TODO: iniitalize structures for empy database
}



/*
*
* @brief StorageIO destructor and finalizations
*
*/
StorageIO::~StorageIO() {
	close();
}



/*
*
* @brief Open database file
* @param dbName - path to database file
* @return true - if file opened, false - otherwise
* 
*/
bool StorageIO::open(const std::string& dbName) {
	if (!storageFile.open(dbName.c_str())) return false;
	if (storageFile.getFileSize() == 0) {
		initStorageHeader();
	}
	if (!loadStorageHeader()) return false;
	return true;
}



/*
*
* @brief Close database file
* @return true - if correctly closed, false - otherwise
*
*/
bool StorageIO::close() {
	if (!storageFile.isOpen()) return false;
	// FIXME: consistency is not checked
	bool headerSaved = saveStorageHeader();

	return storageFile.close() && headerSaved;
}


/*
*
* @brief Get total number of records in storage
* @return total number of records
*
*/
size_t StorageIO::getTotalRecords() {
	return storageHeader.totalRecords;
}



/*
*
* @brief Set cursor position
* @return true - if offset points to consistent record, false - otherwise
*
*/
bool StorageIO::setPosition(uint64_t offset) {
	if (!storageFile.isOpen()) return false;	
	// Try to read record header
	RecordHeader header;
	if (getRecordHeader(offset, header)==NOT_FOUND) return false;
	// If everything is ok - copy to own buffer
	memcpy(&recordHeader, &header, sizeof RecordHeader);
	cursorOffset = offset;
	return true;
}



/*
*
* @brief Get cursor position
* @return current cursor position in database
*
*/
size_t StorageIO::getPosition() {
	if (!storageFile.isOpen()) return NOT_FOUND;
	return cursorOffset;
}



/*
*
* @brief Moves cursor to first record in database
* @return true - if offset points to consistent record, false - otherwise
*
*/
bool StorageIO::first() {
	if (!storageFile.isOpen()) return false;
	return setPosition(storageHeader.firstDataRecord);
}



/*
*
* @brief Moves cursor to last record in database
* @return true - if offset points to consistent record, false - otherwise
*
*/
bool StorageIO::last() {
	if (!storageFile.isOpen()) return false;
	return setPosition(storageHeader.lastDataRecord);
}



/*
*
* @brief Moves cursor to the next record in database
* @return true - if next record exists, false - otherwise
*
*/
bool StorageIO::next() {
	if (!storageFile.isOpen() || cursorOffset==NOT_FOUND) return false;
	return setPosition(recordHeader.next);
}



/*
*
* @brief Moves cursor to the previois record in database
* @return true - if previous record exists, false - otherwise
* 
*/
bool StorageIO::previous() {
	if (!storageFile.isOpen() || cursorOffset == NOT_FOUND) return false;
	return setPosition(recordHeader.previous);
}



/*
*
* @brief Inserts new record into database
* 
* @param[in] data - pointer to data
* @param[in] length - length of data in bytes
* @param[out] result - created record header information
* 
* @return returns offset of the new record or NOT_FOUND if fails
*
*/
size_t StorageIO::insert(const void* data, uint32_t length) {
	RecordHeader newRecordHeader;

	// find free record of required length
	size_t offset = getFromFreeList(length, newRecordHeader);
	
	// Fill record header fields and link to previous record
	newRecordHeader.next = NOT_FOUND;                               
	//newRecordHeader.previous = storageHeader.lastDataRecord;
	newRecordHeader.recordID = generateID();
	newRecordHeader.length = length;
	newRecordHeader.checksum = checksum((uint8_t*) data, length);

	// Update storage header
    // if its a first inserted record
	if (storageHeader.firstDataRecord == NOT_FOUND) {
		storageHeader.firstDataRecord = offset;
	}
	// pointer to last record and increment storage header counter
	storageHeader.lastDataRecord = offset;	
	storageHeader.endOfFile += length;
	storageHeader.totalRecords++;
	saveStorageHeader();

	// Write record header and data to the storage file
	constexpr uint64_t HEADER_SIZE = sizeof RecordHeader;
	storageFile.write(offset, &newRecordHeader, HEADER_SIZE);
	storageFile.write(offset + HEADER_SIZE, data, length);
	
	// Update previous record
 	if (newRecordHeader.previous != NOT_FOUND) {
		size_t leftSiblingOffset = newRecordHeader.previous;		
		RecordHeader leftSiblingHeader;
		getRecordHeader(leftSiblingOffset, leftSiblingHeader);
		leftSiblingHeader.next = offset;
		putRecordHeader(leftSiblingOffset, leftSiblingHeader);
	}

	// DEBUG
	storageFile.flush();

	memcpy(&recordHeader, &newRecordHeader, HEADER_SIZE);
	cursorOffset = offset;

	// Return offset of new record
	return offset;
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
size_t StorageIO::update(const void* data, uint32_t length) {
	
	// if there is enough capacity in record
	if (length < recordHeader.capacity) {
		// Update header data length info without affecting ID
		recordHeader.length = length;
		// Write record header and data to the storage file
		constexpr uint64_t HEADER_SIZE = sizeof RecordHeader;
		storageFile.write(cursorOffset, &recordHeader, HEADER_SIZE);
		storageFile.write(cursorOffset + HEADER_SIZE, data, length);
		return cursorOffset;
	} 

	// if there is not enough record capacity, then move record		
	RecordHeader newRecordHeader;
	// find free record of required length
	size_t offset = getFromFreeList(length, newRecordHeader);
	if (offset == NOT_FOUND) return NOT_FOUND;
	// Copy record header fields, update data length and checksum
	newRecordHeader.next = recordHeader.next;
	newRecordHeader.previous = recordHeader.previous;
	newRecordHeader.recordID = recordHeader.recordID;
	newRecordHeader.length = length;
	newRecordHeader.checksum = checksum((uint8_t*)data, length);
		
	// Delete old record and add it to the free records list
	if (!putToFreeList(cursorOffset)) return NOT_FOUND;

	// if this is first record, then update storage header
	if (recordHeader.previous == NOT_FOUND) {
		storageHeader.firstDataRecord = offset;
		saveStorageHeader();
	};

	// Write record header and data to the storage file
	constexpr uint64_t HEADER_SIZE = sizeof RecordHeader;
	storageFile.write(offset, &newRecordHeader, HEADER_SIZE);
	storageFile.write(offset + HEADER_SIZE, data, length);

	// Update current record in memory
	memcpy(&recordHeader, &newRecordHeader, HEADER_SIZE);

	// Set cursor to new updated position
	return cursorOffset = offset;
	
}



/*
*
* @brief Delete record in current position
*
* @return returns offset of the next sibling or NOT_FOUND if it was last one
*
*/
size_t StorageIO::remove() {

	// TODO: add check if this record exists

	size_t leftSiblingOffset = recordHeader.previous;
	size_t rightSiblingOffset = recordHeader.next;
	bool leftSiblingExists = (leftSiblingOffset != NOT_FOUND);
	bool rightSiblingExists = (rightSiblingOffset != NOT_FOUND);
	RecordHeader leftSiblingHeader;
	RecordHeader rightSiblingHeader;
	size_t returnOffset;

	if (leftSiblingExists && rightSiblingExists) {
		// Read left & right sibling record headers
		getRecordHeader(recordHeader.previous, leftSiblingHeader);
		getRecordHeader(recordHeader.next, rightSiblingHeader);
		// Interlink left and right sibling
		leftSiblingHeader.next = rightSiblingOffset;
		rightSiblingHeader.previous = leftSiblingOffset;
		// Save siblings headers to the storage
		putRecordHeader(leftSiblingOffset, leftSiblingHeader);
		putRecordHeader(rightSiblingOffset, rightSiblingHeader);
		// Add deleted record to the free list
		putToFreeList(cursorOffset);
		// Return offset of the right sibling
		returnOffset = rightSiblingOffset;
	} else if (leftSiblingExists) {
		// removing last record
		getRecordHeader(recordHeader.previous, leftSiblingHeader);
		leftSiblingHeader.next = NOT_FOUND;
		putRecordHeader(leftSiblingOffset, leftSiblingHeader);
		// Add deleted record to the free list
		putToFreeList(cursorOffset);
		// Update storage header last record field
		storageHeader.lastDataRecord = leftSiblingOffset;
		// Return offset of the right sibling
		returnOffset = leftSiblingOffset;
	} else if (rightSiblingExists) {
		// removing first record
		getRecordHeader(recordHeader.next, rightSiblingHeader);
		rightSiblingHeader.previous = NOT_FOUND;
		putRecordHeader(rightSiblingOffset, rightSiblingHeader);
		// Add deleted record to the free list
		putToFreeList(cursorOffset);
		// Update storage header first record field
		storageHeader.firstDataRecord = rightSiblingOffset;
		// Return offset of the right sibling
		returnOffset = rightSiblingOffset;
	} else {
		// removing the only record
		// Add deleted record to the free list
		putToFreeList(cursorOffset);
		// Update storage header first/last record field
		storageHeader.firstDataRecord = NOT_FOUND;
		storageHeader.lastDataRecord = NOT_FOUND;
		// Return NOT_FOUND because it was the last record
		returnOffset = NOT_FOUND;
	}
	
	// Update storage header information about total records number
	storageHeader.totalRecords--;
	saveStorageHeader();

	return returnOffset;
}



/*
*
* @brief Get ID of current record
* @return returns ID of current record or NOT_FOUND if fails
*
*/
size_t StorageIO::getID() {
	if (!storageFile.isOpen() || cursorOffset == NOT_FOUND) return NOT_FOUND;
	return recordHeader.recordID;
}


/*
*
* @brief Get actual data payload length in bytes of current record
* @return returns data payload length in bytes or NOT_FOUND if fails
*
*/
size_t StorageIO::getLength() {
	if (!storageFile.isOpen() || cursorOffset == NOT_FOUND) return NOT_FOUND;
	return recordHeader.length;
}


/*
*
* @brief Get maximum capacity in bytes of current record
* @return returns maximum capacity in bytes or NOT_FOUND if fails
*
*/
size_t StorageIO::getCapacity() {
	if (!storageFile.isOpen() || cursorOffset == NOT_FOUND) return NOT_FOUND;
	return recordHeader.capacity;
}




/*
*
* @brief Get current record's next neighbour
* @return returns offset of next neighbour or NOT_FOUND if fails
*
*/
size_t StorageIO::getNextPosition() {
	if (!storageFile.isOpen() || cursorOffset == NOT_FOUND) return NOT_FOUND;
	return recordHeader.next;
}


/*
*
* @brief Get current record's previous neighbour
* @return returns offset of previous neighbour or NOT_FOUND if fails
*
*/
size_t StorageIO::getPreviousPosition() {
	if (!storageFile.isOpen() || cursorOffset == NOT_FOUND) return NOT_FOUND;
	return recordHeader.previous;
}


/*
*
* @brief Reads record data in current position
*
* @param[out] data - pointer to the user buffer
* @param[in]  length - bytes to read to the user buffer
*
* @return returns offset of the record or NOT_FOUND if fails
*
*/
size_t StorageIO::getData(void* data, uint32_t length) {
	size_t bytesToRead = std::min(recordHeader.length, length);
	size_t dataOffset = cursorOffset + sizeof(recordHeader);
	storageFile.read(dataOffset, data, bytesToRead);
		
	// TODO: check data consistency by checksum

	return cursorOffset;
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
void StorageIO::initStorageHeader() {
	
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

	// DEBUG
	storageFile.flush();
}


/*
*  @brief Saves in memory storage header to the file storage
*  @return true - if succeeded, false - if failed
*/
bool StorageIO::saveStorageHeader() {
	size_t bytesWritten = storageFile.write(0, &storageHeader, sizeof StorageHeader);
	// check read success
	if (bytesWritten != sizeof StorageHeader) return false;
	return true;
}


/*
*  @brief Loads file storage header to memory storage header
*  @return true - if succeeded, false - if failed
*/
bool StorageIO::loadStorageHeader() {
	StorageHeader sh;
	size_t bytesRead = storageFile.read(0, &sh, sizeof StorageHeader);
	// check read success
	if (bytesRead != sizeof StorageHeader) return false;  
	// check signature
	if (sh.signature != BOSONDB_SIGNATURE) return false;            
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
size_t StorageIO::getRecordHeader(size_t offset, RecordHeader& result) {	
	size_t bytesRead = storageFile.read(offset, &result, sizeof RecordHeader);
	if (bytesRead != sizeof RecordHeader) return NOT_FOUND;
	return offset;
}



/**
*  @brief Write record header at the given file position
*  @param[in] offset - record position in the file
*  @param[in] result - user buffer to load record header
*  @return record offset in file or NOT_FOUND if can't write
*/
size_t StorageIO::putRecordHeader(size_t offset, const RecordHeader& header) {
	size_t bytesWritten = storageFile.write(offset, &header, sizeof RecordHeader);
	if (bytesWritten != sizeof RecordHeader) return NOT_FOUND;
	return offset;
}


/*
* @brief Get free record with desired capacity or allocate new one
*/
size_t StorageIO::getFromFreeList(uint32_t capacity, RecordHeader& result) {

	size_t offset;
	size_t freeRecordOffset;

	// if there is no free records yet
	if (storageHeader.firstFreeRecord == NOT_FOUND) {
		// if there is no records at all
		if (storageHeader.lastDataRecord == NOT_FOUND) {
			// clear record header
			memset(&result, 0, sizeof RecordHeader);
			// set value to capacity
			result.next = NOT_FOUND;
			result.previous = NOT_FOUND;
			result.capacity = capacity;
			result.length = 0;
			
			storageHeader.endOfFile += sizeof(RecordHeader) + capacity;
			saveStorageHeader();

			// calculate offset right after Storage header
			freeRecordOffset = sizeof StorageHeader;
			// return offset and record header
			return freeRecordOffset;
		}
	} else {
		// If there are free records
		RecordHeader freeRecord;
		size_t counter = 0;
		freeRecord.next = storageHeader.firstFreeRecord;
		offset = freeRecord.next;

		// iterate through free list
		while (freeRecord.next != NOT_FOUND && counter < storageHeader.totalFreeRecords) {
			getRecordHeader(offset, freeRecord);
			// if found record with requested capacity
			if (freeRecord.capacity >= capacity) {
				memcpy(&result, &freeRecord, sizeof RecordHeader);

				RecordHeader lastRecord;
				getRecordHeader(storageHeader.lastDataRecord, lastRecord);
				lastRecord.next = offset;
				putRecordHeader(storageHeader.lastDataRecord, lastRecord);
				result.next = NOT_FOUND;
				result.previous = storageHeader.lastDataRecord;				
				result.length = 0;

				removeFromFreeList(freeRecord);
				return offset;
			}
			offset = freeRecord.next;
			counter++;
		} 
	}

	// if there is no free records, append to end of file
	// BUG: find end of file!!!
	// BUG: not always last data record is in the end!!!!!!!!!!

	// update previous free record
	RecordHeader lastRecord;
	getRecordHeader(storageHeader.lastDataRecord, lastRecord);
	lastRecord.next = freeRecordOffset = storageHeader.endOfFile;
	putRecordHeader(storageHeader.lastDataRecord, lastRecord);	

	result.next = NOT_FOUND;
	result.previous = storageHeader.lastDataRecord;
	result.capacity = capacity;
	result.length = 0;

	storageHeader.lastDataRecord = freeRecordOffset;
	storageHeader.endOfFile += sizeof(RecordHeader) + capacity;
	saveStorageHeader();

	
		

	

	return freeRecordOffset;

}



/*
*  @brief Put record to the free list
*/
bool StorageIO::putToFreeList(size_t offset) {
	RecordHeader freeRecord;
	if (getRecordHeader(offset, freeRecord) == NOT_FOUND) return false;
	freeRecord.next = NOT_FOUND;
	freeRecord.previous = storageHeader.lastFreeRecord;
	freeRecord.recordID = NOT_FOUND;
	freeRecord.length = 0;
	freeRecord.checksum = 0;
	putRecordHeader(offset, freeRecord);
		
	if (storageHeader.firstFreeRecord == NOT_FOUND) {
		storageHeader.firstFreeRecord = offset;
	}

	storageHeader.lastFreeRecord = offset;
	storageHeader.totalFreeRecords++;
	saveStorageHeader();
	return true;
}


/*
*  @brief Remove record from free list
*/
bool StorageIO::removeFromFreeList(RecordHeader& freeRecord) {

	size_t leftSiblingOffset = freeRecord.previous;
	size_t rightSiblingOffset = freeRecord.next;
	bool leftSiblingExists = (leftSiblingOffset != NOT_FOUND);
	bool rightSiblingExists = (rightSiblingOffset != NOT_FOUND);
	RecordHeader leftSiblingHeader;
	RecordHeader rightSiblingHeader;
	size_t returnOffset;

	if (leftSiblingExists && rightSiblingExists) {
		// Read left & right sibling record headers
		getRecordHeader(freeRecord.previous, leftSiblingHeader);
		getRecordHeader(freeRecord.next, rightSiblingHeader);
		// Interlink left and right sibling
		leftSiblingHeader.next = rightSiblingOffset;
		rightSiblingHeader.previous = leftSiblingOffset;
		// Save siblings headers to the storage
		putRecordHeader(leftSiblingOffset, leftSiblingHeader);
		putRecordHeader(rightSiblingOffset, rightSiblingHeader);
		// Add deleted record to the free list
		putToFreeList(cursorOffset);
		// Return offset of the right sibling
		returnOffset = rightSiblingOffset;
	}
	else if (leftSiblingExists) {
		// removing last record
		getRecordHeader(freeRecord.previous, leftSiblingHeader);
		leftSiblingHeader.next = NOT_FOUND;
		putRecordHeader(leftSiblingOffset, leftSiblingHeader);
		// Add deleted record to the free list
		putToFreeList(cursorOffset);
		// Update storage header last record field
		storageHeader.lastDataRecord = leftSiblingOffset;
		// Return offset of the right sibling
		returnOffset = leftSiblingOffset;
	}
	else if (rightSiblingExists) {
		// removing first record
		getRecordHeader(freeRecord.next, rightSiblingHeader);
		rightSiblingHeader.previous = NOT_FOUND;
		putRecordHeader(rightSiblingOffset, rightSiblingHeader);
		// Add deleted record to the free list
		putToFreeList(cursorOffset);
		// Update storage header first record field
		storageHeader.firstDataRecord = rightSiblingOffset;
		// Return offset of the right sibling
		returnOffset = rightSiblingOffset;
	}
	else {
		// removing the only record		
		// Update storage header first/last record field
		storageHeader.firstFreeRecord = NOT_FOUND;
		storageHeader.lastFreeRecord = NOT_FOUND;
		// Return NOT_FOUND because it was the last record
		returnOffset = NOT_FOUND;
	}

	storageHeader.totalFreeRecords--;
	saveStorageHeader();

	return true;
}


int idCounter = 0;
/*
* @brief Generate 64-bit time sortable and locally unique ID
* @return unsigned 64-bit ID (48-bit clock value, 16-bit random suffix)
*/
uint64_t StorageIO::generateID() {
	/*auto currentTime = std::chrono::steady_clock::now().time_since_epoch();
	uint64_t timeSinceEpoch = currentTime.count();  // 48-bit steady clock
	uint64_t randomNumber = std::rand();            // 16-bit random value
	uint64_t almostUniqueID = (timeSinceEpoch << 16) | randomNumber;
	return almostUniqueID;*/
	return idCounter++;
}



/**
*  @brief Adler-32 checksum algoritm
*  @param[in] data - byte array of data to be checksummed
*  @param[in] length - length of data in bytes
*  @return 32-bit checksum of given data
*/
uint32_t StorageIO::checksum(const uint8_t* data, size_t length) {
	
	// FIXME: inefficient, but straightforward algorithm

	const uint32_t MOD_ADLER = 65521;
	uint32_t a = 1, b = 0;
	size_t index;
	// Process each byte of the data in order
	for (index = 0; index < length; ++index)
	{
		a = (a + data[index]) % MOD_ADLER;
		b = (b + a) % MOD_ADLER;
	}
	return (b << 16) | a;
}