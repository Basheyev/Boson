﻿/*
* Boson Database
*
*
* (C) Bolat Basheyev 2022
*/

#include "Boson.h"
#include "registry/BalancedTree.h"

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
	ln.print();

	int index = ln.search(73);
	char* value = "NOT FOUND";
	if (index!=NOT_FOUND) value = ln.getValue(index);
	cout << "search key 73 = " << value << endl;
	
	ln.deleteKey(69);
	cout << "delete key 69 " << endl;
	
	index = ln.search(69);
	value = "NOT FOUND";
	if (index != NOT_FOUND) value = ln.getValue(index);
	cout << "search key 69 = " << value << endl;



	ln.print();
}


void BalancedTreeTest() {
	BalancedTree* bt = new BalancedTree();
	cout << bt->getTreeOrder() << endl;
	delete bt;
}


int main()
{
	LeafNodeTest();	    
	return 0;
}
