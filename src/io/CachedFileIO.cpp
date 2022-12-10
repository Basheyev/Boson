/******************************************************************************
*  
*  CachedFileIO class implementation
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

#include "CachedFileIO.h"

#include <algorithm>
#include <iostream>
#include <chrono>

using namespace Boson;

/**
*
* @brief Constructor
*
*/
CachedFileIO::CachedFileIO() {
	this->readOnly = false;
	this->fileHandler = nullptr;
	this->cachePageInfoPool = nullptr;		
	this->cachePageDataPool = nullptr;
	this->maxPagesCount = 0;
	this->pageCounter = 0;
	clearStats();
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
bool CachedFileIO::open(const char* path, size_t cacheSize, bool isReadOnly) {
	// return if null pointer
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
	// set mode to no buffering, we will manage buffers and caching by our selves
	setvbuf(this->fileHandler, nullptr, _IONBF, 0);
	// Allocated cache
	setCacheSize(cacheSize);
	// Set readOnly flag
	this->readOnly = isReadOnly;
	// Clear statistics
	this->clearStats();
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

	// Time point A
	auto startTime = std::chrono::steady_clock::now();
		
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

	// Time point B
	auto endTime = std::chrono::steady_clock::now();
	// Calculate and increment read duration
	this->totalReadDuration += (endTime - startTime).count();
	// Increment bytes read
	this->totalBytesRead += bytesRead;
	// return bytes read
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

	// Time point A
	auto startTime = std::chrono::steady_clock::now();
	
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
			pageInfo->availableDataLength = std::max(pageDataLength, bytesToCopy);
			bytesWritten += bytesToCopy;         // increment written bytes counter
			src += bytesToCopy;                  // increment pointer in user buffer
		}
	}
	// Time point B
	auto endTime = std::chrono::steady_clock::now();
	// Calculate and increment write duration
	this->totalWriteDuration += (endTime - startTime).count();
	// Increment bytes written
	this->totalBytesWritten += bytesWritten;
	// return bytes written
	return bytesWritten;
}



/**
*
*  @brief Read page from cached file to user buffer
*
*  @param[in]  pageNo - file page number
*  @param[out] userPageBuffer - data buffer (Boson::PAGE_SIZE)
*
*  @return total bytes amount actually read to the data buffer
*
*/
size_t CachedFileIO::readPage(size_t pageNo, void* userPageBuffer) {

	// Check if file handler, data buffer and length are not null
	if (fileHandler == nullptr || userPageBuffer == nullptr) return 0;

	// Time point A
	auto startTime = std::chrono::steady_clock::now();

	// Lookup or load file page to cache
	CachePage* pageInfo = searchPageInCache(pageNo);
	if (pageInfo == nullptr) {
		pageInfo = loadPageToCache(pageNo);
		if (pageInfo == nullptr) return 0;
	}

	uint8_t *src = pageInfo->data;
	uint8_t* dst = (uint8_t*) userPageBuffer;
	size_t availableData = pageInfo->availableDataLength;

	// Copy available data from cache page to user's data buffer
	if (availableData > 0) memcpy(dst, src, availableData);
		
	// Time point B
	auto endTime = std::chrono::steady_clock::now();
	// Calculate and increment read duration
	this->totalReadDuration += (endTime - startTime).count();
	// Increment bytes read
	this->totalBytesRead += availableData;

	return availableData;
}



/**
*
*  @brief Writes page from user buffer to cached file
*
*  @param[in]  position   - offset from beginning of the file
*  @param[in]  dataBuffer - data buffer with write data
*  @param[in]  length     - data amount to write
*
*  @return total bytes amount written to the cached file
*
*/
size_t CachedFileIO::writePage(size_t pageNo, const void* userPageBuffer) {
	// Check if file handler and data buffer are not null, and write is allowed
	if (fileHandler == nullptr || this->readOnly || userPageBuffer == nullptr) return 0;

	// Time point A
	auto startTime = std::chrono::steady_clock::now();

	// Fetch-before-write (FBW)
	CachePage* pageInfo = searchPageInCache(pageNo);
	if (pageInfo == nullptr) {
		pageInfo = loadPageToCache(pageNo);
		if (pageInfo == nullptr) return 0;
	}

	// Initialize local variables
	uint8_t* src = (uint8_t*)userPageBuffer;
	uint8_t* dst = nullptr;
	size_t bytesToCopy = PAGE_SIZE;

	memcpy(dst, src, bytesToCopy);               // copy user buffer data to cache page
	pageInfo->state = PageState::DIRTY;          // mark page as "dirty" (rewritten)
	pageInfo->availableDataLength = bytesToCopy; // set available data as PAGE_SIZE

	// Time point B
	auto endTime = std::chrono::steady_clock::now();
	// Calculate and increment write duration
	this->totalWriteDuration += (endTime - startTime).count();
	// Increment bytes written
	this->totalBytesWritten += bytesToCopy;

	return bytesToCopy;
}



/**
* 
*  @brief Persists all changed cache pages to storage device
* 
*  @return true if all changed cache pages been persisted, false otherwise
* 
*/
size_t CachedFileIO::flush() {

	if (fileHandler == nullptr || this->readOnly) return 0;

	// Time point A
	auto startTime = std::chrono::steady_clock::now();

	// Suppose all pages will be persisted
	bool allDirtyPagesPersisted = true;

	// Sort cache list by file page number in ascending order for sequential write
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
	
	// flush buffers to storage device
	bool buffersFlushed = (fflush(fileHandler) == 0);

	// Time point B
	auto endTime = std::chrono::steady_clock::now();
	// Calculate and increment write duration
	this->totalWriteDuration += (endTime - startTime).count();

	return allDirtyPagesPersisted && buffersFlushed;

}



/**
* @brief Clear IO statistics
* @param type - requested stats type
* @return value of stats
*/
void CachedFileIO::clearStats() {
	this->cacheRequests = 0;
	this->cacheMisses = 0;
	this->totalBytesRead = 0;
	this->totalBytesWritten = 0;
	this->totalReadDuration = 0;
	this->totalWriteDuration = 0;
}



/**
* @brief Return IO statistics
* @param type - requested stats type
* @return value of stats
*/
double CachedFileIO::getStats(CacheStats type) {

	double totalRequests = (double)cacheRequests;
	double totalCacheMisses = (double)cacheMisses;
	double seconds = 0;
	double megabytes = 0;

	switch (type) {
	case CacheStats::TOTAL_REQUESTS:
		return totalRequests;
	case CacheStats::CACHE_HITS_RATE:
		if (totalRequests == 0) return 0;
		return (totalRequests - totalCacheMisses) / totalRequests * 100.0;
	case CacheStats::CACHE_MISSES_RATE:
		if (totalRequests == 0) return 0;
		return totalCacheMisses / totalRequests * 100.0;
	case CacheStats::READ_THROUGHPUT:
		if (this->totalReadDuration == 0) return 0;
		seconds = double(this->totalReadDuration) / 1000000000.0;
		megabytes = double(this->totalBytesRead) / (1024 * 1024);
		return megabytes / seconds;
	case CacheStats::WRITE_THROUGHPUT:
		if (this->totalWriteDuration == 0) return 0;
		seconds = double(this->totalWriteDuration) / 1000000000.0;
		megabytes = double(this->totalBytesWritten) / (1024 * 1024);
		return megabytes / seconds;
	case CacheStats::WRITE_TIME_NS:
		return double(totalWriteDuration);
	case CacheStats::READ_TIME_NS:
		return double(totalReadDuration);
	}
	return 0.0;
}



/**
*
*  @brief Get current file size
*
*  @return actual file size in bytes
*
*/
size_t CachedFileIO::getFileSize() {
	if (fileHandler == nullptr) return 0;
	size_t currentPosition = _ftelli64(fileHandler);
	_fseeki64(fileHandler, 0, SEEK_END);
	size_t fileSize = _ftelli64(fileHandler);
	_fseeki64(fileHandler, currentPosition, SEEK_SET);
	return fileSize;
}


//=============================================================================
// 
// 
//                       Cached pages control methods
// 
// 
//=============================================================================

/**
*
*  @brief Get cache size in bytes
*  @return actual cache size in bytes
*
*/
size_t CachedFileIO::getCacheSize() {
	return this->maxPagesCount * PAGE_SIZE;
}



/**
*
*  @brief Resize cache at runtime: releases memory and allocate new one
*  @param cacheSize - new cache size
*  @return actual cache size in bytes
*
*/
size_t CachedFileIO::setCacheSize(size_t cacheSize) {
	
	// check if cache is already allocated
	if (cachePageInfoPool != nullptr) {
		// Persist all changed pages to storage device
		this->flush();
		// Release allocated memory, list and map
		this->releasePool();
	} 

	// Check minimal cache size
	if (cacheSize < MINIMAL_CACHE) cacheSize = MINIMAL_CACHE;
	// Calculate pages count
	this->pageCounter = 0;
	this->maxPagesCount = cacheSize / PAGE_SIZE;
	// Allocate new cache
	this->allocatePool(this->maxPagesCount);
	// Clear cache pages list and map
	this->cacheList.clear();
	this->cacheMap.clear();
	// Reset stats
	this->cacheRequests = 0;
	this->cacheMisses = 0;
	// Return cache size in bytes
	return this->maxPagesCount * PAGE_SIZE;
}



/**
* @brief Allocates memory pool for cache pages
*/
void CachedFileIO::allocatePool(size_t pagesToAllocate) {
	this->cachePageInfoPool = new CachePage[pagesToAllocate];
	this->cachePageDataPool = new uint8_t[pagesToAllocate * PAGE_SIZE];
}


/**
* @brief Releases memory pool
*/
void CachedFileIO::releasePool() {
	this->pageCounter = 0;
	cacheList.clear();
	cacheMap.clear();
	delete[] cachePageInfoPool;
	delete[] cachePageDataPool;
	cachePageInfoPool = nullptr;
	cachePageDataPool = nullptr;
}


/**
* @brief Allocates cache page from memory pool
*/
CachePage* CachedFileIO::allocatePage() {

	if (pageCounter >= maxPagesCount) return nullptr;

	// Allocate memory for cache page
	CachePage* newPage = &cachePageInfoPool[pageCounter];
	// Clear cache page info fields
	newPage->filePageNo = PAGE_NOT_FOUND;
	newPage->state = PageState::CLEAN;
	newPage->availableDataLength = 0;
	newPage->data = &cachePageDataPool[pageCounter * PAGE_SIZE];
	// Increment page counter
	pageCounter++;

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



