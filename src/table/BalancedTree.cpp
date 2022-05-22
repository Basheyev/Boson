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
#include "BalancedTree.h"
#include <iostream>

using namespace Boson;
using namespace std;

//-------------------------------------------------------------------------------------------------
// Balanced Tree Constructor
// - M-1 - maximum Keys count per Inner node and Key-Value pairs per Leaf node
// - M/2 - minimal keys count per Inner node and Key-Value pairs per Leaf node
// - M   - maximum Child nodes per Inner node
//-------------------------------------------------------------------------------------------------
BalancedTree::BalancedTree(size_t M) {
	if (M < MINIMAL_TREE_ORDER) M = MINIMAL_TREE_ORDER;
	this->treeOrder = M;
	root = new LeafNode(M);
}


//-------------------------------------------------------------------------------------------------
// Balanced Tree Desctructor
//-------------------------------------------------------------------------------------------------
BalancedTree::~BalancedTree() {
	delete root;
}


//-------------------------------------------------------------------------------------------------
// Insert key/value pair
//-------------------------------------------------------------------------------------------------
void BalancedTree::insert(KEY key, VALUE value) {
	#ifdef DEBUG
		cout << "Inserting key = " << key << " - " << value << endl;
	#endif // DEBUG
	
	LeafNode* leaf = findLeafNode(key);
	leaf->insertKey(key, value);
	if (leaf->isOverflow()) {
		Node* n = leaf->dealOverflow();
		if (n != nullptr) root = n;
	}
}


//-------------------------------------------------------------------------------------------------
// Search value by key (binary search)
//-------------------------------------------------------------------------------------------------
VALUE BalancedTree::search(KEY key) {
	LeafNode* leaf = findLeafNode(key);
	size_t index = leaf->search(key);
	return (index == NOT_FOUND) ? nullptr : leaf->getValueAt(index);
}


//-------------------------------------------------------------------------------------------------
// Search value by key (list scanning)
//-------------------------------------------------------------------------------------------------
VALUE BalancedTree::directSearch(KEY key) {
	Node* firstLeaf = root;
	// go down tree
	while (firstLeaf->getNodeType() == NodeType::INNER) {
		firstLeaf = ((InnerNode*)firstLeaf)->getChild(0);
	}
	// print list
	while (firstLeaf != nullptr) {
		for (size_t i = 0; i < firstLeaf->getKeyCount(); i++) {
			if (firstLeaf->getKeyAt(i) == key) {
				return ((LeafNode*)firstLeaf)->getValueAt(i);
			}
		}
		firstLeaf = firstLeaf->getRightSibling();
	}
	return nullptr;
}


//-------------------------------------------------------------------------------------------------
// Delete key/value pair
//-------------------------------------------------------------------------------------------------
bool BalancedTree::erase(KEY key) {
//	#ifdef DEBUG
		cout << "Erasing key = " << key << endl;
//	#endif // DEBUG
		
	LeafNode* leaf = findLeafNode(key);
	if (leaf->deleteKey(key) && leaf->isUnderflow()) {
		Node* n = leaf->dealUnderflow();
		if (n != nullptr) {
			n->setParent(nullptr);
			root = n;
		}
	}
	return true;
}


//-------------------------------------------------------------------------------------------------
// Search for LeafNode that contains specified key
//-------------------------------------------------------------------------------------------------
LeafNode* BalancedTree::findLeafNode(KEY key) {
	Node* node = root;
	InnerNode* innerNode;
	size_t index;
	while (node->getNodeType() == NodeType::INNER) {
		index = node->search(key);
		innerNode = (InnerNode*)node;
		node = innerNode->getChild(index);
	}
	return (LeafNode*)node;
}


//-------------------------------------------------------------------------------------------------
// Return tree order M:

//-------------------------------------------------------------------------------------------------
size_t BalancedTree::getTreeOrder() {
	return this->treeOrder;
}


Node* BalancedTree::getRoot() {
	return root;
}


void BalancedTree::printTree() {
	cout << "----------------------------------------" << endl;
	root->print(0);
	cout << "----------------------------------------" << endl;
}


void BalancedTree::printContent() {
	Node* firstLeaf = root;
	// go down tree
	while (firstLeaf->getNodeType() == NodeType::INNER) {
		firstLeaf = ((InnerNode*)firstLeaf)->getChild(0);
	}
	// print list
	while (firstLeaf != nullptr) {
		firstLeaf->print(0);
		firstLeaf = firstLeaf->getRightSibling();
	}
	cout << "----------------------------------------" << endl;
}