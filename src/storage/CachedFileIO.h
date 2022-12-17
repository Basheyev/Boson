/******************************************************************************
*
*  CachedFileIO class header
*
*  CachedFileIO is designed to improve performance of file I/O
*  operations. Almost all real world apps show some form of locality of
*  reference. Research says that 10-15% of database size cache gives
*  more than 95% cache hits.
*
*  Most JSON documents size are less than 1000 bytes. Most apps database
*  read/write operations ratio is 70% / 30%. Read/write operations are
*  faster when aligned to storage device sector/block size and sequential.
*
*  CachedFileIO LRU/FBW (Linked list + Hashmap) caching strategy gives:
*    - O(1) time complexity of page look up
*    - O(1) time complexity of page insert
*    - O(1) time complexity of page remove
* 
*  CachedFileIO vs STDIO performance tests (Release Mode):
*    - 50%-97% cache read hits leads to 50%-600% performance growth
*    - 35%-49% cache read hits leads to 12%-36% performance growth
*    - 1%-25%  cache read hits leads to 5%-20% performance drop
*
*  (C) Boson Database, Bolat Basheyev 2022
*
******************************************************************************/

#pragma once

#define _ITERATOR_DEBUG_LEVEL 0             // MSVC debug performance fix

#include <cstdio>
#include <cstring>
#include <cstdint>
#include <unordered_map>

namespace Boson {

	//-------------------------------------------------------------------------
	constexpr size_t PAGE_SIZE      = 8192;         // 8192 bytes page size
	constexpr size_t MINIMAL_CACHE  = 256 * 1024;   // 256Kb minimal cache
	constexpr size_t DEFAULT_CACHE  = 1*1024*1024;  // 1Mb default cache
	constexpr size_t NOT_FOUND      = -1;           // "Not found" signature
	//-------------------------------------------------------------------------

	typedef enum {                              // Cache Page State
		CLEAN = 0,                              // Page has not been changed
		DIRTY = 1                               // Cache page is rewritten
	} PageState;

	class alignas(64) CachePage {               // Align to CPU cache line
	public:
		uint64_t  filePageNo;                   // Page number in file
		PageState state;                        // Current page state
		size_t    availableDataLength;          // Available amount of data
		uint8_t   *data;                        // Pointer to data (payload)
		std::list<CachePage*>::iterator it;     // Cache list iterator
	};

	//-------------------------------------------------------------------------

	typedef                                     // Double linked list
		std::list<CachePage*>                   // of cached pages pointers
		CacheLinkedList;

	typedef                                     // Hashmap of cached pages
		std::unordered_map<size_t, CachePage*>  // File page No. -> CachePage*           
		CachedPagesMap;                         

	//-------------------------------------------------------------------------
	typedef enum {                              // CachedFileIO stats types
		TOTAL_REQUESTS,                         // Total requests to cache
		CACHE_HITS_RATE,                        // Cache hits rate (0-100%)
		CACHE_MISSES_RATE,                      // Cache misses rate (0-100%)
		WRITE_THROUGHPUT,                       // Write throughput Mb/sec
		READ_THROUGHPUT,                        // Read throughput Mb/sec
		WRITE_TIME_NS,                          // Total write time (ns)
		READ_TIME_NS                            // Total read time (ns)
	} CacheStats;


	//-------------------------------------------------------------------------
	// Binary random access LRU cached file IO
	//-------------------------------------------------------------------------
	class CachedFileIO {
	public:
		CachedFileIO();
		~CachedFileIO();
		
		bool open(const char* path, size_t cache = DEFAULT_CACHE, bool readOnly = false);
		bool close();

		size_t read(size_t position, void* dataBuffer, size_t length);
		size_t write(size_t position, const void* dataBuffer, size_t length);
		size_t readPage(size_t pageNo, void* userPageBuffer);
		size_t writePage(size_t pageNo, const void* userPageBuffer);
		size_t flush();

		void   resetStats();
		double getStats(CacheStats type);
		size_t getFileSize();
		size_t getCacheSize();
		size_t setCacheSize(size_t cacheSize);

	private:

		void       allocatePool(size_t pagesCount);
		void       releasePool();
		CachePage* allocatePage();
		CachePage* getFreeCachePage();                            
		CachePage* searchPageInCache(size_t filePageNo);
		CachePage* loadPageToCache(size_t filePageNo);
		bool       persistCachePage(CachePage* pageInfo);
		bool       clearCachePage(CachePage* pageInfo);		
				
		size_t          maxPagesCount;           // Maximum cache capacity (pages)
		size_t          pageCounter;             // Allocated pages counter
				
		size_t          totalBytesRead;          // Total bytes read
		size_t          totalBytesWritten;       // Total bytes written
		size_t          totalReadDuration;       // Time of read operations (ns)
		size_t          totalWriteDuration;      // Time of write operations (ns)
		size_t          cacheRequests;           // Cache requests counter
		size_t          cacheMisses;             // Cache misses counter

		std::FILE*      fileHandler;             // OS file handler
		bool            readOnly;                // Read only flag
		CachedPagesMap  cacheMap;                // Cached pages map 
		CacheLinkedList cacheList;               // Cached pages double linked list
		CachePage*      cachePageInfoPool;       // Cache pages info memory pool
		uint8_t*        cachePageDataPool;       // Cache pages data memory pool
	};




}