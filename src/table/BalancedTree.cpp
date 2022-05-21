#include "BalancedTree.h"
#include <iostream>

using namespace Boson;
using namespace std;


BalancedTree::BalancedTree(size_t order) {
	if (order < MINIMAL_TREE_ORDER) order = MINIMAL_TREE_ORDER;
	this->treeOrder = order;
	root = new LeafNode(order);
}


BalancedTree::~BalancedTree() {
	delete root;
}




void BalancedTree::insert(KEY key, VALUE value) {
	cout << "Inserting " << key << " - " << value << '\n';
	cout << "------------------------" << endl;
	LeafNode* leaf = findLeaf(key);
	leaf->insertKey(key, value);
	if (leaf->isOverflow()) {
		Node* n = leaf->dealOverflow();
		if (n != nullptr) root = n;
	}
}


VALUE BalancedTree::search(KEY key) {
	LeafNode* leaf = findLeaf(key);
	size_t index = leaf->search(key);
	return (index == NOT_FOUND) ? nullptr : leaf->getValueAt(index);
}


bool BalancedTree::erase(KEY key) {
	LeafNode* leaf = findLeaf(key);
	if (leaf->deleteKey(key) && leaf->isUnderflow()) {
		Node* n = leaf->dealUnderflow();
		if (n != nullptr) root = n;
	}
	return false;
}


LeafNode* BalancedTree::findLeaf(KEY key) {
	// fixme
	Node* node = root;
	InnerNode* innerNode;
	size_t index;
	while (node->getNodeType() == NodeType::INNER) {
		index = node->search(key);
		innerNode = (InnerNode*)node;
		node = innerNode->getChild(index);
	}
	return (LeafNode*)node;
}


size_t BalancedTree::getTreeOrder() {
	return this->treeOrder;
}


Node* BalancedTree::getRoot() {
	return root;
}


void BalancedTree::print() {
	root->print(0);
	cout << "=======================================" << endl;
}

