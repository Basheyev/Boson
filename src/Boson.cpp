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
	bi->insert(1, "Unknown guy");
	bi->insert(10, "Bolat");
	bi->insert(20, "Ayoka");
	bi->insert(30, "Teya"); 
	bi->insert(40, "Malika"); 
	bi->insert(50, "Shariha");
	bi->insert(60, "Nikhya"); 
	bi->insert(70, "Arman");
	bi->insert(80, "Khanat"); 
	bi->insert(90, "Baurzhan");
	bi->insert(100, "Igor"); 
	bi->insert(110, "Tanya");
	bi->insert(120, "Azat");
	bi->insert(130, "Dualet");
	bi->insert(140, "Berik");
	bi->insert(150, "Meirzhan");
	bi->insert(160, "Rakhim");
	bi->insert(170, "Anastasiya");
	bi->insert(180, "Victor");
	bi->insert(190, "Andrew");
	bi->insert(200, "Asem");
	bi->insert(210, "Aset");
	bi->insert(220, "Yerlan");
	bi->insert(230, "Sanzhar");
	bi->insert(240, "Askhat");
	bi->insert(250, "Sheriazdan");
	bi->insert(260, "Mariyam");
	bi->insert(270, "Aliya");
	bi->insert(280, "Erbol");
	bi->insert(290, "Asker");
	bi->insert(300, "Galiya");
	bi->insert(310, "Amantai");
	bi->insert(320, "Kuantai");
	bi->insert(330, "Nartai");
	bi->insert(340, "Tulkibai");
	bi->insert(350, "Dyisenbai");
	bi->insert(360, "Seisenbai");
	bi->insert(370, "Sarsenbai");
	bi->insert(380, "Beisembai");
	bi->insert(390, "Zhumabai");
	bi->insert(400, "Senbai");
	bi->insert(410, "Zheksenbai");
	bi->insert(420, "Otyzbai");
	bi->insert(430, "Kyrykbai");
	bi->insert(440, "Elubai");
	bi->insert(450, "Alpysbai");
	bi->insert(460, "Zhetpisbai");
	bi->insert(470, "Seksenbai");
	bi->insert(480, "Toksanbai");
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
			//bi->printTree();
			cout << "-------------------------------------------------------------------------\n";
			
			// FIXME: remove records - 
			// fails when accessing merged node from root
			// Suspicious methods:
			// - dealUnderflow()
			// - mergeChildren()
			// - mergeWithSibling()
			// - pushUpKey()

			removeRecords(bi);
			bi->printTree();
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
