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
*    - create/read/update/delete records of arbitrary size
*    - navigate records: first, last, next, previous, exact position
*    - reuse space of deleted records
*    - data consistency check (checksum)
*
*  (C) Boson Database, Bolat Basheyev 2022
*
******************************************************************************/
#pragma once

#include "CachedFileIO.h"

#define _ITERATOR_DEBUG_LEVEL 0

#include <vector>
#include <string>

namespace Boson {

	//----------------------------------------------------------------------------
	// Boson storage header signature and version
	//----------------------------------------------------------------------------
	constexpr uint64_t BOSONDB_SIGNATURE = 0x0042444E4F534F42;
	constexpr uint32_t BOSONDB_VERSION = 0x0001;

	//----------------------------------------------------------------------------
	// Boson storage header structure
	//----------------------------------------------------------------------------
	typedef struct {
		uint64_t      signature;           // BOSONDB signature
		uint32_t      version;             // Format version		
		uint32_t      flags;               // Reserved for flags
		uint64_t      endOfFile;           // Size of file

		uint64_t      totalRecords;        // Total number of records
		uint64_t      firstRecord;         // First data record
		uint64_t      lastRecord;          // Last data record

		uint64_t      totalFreeRecords;    // Total number of free records
		uint64_t      firstFreeRecord;     // First free record offset
		uint64_t      lastFreeRecord;      // Last free record offset
	} StorageHeader;


	//----------------------------------------------------------------------------
	// Record header structure (32 bytes)
	//----------------------------------------------------------------------------
	typedef struct {
		uint64_t    next;              // Next record position in data file
		uint64_t    previous;          // Previous record position in data file		
		uint32_t    capacity;          // Record length in bytes including padding
		uint32_t    length;            // Data length in bytes		
		uint32_t    checksum;          // Checksum for data consistency check
		uint32_t    type;              // User defined classification of payload
	} RecordHeader;


	//----------------------------------------------------------------------------
	// RecordFileIO
	//----------------------------------------------------------------------------
	class RecordFileIO {
	public:
		RecordFileIO(CachedFileIO& cachedFile, size_t freeDepth = NOT_FOUND);
		~RecordFileIO();
		uint64_t getTotalRecords();
		uint64_t getTotalFreeRecords();
		void     setFreeRecordLookupDepth(uint64_t maxDepth) { freeLookupDepth = maxDepth; }

		// records navigation
		bool     setPosition(uint64_t offset);
		uint64_t getPosition();
		bool     first();
		bool     last();
		bool     next();
		bool     previous();

		// create, read, update, delete (CRUD)
		uint64_t createRecord(const void* data, uint32_t length, uint32_t type=0);
		uint64_t removeRecord();
		uint64_t setRecordType(uint32_t recordType);
		uint32_t getRecordType();
		uint32_t getRecordLength();
		uint32_t getRecordCapacity();
		uint64_t getNextPosition();
		uint64_t getPrevPosition();
		uint64_t getRecordData(void* data, uint32_t length);
		uint64_t setRecordData(const void* data, uint32_t length);

	private:
		CachedFileIO& cachedFile;
		StorageHeader storageHeader;
		RecordHeader  recordHeader;
		size_t        currentPosition;
		size_t        freeLookupDepth;

		void     initStorageHeader();
		bool     saveStorageHeader();
		bool     loadStorageHeader();
		uint64_t getRecordHeader(uint64_t offset, RecordHeader& result);
		uint64_t putRecordHeader(uint64_t offset, const RecordHeader& header);
		uint64_t allocateRecord(uint32_t capacity, RecordHeader& result);
		uint64_t createFirstRecord(uint32_t capacity, RecordHeader& result);
		uint64_t appendNewRecord(uint32_t capacity, RecordHeader& result);
		uint64_t getFromFreeList(uint32_t capacity, RecordHeader& result);
		bool     putToFreeList(uint64_t offset);
		void     removeFromFreeList(RecordHeader& freeRecord);
		uint32_t checksum(const uint8_t* data, uint64_t length);
	};


}
