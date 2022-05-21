/*
* Boson Database
*
*
* (C) Bolat Basheyev 2022
*/

#include "Boson.h"
#include "table/BalancedTree.h"

using namespace std;
using namespace Boson;



void LeafNodeTest() {
	LeafNode ln(10);
	ln.insertKey(10, "Baurzhan");
	ln.insertKey(73, "Theya");
	ln.insertKey(14, "Bolat");
	ln.insertKey(32, "Aimgul");
	ln.insertKey(57, "Tair");
	ln.insertKey(98, "Igor");
	ln.insertKey(69, "Julia");
	ln.insertKey(24, "Tamara");
	ln.insertKey(45, "Maxim");
	ln.insertKey(86, "Hasar");
	ln.print(0);

	size_t index = ln.search(73);
	char* value = "NOT FOUND";
	if (index!=NOT_FOUND) value = ln.getValueAt(index);
	cout << "search key 73 = " << value << endl;
	
	ln.deleteKey(69);
	cout << "delete key 69 " << endl;
	
	index = ln.search(69);
	value = "NOT FOUND";
	if (index != NOT_FOUND) value = ln.getValueAt(index);
	cout << "search key 69 = " << value << endl;

	ln.print(0);

	cout << "split\n";

	LeafNode* ln2 = (LeafNode*) ln.split();
	ln.print(0);
	ln2->print(0);


}


void BalancedTreeTest() {
	BalancedTree* bt = new BalancedTree(3);
	bt->insert(10, "Baurzhan");
	bt->print();
	bt->insert(73, "Theya");
	bt->print();
	bt->insert(14, "Bolat");
	bt->print();
	bt->insert(32, "Aimgul");
	bt->print();
	bt->insert(57, "Tair");
	bt->print();
	/*
	bt->insert(98, "Igor");
	bt->print();
	bt->insert(69, "Julia");
	bt->print();
	bt->insert(24, "Tamara");
	bt->print();
	bt->insert(45, "Maxim");
	bt->print();
	bt->insert(86, "Hasar");
	bt->print();*/
	delete bt;
}


int main()
{
	BalancedTreeTest();
	return 0;
}
