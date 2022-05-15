#include "BalancedTree.h"

#include <iostream>

using namespace Boson;
using namespace std;


LeafNode::LeafNode(size_t m) : Node(m) {
	values.reserve(this->keys.capacity() + 1);
}


LeafNode::~LeafNode() {
	values.clear();
}



VALUE LeafNode::getValueAt(int index) {
	return values[index];
}


void LeafNode::setValueAt(int index, VALUE value) {
	values[index] = value;
}


size_t LeafNode::search(KEY key) {
	for (size_t i = 0; i < keys.size(); i++) {
		if (keys[i] == key) return i;
	}
	return NOT_FOUND;
}


void LeafNode::insertKey(KEY key, VALUE value) {
	// find index to insert new key/value pair in sorted order
	size_t insertIndex = keys.size();
	for (int i = 0; i < keys.size(); i++) {
		if (key < keys[i]) {
			insertIndex = i;
			break;
		}
	}
	// insert key/value
	insertAt(insertIndex, key, value);

}


void LeafNode::insertAt(size_t index, KEY key, VALUE value) {
	keys.insert(keys.begin() + index, 1, key);
	values.insert(values.begin() + index, 1, value);
}


bool LeafNode::deleteKey(KEY key) {
	int deleteIndex = search(key);
	if (deleteIndex == NOT_FOUND) return false;
	return deleteAt(deleteIndex);;
}


bool LeafNode::deleteAt(int index) {
	keys.erase(keys.begin() + index);
	values.erase(values.begin() + index);
	return true;
}



Node* LeafNode::split() {
	size_t midIndex = keys.size() / 2;
	LeafNode* newNode = new LeafNode(this->keys.capacity());
	for (size_t i = midIndex; i < keys.size(); ++i) {
		newNode->insertKey(keys[i], values[i]);
	}
	keys.resize(midIndex);
	values.resize(midIndex);
	return newNode;
}


void LeafNode::merge(KEY sinkkey, Node* sibling) {
	LeafNode* siblingLeaf = (LeafNode*)sibling;

	for (size_t i = 0; siblingLeaf->getKeyCount(); i++) {
		keys.push_back(siblingLeaf->getKeyAt(i));
		values.push_back(siblingLeaf->getValueAt(i));
	}

	setRightSibling(siblingLeaf->rightSibling);
	if (siblingLeaf->rightSibling != nullptr) {
		siblingLeaf->rightSibling->setLeftSibling(this);
	}
}


Node* LeafNode::pushUpKey(KEY key, Node* leftChild, Node* rightChild) {
	throw exception("Unsupported operation");
	return nullptr;
}


Node* LeafNode::transferChildren(Node* borrower, Node* lender, int borrowIndex) {
	return nullptr;
}


Node* LeafNode::mergeChildren(Node* leftChild, Node* rightChild) {
	return nullptr;
}

KEY LeafNode::borrowFromSibling(KEY sinkKey, Node* sibling, int borrowIndex) {
	LeafNode* siblingNode = (LeafNode*)sibling;
	this->insertKey(siblingNode->getKeyAt(borrowIndex), siblingNode->getValueAt(borrowIndex));
	siblingNode->deleteAt(borrowIndex);
	return borrowIndex == 0 ? sibling->getKeyAt(0) : this->getKeyAt(0);
}


NodeType LeafNode::getNodeType() {
	return NodeType::LEAF;
}


void LeafNode::print() {
	cout << "-------- " << keys.size() << " records -------" << endl;
	for (int i = 0; i < keys.size(); i++) {
		cout << keys[i] << " - " << values[i] << endl;
	}
	cout << "-------------------------------\n";
}