#include "BalancedTree.h"
#include <iostream>

using namespace Boson;
using namespace std;


InnerNode::InnerNode(size_t m) : Node(m) {
	children.reserve(keys.capacity() + 2);
}


InnerNode::~InnerNode() {
	for (auto child : children) {
		delete child;
	}
	children.clear();
}


Node* InnerNode::getChild(size_t index) {
	return children[index];
}


void InnerNode::setChild(size_t index, Node* childNode) {
	children[index] = childNode;
}


size_t InnerNode::search(KEY key) {
	for (size_t i = 0; i < keys.size(); i++) {
		if (keys[i] == key) return i;
	}
	return NOT_FOUND;
}


void InnerNode::insertAt(size_t index, KEY key, Node* leftChild, Node* rightChild) {
	// todo check logic
	keys.insert(keys.begin() + index, key);
	children.insert(children.begin() + index, leftChild);
	children[index + 1] = rightChild;
}




Node* InnerNode::split() {
	// When splits an internal node, the middle key is kicked out and pushed to parent node.

	size_t midIndex = this->getKeyCount() / 2;

	InnerNode* newNode = new InnerNode(this->keys.capacity());

	for (size_t i = midIndex + 1; i < keys.size(); ++i) {
		newNode->keys.push_back(this->keys[i]);                      // copy keys to new node
	}
	this->keys.resize(midIndex);                                     // truncate keys list

	for (size_t i = midIndex + 1; i <= keys.size(); ++i) {
		newNode->children.push_back(this->children[i]);              // move children to the new node
		newNode->children[i - midIndex - 1]->setParent(newNode);     // reattach children to new parent
	}	
	this->children.resize(midIndex + 1);                             // truncate children list
	
	return newNode;

}



Node* InnerNode::pushUpKey(KEY key, Node* leftChild, Node* rightChild) {
	size_t index = search(key);
	insertAt(index, key, leftChild, rightChild);
	if (isOverflow())
		return dealOverflow();
	else
		return getParent() == nullptr ? this : nullptr;
}


void InnerNode::transferChildren(Node* borrower, Node* lender, size_t borrowIndex) {
	size_t borrowerChildIndex = 0;
	while ((borrowerChildIndex < keys.size() + 1) && children[borrowerChildIndex] != borrower) {
		borrowerChildIndex++;
	}
	if (borrowIndex == 0) {
		KEY upKey = borrower->borrowFromSibling(keys[borrowerChildIndex], lender, borrowIndex);
		keys[borrowerChildIndex] = upKey;
	}
	else {
		KEY upKey = borrower->borrowFromSibling(keys[borrowerChildIndex - 1], lender, borrowIndex);
		keys[borrowerChildIndex - 1] = upKey;
	}
}


Node* InnerNode::mergeChildren(Node* leftChild, Node* rightChild) {
	size_t index = 0;
	while (index < keys.size() && children[index] != leftChild) index++;
	KEY sinkKey = keys[index];
	// merge two children and the sink key into the left child node
	leftChild->mergeWithSibling(sinkKey, rightChild);
	// remove the sink key, keep the left child and abandon the right child
	deleteAt(index);
	// check whether need to propagate borrow or fusion to parent
	if (isUnderflow()) {
		if (getParent() == nullptr) {
			if (keys.size() == 0) {
				leftChild->setParent(nullptr);
				return leftChild;
			} else {
				return nullptr;
			}

		}
		return dealUnderflow();
	}
	return nullptr;
}


void  InnerNode::mergeWithSibling(KEY key, Node* rightSiblingNode) {

	InnerNode* rightSibling = (InnerNode*)rightSiblingNode;
	size_t j = keys.size();
	
	// TODO:

}

KEY InnerNode::borrowFromSibling(KEY sinkKey, Node* sibling, size_t borrowIndex) {
	// TODO:

	return 0;
}


void InnerNode::deleteAt(size_t index) {
	keys.erase(keys.begin() + index);
	children.erase(children.begin() + index + 1);
}


inline NodeType InnerNode::getNodeType() {
	return NodeType::INNER;
}

