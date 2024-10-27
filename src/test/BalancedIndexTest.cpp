#include "BalancedIndexTest.h"


using namespace Boson;


BalancedIndexTest::BalancedIndexTest(char* path) {
	filename = path;
}


BalancedIndexTest::~BalancedIndexTest() {

}


bool BalancedIndexTest::run(bool clearFile) {
	
	if (clearFile) std::remove(filename);

	CachedFileIO cf;
	cf.open(filename);

	try {
		if (cf.isOpen()) {
			RecordFileIO* rf = new RecordFileIO(cf);
			BalancedIndex* bi = new BalancedIndex(*rf);
			
			insertRecords(bi);
			bi->printTree();
			removeRecords(bi);
			bi->printTree();
			insertRecords(bi);
			bi->printTree();

			delete bi;
			delete rf;

			std::cout << "CACHE HITS RATE: " << cf.getStats(CachedFileStats::CACHE_HITS_RATE) << "%";

			cf.close();
		}
	}
	catch (const std::runtime_error& e) {
		std::cout << std::endl;
		std::cout << "ERROR: " << e.what() << std::endl;
		return false;
	}
	return true;
}


void BalancedIndexTest::insertRecords(BalancedIndex* bi) {
	std::cout << std::endl;
	std::cout << "-------------------------------------------------------------------------\n";
	std::cout << "INSERTING RECRODS\n";
	std::cout << "-------------------------------------------------------------------------\n";
	bi->insert(420, "Otyzbai");
	bi->insert(430, "Kyrykbai");
	bi->insert(440, "Elubai");
	bi->insert(450, "Alpysbai");
	bi->insert(460, "Zhetpisbai");
	bi->insert(470, "Seksenbai");
	bi->insert(480, "Toksanbai");

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

	bi->insert(1, "Unknown guy");
	bi->insert(341, "WWWWWWWWWWW");

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

	bi->insert(10, "Bolat");
	bi->insert(20, "Ayoka");
	bi->insert(30, "Teya");
	bi->insert(40, "Malika");
	bi->insert(50, "Shariha");
	bi->insert(60, "Nikhya");
	bi->insert(70, "Arman");
	bi->insert(80, "Khanat");
	bi->insert(90, "Baurzhan");

}

void BalancedIndexTest::removeRecords(BalancedIndex* bi) {
	std::cout << std::endl;
	std::cout << "-------------------------------------------------------------------------\n";
	std::cout << "DELETING RECRODS\n";
	std::cout << "-------------------------------------------------------------------------\n";
	bi->erase(10);
	bi->erase(30);
	bi->erase(50);
	bi->erase(70);
	bi->erase(90);

	bi->erase(290);
	bi->erase(310);
	bi->erase(330);
	bi->erase(350);
	bi->erase(370);
	bi->erase(390);
	bi->erase(410);
	bi->erase(430);
	bi->erase(450);
	bi->erase(470);

	bi->erase(110);
	bi->erase(130);
	bi->erase(150);
	bi->erase(170);
	bi->erase(190);
	bi->erase(210);
	bi->erase(230);
	bi->erase(250);
	bi->erase(270);

	bi->erase(20);
	bi->erase(40);

	bi->erase(180);
	bi->erase(200);
	bi->erase(220);

	bi->erase(240);
	bi->erase(260);
	bi->erase(280);
	bi->erase(300);
	bi->erase(320);
	bi->erase(340);

	bi->erase(60);
	bi->erase(80);
	bi->erase(100);
	bi->erase(120);
	bi->erase(140);
	bi->erase(160);

	bi->erase(360);
	bi->erase(380);
	bi->erase(400);
	bi->erase(420);
	bi->erase(440);
	bi->erase(460);
	bi->erase(480);

	bi->erase(1);
	bi->erase(341);

}

