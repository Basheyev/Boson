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

#include "RecordFileIO.h"
#include "RecordFileIOTest.h"

#include "BalancedTreeIndex.h"
#include "BalancedTreeTest.h"

#include <string>

using namespace Boson;
using namespace std;


int main()
{
//	BalancedTreeTest bpt;
//	bpt.run();

	RecordFileIOTest rst;
	//rst.run("F:/records.bin");
	std::filesystem::remove("F:/largeFile.bin");
	rst.generateData("F:/largeFile.bin", 10000);
	rst.readAscending("F:/largeFile.bin");
	rst.removeEvenRecords("F:/largeFile.bin");
	rst.insertNewRecords("F:/largeFile.bin", 5000);
	rst.readAscending("F:/largeFile.bin");
	//CachedFileIOTest cft("F:/cachedfile.bin");
	//cft.run(10000000);
	cout << "Storage header: " << sizeof StorageHeader << " bytes" << endl;
	//cout << "Collection header: " << sizeof CollectionHeader << " bytes" << endl;
	cout << "Record header: " << sizeof RecordHeader << " bytes" << endl;
}
