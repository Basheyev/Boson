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

#include "BalancedTree.h"
#include "BalancedTreeTest.h"

#include <string>

using namespace Boson;
using namespace std;


int main()
{
    //BalancedTreeTest bpt;
	//bpt.run();
	RecordFileIOTest rst;
	//rst.run("F:/records.bin");
	rst.runLoadTest("F:/largeFile.bin", 1000000);	
	//CachedFileIOTest cft("F:/cachedfile.bin");
	//cft.run(1000000);
	cout << "Storage header: " << sizeof StorageHeader << " bytes" << endl;
	cout << "Record header: " << sizeof RecordHeader << " bytes" << endl;
}
