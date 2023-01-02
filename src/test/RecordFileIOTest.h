#pragma once



class RecordFileIOTest {
public:
	bool generateData(char* filename, size_t recordCount);
	bool readAscending(char* filename);
	bool readDescending(char* filename);
	bool removeOddRecords(char* filename);
	bool insertNewRecords(char* filename, size_t recordCount);
	void run(char* filename);
private:
	
};