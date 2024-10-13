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
	
	cf.open("D:/bptree.bin");

	if (cf.isOpen()) {
		RecordFileIO *rf = new RecordFileIO(cf);
		BalancedIndex *bi = new BalancedIndex(*rf);


		bi->insert(1, "Bolat");
		bi->insert(2, "Ayoka");
		bi->insert(3, "Teya");
		bi->insert(4, "Malika");
        bi->insert(5, "Shariha");
		bi->insert(6, "Nikhya");
		cf.flush();
					
		
		cout << *bi->search(5) << std::endl;

		delete bi;
		delete rf;

		cf.close();
	
	}

	return 0;
}
