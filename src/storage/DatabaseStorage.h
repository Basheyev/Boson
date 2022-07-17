/*=================================================================================================
*
*    Database Storage Header
*
*    
*     -----------------------
*    |    Database Storage   |
*     -----------------------
*                |
*     -----------------------
*    |     Cached Pager      | 
*     -----------------------
*                |
*     -----------------------
*    |       Data File       |
*     ----------------------- 
*
* 
*    BOSON embedded database
*    (C) Bolat Basheyev 2022
*
=================================================================================================*/
#pragma once

#include "io/CachedFile.h"

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
		size_t    next;                    // Next record position in data file
		size_t    previous;                // Previous record position in data file
		uint64_t  documentID;              // 64-bit document ID (8-bit collection, 56-bit docID)
		uint32_t  capacity;                // Record length in bytes including padding
		uint32_t  length;                  // Data length in bytes
		char*     data;                    // Document data itself
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
		bool next();                                            // jump to next record
		bool previous();                                        // jump to previous record
		bool last();                                            // jump to last record
				
		bool write(DBRecord& record);                           // writes record to database
		bool read(DBRecord& record);                            // reads record from database
		bool erase();                                           // erases record in database

		bool flush();                                           // flushes cache to file

		int  getState();

	protected:

		CachedFilePager pager;
		
		bool seek(size_t position);
		size_t getFreeRecord(size_t capacity);
	


	};


}