#include "BalancedTree.h"
#include <iostream>

using namespace Boson;
using namespace std;


Node::Node(size_t m) {
	
	this->maxDegree = m - 1;
	this->minDegree = m / 2;
	this->parent = nullptr;
	this->leftSibling = nullptr;
	this->rightSibling = nullptr;
	keys.reserve(m);
}



Node::~Node() {
	keys.clear();
}



size_t Node::getKeyCount() {
	return keys.size();
}


bool Node::isOverflow() {
	return keys.size() > maxDegree;
}


bool Node::isUnderflow() {
	return keys.size() < minDegree;

}


bool Node::canLendAKey() {
	return keys.size() > minDegree;
}



KEY Node::getKeyAt(size_t index) {
	return keys[index];
}


void Node::setKeyAt(size_t index, KEY key) {
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
	size_t midIndex = getKeyCount() / 2;
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

// FIXME
Node* Node::dealUnderflow() {
	if (this->getParent() == nullptr) return nullptr;

	// try to borrow top key from left sibling
	Node* leftSibling = this->getLeftSibling();
	if (leftSibling != nullptr && leftSibling->canLendAKey()) {
		this->getParent()->transferChildren(this, leftSibling, leftSibling->getKeyCount() - 1);
		return nullptr;
	}

	// try to borrow lower key from rihgt sibling
	Node* rightSibling = this->getRightSibling();
	if (rightSibling != nullptr && rightSibling->canLendAKey() && rightSibling->parent==parent) {
		this->getParent()->transferChildren(this, rightSibling, 0);
		return nullptr;
	}

	// Can not borrow a key from any sibling, then do fusion with sibling
	// FIXME
	if (leftSibling != nullptr && leftSibling->parent==parent) {
		Node* parentNode = this->getParent();
		return parentNode->mergeChildren(leftSibling, this);
	}
	else {
		Node* parentNode = this->getParent();
		return parentNode->mergeChildren(this, rightSibling);
	}

}



void Node::printTabs(size_t n) {
	for (size_t i = 0; i < n; i++) {
		cout << "\t";
	}
}
