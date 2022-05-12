#include "BalancedTree.h"
#include <iostream>

using namespace Boson;
using namespace std;


Node::Node(int m) {
	
	this->maxDegree = m - 1;
	this->minDegree = m / 2 + 1;
	this->degree = 0;
	this->parent = nullptr;
	this->leftSibling = nullptr;
	this->rightSibling = nullptr;
	keys.reserve(maxDegree);
}



Node::~Node() {
	keys.clear();
}



inline int Node::getKeyCount() {
	return degree;
}


inline bool Node::isOverflow() {
	return degree >= maxDegree;
}


inline bool Node::isUnderflow() {
	return degree < minDegree;

}


inline bool Node::canLendAKey() {
	return degree > minDegree;
}


inline KEY Node::getKeyAt(int index) {
	return keys[index];
}


inline void Node::setKeyAt(int index, KEY key) {
	keys[index] = key;
}


inline Node* Node::getParent() {
	return parent;
}


inline void Node::setParent(Node* parent) {
	this->parent = parent;
}


inline Node* Node::getLeftSibling() {
	return leftSibling;
}


inline void Node::setLeftSibling(Node* leftSibling) {
	this->leftSibling = leftSibling;
}


inline Node* Node::getRightSibling() {
	return rightSibling;
}


inline void Node::setRightSibling(Node* rightSibling) {
	this->rightSibling = rightSibling;
}


Node* Node::dealOverflow() {
	// todo
	return 0;
}


Node* Node::dealUnderflow() {
	// todo
	return 0;
}
