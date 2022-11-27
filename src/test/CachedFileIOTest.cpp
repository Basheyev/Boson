
#include <cstdio>
#include <iostream>
#include <chrono>


#include "CachedFileIOTest.h"


using namespace Boson;

CachedFileIOTest::CachedFileIOTest() {


}


CachedFileIOTest::~CachedFileIOTest() {
	close();
}


void CachedFileIOTest::open(char* filename, size_t cacheSize) {
	if (!cf.open(filename, cacheSize)) {
		std::cout << "Error reading file: '" << filename << ";" << std::endl;
	}
}


void CachedFileIOTest::sequencialReadTest(size_t bufferSize) {

	char* buffer = new char[bufferSize + 1];

	size_t fileSize = cf.getSize();

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

}


void CachedFileIOTest::randomReadTest(size_t position, size_t bufferSize) {
	char* buffer = new char[bufferSize + 1];

	size_t fileSize = cf.getSize();

	std::cout << "file size: " << fileSize << " bytes" << std::endl;

	size_t bytesRead = 0;
	size_t offset = 0;
	auto startTime = std::chrono::steady_clock::now();
	
	while (bytesRead < fileSize) {
		offset = (size_t)std::rand() * 128;
		bytesRead += cf.read(position + offset, buffer, bufferSize);
	}
	
	auto endTime = std::chrono::steady_clock::now();
	auto duration = (endTime - startTime).count() / 1000000.0;
	
	std::cout << "Cached file IO bytes read: " << bytesRead;
	std::cout << " (" << duration << " ms)" << std::endl;

	delete[] buffer;
	
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
	std::cout << "READING 1: '" << readBuf << "' (" << strlen(readBuf) << " bytes)" << std::endl;
	

	if (strcmp(writeBuf, readBuf) == 0) {
		std::cout << "Sequencial write on page boundry passed successfuly." << std::endl;
	} else {
		std::cout << "Sequencial write on page boundry FAILED!" << std::endl;
	}

	delete[] readBuf;
}


void CachedFileIOTest::randomWriteTest() {


}


void CachedFileIOTest::close() {
	cf.close();
}



void CachedFileIOTest::stdioSequencialRead(char* filename, size_t bufferSize) {

	char* bufferData = new char[bufferSize];
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
	fclose(f);
	delete[] bufferData;
	std::cout << "Standart IO bytes read: " << bytesRead;
	std::cout << " (" << (endTime - startTime).count() / 1000000.0 << " ms)" << std::endl;
}



void CachedFileIOTest::stdioRandomRead(char* filename, size_t position, size_t bufferSize) {

	char* bufferData = new char[bufferSize];
	FILE* f = fopen(filename, "r+b");

	size_t bytesRead = 0;
	auto startTime = std::chrono::steady_clock::now();

	_fseeki64(f, 0, SEEK_END);
	size_t fileSize = ftell(f);
	_fseeki64(f, 0, SEEK_SET);
	size_t offset = 0;

	while (bytesRead < fileSize) {
		offset = position + (size_t)std::rand() * 128;
		_fseeki64(f, offset, SEEK_SET);
		bytesRead += fread(bufferData, 1, bufferSize, f);
	}

	auto endTime = std::chrono::steady_clock::now();
	fclose(f);
	delete[] bufferData;
	std::cout << "Standart IO bytes read: " << bytesRead;
	std::cout << " (" << (endTime - startTime).count() / 1000000.0 << " ms)" << std::endl;
}