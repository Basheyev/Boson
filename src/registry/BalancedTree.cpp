#include "BalancedTree.h"

using namespace Boson;


BalancedTree::BalancedTree(int order) {
	if (order < MINIMAL_TREE_ORDER) order = MINIMAL_TREE_ORDER;
	this->treeOrder = order;
	root = new InnerNode(order);
}


BalancedTree::~BalancedTree() {
	delete root;
}




void BalancedTree::insert(KEY key, VALUE value) {

}


VALUE BalancedTree::search(KEY key) {
	return 0;
}


bool BalancedTree::erase(KEY key) {


	return false;
}


int BalancedTree::getTreeOrder() {
	return this->treeOrder;
}

Node* BalancedTree::getRoot() {
	return root;
}