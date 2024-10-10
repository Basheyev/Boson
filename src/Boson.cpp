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

//#include "InMemoryBPTree.h"
//#include "InMemoryTest.h"

#include <BalancedIndex.h>
#include <string>

//using namespace Research;
using namespace Boson;
using namespace std;


int main()
{
	CachedFileIO cf;
	cf.open("F:/bptree.bin");
	if (cf.isOpen()) {
		RecordFileIO rf(cf);
		BalancedIndex bi(rf);

		// FIXME - root is not leaf (!)

		bi.insert(1, "Teya");
		bi.insert(2, "Ayoka");
		bi.insert(3, "Bolat");

		cf.close();
	}

	//InMemoryTest imt;
	//imt.run(true);

	//RecordFileIOTest rst;
	//rst.run("F:/records.bin");
	//rst.runLoadTest("F:/largeFile.bin", 1000000);	
	//CachedFileIOTest cft("F:/cachedfile.bin");
	//cft.run(1000000);

	return 0;
}
