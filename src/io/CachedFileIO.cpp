

#include "CachedFileIO.h"

#include <algorithm>

#include <iostream>

using namespace Boson;


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CachedFileIO::CachedFileIO() {
	this->fileHandler = nullptr;
	this->pagesCount = 0;
	this->pagesInfo = nullptr;
	this->pagesData = nullptr;
	this->readOnly = false;
}


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
CachedFileIO::~CachedFileIO() {
	this->close();
}


//-----------------------------------------------------------------------------
// Open file
//-----------------------------------------------------------------------------
bool CachedFileIO::open(char* dbName, size_t cacheSize, bool isReadOnly) {
	// if current file still open, close it
	if (this->fileHandler != nullptr) close();
	// try to open existing file for binary read/update (file must exist)
	this->fileHandler = fopen(dbName, "r+b");
	// if file does not exist or another problem
	if (this->fileHandler == nullptr) {
		// if can`t open file in read only mode return false
		if (isReadOnly) return false;
		// try to create new file for binary write/read
		this->fileHandler = fopen(dbName, "w+b");
		// if still can't create file return false
		if (this->fileHandler == nullptr) return false;
	}

	// set mode to no buffering, we will manage cache by our selves
	setvbuf(this->fileHandler, nullptr, _IONBF, 0);

	// Check minimal cache size
	if (cacheSize < MINIMAL_CACHE_SIZE) cacheSize = MINIMAL_CACHE_SIZE;

	// Allocate cache memory
	this->pagesCount = cacheSize / DEFAULT_CACHE_PAGE_SIZE;
	this->pagesInfo = new CachePageInfo[pagesCount];
	this->pagesData = new CachePageData[pagesCount];

	// Set cache memory to zero
	memset(this->pagesInfo, 0, pagesCount * sizeof(CachePageInfo));
	memset(this->pagesData, 0, pagesCount * sizeof(CachePageData));

	// Set readOnly flag
	this->readOnly = isReadOnly;

	// file successfuly opened
	return true;
}



//-----------------------------------------------------------------------------
// Close file
//-----------------------------------------------------------------------------
bool CachedFileIO::close() {
	
	// check if file was opened
	if (fileHandler == nullptr) return false;
	
	// flush buffers
	this->flush();
	
	// close file
	fclose(fileHandler);

	// Release cache memory
	delete[] pagesInfo;
	delete[] pagesData;

	// mark that file is closed
	this->fileHandler = nullptr;

	return true;
}



//-----------------------------------------------------------------------------
// Get file size
//-----------------------------------------------------------------------------
size_t CachedFileIO::getSize() {
	if (fileHandler == nullptr) return 0;
	size_t currentPosition = ftell(fileHandler);
	_fseeki64(fileHandler, 0, SEEK_END);
	size_t fileSize = ftell(fileHandler);
	_fseeki64(fileHandler, currentPosition, SEEK_SET);
	return fileSize;
}



//-----------------------------------------------------------------------------
// Reads requested amount of bytes to the data buffer
//-----------------------------------------------------------------------------	
size_t CachedFileIO::read(size_t position, void* dataBuffer, size_t length) {

	if (fileHandler == nullptr || length==0) return 0;

	size_t bytesRead = 0;

	// Calculate first page number and page offset
	size_t firstPageNo = position / DEFAULT_CACHE_PAGE_SIZE;  
	size_t firstPageOffset = position % DEFAULT_CACHE_PAGE_SIZE;

	// Calculate total amount of pages and remaining bytes to read
	size_t pagesToRead = (position + length) / DEFAULT_CACHE_PAGE_SIZE - firstPageNo + 1;
	size_t remainingBytes = (position + length) % DEFAULT_CACHE_PAGE_SIZE;
	if (remainingBytes > length) remainingBytes = length;
		

	size_t index = 0;
	size_t pageNo;
	size_t pageCounter = 0;
	size_t bytesToCopy = 0;
	uint8_t* dst;
	uint8_t* src;

	// Go through required pages of file
	while (pageCounter < pagesToRead) {
				
		// Check if this page is already loaded to the cache
		pageNo = firstPageNo + pageCounter;
		index = searchPageInCache(pageNo);

		// if not, load page from storage device to the cache
		if (index == PAGE_NOT_FOUND) {
			index = loadPageToCache(pageNo);
			if (index == PAGE_NOT_FOUND) return false;
		}

		// if this is last page we reading?
		bool notLastIteration = (pageCounter < pagesToRead - 1);

		// check remaining bytes to read from page in cache
		if (notLastIteration || remainingBytes == 0) {
			bytesToCopy = DEFAULT_CACHE_PAGE_SIZE;
		} else {
			bytesToCopy = remainingBytes;
		}
				
		// get amount of available data in the cache page
		size_t pageDataLength = pagesInfo[index].availableDataLength;

		// if it's a first page, take first page read offset in account
		if (pageNo == firstPageNo && bytesToCopy > (pageDataLength - firstPageOffset)) {
			bytesToCopy = pagesInfo[index].availableDataLength - firstPageOffset; 
		} 				

		// if there is data to copy then copy from cache page to users data buffer
		if (bytesToCopy > 0) {
			pagesInfo[index].age = 0;
			if (pageNo == firstPageNo) {
				src = (uint8_t*)&(pagesData[index].data[firstPageOffset]);
			} else {
				src = (uint8_t*)&(pagesData[index].data[0]);
			}
			
			//dst = ((uint8_t*) dataBuffer) + pageCounter * DEFAULT_CACHE_PAGE_SIZE;
			dst = ((uint8_t*)dataBuffer) + bytesRead;

			// error
			if (bytesToCopy > length) {
				std::cout << "Error: out of bounds error" << std::endl;
				return bytesRead;
			}

			if (bytesToCopy > pageDataLength) bytesToCopy = pageDataLength;

			memcpy(dst, src, bytesToCopy);
			bytesRead += bytesToCopy;
		}

		pageCounter++;
	}

	return bytesRead;
}



//-----------------------------------------------------------------------------
// Writes data to the cached file
//-----------------------------------------------------------------------------	
size_t CachedFileIO::write(size_t position, void* dataBuffer, size_t length) {

	// if file is not open or file is read only, then return
	if (fileHandler == nullptr || this->readOnly) return 0;

	size_t bytesWritten = 0;

	// Calculate first page number and page offset
	size_t firstPageNo = position / DEFAULT_CACHE_PAGE_SIZE;
	size_t firstPageOffset = position % DEFAULT_CACHE_PAGE_SIZE;

	// Calculate total amount of pages and remaining bytes to write
	size_t remainingBytes = (firstPageOffset + length) % DEFAULT_CACHE_PAGE_SIZE;
	size_t pagesToWrite = (firstPageOffset + length) / DEFAULT_CACHE_PAGE_SIZE + (remainingBytes > 0);   // FIXME: on page boundry it return 1 (but 2 expected)

	size_t index = 0;
	size_t pageNo;
	size_t pageCounter = 0;
	size_t bytesToCopy = 0;
	uint8_t* dst;
	uint8_t* src;

	// fetch-before-write
	while (pageCounter < pagesToWrite) {

		// Check if this page is loaded to the cache
		pageNo = firstPageNo + pageCounter;
		index = searchPageInCache(pageNo);

		// if not, load page from storage device to the cache
		if (index == PAGE_NOT_FOUND) {
			index = loadPageToCache(pageNo);
			if (index == PAGE_NOT_FOUND) return false;
		}

		// if this is last page we writing
		bool notLastIteration = (pageCounter < pagesToWrite - 1);

		// check remaining bytes to write to page in cache
		if (notLastIteration || remainingBytes == 0) {
			bytesToCopy = DEFAULT_CACHE_PAGE_SIZE;
		} else {
			bytesToCopy = remainingBytes;
		}
				
		// get amount of available data in the cache page

		size_t pageDataLength = pagesInfo[index].availableDataLength;

		// if it's a first page, take first page write offset in account
		if (pageNo == firstPageNo) {
			if (bytesToCopy > (DEFAULT_CACHE_PAGE_SIZE - firstPageOffset)) {
				bytesToCopy = DEFAULT_CACHE_PAGE_SIZE - firstPageOffset;
			}
		}

		// if there is data to copy then copy from users data buffer to cache page 
		if (bytesToCopy > 0) {
			
			if (pageNo == firstPageNo) {
				dst = (uint8_t*)&(pagesData[index].data[firstPageOffset]);
			} else {
				dst = (uint8_t*)&(pagesData[index].data[0]);
			}
			
			//src = ((uint8_t*)dataBuffer) + pageCounter * DEFAULT_CACHE_PAGE_SIZE;
			src = ((uint8_t*)dataBuffer) + bytesWritten;

			memcpy(dst, src, bytesToCopy);
			pagesInfo[index].age = 0;
			pagesInfo[index].state = PageState::DIRTY;
			pagesInfo[index].availableDataLength = std::max(pageDataLength, bytesToCopy);

			bytesWritten += bytesToCopy;
		}

		pageCounter++;
	}

	return 0;
}



//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------	
size_t CachedFileIO::append(void* dataBuffer, size_t length) {
	return 0;
}



//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------	
size_t CachedFileIO::flush() {
	size_t bytesWritten = 0;
	for (size_t pageNo = 0; pageNo < pagesCount; pageNo++) {
		if (pagesInfo[pageNo].state == PageState::DIRTY) {
			bytesWritten += persistCachePage(pageNo);
			// check logic, handle errors
		}
	}
	return bytesWritten;
}




//=============================================================================
// 
// Cache control methods
// 
//=============================================================================




//-----------------------------------------------------------------------------
// Returns cache index of requested file page or returns PAGE_NOT_FOUND
//-----------------------------------------------------------------------------	
size_t CachedFileIO::searchPageInCache(size_t pageNumber) {
	for (size_t pageNo = 0; pageNo < pagesCount; pageNo++) {
		if (pagesInfo[pageNo].state != PageState::FREE &&
			pagesInfo[pageNo].pageNumber == pageNumber) return pageNo;
	}
	return PAGE_NOT_FOUND;
}


//-----------------------------------------------------------------------------
// Returns free cache page or most aged page
//-----------------------------------------------------------------------------	
size_t CachedFileIO::getFreeCachePageIndex() {

	size_t largestAge = 0;
	size_t mostAgedPageIndex = 0;

	// Search for free cache page and most aged one
	for (size_t pageNo = 0; pageNo < pagesCount; pageNo++) {
		if (pagesInfo[pageNo].state == PageState::FREE) return pageNo;
		if (pagesInfo[pageNo].age > largestAge) {
			largestAge = pagesInfo[pageNo].age;
			mostAgedPageIndex = pageNo;
		}
	}

	// if there is no free pages in cache, free and return most aged page
	freeCachePage(mostAgedPageIndex);

	return mostAgedPageIndex;
}


/**
* 
*  Loads requested page from storage device to cache and returns index in the cache
* 
*  @param pageNumber file page number to load
*  @return loaded page cache index or PAGE_NOT_FOUND if file is not open.
* 
*/
size_t CachedFileIO::loadPageToCache(size_t pageNumber) {

	if (fileHandler == nullptr) return PAGE_NOT_FOUND;

	// increment all pages age
	ageCachePages();

	// get free cache page index
	size_t cacheIndex = getFreeCachePageIndex();

	// read page from storage device
	void* cachePage = &pagesData[cacheIndex];
	size_t offset = pageNumber * DEFAULT_CACHE_PAGE_SIZE;
	size_t bytesToRead = sizeof(CachePageData);
	_fseeki64(fileHandler, offset, SEEK_SET);
	size_t bytesRead = fread(cachePage, 1, bytesToRead, fileHandler);

	// fill loaded page description info
	CachePageInfo& loadedPage = pagesInfo[cacheIndex];
	loadedPage.state = PageState::CLEAN;
	loadedPage.pageNumber = pageNumber;
	loadedPage.age = 0;
	loadedPage.availableDataLength = bytesRead;

	return cacheIndex;
}


/**
* 
*  Writes specified by index cache page to the storage device
* 
*  @param cachePageIndex page index in the cache
*  @return true - page successfuly persisted, false - write failed or file is not open
* 
*/ 
bool CachedFileIO::persistCachePage(size_t cachePageIndex) {

	if (fileHandler == nullptr) return false;

	// Get file page number of cached page and calculate offset in the file
	size_t offset = pagesInfo[cachePageIndex].pageNumber * DEFAULT_CACHE_PAGE_SIZE;
	size_t bytesToWrite = sizeof(CachePageData);
	size_t bytesWritten = 0;
	void*  cachedPage = &pagesData[cachePageIndex];

	// Go to calculated offset in the file
	_fseeki64(fileHandler, offset, SEEK_SET);

	// Write cached page to file
	bytesWritten = fwrite(cachedPage, 1, bytesToWrite, fileHandler);

	// Check success
	return bytesWritten == bytesToWrite;
}


/**
* 
*  Clears page state and persists its data if page has been changed
* 
*  @param cachePageIndex index of the page in cache to clear
*  @return true - if page cleared, false - if can't persist page to storage
* 
*/
bool CachedFileIO::freeCachePage(size_t cachePageIndex) {
	
	// if cache page has been rewritten persist page to storage device
	if (pagesInfo[cachePageIndex].state == PageState::DIRTY) {
		if (!persistCachePage(cachePageIndex)) return false;
	}

	// Clear cache page info fields
	pagesInfo[cachePageIndex].state = PageState::FREE;
	pagesInfo[cachePageIndex].age = 0;
	pagesInfo[cachePageIndex].pageNumber = PAGE_NOT_FOUND;
	pagesInfo[cachePageIndex].availableDataLength = 0;

	// Cache page freed
	return true;
}


/**
* 
*   Increments all cache pages "age"
* 
*/
void CachedFileIO::ageCachePages() {
	for (size_t i = 0; i < pagesCount; i++) {
		pagesInfo[i].age++;
	}
}
