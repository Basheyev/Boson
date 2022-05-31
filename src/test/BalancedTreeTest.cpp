
#include "..\table\BalancedTree.h"

#include <chrono>
#include <iostream>

using namespace Boson;
using namespace std;


void assert(char* msg, bool expr) {
	cout << msg;
	if (expr) cout << "OK"; else cout << "FAILED";
	cout << endl;
}


bool test_LeafNode(bool verbose) {
	
	cout << "Test LeafNode:" << endl;
	//------------------------------------------------------------------------
	LeafNode ln(10);
	assert(" - create leaf node of order 10...", true);
	//------------------------------------------------------------------------
	ln.insertKey(10, "Baurzhan");
	ln.insertKey(73, "Theya");
	assert(" - insert two records and check underflow...",
		ln.isUnderflow() && !ln.isOverflow() && !ln.canLendAKey());
	if (verbose) ln.print(1);
	//------------------------------------------------------------------------
	ln.insertKey(14, "Bolat");
	ln.insertKey(32, "Aimgul");
	ln.insertKey(57, "Tair");
	ln.insertKey(98, "Igor");
	assert(" - insert four records and check canLandAKey...",
		ln.canLendAKey() && !ln.isUnderflow());
	if (verbose) ln.print(1);
	//------------------------------------------------------------------------	
	ln.insertKey(69, "Julia");
	ln.insertKey(24, "Tamara");
	ln.insertKey(45, "Maxim");
	ln.insertKey(86, "Hasar");
	assert(" - insert four records and check overflow...",
		ln.isOverflow() && !ln.isUnderflow() && ln.canLendAKey());
	if (verbose) ln.print(1);
	//------------------------------------------------------------------------	
	bool sorted = true;
	for (size_t i = 1; i < ln.getKeyCount(); i++) {
		if (ln.getKeyAt(i - 1) > ln.getKeyAt(i)) {
			sorted = false;
			break;
		}
	}
	assert(" - check sorted order of inserted key/value pairs...", sorted);
	if (verbose) ln.print(1);
	//------------------------------------------------------------------------
	bool searchResult =
		ln.search(10) != NOT_FOUND &&
		ln.search(98) != NOT_FOUND &&
		ln.search(57) != NOT_FOUND &&
		ln.search(32) != NOT_FOUND &&
		ln.search(69) != NOT_FOUND &&
		ln.search(11) == NOT_FOUND &&
		ln.search(25) == NOT_FOUND;
	assert(" - check search of keys 10,98,57,32 and keys 11,25...", searchResult);
	//------------------------------------------------------------------------
	assert(" - check delete key=69 pair...", ln.deleteKey(69) && ln.search(69)==NOT_FOUND);
	if (verbose) ln.print(1);
	//------------------------------------------------------------------------
	LeafNode* splittedNode = (LeafNode*) ln.split();
	assert(" - check node split...", ln.getKeyCount() == 4 && splittedNode->getKeyCount() == 5);
	if (verbose) {
		ln.print(1);
		cout << "\t------------------" << endl;
		splittedNode->print(1);
	}
	//------------------------------------------------------------------------
	ln.insertKey(39, "Hasar");
	splittedNode->deleteKey(45);
	ln.merge(NOT_FOUND, splittedNode);
	assert(" - check nodes merge: insert key 39 (A), delete key 45 (B) and merge...", ln.getKeyCount() == 9);
	if (verbose) ln.print(1);

	return false;
}


bool test_InnerNode() {
	return false;
}


bool test_BalancedTree() {
	BalancedTree* bt = new BalancedTree(3);
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
	bt->insert(33, "Samal");
	bt->insert(58, "Elizabeth");
	bt->insert(99, "Kymbat");
	bt->insert(70, "Talgat");
	bt->insert(25, "Dastan");
	bt->insert(46, "Laura");
	bt->insert(87, "Youri");
	bt->insert(21, "Peter");


	bt->printTree();
	//bt->printContent();
	bt->erase(14);	bt->printTree();
	bt->erase(11);	bt->printTree();
	bt->erase(15);	bt->printTree();
	bt->erase(24);	bt->printTree();
	/*bt->erase(10);	bt->printTree();
	bt->erase(86);	bt->printTree();
	bt->erase(98);	bt->printTree();
	bt->erase(74);	bt->printTree();
	bt->erase(73);	bt->printTree();
	bt->erase(45);	bt->printTree();
	bt->erase(57);	bt->printTree();
	bt->erase(69);	bt->printTree();
	bt->erase(32);	bt->printTree();
	bt->erase(99);	bt->printTree();
	bt->erase(87);	bt->printTree();
	bt->erase(72);	bt->printTree();
	bt->erase(35);	bt->printTree();
	bt->erase(71);	bt->printTree();
	bt->erase(70);	bt->printTree();
	bt->erase(46);	bt->printTree();
	bt->erase(17);	bt->printTree();
	bt->erase(21);	bt->printTree();
	bt->erase(58);	bt->printTree();
	bt->erase(25);	bt->printTree();
	bt->erase(33);	bt->printTree();
	bt->erase(26);	bt->printTree();
	bt->erase(23);	bt->printTree();
	bt->erase(41);	bt->printTree();*/


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
	return false;
}