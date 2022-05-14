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
	keys.reserve(maxDegree);
}



Node::~Node() {
	keys.clear();
}



int Node::getKeyCount() {
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
	// todo
	return 0;
}


Node* Node::dealUnderflow() {
	// todo
	return 0;
}
