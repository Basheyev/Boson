
#include "CachedFile.h"


using namespace Boson;


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CachedFile::CachedFile() {
	handler = nullptr;
	this->pagesCount = 0;
	this->pagesInfo = nullptr;
	this->pagesData = nullptr;
	this->readOnly = false;
}


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
CachedFile::~CachedFile() {
	this->close();
}


//-----------------------------------------------------------------------------
// Open file
//-----------------------------------------------------------------------------
bool CachedFile::open(char* dbName, size_t cacheSize, bool isReadOnly) {
	// if current file still open, close it
	if (this->handler != nullptr) close();
	// try to open existing file for binary read/update (file must exist)
	this->handler = fopen(dbName, "r+b");
	// if file does not exist or another problem
	if (this->handler == nullptr) {
		// if can`t open file in read only mode return false
		if (isReadOnly) return false;
		// try to create new file for binary write/read
		this->handler = fopen(dbName, "w+b");
		// if still can't create file return false
		if (this->handler == nullptr) return false;
	}

	// set mode to no buffering, we will manage cache by our selves
	setvbuf(this->handler, nullptr, _IONBF, 0);

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
bool CachedFile::close() {
	
	// check if file was opened
	if (handler == nullptr) return false;
	
	// flush buffers
	this->flush();
	
	// close file
	fclose(handler);

	// Release cache memory
	delete[] pagesInfo;
	delete[] pagesData;

	// mark that file is closed
	this->handler = nullptr;

	return true;
}



//-----------------------------------------------------------------------------
// Get file size
//-----------------------------------------------------------------------------
size_t CachedFile::getSize() {
	if (handler == nullptr) return 0;
	size_t currentPosition = ftell(handler);
	fseek(handler, 0, SEEK_END);
	size_t fileSize = ftell(handler);
	fseek(handler, currentPosition, SEEK_SET);
	return fileSize;
}



//-----------------------------------------------------------------------------
// Reads requested amount of bytes to the data buffer
//-----------------------------------------------------------------------------	
size_t CachedFile::read(size_t position, void* dataBuffer, size_t length) {

	if (handler == nullptr) return 0;

	size_t bytesRead = 0;

	// Calculate first page number and page offset
	size_t firstPageNo = position / DEFAULT_CACHE_PAGE_SIZE;  
	size_t firstPageOffset = position % DEFAULT_CACHE_PAGE_SIZE;

	// Calculate total amount of pages and remaining bytes to read
	size_t remainingBytes = length % DEFAULT_CACHE_PAGE_SIZE;
	size_t pagesToRead = length / DEFAULT_CACHE_PAGE_SIZE + (remainingBytes > 0);


	size_t index = 0;
	size_t pageNo;
	size_t pageCounter = 0;
	size_t bytesToCopy = 0;
	char* dst;
	char* src;

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
				src = (char*)&(pagesData[index].data[firstPageOffset]);
			} else {
				src = (char*)&(pagesData[index].data[0]);
			}
			dst = ((char*) dataBuffer) + pageCounter * DEFAULT_CACHE_PAGE_SIZE;
			memcpy(dst, src, bytesToCopy);
			bytesRead += bytesToCopy;
		}

		pageCounter++;
	}

	return bytesRead;
}



//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------	
size_t CachedFile::write(size_t position, void* dataBuffer, size_t length) {

	// if file is not open or file is read only, then return
	if (handler == nullptr || this->readOnly) return 0;

	size_t bytesWritten = 0;

	// Calculate first page number and page offset
	size_t firstPageNo = position / DEFAULT_CACHE_PAGE_SIZE;
	size_t firstPageOffset = position % DEFAULT_CACHE_PAGE_SIZE;

	// Calculate total amount of pages and remaining bytes to write
	size_t remainingBytes = length % DEFAULT_CACHE_PAGE_SIZE;
	size_t pagesToWrite = length / DEFAULT_CACHE_PAGE_SIZE + (remainingBytes > 0);

	size_t index = 0;
	size_t pageNo;
	size_t pageCounter = 0;

	// todo fetch-before-write
	while (pageCounter < pagesToWrite) {


		pageCounter++;
	}

	return 0;
}



//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------	
size_t CachedFile::append(void* dataBuffer, size_t length) {
	return 0;
}



//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------	
size_t CachedFile::flush() {
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
size_t CachedFile::searchPageInCache(size_t pageNumber) {
	for (size_t pageNo = 0; pageNo < pagesCount; pageNo++) {
		if (pagesInfo[pageNo].state != PageState::FREE &&
			pagesInfo[pageNo].pageNumber == pageNumber) return pageNo;
	}
	return PAGE_NOT_FOUND;
}


//-----------------------------------------------------------------------------
// Returns free cache page or most aged page
//-----------------------------------------------------------------------------	
size_t CachedFile::getFreeCachePageIndex() {

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


//-----------------------------------------------------------------------------
// Loads requested page from storage device to cache and returns index in cache 
//-----------------------------------------------------------------------------	
size_t CachedFile::loadPageToCache(size_t pageNumber) {

	if (handler == nullptr) return PAGE_NOT_FOUND;

	// increment all pages age
	ageCachePages();

	// get free cache page index
	size_t cacheIndex = getFreeCachePageIndex();

	// read page from storage device
	fseek(handler, pageNumber * DEFAULT_CACHE_PAGE_SIZE, SEEK_SET);
	size_t bytesRead = fread(&pagesData[cacheIndex], 1, sizeof(CachePageData), handler);

	// fill loaded page info
	CachePageInfo& loadedPage = pagesInfo[cacheIndex];
	loadedPage.state = PageState::CLEAN;
	loadedPage.pageNumber = pageNumber;
	loadedPage.age = 0;
	loadedPage.availableDataLength = bytesRead;

	return cacheIndex;
}


//-----------------------------------------------------------------------------
// Writes specified cache page to the storage device
//-----------------------------------------------------------------------------	
size_t CachedFile::persistCachePage(size_t cachePageIndex) {
	if (handler == nullptr) return 0;
	fseek(handler, cachePageIndex * DEFAULT_CACHE_PAGE_SIZE, SEEK_SET);
	return fwrite(&pagesData[cachePageIndex], 1, sizeof(CachePageData), handler);
}


//-----------------------------------------------------------------------------
// Increments all pages age 
//-----------------------------------------------------------------------------	
size_t CachedFile::ageCachePages() {
	for (size_t i = 0; i < pagesCount; i++) {
		pagesInfo[i].age++;
	}
	return 0;
}


//-----------------------------------------------------------------------------
// Clears state of cache page and persists its data if page has been changed
//-----------------------------------------------------------------------------	
size_t CachedFile::freeCachePage(size_t cachePageIndex) {
	
	size_t bytesFlushed = 0;

	// if page is changed persist page to storage device
	if (pagesInfo[cachePageIndex].state == PageState::DIRTY) {
		bytesFlushed = persistCachePage(cachePageIndex);
	}

	// Clear page info fields
	pagesInfo[cachePageIndex].state = PageState::FREE;
	pagesInfo[cachePageIndex].age = 0;
	pagesInfo[cachePageIndex].pageNumber = PAGE_NOT_FOUND;
	pagesInfo[cachePageIndex].availableDataLength = 0;

	return bytesFlushed;
}