#pragma once

#include <cstdio>
#include <cstring>
#include <cstdint>


namespace Boson {

	//-------------------------------------------------------------------------
	constexpr size_t DEFAULT_CACHE_PAGE_SIZE = 4096;           // 256Kb - SSD Block size (depends on device)
	constexpr size_t MINIMAL_CACHE_SIZE = 1024 * 1024;         // 1024Kb
	constexpr size_t DEFAULT_CACHE_SIZE = MINIMAL_CACHE_SIZE;  // 1024Kb
	constexpr size_t PAGE_NOT_FOUND = 0xFFFFFFFFFFFFFFFF;      // Not found signature
	//-------------------------------------------------------------------------

	typedef enum {
		FREE = 0,                             // Cache page is free (no data loaded)
		CLEAN = 1,                            // Cache page has been rewritten after being loaded
		DIRTY = 2                             // Cache page is changed, must be saved to storage device
	} PageState;

	typedef struct {
		uint64_t  pageNumber;		          // File page number of cached page
		uint32_t  age;                        // Cycles being in the cache (incremented every cache access)
		PageState state;                      // Current cached page state
		size_t    availableDataLength;        // Available amount of data in cached page
	} CachePageInfo;

	typedef struct {
		uint8_t  data[DEFAULT_CACHE_PAGE_SIZE];
	} CachePageData;


	//-------------------------------------------------------------------------
	// Binary random access file cached IO
	//-------------------------------------------------------------------------
	class CachedFileIO {
	public:
		CachedFileIO();
		~CachedFileIO();
		bool open(char* dbName, size_t cacheSize = DEFAULT_CACHE_SIZE, bool readOnly = false);
		bool close();
		size_t getSize();
		size_t read(size_t position, void* dataBuffer, size_t length);
		size_t write(size_t position, void* dataBuffer, size_t length);
		size_t append(void* dataBuffer, size_t length);
		size_t flush();		
	private:
		size_t searchPageInCache(size_t pageNumber);               // slow - implement HashMap?
		size_t getFreeCachePageIndex();                            // slow - implement HashMap?
		size_t loadPageToCache(size_t pageNumber);
		bool   persistCachePage(size_t cachePageIndex);		
		bool   freeCachePage(size_t cachePageIndex);
		void   ageCachePages();
		
		bool           readOnly;                                   // Read only flag
		std::FILE*     fileHandler;                                // OS file handler
		size_t         pagesCount;                                 // Cached pages amount
		CachePageInfo* pagesInfo;                                  // Cached pages description array
		CachePageData* pagesData;                                  // Cached pages data array

	};




}