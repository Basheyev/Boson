#include "BalancedTree.h"
#include <iostream>

using namespace Boson;
using namespace std;

InnerNode::InnerNode(size_t m) : Node(m) {
	children.reserve(keys.capacity() + 1);
	
}


InnerNode::~InnerNode() {
	for (auto child : children) {
		delete child;
	}
	children.clear();
}


inline Node* InnerNode::getChild(int index) {
	return children[index];
}


inline void InnerNode::setChild(int index, Node* childNode) {
	children[index] = childNode;
}


int InnerNode::search(KEY key) {
	for (int i = 0; i < keys.size(); i++) {
		if (keys[i] == key) return i;
	}
	return NOT_FOUND;
}


void InnerNode::insertAt(int index, Node*, Node*) {


}

void InnerNode::deleteAt(int index) {


}


inline NodeType InnerNode::getNodeType() {
	return NodeType::INNER;
}


Node* InnerNode::split() {
	return nullptr;
}

void InnerNode::merge(KEY sinkkey, Node* sibling) {

}