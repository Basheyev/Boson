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



VALUE LeafNode::getValueAt(size_t index) {
	return values[index];
}


void LeafNode::setValueAt(size_t index, VALUE value) {
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


void LeafNode::insertAt(size_t index, KEY key, VALUE value) {
	keys.insert(keys.begin() + index, 1, key);
	values.insert(values.begin() + index, 1, value);
}


bool LeafNode::deleteKey(KEY key) {
	size_t deleteIndex = search(key);
	if (deleteIndex == NOT_FOUND) return false;
	deleteAt(deleteIndex);
	return true;
}


void LeafNode::deleteAt(size_t index) {
	keys.erase(keys.begin() + index);
	values.erase(values.begin() + index);
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
	throw std::runtime_error("Unsupported operation");
}


void LeafNode::transferChildren(Node* borrower, Node* lender, size_t borrowIndex) {
	throw std::runtime_error("Unsupported operation");
}


Node* LeafNode::mergeChildren(Node* leftChild, Node* rightChild) {
	throw std::runtime_error("Unsupported operation");
}


void LeafNode::mergeWithSibling(KEY key, Node* rightSibling) {
	LeafNode* siblingLeaf = (LeafNode*)rightSibling;
	size_t j = keys.size();

	for (size_t i = 0; i < siblingLeaf->getKeyCount(); ++i) {
		keys.push_back(siblingLeaf->getKeyAt(i));
		values.push_back(siblingLeaf->getValueAt(i));
	}

	this->setRightSibling(siblingLeaf->rightSibling);
	if (siblingLeaf->rightSibling != nullptr) siblingLeaf->rightSibling->setLeftSibling(this);

	delete rightSibling;

}



KEY LeafNode::borrowFromSibling(KEY sinkKey, Node* sibling, size_t borrowIndex) {
	LeafNode* siblingNode = (LeafNode*)sibling;
	this->insertKey(siblingNode->getKeyAt(borrowIndex), siblingNode->getValueAt(borrowIndex));
	siblingNode->deleteAt(borrowIndex);
	return borrowIndex == 0 ? sibling->getKeyAt(0) : this->getKeyAt(0);
}


NodeType LeafNode::getNodeType() {
	return NodeType::LEAF;
}


void LeafNode::print(int level) {
	//cout << "-------- " << keys.size() << " records -------" << endl;
	for (size_t i = 0; i < keys.size(); i++) {
		printTabs(level);
		cout << keys[i] << " - " << values[i] << endl;
	}
	//cout << "-------------------------------\n";
}



