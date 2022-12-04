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

}


/**
* 
*  @brief Runs CachedFileIO tests and compares throughput to STDIO
*  @return true if CachedFileIO to STDIO performance ratio > 1, false otherwise
*/
bool CachedFileIOTest::run() {
	generateFileData();
	double cachedThroughput = cachedRandomReads(); 
	double stdioThroughput = stdioRandomReads();
	double ratio = cachedThroughput / stdioThroughput; // more is better
	std::cout << "Throughput ratio (STDIO/CACHED):" << std::setprecision(2) << ratio << "x - ";
	if (ratio > 1.0) std::cout << "SUCCESS\n"; else std::cout << "FAILED\n";
	return ratio > 1.0;
}


/**
*
*  @brief Generates file with data
*  @return sequential write throughput in Mb/s
* 
*/
double CachedFileIOTest::generateFileData() {

	CachedFileIO cachedFile;
	
	char buf[128] = "\n{\n\t\"name:\": \"unknown\",\n\t\"birthDate\": \"unknown\",\n\t"
		            "\"letters\": ['a','b','c','d','e','f','g'],\n\t\"id\": ";
	size_t textLen = strlen(buf);

	size_t length, pos = 0;
	cf.open(this->fileName);
	cf.resizeFile(0);
	std::cout << "[TEST] Sequential write 1000000 of " << textLen + 12 << " byte blocks... ";
	auto startTime = std::chrono::steady_clock::now();
	
	for (size_t i = 0; i < 1000000; i++) {
		_itoa(i, &buf[textLen], 10);
		length = strlen(buf);
		buf[length] = '\n';
		buf[length + 1] = '}';
		buf[length + 2] = 0;
		length += 3;
		cf.write(pos, buf, length);
		pos += length;
	}
	cf.flush();
	auto endTime = std::chrono::steady_clock::now();
	auto cachedDuration = (endTime - startTime).count() / 1000000.0;
	
	cf.close();

	double throughput = (pos / 1024.0 / 1024.0) / (cachedDuration / 1000.0);
	
	std::cout << pos << " bytes (" << cachedDuration << "ms), ";
	std::cout << "Write: " << throughput << " Mb/sec\n";

	return throughput;
}


/**
*
*  @brief  Box Muller Method
*  @return normal distributed random number
*
*/
double CachedFileIOTest::randNormal(double mean, double stddev)
{
	static double n2 = 0.0;
	static int n2_cached = 0;
	if (!n2_cached)
	{
		double x, y, r;
		do
		{
			x = 2.0 * rand() / RAND_MAX - 1.0;
			y = 2.0 * rand() / RAND_MAX - 1.0;

			r = x * x + y * y;
		} while (r == 0.0 || r > 1.0);

		{
			double d = sqrt(-2.0 * log(r) / r);
			double n1 = x * d;
			n2 = y * d;
			double result = n1 * stddev + mean;
			n2_cached = 1;
			return result;
		}
	}
	else
	{
		n2_cached = 0;
		return n2 * stddev + mean;
	}
}



/**
* 
*  @brief Random reads using cache as 10% size of file
*  @return random read throughput in Mb/s
* 
*/
double CachedFileIOTest::cachedRandomReads() {

	CachedFileIO cachedFile;

	char buf[2048] = { 0 };	
	size_t length, bytesRead = 0;
	size_t offset;


	size_t fileSize = std::filesystem::file_size(this->fileName);

	cf.open(this->fileName, fileSize / 10);
		
	std::cout << "[TEST] Random read 1000000 of 760 byte blocks... ";

	auto startTime = std::chrono::steady_clock::now();
	
	length = 760;

	for (size_t i = 0; i < 1000000; i++) {

		offset = randNormal(0.5, 0.15) * (fileSize - length);

		if (offset >= 0 && offset < fileSize) {
			
			cf.read(offset, buf, length);
			buf[length+1] = 0;
			bytesRead += length;
		}

	}
	
	auto endTime = std::chrono::steady_clock::now();
	auto cachedDuration = (endTime - startTime).count() / 1000000.0;

	double throughput = (bytesRead / 1024.0 / 1024.0) / (cachedDuration / 1000.0);

	std::cout << bytesRead << " bytes (" << cachedDuration << "ms), ";
	std::cout << "Cache hit: " << cf.cacheHitRate() << "%, ";
	std::cout << "Read: " << throughput << " Mb/sec\n";

	cf.close();

	return throughput;
}



/**
*
*  @brief Random reads using STDIO
*  @return random read throughput in Mb/s
*/
double CachedFileIOTest::stdioRandomReads() {

	FILE* file;

	char buf[2048] = { 0 };
	size_t length, pos = 0;
	size_t offset;

	file = fopen(this->fileName, "r+b");
	size_t fileSize = std::filesystem::file_size(this->fileName);

	std::cout << "[TEST] STDIO random read 1000000 of 760 byte blocks... ";

	auto startTime = std::chrono::steady_clock::now();

	length = 760;

	for (size_t i = 0; i < 1000000; i++) {

		offset = randNormal(0.5, 0.15) * (fileSize - length);

		if (offset >= 0 && offset < fileSize) {
			fseek(file, offset, SEEK_SET);
			fread(buf, 1, length, file);
			buf[length + 1] = 0;
			pos += length;
		}

	}

	auto endTime = std::chrono::steady_clock::now();
	auto cachedDuration = (endTime - startTime).count() / 1000000.0;

	double throughput = (pos / 1024.0 / 1024.0) / (cachedDuration / 1000.0);

	std::cout << pos << " bytes (" << cachedDuration << "ms), ";
	std::cout << "Read: " << throughput << " Mb/sec\n";

	fclose(file);

	return throughput;
}


