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
	// Boson Database header structure
	//----------------------------------------------------------------------------
	typedef struct {
		uint64_t      signature;           // BOSONDB signature
		uint64_t      totalRecords;        // Total number of records
		uint64_t      firstDataRecord;     // First data record
		uint64_t      lastDataRecord;      // Last data record
		uint64_t      totalFreeRecords;    // Total number of free records
		uint64_t      firstFreeRecord;     // First free record offset
		uint64_t      lastFreeRecord;      // Last free record offset
	} StorageHeader;

	constexpr uint64_t BOSONDB = 0x0042444E4F534F42; // "BOSONDB\x00" signature

	//----------------------------------------------------------------------------
	// Record header structure (double linked list) and record structure itself
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

		bool   open(const std::string dbName);     	
		bool   close();

		bool   setPosition(uint64_t offset);
		size_t getPosition();
		bool   first();
		bool   last();
		bool   next();
		bool   previous();

		size_t insert(const void* data, uint32_t length);
		size_t update(const void* data, uint32_t length);
		size_t remove();

		size_t getID();		
		size_t getLength();
		size_t getCapacity();
		size_t getNextPosition();
		size_t getPreviousPosition();
		size_t getData(void* data, uint32_t length);

	private:

		CachedFileIO  storageFile;
		StorageHeader storageHeader;
		RecordHeader  recordHeader;
		size_t        cursorOffset;

		bool loadStorageHeader();
		bool saveStorageHeader();

		void getFreeRecord(RecordHeader& info);
		void releaseRecord();

		uint64_t generateID();
		uint32_t checksum(uint8_t* data, size_t length);

	};


}