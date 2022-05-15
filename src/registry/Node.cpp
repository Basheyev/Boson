#include "BalancedTree.h"
#include <iostream>

using namespace Boson;
using namespace std;


Node::Node(size_t m) {
	
	this->maxDegree = m - 1;
	this->minDegree = m / 2 + 1;
	this->degree = 0;
	this->parent = nullptr;
	this->leftSibling = nullptr;
	this->rightSibling = nullptr;
	keys.reserve(m);
}



Node::~Node() {
	keys.clear();
}



size_t Node::getKeyCount() {
	return degree;
}


bool Node::isOverflow() {
	return degree >= maxDegree;
}


bool Node::isUnderflow() {
	return degree < minDegree;

}


bool Node::canLendAKey() {
	return degree > minDegree;
}



KEY Node::getKeyAt(int index) {
	return keys[index];
}


void Node::setKeyAt(int index, KEY key) {
	keys[index] = key;
}


Node* Node::getParent() {
	return parent;
}


void Node::setParent(Node* parent) {
	this->parent = parent;
}


Node* Node::getLeftSibling() {
	return leftSibling;
}


void Node::setLeftSibling(Node* leftSibling) {
	this->leftSibling = leftSibling;
}


Node* Node::getRightSibling() {
	return rightSibling;
}


void Node::setRightSibling(Node* rightSibling) {
	this->rightSibling = rightSibling;
}


Node* Node::dealOverflow() {
	int midIndex = getKeyCount() / 2;
	KEY upKey = getKeyAt(midIndex);

	Node* newRightNode = this->split();
	if (getParent() == nullptr) {
		setParent(new InnerNode(maxDegree + 1));
	}
	newRightNode->setParent(this->getParent());

	newRightNode->setLeftSibling(this);
	newRightNode->setRightSibling(this->rightSibling);
	if (this->getRightSibling() != nullptr) {
		this->getRightSibling()->setLeftSibling(newRightNode);
	}
	this->setRightSibling(newRightNode);

	return this->getParent()->pushUpKey(upKey, this, newRightNode);
}


Node* Node::dealUnderflow() {
	if (this->getParent() == nullptr) return nullptr;

	Node* leftSibling = this->getLeftSibling();
	if (leftSibling != nullptr && leftSibling->canLendAKey()) {
		this->getParent()->transferChildren(this, rightSibling, 0);
		return nullptr;
	}

	Node* rightSibling = this->getRightSibling();
	if (rightSibling != nullptr && rightSibling->canLendAKey()) {
		this->getParent()->transferChildren(this, rightSibling, 0);
		return nullptr;
	}

	// Can not borrow a key from any sibling, then do fusion with sibling
	if (leftSibling != nullptr) {
		return this->getParent()->mergeChildren(leftSibling, this);
	}
	else {
		return this->getParent()->mergeChildren(this, rightSibling);
	}

}
