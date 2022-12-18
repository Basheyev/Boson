/*
* ==========================================================================
* Boson Database
* ==========================================================================
* - NoSQL database engine.
* - Single database file.
* - Standard Key/Value store.
* - Document store (JSON).
* - Support cursors for linear records traversal.
* - Support for on-disk as well in-memory databases.
* - Support Terabyte sized databases.
* ==========================================================================
* (C) Bolat Basheyev 2022
*/

#include "Boson.h"

#include "CachedFileIO.h"
#include "CachedFileIOTest.h"

#include "StorageIO.h"

#include <iostream>

using namespace Boson;




uint64_t generateID() {
	auto currentTime = std::chrono::steady_clock::now().time_since_epoch();
	uint64_t timeSinceEpoch = currentTime.count();  // 48-bit steady clock
	uint64_t randomNumber = std::rand();            // 16-bit random value
	uint64_t almostUniqueID = (timeSinceEpoch << 16) | randomNumber;
	return almostUniqueID;
}




int main()
{
	using namespace std;
	/*
	CachedFileIO cf;

	cf.open("F:/sign.txt");
	cf.write(0, &Boson::BOSONDB_SIGNATURE, sizeof(uint64_t));
	cf.close();
	
	cout << sizeof Boson::StorageHeader << endl;
	*/

	cout << setbase(16) << endl;
	for (int i = 0; i < 1000; i++) {
		cout << generateID() << "\n";
	}
	cout << endl;

	return 0;
}
