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
	constexpr size_t DEFAULT_CACHE_PAGE_SIZE = 1024 * 256;     // 8192 bytes
	constexpr size_t MINIMAL_CACHE_SIZE = 2 * 1024 * 1024;     // 2048 Kb
	constexpr size_t DEFAULT_CACHE_SIZE = MINIMAL_CACHE_SIZE;  // 2048 Kb
	constexpr size_t PAGE_NOT_FOUND = 0xFFFFFFFFFFFFFFFF;      // "Not found" signature
	//-------------------------------------------------------------------------

	typedef enum {
		FREE = 0,                             // Cache page is free (no data loaded)
		CLEAN = 1,                            // Cache page has been rewritten after being loaded
		DIRTY = 2                             // Cache page is changed, must be saved to storage device
	} PageState;

	typedef struct {
		uint64_t  filePageNo;		          // File page number of cached page
		uint32_t  age;                        // Cycles being in the cache (incremented every cache access)
		PageState state;                      // Current cached page state
		size_t    availableDataLength;        // Available amount of data in cached page
	} CachePageInfo;

	typedef struct {
		uint8_t  data[DEFAULT_CACHE_PAGE_SIZE];
	} CachePageData;

	typedef                                   // Hash map for cache index:
		std::unordered_map<size_t, size_t>    // Key - file page number
		CachedPagesMap;                       // Value - cache page number
	

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
		size_t resizeFile(size_t size);                          

		double cacheHitRate();
		double cacheMissRate();

	private:

		size_t getFreeCachePageIndex();                             
		size_t searchPageInCache(size_t requestedFilePageNo);    
		size_t loadPageToCache(size_t requestedFilePageNo);
		bool   persistCachePage(size_t cachePageIndex);		
		bool   freeCachePage(size_t cachePageIndex);
		void   ageCachePages();                                  
				
		std::filesystem::path pathToFile;                        // Path to file
		std::FILE*      fileHandler;                             // OS file handler
		bool            readOnly;                                // Read only flag

		CachedPagesMap  cacheMap;                                // Cached pages map (file page, cache Page)
		size_t          cachePagesCount;                         // Cached pages amount
		CachePageInfo*  cachePagesInfo;                          // Cached pages description array
		CachePageData*  cachePagesData;                          // Cached pages data array
		size_t          cacheRequests;                           // Cache requests counter
		size_t          cacheMisses;                             // Cache misses counter
		size_t          mostAgedPageIndex;                       // Most aged page
		
	};




}