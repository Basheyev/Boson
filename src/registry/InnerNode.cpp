#include "BalancedTree.h"
#include <iostream>

using namespace Boson;
using namespace std;

InnerNode::InnerNode(int m) : Node(m) {
	children.reserve(m);
	
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
	return 0;
}


void InnerNode::insertAt(int index, Node*, Node*) {


}

void InnerNode::deleteAt(int index) {


}


inline NodeType InnerNode::getNodeType() {
	return NodeType::INNER;
}

