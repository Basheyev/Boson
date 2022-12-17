/******************************************************************************
*
*  CachedFileIO class tests implementation
*
*  (C) Bolat Basheyev 2022
*
******************************************************************************/

#include "CachedFileIOTest.h"


using namespace Boson;




CachedFileIOTest::CachedFileIOTest(char* path) {
	this->fileName = path;
	this->samplesCount = 1000000;
	this->docSize = 384;
	this->cacheRatio = 0.1;
	this->sigma = 0.045;
}


CachedFileIOTest::~CachedFileIOTest() {

}


/**
* 
*  @brief Runs CachedFileIO tests and compares throughput to STDIO
*  @return true if CachedFileIO to STDIO performance ratio > 1, false otherwise
*/
bool CachedFileIOTest::run(size_t samples, size_t jsonSize, double cacheRatio, double sigma) {
	// initalize parameters
	this->samplesCount = samples;
	this->docSize = jsonSize;
	this->cacheRatio = cacheRatio;
	this->sigma = sigma;
	
	// thousands separator
//	std::cout.imbue(std::locale(std::locale::classic(), new NumberPunctuation()));
		
	std::cout << "[PARAMETERS] CachedFileIO test:" << std::endl;
	std::cout << "\tSamples count = " << samples << std::endl;
	std::cout << "\tJSON size = " << jsonSize << " bytes" << std::endl;
	std::cout << "\tCache page = " << PAGE_SIZE << " bytes" << std::endl;
	std::cout << "\tCache size = " << cacheRatio * 100 << "% of database size" << std::endl;;
	std::cout << "\tDistribution Sigma = " << sigma * 100.0;
	std::cout << "% (93.3% of requests localized in " << sigma * 100.0 * 6 << "% of database)\n\n";

	//double cachedPageWriteThroughput = cachedRandomPageWrites();

	std::this_thread::sleep_for(std::chrono::seconds(1));

	double cachedWriteThroughput = cachedRandomWrites();

	std::this_thread::sleep_for(std::chrono::seconds(1));

	double cachedThroughput = cachedRandomReads();

	std::this_thread::sleep_for(std::chrono::seconds(1));
	
	double stdioThroughput = stdioRandomReads();

	std::this_thread::sleep_for(std::chrono::seconds(1));

	double cachedPageThroughput = cachedRandomPageReads();

	std::this_thread::sleep_for(std::chrono::seconds(1));

	double stdioPageThroughput = stdioRandomPageReads();

	std::this_thread::sleep_for(std::chrono::seconds(1));
		
	double ratio = cachedThroughput / stdioThroughput; 
	double pageRatio = cachedPageThroughput / stdioPageThroughput;

	std::cout << "[RESULT] Read throughput ratio in RANDOM OFFSET test (CACHED/STDIO): " << std::setprecision(4);
	if (ratio > 1.0) {
		std::cout << "+" << (ratio - 1.0) * 100 << "% - ";
		std::cout << "SUCCESS! :)\n";
	}
	else {
		std::cout << (ratio - 1.0) * 100 << "% - ";
		std::cout << "FAILED :(\n";
	}

	std::cout << "[RESULT] Read throughput ratio in PAGE ALIGNED test (CACHED/STDIO): " << std::setprecision(4);
	if (pageRatio > 1.0) {
		std::cout << "+" << (pageRatio - 1.0) * 100 << "% - ";
		std::cout << "SUCCESS! :)\n";
	}
	else {
		std::cout << (pageRatio - 1.0) * 100 << "% - ";
		std::cout << "FAILED :(\n";
	}

	return ratio > 1.0;
}


double CachedFileIOTest::cachedRandomPageWrites() {
	CachedFileIO cachedFile;
	const size_t dataLen = PAGE_SIZE;
	char buf[dataLen];
	memset(buf, 'W', dataLen);
	size_t pos = 0;

	// delete file if exists
	if (std::filesystem::exists(this->fileName)) {
		std::filesystem::remove(this->fileName);
	}

	cf.open(this->fileName, 1024 * 1024 * 16);
	std::cout << "[TEST]  Sequential PAGE ALIGNED write " << samplesCount / 8;
	std::cout << " of ~" << dataLen << " byte blocks...\n\t";

	for (int i = 0; i < samplesCount / 8; i++) {		
		cf.write(pos, buf, dataLen);
		pos += dataLen;
	}
	cf.flush();
	cf.close();

	double cachedDuration = cf.getStats(CacheStats::WRITE_TIME_NS) / 1000000.0;
	double throughput = cf.getStats(CacheStats::WRITE_THROUGHPUT);
	std::cout << pos << " bytes (" << cachedDuration << "ms), ";
	std::cout << "Write: " << throughput << " Mb/sec\n\n";

	return throughput;
}

/**
*
*  @brief Generates file with data
*  @return sequential write throughput in Mb/s
* 
*/
double CachedFileIOTest::cachedRandomWrites() {

	CachedFileIO cachedFile;
	
	char buf[256] = 
		"\n{\n\t\"name:\": \"unknown\",\n\t\"birthDate\": \"unknown\",\n\t"
		"\"GUID\" : \"6B29FC40-CA47-1067-B31D-00DD010662DA\",\n\t"
		"\"letters\": ['a','b','c','d','e','f','g'],\n\t\"id\": ";

	size_t textLen = strlen(buf);

	size_t length, pos = 0;

	// delete file if exists
	if (std::filesystem::exists(this->fileName)) {
		std::filesystem::remove(this->fileName);
	}
	
	cf.open(this->fileName, 1024 * 1024 * 16);

	std::cout << "[TEST]  Sequential write " << samplesCount;
	std::cout << " of ~" << textLen + 10 << " byte blocks...\n\t";
	
	for (int i = 0; i < samplesCount; i++) {
		_itoa_s(i, &buf[textLen], sizeof(buf)-textLen, 10);
		length = strlen(buf);
		buf[length] = '\n';
		buf[length + 1] = '}';
		buf[length + 2] = '\n';
		length += 3;
		cf.write(pos, buf, length);
		pos += length;
	}

	cf.close();

	double cachedDuration = cf.getStats(CacheStats::WRITE_TIME_NS) / 1000000.0;
	double throughput = cf.getStats(CacheStats::WRITE_THROUGHPUT);
	std::cout << pos << " bytes (" << cachedDuration << "ms), ";
	std::cout << "Write: " << throughput << " Mb/sec\n\n";

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
	if (!n2_cached) {
		double x, y, r;
		do {
			x = 2.0 * rand() / RAND_MAX - 1.0;
			y = 2.0 * rand() / RAND_MAX - 1.0;
			r = x * x + y * y;
		} while (r == 0.0 || r > 1.0);
		double d = sqrt(-2.0 * log(r) / r);
		double n1 = x * d;
		n2 = y * d;
		double result = n1 * stddev + mean;
		n2_cached = 1;
		return result;
	}
	else {
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

	char *buf = new char[PAGE_SIZE * 4];
	size_t length, bytesRead = 0;
	size_t offset;


	cf.open(this->fileName);
	size_t fileSize = cf.getFileSize();
	cf.setCacheSize(size_t(fileSize * cacheRatio));
		
	std::cout << "[TEST]  CACHED random read " << samplesCount;
	std::cout << " of " << docSize << " byte blocks...\n\t";
	
	length = docSize;

	for (size_t i = 0; i < samplesCount; i++) {
		// generate random
		offset = size_t(randNormal(0.5, this->sigma) * double(fileSize - length));
		// offset always positive because its size_t
		if (offset < fileSize) {
			cf.read(offset, buf, length);
			buf[length+1] = 0;
			bytesRead += length;
		}

	}

	double readTime = (cf.getStats(CacheStats::READ_TIME_NS) / 1000000.0);
	double throughput = cf.getStats(CacheStats::READ_THROUGHPUT);
	std::cout << bytesRead << " bytes (" << readTime << "ms), ";
	std::cout << "Read: " << throughput << " Mb/sec, \n\t";
	std::cout << "Cache Hit: " << cf.getStats(CacheStats::CACHE_HITS_RATE) << "%\n\n";

	cf.close();

	delete[] buf;

	return throughput;
}



/**
*
*  @brief Random reads using STDIO
*  @return random read throughput in Mb/s
*/
double CachedFileIOTest::stdioRandomReads() {

	FILE* file = nullptr;

	char* buf = new char[PAGE_SIZE * 4];
	size_t length, pos = 0;
	size_t offset;

	errno_t result = fopen_s(&file, this->fileName, "r+b");
	if (result != 0 || file == nullptr) return -1;

	size_t fileSize = std::filesystem::file_size(this->fileName);

	std::cout << "[TEST]  STDIO random read " << samplesCount << " of " << docSize << " byte blocks...\n\t";
		
	length = docSize;

	std::chrono::steady_clock::time_point startTime, endTime;
	size_t stdioDuration = 0;

	for (size_t i = 0; i < samplesCount; i++) {

		offset = (size_t) (randNormal(0.5, sigma) * double(fileSize - length));

		// offset always positive because its size_t
		if (offset < fileSize) {
			startTime = std::chrono::steady_clock::now();
			_fseeki64(file, offset, SEEK_SET);
			fread(buf, 1, length, file);
			endTime = std::chrono::steady_clock::now();
			stdioDuration += (endTime - startTime).count();
			buf[length + 1] = 0;
			pos += length;
		}

	}

	startTime = std::chrono::steady_clock::now();
	fflush(file);
	endTime = std::chrono::steady_clock::now();
	stdioDuration += (endTime - startTime).count();

	double throughput = (pos / 1024.0 / 1024.0) / (stdioDuration / 1000000000.0);

	std::cout << pos << " bytes (" << stdioDuration / 1000000.0 << "ms), ";
	std::cout << "Read: " << throughput << " Mb/sec\n\n";

	fclose(file);

	delete[] buf;

	return throughput;
}



/**
*
*  @brief Random page reads using cache as 10% size of file
*  @return random read throughput in Mb/s
*
*/
double CachedFileIOTest::cachedRandomPageReads() {

	CachedFileIO cachedFile;

	char* buf = new char[PAGE_SIZE];
	size_t bytesRead = 0;
	size_t pageNo;

	cf.open(this->fileName);
	size_t fileSize = cf.getFileSize();
	size_t maxPages = fileSize / PAGE_SIZE;
	cf.setCacheSize(size_t(fileSize * cacheRatio));

	std::cout << "[TEST]  CACHED random PAGE ALIGNED read " << samplesCount;
	std::cout << " of " << PAGE_SIZE << " byte blocks...\n\t";

	for (size_t i = 0; i < samplesCount; i++) {
		// generate random page number
		pageNo = size_t(randNormal(0.5, this->sigma) * double(maxPages));
		// offset always positive because its size_t
		if (pageNo < maxPages) {
			bytesRead += cf.readPage(pageNo, buf);
		}

	}
	double readTime = (cf.getStats(CacheStats::READ_TIME_NS) / 1000000.0);
	double throughput = cf.getStats(CacheStats::READ_THROUGHPUT);
	std::cout << bytesRead << " bytes (" << readTime << "ms), ";
	std::cout << "Read: " << throughput << " Mb/sec, \n\t";
	std::cout << "Cache Hit: " << cf.getStats(CacheStats::CACHE_HITS_RATE) << "%\n\n";

	cf.close();

	delete[] buf;

	return throughput;
}


/**
*
*  @brief Random page reads using STDIO
*  @return random page read throughput in Mb/s
*/
double CachedFileIOTest::stdioRandomPageReads() {

	FILE* file = nullptr;

	errno_t result = fopen_s(&file, this->fileName, "r+b");
	if (result != 0 || file == nullptr) return -1;
	
	std::cout << "[TEST]  STDIO random PAGE ALIGNED read " << samplesCount;
	std::cout << " of " << PAGE_SIZE << " byte blocks...\n\t";

	char* buf = new char[PAGE_SIZE];
	size_t fileSize = std::filesystem::file_size(this->fileName);
	size_t maxPages = fileSize / PAGE_SIZE;
	size_t bytesRead = 0;
	size_t pageNo = 0;

	std::chrono::steady_clock::time_point startTime, endTime;
	size_t stdioDuration = 0;

	for (size_t i = 0; i < samplesCount; i++) {

		pageNo = size_t(randNormal(0.5, this->sigma) * double(maxPages));

		// offset always positive because its size_t
		if (pageNo < maxPages) {
			startTime = std::chrono::steady_clock::now();
			_fseeki64(file, pageNo * PAGE_SIZE, SEEK_SET);
			bytesRead += fread(buf, 1, PAGE_SIZE, file);
			endTime = std::chrono::steady_clock::now();
			stdioDuration += (endTime - startTime).count();
			
		}

	}

	startTime = std::chrono::steady_clock::now();
	fflush(file);
	endTime = std::chrono::steady_clock::now();
	stdioDuration += (endTime - startTime).count();


	double throughput = (bytesRead / 1024.0 / 1024.0) / (stdioDuration / 1000000000.0);

	std::cout << bytesRead << " bytes (" << stdioDuration / 1000000.0 << "ms), ";
	std::cout << "Read: " << throughput << " Mb/sec\n\n";

	fclose(file);

	delete[] buf;

	return throughput;
}
