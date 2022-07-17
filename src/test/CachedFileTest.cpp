
#include <cstdio>
#include <iostream>
#include <chrono>


#include "CachedFileTest.h"


using namespace Boson;

CachedFileTest::CachedFileTest() {


}


CachedFileTest::~CachedFileTest() {
	close();
}


void CachedFileTest::open(char* filename, size_t cacheSize) {
	if (!cf.open(filename, cacheSize)) {
		std::cout << "error reading file: " << filename << std::endl;
	}
}


void CachedFileTest::sequencialReadTest(size_t bufferSize) {

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
	std::cout << " (" << (endTime - startTime).count() / 1000000.0 << " ms)" << std::endl;

	delete[] buffer;

}


void CachedFileTest::randomReadTest(size_t position, size_t bufferSize) {
	char* buffer = new char[bufferSize + 1];

	size_t fileSize = cf.getSize();

	std::cout << "file size: " << fileSize << " bytes" << std::endl;

	size_t bytesRead = 0;
	auto startTime = std::chrono::steady_clock::now();
	bytesRead = cf.read(position, buffer, bufferSize);
	buffer[bytesRead - 1] = 0;
	std::cout << buffer;

	auto endTime = std::chrono::steady_clock::now();
	std::cout << std::endl;

	delete[] buffer;
	
}


void CachedFileTest::sequencialWriteTest() {


}


void CachedFileTest::randomWriteTest() {


}


void CachedFileTest::close() {
	cf.close();
}



void CachedFileTest::stdioSequencialRead(char* filename, size_t bufferSize) {

	char* bufferData = new char[bufferSize];
	FILE* f = fopen(filename, "r+b");

	size_t bytesRead = 0;
	auto startTime = std::chrono::steady_clock::now();

	fseek(f, 0, SEEK_END);
	size_t fileSize = ftell(f);
	fseek(f, 0, SEEK_SET);

	while (bytesRead < fileSize) {
		fseek(f, bytesRead, SEEK_SET);
		bytesRead += fread(bufferData, 1, bufferSize, f);
	}

	auto endTime = std::chrono::steady_clock::now();
	fclose(f);
	delete[] bufferData;
	std::cout << "Standart IO bytes read: " << bytesRead;
	std::cout << " (" << (endTime - startTime).count() / 1000000.0 << " ms)" << std::endl;
}