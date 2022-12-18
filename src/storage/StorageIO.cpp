

#include "StorageIO.h"

using namespace Boson;

/*
* 
* @brief StorageIO constructor and initializations
* 
*/
StorageIO::StorageIO() {

	memset(&storageHeader, 0, sizeof(storageHeader));
	cursorOffset = 0;
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
bool StorageIO::open(std::string dbName) {
	if (storageFile.open(dbName.c_str())) return false;
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
	bool headerSaved = saveStorageHeader();
	return storageFile.close() && headerSaved;
}


/*
*
* @brief Set cursor position
* @return true - if offset points to consistent record, false - otherwise
*
*/
bool StorageIO::setPosition(uint64_t offset) {

	if (!storageFile.isOpen()) return false;

	RecordHeader header;
	uint64_t oldOffset = cursorOffset;
	cursorOffset = offset;
	if (this->readHeader(header) == NOT_FOUND) {
		cursorOffset = oldOffset;
		return false;
	}
	
	// TODO: check data consistency by checksum

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
	if (!storageFile.isOpen()) return false;
	RecordHeader header;
	if (readHeader(header) == NOT_FOUND) return false;
	return setPosition(header.next);
}


/*
*
* @brief Moves cursor to the previois record in database
* @return true - if previous record exists, false - otherwise
* 
*/
bool StorageIO::prev() {
	if (!storageFile.isOpen()) return false;
	RecordHeader header;
	if (readHeader(header) == NOT_FOUND) return false;
	return setPosition(header.previous);
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
size_t StorageIO::insert(const void* data, size_t length, RecordHeader& result) {
	return 0;
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
size_t StorageIO::update(const void* data, size_t length, RecordHeader& result) {
	return 0;
}



/*
*
* @brief Reads record header information in current position
*
* @param[out] result - record header information
*
* @return returns offset of the record or NOT_FOUND if fails
*
*/
size_t StorageIO::readHeader(RecordHeader& result) {
	return 0;
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
size_t StorageIO::readData(void* data, size_t length) {
	return 0;
}



/*
*
* @brief Delete record in current position
*
* @return returns offset of the next record or NOT_FOUND if fails
*
*/
size_t StorageIO::remove() {
	return 0;
}



//=============================================================================
// 
// 
//                       Private Methods
// 
// 
//=============================================================================



bool StorageIO::loadStorageHeader() {
	return false;
}


bool StorageIO::saveStorageHeader() {
	return false;
}


void StorageIO::getFreeRecord(RecordHeader& info) {
	
}


void StorageIO::releaseRecord() {

}



uint64_t StorageIO::generateID() {

}


uint32_t StorageIO::checksum(void* data, size_t length) {
	return 0;
}