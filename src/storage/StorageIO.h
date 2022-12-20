/******************************************************************************
*
*  StorageIO class header
*
*  StorageIO is binary records storage
*
*  (C) Bolat Basheyev 2022
*
******************************************************************************/
#pragma once

#include "CachedFileIO.h"

#define _ITERATOR_DEBUG_LEVEL 0

#include <vector>
#include <string>


namespace Boson {

	//----------------------------------------------------------------------------
	// Boson Database header structure (64 bytes)
	//----------------------------------------------------------------------------
	typedef struct {
		uint64_t      signature;           // BOSONDB signature
		uint32_t      version;             // Format version
		uint32_t      endOfFile;           // End of file

		uint64_t      totalRecords;        // Total number of records
		uint64_t      firstDataRecord;     // First data record
		uint64_t      lastDataRecord;      // Last data record

		uint64_t      totalFreeRecords;    // Total number of free records
		uint64_t      firstFreeRecord;     // First free record offset
		uint64_t      lastFreeRecord;      // Last free record offset
	} StorageHeader;

	constexpr uint64_t BOSONDB_SIGNATURE = 0x0042444E4F534F42;
	constexpr uint32_t BOSONDB_VERSION   = 0x0001;

	//----------------------------------------------------------------------------
	// Record header structure (36 bytes)
	//----------------------------------------------------------------------------
	typedef struct {
		uint64_t    next;              // Next record position in data file
		uint64_t    previous;          // Previous record position in data file				
		uint64_t    recordID;          // 64-bit unique record ID
		uint32_t    capacity;          // Record length in bytes including padding
		uint32_t    length;            // Data length in bytes
		uint32_t    checksum;          // Checksum for data consistency check
	} RecordHeader;
	

	//----------------------------------------------------------------------------
	// StorageIO
	//----------------------------------------------------------------------------
	class StorageIO {
	public:
		StorageIO();
		~StorageIO();

		bool   open(const std::string& dbName);     	
		bool   close();
		uint64_t getTotalRecords();
		uint64_t getTotalFreeRecords();

		bool   setPosition(uint64_t offset);
		uint64_t getPosition();
		bool   first();
		bool   last();
		bool   next();
		bool   previous();

		uint64_t insert(const void* data, uint32_t length);
		uint64_t update(const void* data, uint32_t length);
		uint64_t remove();

		uint64_t getID();
		uint32_t getLength();
		uint32_t getCapacity();
		uint64_t getNextPosition();
		uint64_t getPreviousPosition();
		uint64_t getData(void* data, uint32_t length);

	private:

		CachedFileIO  storageFile;
		StorageHeader storageHeader;
		RecordHeader  recordHeader;
		size_t        cursorOffset;

		void     initStorageHeader();
		bool     saveStorageHeader();
		bool     loadStorageHeader();	
				
		uint64_t getRecordHeader(uint64_t offset, RecordHeader& result);
		uint64_t putRecordHeader(uint64_t offset, const RecordHeader& result);

		uint64_t createNewRecord(uint32_t capacity, RecordHeader& result);
		uint64_t createFirstRecord(uint32_t capacity, RecordHeader& result);
		uint64_t appendNewRecord(uint32_t capacity, RecordHeader& result);
		uint64_t getFromFreeList(uint32_t capacity, RecordHeader& result);

		bool     putToFreeList(uint64_t offset);
		void     removeFromFreeList(RecordHeader& freeRecord);

		uint64_t generateID();
		uint32_t checksum(const uint8_t* data, uint64_t length);

	};


}