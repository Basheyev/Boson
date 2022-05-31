/*=================================================================================================
*
*    Balanced Plus Tree Leaf Node Implementation
*
*    Leaf Node (Data Page) Implementation
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
// Leaf Node Constructor (calls Node Constructor)
// - M-1 - maximum Key-Value pairs per Leaf node
// - M/2 - minimal Key-Value pairs per Leaf node
//-------------------------------------------------------------------------------------------------
LeafNode::LeafNode(size_t m) : Node(m) {
	values.reserve(m);
}


//-------------------------------------------------------------------------------------------------
// Leaf Node Destructor
//-------------------------------------------------------------------------------------------------
LeafNode::~LeafNode() {
	values.clear();
}


//-------------------------------------------------------------------------------------------------
// Return value at specified index in this node
//-------------------------------------------------------------------------------------------------
VALUE LeafNode::getValueAt(size_t index) {
	return values[index];
}


//-------------------------------------------------------------------------------------------------
// Set value at specified index in this node
//-------------------------------------------------------------------------------------------------
void LeafNode::setValueAt(size_t index, VALUE value) {
	values[index] = value;
}


//-------------------------------------------------------------------------------------------------
// Search index of key in this node
//-------------------------------------------------------------------------------------------------
size_t LeafNode::search(KEY key) {
	for (size_t i = 0; i < keys.size(); i++) {
		if (keys[i] == key) return i;
	}
	return NOT_FOUND;
}


//-------------------------------------------------------------------------------------------------
// Insert key/value pair to this node in sorted order
//-------------------------------------------------------------------------------------------------
void LeafNode::insertKey(KEY key, VALUE value) {
	// find index to insert new key/value pair in sorted order
	size_t insertIndex = keys.size();
	for (size_t i = 0; i < keys.size(); i++) {
		if (key < keys[i]) {
			insertIndex = i;
			break;
		} else if (key == keys[i]) {
			cout << "Key duplicate " << key << endl;
			return; // do not allow duplicated values
		}
	}
	// insert key/value
	insertAt(insertIndex, key, value);
}


//-------------------------------------------------------------------------------------------------
// Insert key/value pair at specified index in this node
//-------------------------------------------------------------------------------------------------
void LeafNode::insertAt(size_t index, KEY key, VALUE value) {
	keys.insert(keys.begin() + index, 1, key);
	values.insert(values.begin() + index, 1, value);
}


//-------------------------------------------------------------------------------------------------
// Delete key/value pair by key in this node
//-------------------------------------------------------------------------------------------------
bool LeafNode::deleteKey(KEY key) {
	size_t deleteIndex = search(key);
	if (deleteIndex == NOT_FOUND) return false;
	deleteAt(deleteIndex);
	return true;
}


//-------------------------------------------------------------------------------------------------
// Delete key/value pair at specified index in this node
//-------------------------------------------------------------------------------------------------
void LeafNode::deleteAt(size_t index) {
	keys.erase(keys.begin() + index);
	values.erase(values.begin() + index);
}


//-------------------------------------------------------------------------------------------------
// Split this node by half and return new splitted node
//-------------------------------------------------------------------------------------------------
Node* LeafNode::split() {
	size_t midIndex = keys.size() / 2;
	LeafNode* newNode = new LeafNode(this->treeOrder);
	for (size_t i = midIndex; i < keys.size(); ++i) {
		newNode->insertKey(keys[i], values[i]);
	}
	keys.resize(midIndex);
	values.resize(midIndex);
	return newNode;
}


//-------------------------------------------------------------------------------------------------
// Merges two leaf nodes
//-------------------------------------------------------------------------------------------------
void LeafNode::merge(KEY key, Node* sibling) {
	LeafNode* siblingLeaf = (LeafNode*)sibling;

	for (size_t i = 0; i<siblingLeaf->getKeyCount(); i++) {
		keys.push_back(siblingLeaf->getKeyAt(i));
		values.push_back(siblingLeaf->getValueAt(i));
	}

	setRightSibling(siblingLeaf->rightSibling);
	if (siblingLeaf->rightSibling != nullptr) {
		siblingLeaf->rightSibling->setLeftSibling(this);
	}

	delete siblingLeaf;
}


//-------------------------------------------------------------------------------------------------
// Unsupported operation for leaf node
//-------------------------------------------------------------------------------------------------
Node* LeafNode::pushUpKey(KEY key, Node* leftChild, Node* rightChild) {
	throw std::runtime_error("Unsupported operation");
}


//-------------------------------------------------------------------------------------------------
// Unsupported operation for leaf node
//-------------------------------------------------------------------------------------------------
void LeafNode::borrowChildren(Node* borrower, Node* lender, size_t borrowIndex) {
	throw std::runtime_error("Unsupported operation");
}

//-------------------------------------------------------------------------------------------------
// Unsupported operation for leaf node
//-------------------------------------------------------------------------------------------------
Node* LeafNode::mergeChildren(Node* leftChild, Node* rightChild) {
	throw std::runtime_error("Unsupported operation");
}


//-------------------------------------------------------------------------------------------------
// Merge this leaf node with right sibling
//-------------------------------------------------------------------------------------------------
void LeafNode::mergeWithSibling(KEY key, Node* rightSibling) {

	LeafNode* siblingLeaf = (LeafNode*)rightSibling;

	// Copy keys and values at the tail of this node
	for (size_t i = 0; i < siblingLeaf->getKeyCount(); ++i) {
		keys.push_back(siblingLeaf->getKeyAt(i));
		values.push_back(siblingLeaf->getValueAt(i));
	}

	// Interconnect siblings links
	this->setRightSibling(siblingLeaf->rightSibling);
	if (siblingLeaf->rightSibling != nullptr) {
		siblingLeaf->rightSibling->setLeftSibling(this);
	}

	// Delete sibling node
	delete siblingLeaf;
}


//-------------------------------------------------------------------------------------------------
// Borrow child node at specified index from sibling and return new middle key
//-------------------------------------------------------------------------------------------------
KEY LeafNode::borrowFromSibling(KEY key, Node* sibling, size_t borrowIndex) {
	LeafNode* siblingNode = (LeafNode*)sibling;
	// insert borrowed key/value pair
	KEY borrowedKey = siblingNode->getKeyAt(borrowIndex);
	VALUE borrowedValue = siblingNode->getValueAt(borrowIndex);
	this->insertKey(borrowedKey, borrowedValue);
	// delete borrowed key/value pair in sibling node
	siblingNode->deleteAt(borrowIndex);
	// return new middle key
	KEY midKey = borrowIndex == 0 ? sibling->getKeyAt(0) : this->getKeyAt(0);
	return midKey;
}


//-------------------------------------------------------------------------------------------------
// Return NodeType::LEAF
//-------------------------------------------------------------------------------------------------
NodeType LeafNode::getNodeType() {
	return NodeType::LEAF;
}


//-------------------------------------------------------------------------------------------------
// Prints this leaf node key/value pairs
//-------------------------------------------------------------------------------------------------
void LeafNode::print(int level) {
	//cout << "-------- " << keys.size() << " records -------" << endl;
	for (size_t i = 0; i < keys.size(); i++) {
		printTabs(level);
		cout << keys[i] << " - " << values[i] << endl;
	}
	//cout << "-------------------------------\n";
}



