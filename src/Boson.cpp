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

	try {
		if (cf.isOpen()) {
			RecordFileIO* rf = new RecordFileIO(cf);
			BalancedIndex* bi = new BalancedIndex(*rf);

			bi->insert(1, "Bolat");


			
			bi->insert(2, "Ayoka");
			bi->insert(3, "Teya");
			bi->insert(4, "Malika");
			bi->insert(5, "Shariha");
			bi->insert(6, "Nikhya");
			bi->insert(7, "Arman");
			bi->insert(8, "Khanat");
			bi->insert(9, "Baurzhan");
			bi->insert(10, "Igor");
			bi->insert(11, "Tanya");
			bi->insert(12, "Azat");
			bi->insert(13, "Dualet");
			bi->insert(14, "Berik");
			bi->insert(15, "Meirzhan");
			bi->insert(16, "Rakhim");
			bi->insert(17, "Anastasiya");
			bi->insert(18, "Victor");
			bi->insert(19, "Andrew");
			bi->insert(20, "Asem");
			bi->insert(21, "Aset");
			bi->insert(22, "Yerlan");
			bi->insert(23, "Sanzhar");
			bi->insert(24, "Askhat");

			cf.flush();

			bi->printTree();

			//bi->update(23, "Aishoka");

			//cout << *bi->search(23) << std::endl;

			delete bi;
			delete rf;

			cf.close();

		}
	}
	catch (const std::runtime_error& e) {
		std:cout << "Error: " << e.what() << std::endl;
	}

	return 0;
}
