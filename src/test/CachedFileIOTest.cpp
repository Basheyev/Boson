/******************************************************************************
*
*  CachedFileIO class tests implementation
*
*  (C) Bolat Basheyev 2022
*
******************************************************************************/
#include <cstdio>
#include <iostream>
#include <chrono>


#include "CachedFileIOTest.h"


using namespace Boson;

CachedFileIOTest::CachedFileIOTest(char* path) {
	this->fileName = path;
}


CachedFileIOTest::~CachedFileIOTest() {
	close();
}





/**
*
*  @brief Generates file with data
* 
*  CachedFileIO uses Least-Recent-Used and Fetch-before-Write
*
*/
void CachedFileIOTest::generateFileData() {
	CachedFileIO cachedFile;
	char buf[32] = { 0 };
	size_t length, pos = 0;
	cf.open(this->fileName);
	cf.resizeFile(0);
	std::cout << "[TEST] Sequential write... ";
	auto startTime = std::chrono::steady_clock::now();
	for (size_t i = 0; i < 10000000; i++) {
		_itoa(i, buf, 10);
		length = strlen(buf);
		buf[length] = ' ';
		buf[length + 1] = 0;
		cf.write(pos, buf, length + 1);
		pos += length + 1;
	}
	cf.flush();
	auto endTime = std::chrono::steady_clock::now();
	auto cachedDuration = (endTime - startTime).count() / 1000000.0;
	
	cf.close();
	char data[4096] = { 0 };

	startTime = std::chrono::steady_clock::now();
	FILE* f = fopen(this->fileName, "r+b");
	for (size_t i = 10000000; i < 20000000; i++) {
		_itoa(i, buf, 10);
		length = strlen(buf);
		buf[length] = ' ';
		buf[length + 1] = 0;
		_fseeki64(f, pos, SEEK_SET);
		fread(data, 1, 4096, f);
		_fseeki64(f, pos, SEEK_SET);
		fwrite(buf, 1, length + 1, f);
		pos += length + 1;
	}
	fclose(f);
	endTime = std::chrono::steady_clock::now();
	auto stdioDuration = (endTime - startTime).count() / 1000000.0;
	auto ratio = (stdioDuration / cachedDuration);

	std::cout << pos << " bytes (CACHED: " << cachedDuration << "ms, ";
	std::cout << "STDIO: " << stdioDuration << "ms, ratio=" << ratio << ") - ";
	if (ratio > 1) std::cout << "PASSED\n"; else std::cout << "FAILED\n";
}





void CachedFileIOTest::open(char* filename, size_t cacheSize) {
	if (!cf.open(filename, cacheSize)) {
		std::cout << "Error reading file: '" << filename << ";" << std::endl;
	}
}


double CachedFileIOTest::sequencialReadTest(size_t bufferSize) {

	char* buffer = new char[bufferSize + 1];

	size_t fileSize = cf.getFileSize();

	std::cout << "file size: " << fileSize << " bytes" << std::endl;

	size_t totalBytesRead = 0;
	size_t bytesRead = 0;
	auto startTime = std::chrono::steady_clock::now();
	while (totalBytesRead < fileSize) {
		bytesRead = cf.read(totalBytesRead, buffer, bufferSize);
		totalBytesRead += bytesRead;
	}
	auto endTime = std::chrono::steady_clock::now();
	std::cout << std::endl;
	std::cout << "Cached file bytes read: " << totalBytesRead;
	auto duration = (endTime - startTime).count() / 1000000.0;

	std::cout << " (" << duration << " ms)" << std::endl;

	delete[] buffer;

	return duration;
}


double CachedFileIOTest::randomReadTest(size_t position, size_t bufferSize) {
	char* buffer = new char[bufferSize + 1];

	size_t fileSize = cf.getFileSize();

	size_t bytesRead = 0;
	size_t offset = 0;
	auto startTime = std::chrono::steady_clock::now();
	
	while (bytesRead < fileSize) {
		offset = (size_t) std::rand() * 2048;
		bytesRead += cf.read(position + offset, buffer, bufferSize);
	}
	
	auto endTime = std::chrono::steady_clock::now();
	auto duration = (endTime - startTime).count() / 1000000.0;
	
	std::cout << "Cached file IO bytes read: " << bytesRead;
	std::cout << " (" << duration << " ms) - with " << cf.cacheMissRate() << "% cache misses" << std::endl;

	delete[] buffer;
	return duration;
}


void CachedFileIOTest::sequencialWriteTest(size_t bufferSize) {
	char* writeBuf = "This text is written on the boundary of pages and supposed test algorithm";
	size_t length = strlen(writeBuf);
	char *readBuf = new char[length + 1];

	std::cout << "WRITING: '" << writeBuf << "' (" << length << " bytes)" << std::endl;
	cf.write(DEFAULT_CACHE_PAGE_SIZE - length / 2, writeBuf, length);
	cf.flush();


	cf.read(DEFAULT_CACHE_PAGE_SIZE - length / 2, readBuf, length);
	readBuf[length] = 0;
	std::cout << "READING: '" << readBuf << "' (" << strlen(readBuf) << " bytes)" << std::endl;
	

	if (strcmp(writeBuf, readBuf) == 0) {
		std::cout << "Sequencial write on page boundry passed successfuly." << std::endl;
	} else {
		std::cout << "Sequencial write on page boundry FAILED!" << std::endl;
	}

	delete[] readBuf;

}


void CachedFileIOTest::randomWriteTest() {

	char buf[32] = { 0 };
	size_t length, pos = 0;

	auto startTime = std::chrono::steady_clock::now();

	for (size_t i = 0; i < 1000072; i++) {
		_itoa(i, buf, 10);
		length = strlen(buf);
		buf[length] = ' ';
		buf[length + 1] = 0;
		cf.write(pos, buf, length + 1);
		pos += length + 1;
	}

	cf.flush();

	auto endTime = std::chrono::steady_clock::now();
	auto duration = (endTime - startTime).count() / 1000000.0;

	std::cout << "Cached file IO bytes written: " << pos;
	std::cout << " (" << duration << " ms)" << std::endl;

	std::cout << "File size: " << cf.getFileSize() << " bytes" << std::endl;

	std::cout << "Truncate" << std::endl;
	cf.resizeFile(1024 * 1024); // result 6 889 472

	std::cout << "File size: " << cf.getFileSize() << " bytes" << std::endl;

}


void CachedFileIOTest::close() {
	cf.close();
}



double CachedFileIOTest::stdioSequencialRead(char* filename, size_t bufferSize) {

	char* bufferData = new char[bufferSize + 1];
	FILE* f = fopen(filename, "r+b");

	size_t bytesRead = 0;
	auto startTime = std::chrono::steady_clock::now();

	_fseeki64(f, 0, SEEK_END);
	size_t fileSize = ftell(f);
	_fseeki64(f, 0, SEEK_SET);

	while (bytesRead < fileSize) {
		_fseeki64(f, bytesRead, SEEK_SET);
		bytesRead += fread(bufferData, 1, bufferSize, f);
	}

	auto endTime = std::chrono::steady_clock::now();
	auto duration = (endTime - startTime).count() / 1000000.0;
	fclose(f);
	delete[] bufferData;
	std::cout << "STDIO bytes read: " << bytesRead;
	std::cout << " (" << duration << " ms)" << std::endl;
	return duration;
}



double CachedFileIOTest::stdioRandomRead(char* filename, size_t position, size_t bufferSize) {

	char* bufferData = new char[bufferSize + 1];
	FILE* f = fopen(filename, "r+b");

	size_t bytesRead = 0;
	auto startTime = std::chrono::steady_clock::now();

	_fseeki64(f, 0, SEEK_END);
	size_t fileSize = ftell(f);
	_fseeki64(f, 0, SEEK_SET);
	size_t offset = 0;

	while (bytesRead < fileSize) {
		offset = position + (size_t)std::rand() * 2048;
		_fseeki64(f, offset, SEEK_SET);
		bytesRead += fread(bufferData, 1, bufferSize, f);
	}

	auto endTime = std::chrono::steady_clock::now();
	auto duration = (endTime - startTime).count() / 1000000.0;
	fclose(f);
	delete[] bufferData;
	std::cout << "STDIO bytes read: " << bytesRead;
	std::cout << " (" << duration << " ms)" << std::endl;
	return duration;
}

