/*=================================================================================================
*
*    Balanced PLus Tree Implementation
*
*    Indexed key/value store
*
*    BOSON embedded database
*    (C) Bolat Basheyev 2022
*
=================================================================================================*/
#include "BalancedTreeIndex.h"
#include <iostream>

using namespace Boson;
using namespace std;

//-------------------------------------------------------------------------------------------------
// Balanced Tree Constructor
// - M-1 - maximum Keys count per Inner node and Key-Value pairs per Leaf node
// - M/2 - minimal keys count per Inner node and Key-Value pairs per Leaf node
// - M   - maximum Child nodes per Inner node
//-------------------------------------------------------------------------------------------------
BalancedTreeIndex::BalancedTreeIndex(size_t M) {
	if (M < MINIMAL_TREE_ORDER) M = MINIMAL_TREE_ORDER;
	this->treeOrder = M;
	this->entriesCount = 0;
	root = new LeafNode(M);
}


//-------------------------------------------------------------------------------------------------
// Balanced Tree Desctructor
//-------------------------------------------------------------------------------------------------
BalancedTreeIndex::~BalancedTreeIndex() {
	delete root;
}


//-------------------------------------------------------------------------------------------------
// Insert key/value pair
//-------------------------------------------------------------------------------------------------
bool BalancedTreeIndex::insert(KEY key, VALUE value) {
	LeafNode* leaf = findLeafNode(key);
	bool isInserted = leaf->insertKey(key, value);
	if (leaf->isOverflow()) {
		Node* n = leaf->dealOverflow();
		if (n != nullptr) root = n;
	}
	entriesCount++;
	return isInserted;
}


//-------------------------------------------------------------------------------------------------
// Search value by key (binary search)
//-------------------------------------------------------------------------------------------------
VALUE BalancedTreeIndex::search(KEY key) {
	LeafNode* leaf = findLeafNode(key);
	size_t index = leaf->search(key);
	return (index == NOT_FOUND) ? nullptr : leaf->getValueAt(index);
}


//-------------------------------------------------------------------------------------------------
// Delete key/value pair
//-------------------------------------------------------------------------------------------------
bool BalancedTreeIndex::erase(KEY key) {

	LeafNode* leaf = findLeafNode(key);
	
	if (leaf->deleteKey(key)) {
		entriesCount--;
		if (leaf->isUnderflow()) {
			Node* n = leaf->dealUnderflow();
			if (n != nullptr) {
				n->setParent(nullptr);
				root = n;
			}
		}
	}
	return true;
}


//-------------------------------------------------------------------------------------------------
// Search for LeafNode that contains specified key
//-------------------------------------------------------------------------------------------------
LeafNode* BalancedTreeIndex::findLeafNode(KEY key) {
	Node* node = root;
	InnerNode* innerNode;
	size_t index;
	while (node->getNodeType() == NodeType::INNER) {
		index = node->search(key);
		innerNode = (InnerNode*)node;
		node = innerNode->getChildAt(index);
	}
	return (LeafNode*)node;
}


//-------------------------------------------------------------------------------------------------
// Return tree order M
//-------------------------------------------------------------------------------------------------
size_t BalancedTreeIndex::getTreeOrder() {
	return this->treeOrder;
}


//-------------------------------------------------------------------------------------------------
// Return tree height
//-------------------------------------------------------------------------------------------------
size_t BalancedTreeIndex::getTreeHeight() {
	size_t levelCounter = 0;
	Node* firstLeaf = root;
	while (firstLeaf->getNodeType() == NodeType::INNER) {
		firstLeaf = ((InnerNode*)firstLeaf)->getChildAt(0);
		levelCounter++;
	}
	return levelCounter;
}


//-------------------------------------------------------------------------------------------------
// Returns records count
//-------------------------------------------------------------------------------------------------
size_t BalancedTreeIndex::getEntriesCount() {
	return entriesCount;
}

Node* BalancedTreeIndex::getRoot() {
	return root;
}


void BalancedTreeIndex::printTree() {
	cout << "----------------------------------------" << endl;
	root->print(0);
}


void BalancedTreeIndex::printContent() {
	cout << "----------------------------------------" << endl;
	Node* firstLeaf = root;
	// go down tree
	while (firstLeaf->getNodeType() == NodeType::INNER) {
		firstLeaf = ((InnerNode*)firstLeaf)->getChildAt(0);
	}
	// print list
	while (firstLeaf != nullptr) {
		firstLeaf->print(0);
		firstLeaf = firstLeaf->getRightSibling();
	}
}