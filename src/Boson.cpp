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

#include <string>

using namespace Boson;
using namespace std;


int main()
{
	
	RecordFileIOTest rst;
	rst.run("F:/records.bin");
	//CachedFileIOTest cft("F:/cachedfile.bin");
	//cft.run(10000000);
	cout << "Storage header: " << sizeof StorageHeader << " bytes" << endl;
	//cout << "Collection header: " << sizeof CollectionHeader << " bytes" << endl;
	cout << "Record header: " << sizeof RecordHeader << " bytes" << endl;
}
