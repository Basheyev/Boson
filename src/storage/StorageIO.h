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

	//----------------------------------------------------------------------------
	// Boson Database header structure
	//----------------------------------------------------------------------------
	typedef struct {
		//------------------------------------------------------------------------
		uint64_t      signature;           // BOSONDB signature
		uint16_t      pageSize;            // Database page size (8192)
		uint16_t      version;             // Database format version
		uint32_t      encoding;            // Database encoding UTF-8 default - 0
		uint64_t      databaseSize;        // Database size in pages
		uint64_t      changeCounter;       // File change counter
		//------------------------------------------------------------------------
		uint64_t      indexRoot;           // Index root record offset
		uint64_t      indexNodePayload;    // Elements in index node
		//------------------------------------------------------------------------
		uint64_t      totalFreeRecords;    // Total number of free records
		uint64_t      firstFreeRecord;     // First free record offset
		uint64_t      lastFreeRecord;      // Last free record offset
		//------------------------------------------------------------------------
		uint64_t      firstDataRecord;     // First record offset
		uint64_t      lastDataRecord;      // Last record offset
		//------------------------------------------------------------------------
		uint64_t      reserved0;           // Reserved for future use
		uint64_t      reserved1;           // Reserved for future use
		uint64_t      reserved2;           // Reserved for future use
		uint64_t      reserved3;           // Reserved for future use
		uint64_t      reserved4;           // Reserved for future use
		//------------------------------------------------------------------------
	} StorageHeader;

	constexpr uint64_t SIGNATURE = 0x0042444E4F534F42; // "BOSONDB\x00" signature

	//----------------------------------------------------------------------------
	// Record types
	//----------------------------------------------------------------------------
	typedef enum {
		FREE      = 0,                       // Free record 
		INDEX     = 1,                       // Index node
		DATA      = 2,                       // Data (document)
	} RecordType;


	//----------------------------------------------------------------------------
	// Record header structure (double linked list)
	//----------------------------------------------------------------------------
	typedef struct {
		uint64_t    next;              // Next record position in data file
		uint64_t    previous;          // Previous record position in data file				
		uint64_t    recordID;          // 64-bit unique record ID
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
		
		size_t     first();                        // go to first record
		size_t     last();                         // go to last record
		size_t     next();                         // go to next record
		size_t     prev();                         // go to previous record

		RecordType getRecordType();
		
		size_t insert(std::string& doc);           // create document
		size_t erase();                            // delete document

		size_t getRecordID();                      // get record ID

		size_t getRecordCapacity();                // get record capacity
		size_t getDocumentLength();                // get document length

		size_t getDocument(std::string& doc);      // get document from record
		size_t putDocument(std::string& doc);      // put document to record

		size_t getIndexNode(std::string& doc);      // get document from record
		size_t putIndexNode(std::string& doc);      // put document to record

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

		bool     getIndexRoot(RecordCursor& cursor);
		bool     getFirstRecord(RecordCursor& cursor);
		bool     getLastRecord(RecordCursor& cursor);
		
				
	private:

		CachedFileIO*               storageFile;
		StorageHeader               sorageHeader;
				
		bool     getCursor(size_t offset, RecordCursor& cursor);
		bool     getFreeRecord(RecordCursor& root);


		friend class RecordCursor;
	};


}