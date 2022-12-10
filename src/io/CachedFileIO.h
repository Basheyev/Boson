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
*  CachedFileIO comply LRU/FBW caching strategy:
*    - O(1) time complexity of look up
*    - O(1) time complexity of insert / remove
*    - 50%-99% cache read hits leads to 10%-490% performance growth (vs STDIO)
*    - 1%-42% cache read hits leads to 1%-35% performance drop (vs STDIO)
*
*  (C) Bolat Basheyev 2022
*
******************************************************************************/

#pragma once

#include <cstdio>
#include <cstring>
#include <cstdint>
#include <unordered_map>

namespace Boson {

	//-------------------------------------------------------------------------
	constexpr size_t PAGE_SIZE      = 8 * 1024;     // 8192 bytes page size
	constexpr size_t MINIMAL_CACHE  = 256*1024;     // 256Kb minimal cache
	constexpr size_t DEFAULT_CACHE  = 1*1024*1024;  // 1Mb default cache
	constexpr size_t PAGE_NOT_FOUND = -1;           // "Not found" signature
	//-------------------------------------------------------------------------

	typedef enum {                              // Cache Page State
		CLEAN = 0,                              // Page has not been changed
		DIRTY = 1                               // Cache page is rewritten
	} PageState;

	typedef struct {
		uint64_t  filePageNo;                   // Page number in file
		PageState state;                        // Current page state
		size_t    availableDataLength;          // Available amount of data
		uint8_t   data[PAGE_SIZE];              // Data itself (payload)
	} CachePage;

	typedef                                     // Hash map for cache index:
		std::unordered_map<size_t, CachePage*>  // Key - file page number
		CachedPagesMap;                         // Value - cache page pointer
	
	typedef                                     // Double linked list 
		std::list<CachePage*>                   // for cached pages
		CacheLinkedList;

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
	// Binary random access cached file IO
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

		void   clearStats();
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
				
		std::FILE*      fileHandler;             // OS file handler
		bool            readOnly;                // Read only flag

		size_t          maxPagesCount;           // Maximum cache capacity (pages)
		size_t          pageCounter;             // Allocated pages counter
		CachedPagesMap  cacheMap;                // Cached pages map 
		CacheLinkedList cacheList;               // Cached pages double linked list
		CachePage*      cacheMemoryPool;         // Cache pages memory pool
		
		size_t          cacheRequests;           // Cache requests counter
		size_t          cacheMisses;             // Cache misses counter
		size_t          totalBytesRead;          // Total bytes read
		size_t          totalBytesWritten;       // Total bytes written
		size_t          totalReadDuration;       // Time of read operations (ns)
		size_t          totalWriteDuration;      // Time of write operations (ns)
		
	};




}