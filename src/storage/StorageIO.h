/******************************************************************************
*
*  StorageIO class header
*
*  StorageIO is designed to improve provide document storage level
*  of abstraction to file. Accessing files
*
*  DatabaseStorage features:
*    - Records and collection abstraction level read/write/navigate
*    - 
*
*  (C) Bolat Basheyev 2022
*
******************************************************************************/
#pragma once

#include "CachedFileIO.h"

#include <vector>
#include <string>


namespace Boson {

	



	//------------------------------------------------------------------------------------------------
	// Database header structure
	//------------------------------------------------------------------------------------------------
	typedef struct {
		uint32_t      signature;           // Database signature "BSND"
		uint16_t      version;             // Database version


		uint8_t       collectionsCount;    // Collections count
		uint8_t       reserved;            // Reserved

	} DBHeader;


	//------------------------------------------------------------------------------------------------
	// Record structure
	//------------------------------------------------------------------------------------------------
	typedef struct {
		uint64_t  next;                    // Next record position in data file
		uint64_t  previous;                // Previous record position in data file
		uint64_t  documentID;              // 64-bit document ID (16-bit for collection, 48-bit docID)
		uint32_t  capacity;                // Record length in bytes including padding
		uint32_t  length;                  // Data length in bytes
		uint8_t*  data;                    // Document data itself
	} DBRecord;



	//------------------------------------------------------------------------------------------------
	// Database Storage - records linked list
	//------------------------------------------------------------------------------------------------
	class DatabaseStorage {
	public:

		DatabaseStorage();
		~DatabaseStorage();

		bool open(std::string dbName, bool readOnly);           // open database file in required mode
		bool close();                                           // close database file

		bool first();                                           // jump to first record
		bool last();                                            // jump to last record
		bool next();                                            // jump to next record
		bool previous();                                        // jump to previous record
		
				
		bool insert(DBRecord& record);
		bool write(DBRecord& record);                           // writes record to database
		bool read(DBRecord& record);                            // reads record from database
		bool erase();                                           // erases record in database

		bool flush();                                           // flushes cache to file

		int  getState();

	protected:

		CachedFileIO pager;
		
		bool seek(size_t position);
		size_t getFreeRecord(size_t capacity);
	


	};


}