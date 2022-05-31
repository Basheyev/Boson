
#include "..\table\BalancedTree.h"
#include "BalancedTreeTest.h"

#include <chrono>
#include <iostream>

using namespace Boson;
using namespace std;


bool BalancedTreeTest::run(bool verbose) {
	bool result = true;
	result &= testLeafNode(verbose);
	result &= testInnerNode(verbose);
	result &= testBalancedTree(verbose);
	return result;
}



bool BalancedTreeTest::assert(char* msg, bool expr) {
	cout << msg;
	if (expr) cout << "OK"; else cout << "FAILED";
	cout << endl;
	return expr;
}



bool BalancedTreeTest::testLeafNode(bool verbose) {
	bool testPassed = true;
	cout << "Testing LeafNode class logic:" << endl;
	//------------------------------------------------------------------------
	LeafNode ln(10);
	testPassed &= assert(" - create leaf node of order 10...", ln.getKeyCount()==0);
	//------------------------------------------------------------------------
	ln.insertKey(10, "Baurzhan");
	ln.insertKey(73, "Theya");
	testPassed &= assert(" - insert two records and check underflow...",
		ln.isUnderflow() && !ln.isOverflow() && !ln.canLendAKey());
	if (verbose) ln.print(1);
	//------------------------------------------------------------------------
	ln.insertKey(14, "Bolat");
	ln.insertKey(32, "Aimgul");
	ln.insertKey(57, "Tair");
	ln.insertKey(98, "Igor");
	testPassed &= assert(" - insert four records and check canLandAKey...",
		ln.canLendAKey() && !ln.isUnderflow());
	if (verbose) ln.print(1);
	//------------------------------------------------------------------------	
	ln.insertKey(69, "Julia");
	ln.insertKey(24, "Tamara");
	ln.insertKey(45, "Maxim");
	ln.insertKey(86, "Hasar");
	testPassed &= assert(" - insert four records and check overflow...",
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
	testPassed &= assert(" - check sorted order of inserted key/value pairs...", sorted);
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
	testPassed &= assert(" - check search of keys 10,98,57,32 (found) and keys 11,25 (not found)...", searchResult);
	//------------------------------------------------------------------------
	testPassed &= assert(" - check delete key=69 pair and search for it - not found...", 
		ln.deleteKey(69) && ln.search(69)==NOT_FOUND);
	if (verbose) ln.print(1);
	//------------------------------------------------------------------------
	LeafNode* splittedNode = (LeafNode*) ln.split();
	testPassed &= assert(" - check node split...",
		ln.getKeyCount() == 4 && !ln.canLendAKey() && splittedNode->getKeyCount() == 5 &&
		splittedNode->getKeyAt(0)==45);
	if (verbose) {
		ln.print(1);
		cout << "\t------------------" << endl;
		splittedNode->print(1);
	}
	//------------------------------------------------------------------------
	ln.insertKey(39, "Hasar");
	splittedNode->deleteKey(45);
	ln.merge(NOT_FOUND, splittedNode);
	testPassed &= assert(" - check nodes merge: insert key 39 (A), delete key 45 (B) and merge A+B...", 
		ln.getKeyCount() == 9);
	if (verbose) ln.print(1);
	//------------------------------------------------------------------------
	cout << "LeafNode class tests - ";
	if (testPassed) cout << "PASSED." << endl; else cout << "FAILED!" << endl;
	return testPassed;
}


bool BalancedTreeTest::testInnerNode(bool verbose) {
	return true;
}


bool BalancedTreeTest::testBalancedTree(bool verbose) {
	bool result = true;
	BalancedTree* bt = buildTree(verbose);
	result &= (bt != nullptr);
	result &= deleteTree(bt, verbose);

	bt = buildRandomTree(verbose);
	result &= testPerformance(bt, verbose);
	delete bt;

	return result;
}


BalancedTree* BalancedTreeTest::buildTree(bool verbose) {
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
	return bt;
}


BalancedTree* BalancedTreeTest::buildRandomTree(bool verbose) {
	int entriesCount = 1000000;
	BalancedTree* bt = new BalancedTree(32);
	cout << "Generating " << entriesCount << " entries of data...";
	auto start1 = chrono::steady_clock::now();
	char* value = new char[entriesCount*8];
	for (int i = 0; i < entriesCount; i++) {
		value[i * 8 + 0] = 'A' + i % 26;
		value[i * 8 + 1] = 'A' + (i+1) % 26;
		value[i * 8 + 2] = 'A' + (i+2) % 26;
		value[i * 8 + 3] = 'A' + (i+3) % 26;
		value[i * 8 + 4] = 'A' + (i+4) % 26;
		value[i * 8 + 5] = 'A' + (i+5) % 26;
		value[i * 8 + 6] = 'A' + (i+6) % 26;
		value[i * 8 + 7] = 0;
	}
	auto end1 = chrono::steady_clock::now();
	cout << "OK (" << (end1 - start1).count()/1000000000.0 << " s)" << endl;


	cout << "Inserting " << entriesCount << " entries intro B+ Tree...";
	start1 = chrono::steady_clock::now();
	for (int i = 0; i < entriesCount; i++) {
		bt->insert(i, value);
	}
	end1 = chrono::steady_clock::now();
	cout << "OK (" << (end1 - start1).count()/1000000000.0 << " s)" << endl;

	// FIXME planned memory leak;

	return bt;
}


bool BalancedTreeTest::deleteTree(BalancedTree* bt, bool verbose) {
	bt->erase(14);	if (verbose) bt->printTree();
	bt->erase(11);	if (verbose) bt->printTree();
	bt->erase(15);	if (verbose) bt->printTree();
	bt->erase(24);	if (verbose) bt->printTree();
	bt->erase(10);	if (verbose) bt->printTree();
	bt->erase(86);	if (verbose) bt->printTree();
	bt->erase(98);	if (verbose) bt->printTree();
	bt->erase(74);	if (verbose) bt->printTree();
	bt->erase(73);	if (verbose) bt->printTree();
	bt->erase(45);	if (verbose) bt->printTree();
	bt->erase(57);	if (verbose) bt->printTree();
	bt->erase(69);	if (verbose) bt->printTree();
	bt->erase(32);	if (verbose) bt->printTree();
	bt->erase(99);	if (verbose) bt->printTree();
	bt->erase(87);	if (verbose) bt->printTree();
	bt->erase(72);	if (verbose) bt->printTree();
	bt->erase(35);	if (verbose) bt->printTree();
	bt->erase(71);	if (verbose) bt->printTree();
	bt->erase(70);	if (verbose) bt->printTree();
	bt->erase(46);	if (verbose) bt->printTree();
	bt->erase(17);	if (verbose) bt->printTree();
	bt->erase(21);	if (verbose) bt->printTree();
	bt->erase(58);	if (verbose) bt->printTree();
	bt->erase(25);	if (verbose) bt->printTree();
	bt->erase(33);	if (verbose) bt->printTree();
	delete bt;
	return true;
}



bool BalancedTreeTest::testPerformance(BalancedTree* bt, bool verbose) {

	cout << "Testing tree search performance:" << endl;

	size_t searchKey = 456321;
	
	cout << "B+ tree index search for key: " << searchKey << " - ";

	auto start1 = chrono::steady_clock::now();
	auto value = bt->search(searchKey);
	auto end1 = chrono::steady_clock::now();
	if (value == nullptr) cout << "NOT FOUND"; else cout << value;
	cout << " ("
		<< (end1 - start1).count() / 1000000.0
		<< " ms)" << endl;
	cout << flush;

	cout << "Sequential search for key: " << searchKey << " - ";
	auto start2 = chrono::steady_clock::now();
	value = bt->sequencialSearch(searchKey);
	auto end2 = chrono::steady_clock::now();
	if (value == nullptr) cout << "NOT FOUND"; else cout << value;
	cout << " ("
		<< (end2 - start2).count() / 1000000.0
		<< " ms)" << endl;

	return true;
}