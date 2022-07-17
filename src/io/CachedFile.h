#pragma once

#include <cstdio>
#include <cstring>
#include <cstdint>


namespace Boson {

	//-------------------------------------------------------------------------
	constexpr size_t DEFAULT_CACHE_PAGE_SIZE = 256 * 1024;     // 256Kb - SSD Block size (depends on device)
	constexpr size_t MINIMAL_CACHE_SIZE = 1024 * 1024;         // 1024Kb
	constexpr size_t DEFAULT_CACHE_SIZE = MINIMAL_CACHE_SIZE;  // 1024Kb
	constexpr size_t PAGE_NOT_FOUND = 0xFFFFFFFFFFFFFFFF;      // Not found signature
	//-------------------------------------------------------------------------

	typedef enum {
		FREE = 0,
		CLEAN = 1,
		DIRTY = 2
	} PageState;

	typedef struct {
		uint64_t  pageNumber;		
		uint32_t  age;
		PageState state;
		size_t    availableDataLength;
	} CachePageInfo;

	typedef struct {
		uint8_t  data[DEFAULT_CACHE_PAGE_SIZE];
	} CachePageData;


	//-------------------------------------------------------------------------
	// Binary random access file cached IO
	//-------------------------------------------------------------------------
	class CachedFile {
	public:
		CachedFile();
		~CachedFile();
		bool open(char* dbName, size_t cacheSize = DEFAULT_CACHE_SIZE, bool readOnly = false);
		bool close();
		size_t getSize();
		size_t read(size_t position, void* dataBuffer, size_t length);
		size_t write(size_t position, void* dataBuffer, size_t length);
		size_t append(void* dataBuffer, size_t length);
		size_t flush();		
	private:
		size_t searchPageInCache(size_t pageNumber);               
		size_t getFreeCachePageIndex();                            
		size_t loadPageToCache(size_t pageNumber);
		size_t persistCachePage(size_t cachePageIndex);
		size_t ageCachePages();
		size_t freeCachePage(size_t cachePageIndex);
		
		bool           readOnly;
		std::FILE*     handler;
		size_t         pagesCount;
		CachePageInfo* pagesInfo;
		CachePageData* pagesData;

	};




}