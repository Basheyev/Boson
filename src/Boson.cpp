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


//using namespace Research;
using namespace Boson;
using namespace std;


void insertRecords(BalancedIndex* bi) {
	//bi->insert(1, "Unknown guy");
	bi->insert(341, "WWWWWWWWWWW");
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
	cout << std::endl;
	cout << "-------------------------------------------------------------------------\n";
	cout << "DELETING RECRODS\n";
	cout << "-------------------------------------------------------------------------\n";
	bi->erase(10);	
	bi->erase(30);
	bi->erase(50);
	bi->erase(70);
	bi->erase(90);
	bi->erase(110);
	bi->erase(130);
	bi->erase(150);
	bi->erase(170);
	bi->erase(190);
	bi->erase(210);
	bi->erase(230);
	bi->erase(250);
	bi->erase(270);
	bi->erase(290);
	/*bi->erase(310);
	bi->erase(330);
	bi->erase(350);
	bi->erase(370);
	bi->erase(390);
	bi->erase(410);
	bi->erase(430);
	bi->erase(450);
	bi->erase(470);*/
}



// FIXME: why five children after merge?
// FIXME: borrowFromSibling - debug

int main()
{
	char* filename = "D:/bptree.bin";

	std::remove(filename);

	CachedFileIO cf;	
	cf.open(filename);

	try {
		if (cf.isOpen()) {
			RecordFileIO* rf = new RecordFileIO(cf);
			BalancedIndex* bi = new BalancedIndex(*rf);					

			//bi->printTree();
			insertRecords(bi);
			bi->printTree();
			
			uint64_t key = 480;
			auto value = *bi->search(key);

			std::cout << "SEARCH KEY:" << std::endl;
			std::cout << key << "=" << value << std::endl;

			removeRecords(bi);
			bi->printTree();

		//	insertRecords(bi);
		//	bi->printTree();

			delete bi;
			delete rf;

			cf.close();

		}
	}
	catch (const std::runtime_error& e) {
		std::cout << std::endl;
		std::cout << "ERROR: " << e.what() << std::endl;
	}

	return 0;
}
