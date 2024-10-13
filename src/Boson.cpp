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


void insertRecords(BalancedIndex* bi) {
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
	bi->insert(25, "Sheriazdan");
	bi->insert(26, "Mariyam");
	bi->insert(27, "Aliya");
	bi->insert(28, "Erbol");
	bi->insert(29, "Asker");
	bi->insert(30, "Galiya");
	bi->insert(31, "Amantai");
	bi->insert(32, "Kuantai");
	bi->insert(33, "Nartai");
	bi->insert(34, "Tulkibai");
	bi->insert(35, "Dyisenbai");
	bi->insert(36, "Seisenbai");
	bi->insert(37, "Sarsenbai");
	bi->insert(38, "Beisembai");
	bi->insert(39, "Zhumabai");
	bi->insert(40, "Senbai");
	bi->insert(41, "Zheksenbai");
	bi->insert(42, "Otyzbai");
	bi->insert(43, "Kyrykbai");
	bi->insert(44, "Elubai");
	bi->insert(45, "Alpysbai");
	bi->insert(46, "Zhetpisbai");
	bi->insert(47, "Seksenbai");
	bi->insert(48, "Toksanbai");
}



void removeRecords(BalancedIndex* bi) {
	bi->erase(1);	
	/*bi->erase(3); // FIXME: bug
	bi->erase(5); 
	bi->erase(7);
	bi->erase(9);
	bi->erase(11);
	bi->erase(13);
	bi->erase(15);
	bi->erase(17);
	bi->erase(19);
	bi->erase(21);
	bi->erase(23);
	bi->erase(25);
	bi->erase(27);
	bi->erase(29);
	bi->erase(31);
	bi->erase(33);
	bi->erase(35);
	bi->erase(37);
	bi->erase(39);
	bi->erase(41);
	bi->erase(43);
	bi->erase(45);
	bi->erase(47);*/
}




int main()
{
	CachedFileIO cf;
	
	cf.open("D:/bptree.bin");

	try {
		if (cf.isOpen()) {
			RecordFileIO* rf = new RecordFileIO(cf);
			BalancedIndex* bi = new BalancedIndex(*rf);
			insertRecords(bi);
			cf.flush();
			bi->printTree();
			cout << "-------------------------------------------------------------------------\n";
			// TODO: debug remove records

			removeRecords(bi);
			bi->printTree();
			//bi->update(23, "Aishoka");
			//cout << "Replace record: " << 23 << " to '" << *bi->search(23) << "'" << std::endl;
			//bi->printTree();
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
