/******************************************************************************
*
*  StorageIO class header
*
*  StorageIO is designed to improve provide document storage level
*  of abstraction to file. Accessing files
*
*  DatabaseStorage features:
*    - Allocation and releasing records in file heap like structure
*    - Records read/write/navigate/delete abstraction level API
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

	constexpr uint64_t SIGNATURE = 0x0042444E4F534F42; // "BOSONDB\x00" signature
	

	//----------------------------------------------------------------------------
	// Database header structure
	//----------------------------------------------------------------------------
	typedef struct {
		
		uint64_t      signature;           // BOSONDB signature
		uint16_t      pageSize;            // Database page size (8192)
		uint16_t      version;             // Database format version
		uint32_t      encoding;            // Database encoding UTF-8 default - 0
		uint64_t      databaseSize;        // Database size in pages
		uint64_t      changeCounter;       // File change counter

		uint64_t      indexNodePayload;    // Elements in index node
		uint64_t      indexRoot;           // Index root record offset

		uint64_t      firstDataRecord;     // First record offset
		uint64_t      lastDataRecord;      // Last record offset

		uint64_t      totalFreePages;      // Total number of free records
		uint64_t      firstFreeRecord;     // First free record offset
		uint64_t      lastFreeRecord;      // Last free record offset

		uint64_t      reserved0;
		uint64_t      reserved1;
		uint64_t      reserved2;
		uint64_t      reserved3;
		uint64_t      reserved4;
		
	} StorageHeader;


	//----------------------------------------------------------------------------
	// Record types
	//----------------------------------------------------------------------------
	typedef enum {
		FREE      = 0,                       // Free record 
		INDEX     = 1,                       // Index node
		DOCUMENT  = 2,                       // Document (data)
	} RecordType;


	//----------------------------------------------------------------------------
	// Record header structure (double linked list)
	//----------------------------------------------------------------------------
	typedef struct {
		uint64_t    next;              // Next record position in data file
		uint64_t    previous;          // Previous record position in data file
		uint64_t    documentID;        // 64-bit unique document ID
		RecordType  type;              // Record type (should be enumeration)
		uint32_t    capacity;          // Record length in bytes including padding
		uint32_t    length;            // Data length in bytes
		uint32_t    checksum;          // Checksum for data consistency check
	} RecordHeader;

	
	//----------------------------------------------------------------------------
	// Cursor (storage linked list iterator)
	//----------------------------------------------------------------------------
	class RecordCursor {
    friend class StorageIO;
	public:

		RecordCursor(StorageIO& storageIO);        // 
		~RecordCursor();                           //
				
		size_t first();                            // jump to first record
		size_t last();                             // jump to last record
		size_t next();                             // go to next record
		size_t prev();                             // go to previous record
		size_t seek(size_t fileOffset);            // jump to desired offset

		size_t insert(std::string& doc);           // create document
		size_t erase();                            // delete document

		size_t getDocumentID();                    // get document ID
		size_t getRecordCapacity();                // get record capacity
		size_t getDocumentLength();                // get document length
		size_t getDocument(std::string& doc);      // get document from record
		size_t putDocument(std::string& doc);      // put document to record

	private:		

		StorageIO& storage;
		size_t position;

		bool checkSum(std::string& doc);

	};

	//----------------------------------------------------------------------------
	// StorageIO
	//----------------------------------------------------------------------------
	class StorageIO {

	public:

		StorageIO();
		~StorageIO();

		bool     open(const std::string dbName, bool readOnly); // open database		
		bool     close();                                       // close database
		bool     isOpen();                                      // check if opened

		bool     setCacheSize(size_t size);
		size_t   getCacheSize(size_t size);

		bool     getCursor(RecordCursor& cursor);              // get cursor (iterator)

	private:

		CachedFileIO*               pager;
		
		
		size_t getFreeRecord(size_t capacity);
		size_t putFreeRecord(size_t);
	
		
		friend class RecordCursor;
	};


}