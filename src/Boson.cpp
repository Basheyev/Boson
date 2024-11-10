/******************************************************************************
*
*  Boson Database
*
*  Features:
*  - NoSQL database engine.
*  - Single database file.
*  - Standard Key/Value store.
*  - Document store (JSON).
*  - Support cursors for linear records traversal.
*  - Support for on-disk as well in-memory databases.
*  - Support Terabyte sized databases.
*
*  (C) Boson Database, Bolat Basheyev 2022-2024
*
******************************************************************************/

#include "api/BosonAPI.h"
#include "BalancedIndexTest.h"
#include "RecordFileIOTest.h"
#include "BosonAPITest.h"

#include <chrono>

using namespace Boson;
using namespace std;


void performacnceTest() {

	char* fileName = "D:/large.db";

	BosonAPI db;

	std::remove(fileName);

	if (!db.open(fileName)) return;

	char* msg = "{ \"name\":\"Bolat Basheyev\", \"birthDate\": \"1985.04.15\", "
		"\"city\":\"Astana\", \"mobile\": \"+7 777 777 77 77\", "
		"\"occupation\":\"software developer\", \"INN\": \"840415460108\", "
	    "\"about\": \"Investor, Entrepreneur, Developor\"}";
		
	int total = 1000000;
	
	std::cout << "Inserting " << total << " values (each " << strlen(msg) << " bytes)...";

	auto start = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < total; i++) {
		db.insert(msg);
	}
	
	// Запоминаем время окончания
	auto end = std::chrono::high_resolution_clock::now();

	// Вычисляем длительность в секундах
	std::chrono::duration<double> duration = end - start;

	
	std::cout << " in " << duration.count() << " sec " << std::endl;
	std::cout << "Cache Write Throughput: " << db.getWriteThroughput() << "Mb/s" << std::endl;

	// even number
	uint64_t ID = 621923; 
	std::cout << "Traverse 10 records starting from ID = " << ID << std::endl;

	start = std::chrono::high_resolution_clock::now();

	std::pair<uint64_t, std::shared_ptr<std::string>> pair;
	std::shared_ptr<std::string> value = db.get(ID);
	int traversal = 5;
	for (int i = 0; i < traversal; i++) {		
		std::cout << ID << " = " << *value << std::endl;
		pair = db.next();		
		ID = pair.first;
		value = pair.second;
		if (value == nullptr) break;
	}

	end = std::chrono::high_resolution_clock::now();
	duration = end - start;

	std::cout << std::endl;
	std::cout << traversal << " records found and data fetched in " << duration.count() << " sec" << std::endl;
	//std::cout << "Cache hits: " << db.getCacheHits() << "%" << std::endl;

	db.close();

}




int main()
{
	//BosonAPITest bapit("D:/data.bin");
	//bapit.run();

	performacnceTest();

	return 0;
}
