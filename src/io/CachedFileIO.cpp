/******************************************************************************
*  
*  CachedFileIO class implementation
* 
*  CachedFileIO is designed to improve performance of file I/O 
*  operations. Almost all real world apps show some form of locality of 
*  reference. Research say that 10-15% of database size cache gives 
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

#include "CachedFileIO.h"

#include <algorithm>
#include <iostream>
#include <filesystem>

using namespace Boson;

/**
*
* @brief Constructor
*
*/
CachedFileIO::CachedFileIO() {
	this->fileHandler = nullptr;
	this->readOnly = false;
	this->maxPagesCount = 0;
	this->pageCounter = 0;
	this->cacheMemoryPool = nullptr;
	this->cacheList.clear();
	this->cacheMap.clear();	
	this->cacheRequests = 0;
	this->cacheMisses = 0;
}


/**
* 
* @brief Destructor closes file if it still open
* 
*/
CachedFileIO::~CachedFileIO() {
	this->close();
}


/**
*
*  @brief Opens file and allocates cache memory
* 
*  @param[in] fileName   - the name of the file to be opened (path)
*  @param[in] cacheSize  - how much memory for cache to allocate (bytes) 
*  @param[in] isReadOnly - if true, write operations are not allowed
*
*  @return true if file opened, false if can't open file
*
*/
bool CachedFileIO::open(const char* path, size_t cache, bool isReadOnly) {
	// if null pointer
	if (path == nullptr) return false;

	// if current file still open, close it
	if (this->fileHandler != nullptr) close();

	// try to open existing file for binary read/update (file must exist)
	errno_t errNo = fopen_s(&(this->fileHandler), path, "r+b");
	// if file does not exist or another problem
	if (errNo != 0 || this->fileHandler == nullptr) {
		// if can`t open file in read only mode return false
		if (isReadOnly) return false;
		// try to create new file for binary write/read
		errNo = fopen_s(&(this->fileHandler), path, "w+b");
		// if still can't create file return false
		if (errNo != 0 || this->fileHandler == nullptr) return false;
	}

	// save path to file
	this->pathToFile = path;

	// set mode to no buffering, we will manage buffers and caching by our selves
	setvbuf(this->fileHandler, nullptr, _IONBF, 0);

	// Check minimal cache size
	if (cache < MINIMAL_CACHE) cache = MINIMAL_CACHE;

	// Allocate cache memory for cache pages (aligned to page size)
	this->maxPagesCount = cache / PAGE_SIZE;
	this->allocatePool(this->maxPagesCount);
	
	// Clear cache pages list and map
	this->cacheList.clear();
	this->cacheMap.clear();

	// Set readOnly flag
	this->readOnly = isReadOnly;

    // Reset stats
	this->cacheRequests = 0;
	this->cacheMisses = 0;

	// file successfuly opened
	return true;
}



/**
*
*  @brief Closes file, persists changed pages and releases cache memory
*
*  @return true if file correctly closed, false if file has not been opened
*
*/
bool CachedFileIO::close() {
	
	// check if file was opened
	if (fileHandler == nullptr) return false;
	
	// flush buffers
	this->flush();
	
	// close file
	fclose(fileHandler);

	// Release memory pool of cached pages
	this->releasePool();

	// mark that file is closed
	this->fileHandler = nullptr;

	return true;
}



/**
*
*  @brief Get current file size
*
*  @return actual file size in bytes
*
*/
size_t CachedFileIO::size() {
	if (fileHandler == nullptr) return 0;
	size_t currentPosition = ftell(fileHandler);
	_fseeki64(fileHandler, 0, SEEK_END);
	size_t fileSize = ftell(fileHandler);
	_fseeki64(fileHandler, currentPosition, SEEK_SET);
	return fileSize;
}




/**
* 
*  @brief Read data from cached file
* 
*  @param[in]  position   - offset from beginning of the file
*  @param[out] dataBuffer - data buffer where data copied
*  @param[in]  length     - data amount to read
* 
*  @return total bytes amount actually read to the data buffer
* 
*/
size_t CachedFileIO::read(size_t position, void* dataBuffer, size_t length) {
	
	// Check if file handler, data buffer and length are not null
	if (fileHandler == nullptr || dataBuffer == nullptr || length == 0) return 0;
	
	// Calculate start and end page number in the file
	size_t firstPageNo = position / PAGE_SIZE;
	size_t lastPageNo = (position + length) / PAGE_SIZE;
	
	// Initialize local variables
	CachePage* pageInfo = nullptr;
	uint8_t* src = nullptr;
	uint8_t* dst = (uint8_t*) dataBuffer;
	size_t bytesToCopy = 0, bytesRead = 0;	
	size_t pageDataLength = 0;
	
	// Iterate through requested file pages
	for (size_t filePage = firstPageNo; filePage <= lastPageNo; filePage++) {
		
		// Lookup or load file page to cache
		pageInfo = searchPageInCache(filePage);
		if (pageInfo == nullptr) {
			pageInfo = loadPageToCache(filePage);
			if (pageInfo == nullptr) return 0;
		}
		
		// Get cached page description and data
		//pageInfo = &cachePages[cacheIndex];
		pageDataLength = pageInfo->availableDataLength;
		
		// Calculate source pointers and data length to copy
		if (filePage == firstPageNo) {

			// Case 1: if reading first page
			size_t firstPageOffset = position % PAGE_SIZE;   
			src = &pageInfo->data[firstPageOffset];             
			if (firstPageOffset < pageDataLength)                          
				if (firstPageOffset + length > pageDataLength)             
					bytesToCopy = pageDataLength - firstPageOffset;        
				else bytesToCopy = length;                                 
			else bytesToCopy = 0;  

		} else if (filePage == lastPageNo) {  

			// Case 2: if reading last page
			size_t remainingBytes = (position + length) % PAGE_SIZE;
			src = pageInfo->data;                               
			if (remainingBytes < pageDataLength)                           
				bytesToCopy = remainingBytes;                              
			else bytesToCopy = pageDataLength;  

		} else {                       

			// Case 3: if reading middle page 
			src = pageInfo->data;
			bytesToCopy = PAGE_SIZE;

		}

		// Copy available data from cache page to user's data buffer
		if (bytesToCopy > 0) {			
			memcpy(dst, src, bytesToCopy);   // copy data to user buffer
			bytesRead += bytesToCopy;        // increment read bytes counter
			dst += bytesToCopy;              // increment pointer in user buffer
		}
	}
	return bytesRead;
}



/**
*
*  @brief Writes data to cached file
*
*  @param[in]  position   - offset from beginning of the file
*  @param[in]  dataBuffer - data buffer with write data
*  @param[in]  length     - data amount to write
*
*  @return total bytes amount written to the cached file
*
*/
size_t CachedFileIO::write(size_t position, const void* dataBuffer, size_t length) {

	// Check if file handler, data buffer and length are not null
	if (fileHandler == nullptr || this->readOnly || dataBuffer == nullptr || length == 0) return 0;

	// Calculate start and end page number in the file
	size_t firstPageNo = position / PAGE_SIZE;
	size_t lastPageNo = (position + length) / PAGE_SIZE;

	// Initialize local variables
	CachePage* pageInfo = nullptr;
	uint8_t* src = (uint8_t*)dataBuffer;
	uint8_t* dst = nullptr;
	size_t bytesToCopy = 0, bytesWritten = 0;
	size_t pageDataLength = 0;

	// Iterate through file pages
	for (size_t filePage = firstPageNo; filePage <= lastPageNo; filePage++) {

		// Fetch-before-write (FBW)
		pageInfo = searchPageInCache(filePage);
		if (pageInfo == nullptr) {
			pageInfo = loadPageToCache(filePage);
			if (pageInfo == nullptr) return 0;
		}

		// Get cached page description and data
		//pageInfo = &cachePages[cacheIndex];
		pageDataLength = pageInfo->availableDataLength;

		// Calculate source pointers and data length to write
		if (filePage == firstPageNo) {
			// Case 1: if writing first page
			size_t firstPageOffset = position % PAGE_SIZE;
			dst = &pageInfo->data[firstPageOffset];
			bytesToCopy = std::min(length, PAGE_SIZE - firstPageOffset);
		} else if (filePage == lastPageNo) {
			// Case 2: if writing last page
			dst = pageInfo->data;
			bytesToCopy = length - bytesWritten;
		} else {
			// Case 3: if reading middle page 
			dst = pageInfo->data;
			bytesToCopy = PAGE_SIZE;
		}

		// Copy available data from user's data buffer to cache page 
		if (bytesToCopy > 0) {
			
			memcpy(dst, src, bytesToCopy);       // copy user buffer data to cache page
	
			pageInfo->state = PageState::DIRTY;  // mark page as "dirty" (rewritten)
			pageInfo->availableDataLength = std::max(pageDataLength, bytesToCopy);       // wrong!
			
			bytesWritten += bytesToCopy;         // increment written bytes counter
			src += bytesToCopy;                  // increment pointer in user buffer
			
		}

	}

	return bytesWritten;

}



/**
* 
*  @brief Persists all changed cache pages to storage device
* 
*  @return true if all changed cache pages been persisted, false otherwise
* 
*/
size_t CachedFileIO::flush() {

	// Suppose all pages will be persisted
	bool allDirtyPagesPersisted = true;

	// Sort cache list by file page number for sequential write
	cacheList.sort([](const CachePage* cp1, const CachePage* cp2)
		{
			if (cp1->filePageNo == cp2->filePageNo)
				return cp1->filePageNo < cp2->filePageNo;
			return cp1->filePageNo < cp2->filePageNo;
		});

	// Persist pages to storage device
	for (CachePage* node : cacheList) {
		allDirtyPagesPersisted = allDirtyPagesPersisted && clearCachePage(node);
	}

	return allDirtyPagesPersisted;
}



/**
* @brief Return stats
* @param type - requested stats type
* @return value of stats
*/
double CachedFileIO::getStats(CacheStats type) {
	double totalRequests = (double) cacheRequests;
	double totalCacheMisses = (double)cacheMisses;
	switch (type) {
	case CacheStats::TOTAL_REQUESTS:
		return totalRequests;
	case CacheStats::CACHE_HITS_RATE:
		return (totalRequests - totalCacheMisses) / totalRequests * 100.0;
	case CacheStats::CACHE_MISSES_RATE:
		return totalCacheMisses / totalRequests * 100.0;
	}
	return 0.0;
}



//=============================================================================
// 
// 
//                       Cached pages control methods
// 
// 
//=============================================================================


/**
* @brief Allocates memory pool for cache pages
*/
void CachedFileIO::allocatePool(size_t pagesCount) {
	this->cacheMemoryPool = new CachePage[pagesCount];
}


/**
* @brief Releases memory pool
*/
void CachedFileIO::releasePool() {
	this->pageCounter = 0;
	cacheList.clear();
	cacheMap.clear();
	delete[] cacheMemoryPool;
}


/**
* @brief Allocates cache page from memory pool
*/
CachePage* CachedFileIO::allocatePage() {

	if (pageCounter >= maxPagesCount) return nullptr;

	// Allocate memory for cache page
	CachePage* newPage = &cacheMemoryPool[pageCounter];
	pageCounter++;

	// Clear cache page info fields
	newPage->filePageNo = PAGE_NOT_FOUND;
	newPage->availableDataLength = 0;

	return newPage;
}


/**
*
* @brief Returns free page: allocates new or return most aged cache page if cache limit reached
* 
*
* @return most aged page reference
*
*/
CachePage* CachedFileIO::getFreeCachePage() {
	if (cacheList.size() < maxPagesCount) {
		return allocatePage();
	} else {
		// get most aged cache page (back of the list)
		CachePage* freePage = this->cacheList.back();
		// clear cache page state
		clearCachePage(freePage);
		// remove cache page from list back
		this->cacheList.pop_back();
		// return cache page reference
		return freePage;
	}
}



/**
* 
* @brief Lookup cache page of requested file page if it exists
* 
* @param requestedFilePageNo - requested file page number
* @return cache page reference of requested file page or returns nullptr
* 
*/
CachePage* CachedFileIO::searchPageInCache(size_t filePageNo) {
	
	// increment total cache lookup requests
	cacheRequests++;

	// Search file page in index map and return if its found
	auto result = cacheMap.find(filePageNo);
	if (result != cacheMap.end()) {
		CachePage* cachePage = result->second;
		// TODO: bubble up page to beginning of list
		return cachePage;
	}
	
	// increment cache misses counter
	cacheMisses++;

	return nullptr;
}



/**
* 
*  @brief Loads requested page from storage device to cache and returns cache page
* 
*  @param requestedFilePageNo - file page number to load
*  @return loaded page cache index or nullptr if file is not open.
* 
*/
CachePage* CachedFileIO::loadPageToCache(size_t filePageNo) {

	if (fileHandler == nullptr) return nullptr;

	// get free cache page index
	CachePage* cachePage = getFreeCachePage();

	// calculate offset and initialize variables
	size_t offset = filePageNo * PAGE_SIZE;
	size_t bytesToRead = PAGE_SIZE;	
	size_t bytesRead = 0;

	// Fetch page from storage device
	_fseeki64(fileHandler, offset, SEEK_SET);
	bytesRead = fread(cachePage->data, 1, bytesToRead, fileHandler);
	
	// if available data less than page size
	if (bytesRead < PAGE_SIZE) {
		// fill remaining part of page with zero to avoid artifacts
		memset(&(cachePage->data[bytesRead]), 0, PAGE_SIZE - bytesRead);
	}

	// fill loaded page description info
	cachePage->state = PageState::CLEAN;
	cachePage->filePageNo = filePageNo;
	cachePage->availableDataLength = bytesRead;

	// insert page index to cache list
	cacheList.push_front(cachePage);

	// Add filePage/cachePage key-value pair to the map
	cacheMap.insert({ filePageNo, cachePage });
	

	return cachePage;
}



/**
* 
*  @brief Writes specified cache page to the storage device
* 
*  @param cachePageIndex - page index in the cache
*  @return true - page successfuly persisted, false - write failed or file is not open
* 
*/ 
bool CachedFileIO::persistCachePage(CachePage* cachedPage) {

	if (fileHandler == nullptr) return false;

	// Get file page number of cached page and calculate offset in the file
	size_t offset = cachedPage->filePageNo * PAGE_SIZE;
	size_t bytesToWrite = PAGE_SIZE;
	size_t bytesWritten = 0;
	
	// Go to calculated offset in the file
	_fseeki64(fileHandler, offset, SEEK_SET);

	// Write cached page to file
	bytesWritten = fwrite(cachedPage->data, 1, bytesToWrite, fileHandler);

	// Check success
	return bytesWritten == bytesToWrite;
}



/**
* 
*  @brief Clears cache page state and persists its data to storage if page has been changed
* 
*  @param cachePageIndex - index of the page in cache to clear
*  @return true - if page cleared, false - if can't persist page to storage
* 
*/
bool CachedFileIO::clearCachePage(CachePage* pageInfo) {
	
	// if cache page has been rewritten persist page to storage device
	if (pageInfo->state == PageState::DIRTY) {
		if (!persistCachePage(pageInfo)) return false;
	}

	// Remove from index map
	cacheMap.erase(pageInfo->filePageNo);

	// Clear cache page info fields
	pageInfo->filePageNo = PAGE_NOT_FOUND;
	pageInfo->availableDataLength = 0;
			
	// Cache page freed
	return true;
}

