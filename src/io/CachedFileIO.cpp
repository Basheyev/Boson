/******************************************************************************
*  
*  CachedFileIO class implementation
* 
* 
* 
* 
*  (C) Bolat Basheyev 2022
* 
* 
*  Performance research results:
*     
*  - In case of sequencial read CachedFileIO outperforms STDIO by 
*    20%-200% for JSONs under 180-1565 bytes (average JSON size). 
*    But for JSONs in range 2600-8096 bytes works slower 13-32%.
* 
*  - In case of random read with cache misses less than 33-50% 
*    CachedFileIO outperforms STDIO by 20%-40%. With cache misses 
*    more than 95%, CachedFileIO works slower than STDIO by 5%-13%.
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
	this->cachePagesCount = 0;
	this->cachePagesInfo = nullptr;
	this->cachePagesData = nullptr;
	this->readOnly = false;
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
*  @param[in] fileName   - the name of the file to be opened
*  @param[in] cacheSize  - how much memory for cache to allocate 
*  @param[in] isReadOnly - if true, write operations are not allowed
*
*  @return true if file opened, false if can't open file
*
*/
bool CachedFileIO::open(char* fileName, size_t cacheSize, bool isReadOnly) {
	// if current file still open, close it
	if (this->fileHandler != nullptr) close();
	// try to open existing file for binary read/update (file must exist)
	this->fileHandler = fopen(fileName, "r+b");
	// if file does not exist or another problem
	if (this->fileHandler == nullptr) {
		// if can`t open file in read only mode return false
		if (isReadOnly) return false;
		// try to create new file for binary write/read
		this->fileHandler = fopen(fileName, "w+b");
		// if still can't create file return false
		if (this->fileHandler == nullptr) return false;
	}

	this->pathToFile = fileName;

	// set mode to no buffering, we will manage buffers and caching by our selves
	setvbuf(this->fileHandler, nullptr, _IONBF, 0);

	// Check minimal cache size
	if (cacheSize < MINIMAL_CACHE_SIZE) cacheSize = MINIMAL_CACHE_SIZE;

	// Allocate cache memory aligned to page size
	this->cachePagesCount = cacheSize / DEFAULT_CACHE_PAGE_SIZE;
	this->cachePagesInfo = new CachePageInfo[cachePagesCount];
	this->cachePagesData = new CachePageData[cachePagesCount];

	// Set cache memory to zero
	memset(this->cachePagesInfo, 0, cachePagesCount * sizeof(CachePageInfo));
	memset(this->cachePagesData, 0, cachePagesCount * sizeof(CachePageData));

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

	// Release cache memory
	delete[] cachePagesInfo;
	delete[] cachePagesData;

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
size_t CachedFileIO::getFileSize() {
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
	size_t firstPageNo = position / DEFAULT_CACHE_PAGE_SIZE;
	size_t lastPageNo = (position + length) / DEFAULT_CACHE_PAGE_SIZE;
	
	// Initialize local variables
	CachePageInfo* pageInfo = nullptr;
	CachePageData* pageData = nullptr;
	uint8_t* src = nullptr;
	uint8_t* dst = (uint8_t*) dataBuffer;
	size_t cacheIndex = PAGE_NOT_FOUND;
	size_t bytesToCopy = 0, bytesRead = 0;	
	size_t pageDataLength = 0;
	
	// Iterate through requested file pages
	for (size_t filePage = firstPageNo; filePage <= lastPageNo; filePage++) {
		
		// Lookup or load file page to cache
		cacheIndex = searchPageInCache(filePage);
		if (cacheIndex == PAGE_NOT_FOUND) {
			cacheIndex = loadPageToCache(filePage);
			if (cacheIndex == PAGE_NOT_FOUND) return 0;
		}
		
		// Get cached page description and data
		pageInfo = &cachePagesInfo[cacheIndex];
		pageData = &cachePagesData[cacheIndex];
		pageDataLength = pageInfo->availableDataLength;
		
		// Calculate source pointers and data length to copy
		if (filePage == firstPageNo) {

			// Case 1: if reading first page
			size_t firstPageOffset = position % DEFAULT_CACHE_PAGE_SIZE;   
			src = (uint8_t*) &pageData->data[firstPageOffset];             
			if (firstPageOffset < pageDataLength)                          
				if (firstPageOffset + length > pageDataLength)             
					bytesToCopy = pageDataLength - firstPageOffset;        
				else bytesToCopy = length;                                 
			else bytesToCopy = 0;  

		} else if (filePage == lastPageNo) {  

			// Case 2: if reading last page
			size_t remainingBytes = (position + length) % DEFAULT_CACHE_PAGE_SIZE;
			src = (uint8_t*) pageData->data;                               
			if (remainingBytes < pageDataLength)                           
				bytesToCopy = remainingBytes;                              
			else bytesToCopy = pageDataLength;  

		} else {                       

			// Case 3: if reading middle page 
			src = (uint8_t*)pageData->data;
			bytesToCopy = DEFAULT_CACHE_PAGE_SIZE;

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
	size_t firstPageNo = position / DEFAULT_CACHE_PAGE_SIZE;
	size_t lastPageNo = (position + length) / DEFAULT_CACHE_PAGE_SIZE;

	// Initialize local variables
	CachePageInfo* pageInfo = nullptr;
	CachePageData* pageData = nullptr;
	uint8_t* src = (uint8_t*)dataBuffer;
	uint8_t* dst = nullptr;
	size_t cacheIndex = PAGE_NOT_FOUND;
	size_t bytesToCopy = 0, bytesWritten = 0;
	size_t pageDataLength = 0;

	// Iterate through file pages
	for (size_t filePage = firstPageNo; filePage <= lastPageNo; filePage++) {

		// Fetch-before-write (FBW)
		cacheIndex = searchPageInCache(filePage);
		if (cacheIndex == PAGE_NOT_FOUND) {
			cacheIndex = loadPageToCache(filePage);
			if (cacheIndex == PAGE_NOT_FOUND) return 0;
		}

		// Get cached page description and data
		pageInfo = &cachePagesInfo[cacheIndex];
		pageData = &cachePagesData[cacheIndex];
		pageDataLength = pageInfo->availableDataLength;

		// Calculate source pointers and data length to write
		if (filePage == firstPageNo) {
			// Case 1: if writing first page
			size_t firstPageOffset = position % DEFAULT_CACHE_PAGE_SIZE;
			dst = (uint8_t*)&pageData->data[firstPageOffset];
			bytesToCopy = std::min(length, DEFAULT_CACHE_PAGE_SIZE - firstPageOffset);
		} else if (filePage == lastPageNo) {
			// Case 2: if writing last page
			dst = (uint8_t*)pageData->data;
			bytesToCopy = length - bytesWritten;
		} else {
			// Case 3: if reading middle page 
			dst = (uint8_t*)pageData->data;
			bytesToCopy = DEFAULT_CACHE_PAGE_SIZE;
		}

		// Copy available data from user's data buffer to cache page 
		if (bytesToCopy > 0) {
			
			memcpy(dst, src, bytesToCopy);       // copy user buffer data to cache page
			
			pageInfo->age = 0;                   // reset cache page age to zero
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

	// Persist all "dirty" pages one by one
	for (size_t cacheIndex = 0; cacheIndex < cachePagesCount; cacheIndex++) {
		allDirtyPagesPersisted = allDirtyPagesPersisted && freeCachePage(cacheIndex);
	}

	return allDirtyPagesPersisted;
}



/**
* 
*  @brief Truncates or extends file size to specified size aligned to page size
* 
*  @return current file size in bytes
* 
*/
size_t CachedFileIO::resizeFile(size_t size) {

	
	// Persist cached pages
	this->flush();

	// Check page size alignment
	size_t alignedSize = (size / DEFAULT_CACHE_PAGE_SIZE) * DEFAULT_CACHE_PAGE_SIZE;

	// Resize file
	std::filesystem::resize_file(pathToFile, alignedSize);

	return 0;
}




// @brief cache hit percentage (%)
double CachedFileIO::cacheHitRate() {
	return ((double)cacheRequests - (double)cacheMisses) / (double)cacheRequests * 100.0;
}



// @brief cache miss percentage (%)
double CachedFileIO::cacheMissRate() {
	return (double)cacheMisses / (double)cacheRequests * 100.0;
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
* @brief Returns free or most aged cache page
*
* @return free cache page or most aged page index
*
*/
size_t CachedFileIO::getFreeCachePageIndex() {

	size_t largestAge = 0;
	size_t mostAgedPageIndex = 0;

	// Search for free cache page and most aged one
	for (size_t cacheIndex = 0; cacheIndex < cachePagesCount; cacheIndex++) {
		if (cachePagesInfo[cacheIndex].state == PageState::FREE) return cacheIndex;
		if (cachePagesInfo[cacheIndex].age > largestAge) {
			largestAge = cachePagesInfo[cacheIndex].age;
			mostAgedPageIndex = cacheIndex;
		}
	}

	// if there is no free pages in the cache, free and return most aged page
	freeCachePage(mostAgedPageIndex);

	return mostAgedPageIndex;
}



/**
* 
* @brief Lookup cache page index of requested file page if it exists
* 
* @param requestedFilePageNo - requested file page number
* @return cache page index of requested file page or returns PAGE_NOT_FOUND
* 
*/
size_t CachedFileIO::searchPageInCache(size_t requestedFilePageNo) {
	
	cacheRequests++;

	// Search file page in index map and return if its found
	auto result = cacheMap.find(requestedFilePageNo);
	if (result != cacheMap.end()) return result->second;
	
	cacheMisses++;

	return PAGE_NOT_FOUND;
}



/**
* 
*  @brief Loads requested page from storage device to cache and returns cache index
* 
*  @param requestedFilePageNo - file page number to load
*  @return loaded page cache index or PAGE_NOT_FOUND if file is not open.
* 
*/
size_t CachedFileIO::loadPageToCache(size_t requestedFilePageNo) {

	if (fileHandler == nullptr) return PAGE_NOT_FOUND;

	// increment all pages age
	ageCachePages();

	// get free cache page index
	size_t cachePageIndex = getFreeCachePageIndex();

	// read page from storage device
	uint8_t* cachePage = cachePagesData[cachePageIndex].data;
	size_t offset = requestedFilePageNo * DEFAULT_CACHE_PAGE_SIZE;
	size_t bytesToRead = DEFAULT_CACHE_PAGE_SIZE;
	_fseeki64(fileHandler, offset, SEEK_SET);
	size_t bytesRead = fread(cachePage, 1, bytesToRead, fileHandler);

	// if available data less than page size
	if (bytesRead < DEFAULT_CACHE_PAGE_SIZE) {
		// fill remaining part of page with zero to avoid artifacts
		memset(cachePage + bytesRead, 0, DEFAULT_CACHE_PAGE_SIZE - bytesRead);

	}

	// fill loaded page description info
	CachePageInfo& loadedPage = cachePagesInfo[cachePageIndex];
	loadedPage.state = PageState::CLEAN;
	loadedPage.filePageNo = requestedFilePageNo;
	loadedPage.age = 0;
	loadedPage.availableDataLength = bytesRead;

	// Add filePage/cachePage key-value pair to the map
	cacheMap.insert({ requestedFilePageNo, cachePageIndex });

	return cachePageIndex;
}



/**
* 
*  @brief Writes specified cache page to the storage device
* 
*  @param cachePageIndex - page index in the cache
*  @return true - page successfuly persisted, false - write failed or file is not open
* 
*/ 
bool CachedFileIO::persistCachePage(size_t cachePageIndex) {

	if (fileHandler == nullptr) return false;

	// Get file page number of cached page and calculate offset in the file
	size_t offset = cachePagesInfo[cachePageIndex].filePageNo * DEFAULT_CACHE_PAGE_SIZE;
	size_t bytesToWrite = DEFAULT_CACHE_PAGE_SIZE;
	size_t bytesWritten = 0;
	void*  cachedPage = &cachePagesData[cachePageIndex];

	// Go to calculated offset in the file
	_fseeki64(fileHandler, offset, SEEK_SET);

	// Write cached page to file
	bytesWritten = fwrite(cachedPage, 1, bytesToWrite, fileHandler);

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
bool CachedFileIO::freeCachePage(size_t cachePageIndex) {

	CachePageInfo& pageInfo = cachePagesInfo[cachePageIndex];
	
	// if cache page has been rewritten persist page to storage device
	if (pageInfo.state == PageState::DIRTY) {
		if (!persistCachePage(cachePageIndex)) return false;
	}

	// Remove from index map
	cacheMap.erase(pageInfo.filePageNo);

	// Clear cache page info fields
	pageInfo.state = PageState::FREE;
	pageInfo.age = 0;
	pageInfo.filePageNo = PAGE_NOT_FOUND;
	pageInfo.availableDataLength = 0;

		
	// Cache page freed
	return true;
}



/**
* 
*  @brief Increments all cache pages "age"
* 
*/
void CachedFileIO::ageCachePages() {
	for (size_t cacheIndex = 0; cacheIndex < cachePagesCount; cacheIndex++) {
		cachePagesInfo[cacheIndex].age++;
	}
}
