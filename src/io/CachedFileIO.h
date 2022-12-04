/******************************************************************************
*
*  CachedFileIO class header
*
*  (C) Bolat Basheyev 2022
*
******************************************************************************/

#pragma once

#include <cstdio>
#include <filesystem>
#include <cstring>
#include <cstdint>
#include <unordered_map>

namespace Boson {

	//-------------------------------------------------------------------------
	constexpr size_t DEFAULT_CACHE_PAGE_SIZE = 4 * 1024;       // 4096 bytes
	constexpr size_t MINIMAL_CACHE_SIZE = 256 * 1024;          // 256 Kb minimal cache
	constexpr size_t DEFAULT_CACHE_SIZE = 1 * 1024 * 1024;     // 1Mb default cache
	constexpr size_t PAGE_NOT_FOUND = 0xFFFFFFFFFFFFFFFF;      // "Not found" signature
	//-------------------------------------------------------------------------

	typedef enum {
		FREE = 0,                                // Cache page is free (no data loaded)
		CLEAN = 1,                               // Cache page has been rewritten after being loaded
		DIRTY = 2                                // Cache page is changed, must be saved to storage device
	} PageState;

	typedef struct {
		uint64_t  filePageNo;		             // File page number of cached page
		PageState state;                         // Current cached page state
		size_t    availableDataLength;           // Available amount of data in cached page
		uint8_t   data[DEFAULT_CACHE_PAGE_SIZE]; // Data payload
	} CachePage;

	typedef                                      // Hash map for cache index:
		std::unordered_map<size_t, CachePage*>   // Key - file page number
		CachedPagesMap;                          // Value - cache page reference
	
	typedef	                                     // Double linked list for cached pages
		std::list<CachePage*>                    // Page index
		CacheLinkedList;

	//-------------------------------------------------------------------------
	// Binary random access cached file IO
	//-------------------------------------------------------------------------
	class CachedFileIO {
	public:
		CachedFileIO();
		~CachedFileIO();

		bool open(char* fileName, size_t cacheSize = DEFAULT_CACHE_SIZE, bool readOnly = false);
		bool close();

		size_t getFileSize();
		size_t read(size_t position, void* dataBuffer, size_t length);
		size_t write(size_t position, const void* dataBuffer, size_t length);
		size_t flush();
		bool   resizeFile(size_t size);                          

		double cacheHitRate();
		double cacheMissRate();

	private:

		CachePage* allocatePage();
		void       releasePages();
		CachePage* getFreeCachePage();                            
		CachePage* searchPageInCache(size_t filePageNo);
		CachePage* loadPageToCache(size_t filePageNo);
		bool   persistCachePage(CachePage* pageInfo);
		bool   clearCachePage(CachePage* pageInfo);
				
		std::filesystem::path pathToFile;                        // Path to file
		std::FILE*      fileHandler;                             // OS file handler
		bool            readOnly;                                // Read only flag

		size_t          maxPagesCount;                           // Maximum cache capacity (pages)
		size_t          pageCounter;                             // Allocated pages counter
		CachedPagesMap  cacheMap;                                // Cached pages map (file page, cache Page)
		CacheLinkedList cacheList;                               // Cached pages double linked list
		CachePage*      memoryPool;                              // Cache pages memory pool
		
		size_t          cacheRequests;                           // Cache requests counter
		size_t          cacheMisses;                             // Cache misses counter
		
	};




}