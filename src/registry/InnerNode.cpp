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


Node* InnerNode::getChild(int index) {
	return children[index];
}


void InnerNode::setChild(int index, Node* childNode) {
	children[index] = childNode;
}


size_t InnerNode::search(KEY key) {
	for (size_t i = 0; i < keys.size(); i++) {
		if (keys[i] == key) return i;
	}
	return NOT_FOUND;
}


void InnerNode::insertAt(int index, KEY key, Node* leftChild, Node* rightChild) {
	// todo check logic
	keys.insert(keys.begin() + index, key);
	children.insert(children.begin() + index, leftChild);
	children.insert(children.begin() + index + 1, rightChild);
}



Node* InnerNode::split() {
	// TODO
	return nullptr;

}


void InnerNode::merge(KEY sinkkey, Node* sibling) {
	// TODO
}


Node* InnerNode::pushUpKey(KEY key, Node* leftChild, Node* rightChild) {
	// TODO
	return nullptr;
}


Node* InnerNode::transferChildren(Node* borrower, Node* lender, int borrowIndex) {
	// TODO
	return nullptr;
}


Node* InnerNode::mergeChildren(Node* leftChild, Node* rightChild) {
	// TODO
	return nullptr;
}


KEY InnerNode::borrowFromSibling(KEY sinkKey, Node* sibling, int borrowIndex) {
	// TODO
	return 0;
}


void InnerNode::deleteAt(int index) {
	// TODO

}


inline NodeType InnerNode::getNodeType() {
	return NodeType::INNER;
}

