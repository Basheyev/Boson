#pragma once



class RecordFileIOTest {
public:
	bool generateData(const char* filename, size_t recordCount);
	bool readAscending(const char* filename, bool verbose);
	bool readDescending(const char* filename, bool verbose);
	bool removeEvenRecords(const char* filename, bool verbose);
	bool insertNewRecords(const char* filename, size_t recordCount);
	void run(const char* filename);
	void runLoadTest(const char* filename, size_t amount);
private:
	
};