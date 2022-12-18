

#include "StorageIO.h"


#include <algorithm>

using namespace Boson;

/*
* 
* @brief StorageIO constructor and initializations
* 
*/
StorageIO::StorageIO() {

	memset(&storageHeader, 0, sizeof(storageHeader));
	cursorOffset = NOT_FOUND;
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
	size_t bytesRead;	
	bytesRead = storageFile.read(offset, &header, sizeof RecordHeader);
	if (bytesRead != sizeof RecordHeader) return false;

	// TODO: check data consistency by checksum

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



	// TODO:




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
size_t StorageIO::update(const void* data, uint32_t length) {

	// TODO:
	// if capcaity is larger than given length
	// a) then just update data and header
	// b) else delete record insert in new place with the same ID

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

	// TODO:

	return 0;
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
	return cursorOffset;
}



//=============================================================================
// 
// 
//                       Private Methods
// 
// 
//=============================================================================



bool StorageIO::loadStorageHeader() {
	StorageHeader sh;
	size_t bytesRead = storageFile.read(0, &sh, sizeof StorageHeader);
	// check read success
	if (bytesRead != sizeof StorageHeader) return false;  
	// check signature
	if (sh.signature != BOSONDB) return false;            
	

	// TODO: another logical checks


	// Copy header data to working structure
	memcpy(&storageHeader, &sh, sizeof StorageHeader);
	return true;
}


bool StorageIO::saveStorageHeader() {

	// TODO:

	return false;
}


void StorageIO::getFreeRecord(RecordHeader& info) {

	// TODO:
	
}


void StorageIO::releaseRecord() {

	// TODO:

}



uint64_t StorageIO::generateID() {

	// TODO:
	return 0;
}


/**
*  @brief Adler-32 checksum algoritm
*  @param[in] data - byte array of data to be checksummed
*  @param[in] length - length of data in bytes
*  @return 32-bit checksum of given data
*/
uint32_t StorageIO::checksum(uint8_t* data, size_t length) {
	
	// FIXME: inefficient but straightforward

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