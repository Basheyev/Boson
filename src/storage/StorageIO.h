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

#include <vector>
#include <string>


namespace Boson {
	

	//----------------------------------------------------------------------------
	// Database header structure
	//----------------------------------------------------------------------------
	typedef struct {
		uint64_t      signature;           // Signature "BOSONDB" + 0x00
		uint16_t      version;             // Database version
		
				
		uint8_t       reserved;            // Reserved

		uint64_t      indexRoot;           // Index root record offset

		uint64_t      firstRecord;         // First record offset
		uint64_t      lastRecord;          // Last record offset
		uint64_t      freeRecord;          // Free list first record offset

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
	// Record header structure (linked list)
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

		size_t insertDocument(std::string& doc);   // create document
		size_t eraseDocument();                    // delete document

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