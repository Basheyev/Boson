#pragma once



class RecordFileIOTest {
public:
	bool generateData(const char* filename, size_t recordCount);
	bool readAscending(const char* filename);
	bool readDescending(const char* filename);
	bool removeEvenRecords(const char* filename);
	bool insertNewRecords(const char* filename, size_t recordCount);
	void run(const char* filename);
private:
	
};