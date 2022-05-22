/*
* Boson Database
*
*
* (C) Bolat Basheyev 2022
*/

#include "Boson.h"
#include "table/BalancedTree.h"

#include <chrono>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

using namespace Boson;
using namespace std;
using namespace chrono;


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
	bt->insert( 10, "Baurzhan");
	bt->insert( 73, "Theya");
	bt->insert( 14, "Bolat");
	bt->insert( 32, "Aimgul");
	bt->insert( 57, "Tair");
	bt->insert( 98, "Igor");
	bt->insert( 69, "Julia");
	bt->insert( 24, "Tamara");
	bt->insert( 45, "Maxim");
	bt->insert( 86, "Hasar");
	bt->insert( 11, "Umitzhan");
	bt->insert( 74, "Zhumagali");
	bt->insert(15, "Louisa");
	/*bt->insert(33, "Samal");
	bt->insert( 58, "Elizabeth");
	bt->insert( 99, "Kymbat");
	bt->insert( 70, "Talgat");
	bt->insert( 25, "Dastan");
	bt->insert( 46, "Laura");
	bt->insert( 87, "Youri");
	bt->insert( 21, "Peter");
	bt->insert( 23, "Ivan");
	bt->insert( 17, "Andrew");
	bt->insert( 35, "John");
	bt->insert( 26, "Anabele");
	bt->insert( 27, "Serenata");
	bt->insert( 29, "Erdogan");
	bt->insert( 41, "Vladimit");
	bt->insert( 42, "Nikolay");
	bt->insert( 71, "Hector");
	bt->insert( 72, "Augustus");*/

	bt->printTree();
	//->printContent();
	bt->erase(14);
	bt->printTree();
	bt->erase(11);
	bt->printTree();
	bt->erase(15);
	bt->printTree();
	bt->erase(24);
	bt->printTree();
	bt->erase(10);
	bt->printTree();
	bt->erase(86);
	bt->printTree();
	bt->erase(98);
	bt->printTree();
	bt->erase(74);
	bt->printTree();
	bt->erase(73);
	bt->printTree();
	bt->erase(45);
	bt->printTree();
	bt->erase(57);
	bt->printTree();
	bt->erase(69);
	bt->printTree();
	bt->erase(32);
	bt->printTree();

	bt->insert(10, "Baurzhan");
	bt->insert(73, "Theya");
	bt->insert(14, "Bolat");
	bt->insert(32, "Aimgul");
	bt->insert(57, "Tair");
	bt->insert(98, "Igor");
	bt->insert(69, "Julia");
	bt->insert(24, "Tamara");
	bt->insert(45, "Maxim");
	bt->insert(86, "Hasar");
	bt->insert(11, "Umitzhan");
	bt->insert(74, "Zhumagali");
	bt->insert(15, "Louisa");
	bt->printTree();
	//->printContent();
	/*
	size_t searchKey = 72;
	
	auto start1 = chrono::steady_clock::now();
	cout << "Tree lookup: " << searchKey << " - " << bt->search(searchKey) << endl;
	auto end1 = chrono::steady_clock::now();
	cout << "Elapsed time in nanoseconds: "
		<< (end1 - start1).count()
		<< " ns" << endl;

	
	auto start2 = chrono::steady_clock::now();
	cout << "List lookup: " << searchKey << " - " << bt->directSearch(searchKey) << endl;
	auto end2 = chrono::steady_clock::now();
	cout << "Elapsed time in nanoseconds: "
		<< (end2 - start2).count()
		<< " ns" << endl;

		*/





	delete bt;
}


int main()
{
	BalancedTreeTest();
	return 0;
}
