/******************************************************************************
*
*  DocumentStorageIO class header
*
*
*  Features:
*    - group documents into collections (schema)
*    - create, read, update and delete JSON documents
*    - navigate collections by using cursors: first, last, next, previous
*    - read documents by key/value pairs
* 
*  (C) Boson Database, Bolat Basheyev 2022
*
******************************************************************************/
#pragma once

#include "CachedFileIO.h"
#include "RecordStorageIO.h"

#define _ITERATOR_DEBUG_LEVEL 0

#include <vector>
#include <string>

namespace Boson {


	typedef struct {
		uint64_t      collectionID;        // Collection ID 	
		uint64_t      totalRecords;        // Total number of records
		uint64_t      firstRecord;         // First data record
		uint64_t      lastRecord;          // Last data record		
		char          name[64];            // Name of the collection
	} CollectionHeader;
	


	//----------------------------------------------------------------------------
	// DocumentStorageIO
	//----------------------------------------------------------------------------
	class DocumentStorageIO {
	public:
		DocumentStorageIO();
		~DocumentStorageIO();

		bool     open(const std::string& dbName, bool readonly = false);
		bool     close();

		void createCollection();
		void getCollectionList();
		void getCollectionCursor();
		void renameCollection();
		void removeCollection();


		
	private:

		RecordStorageIO recordStorage;


	};


}