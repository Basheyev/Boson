#include "BalancedTree.h"


using namespace Boson;

BalancedTree::BalancedTree() {
	this->entriesCount = 0;
	root = new LeafNode();
}


//-------------------------------------------------------------------------------------------------
// Balanced Tree Desctructor
//-------------------------------------------------------------------------------------------------
BalancedTree::~BalancedTree() {
	delete root;
}


//-------------------------------------------------------------------------------------------------
// Insert key/value pair
//-------------------------------------------------------------------------------------------------
bool BalancedTree::insert(KEY key, VALUE value) {
	LeafNode* leaf = findLeafNode(key);
	bool isInserted = leaf->insertKey(key, value);
	if (leaf->isOverflow()) {
		Node* n = leaf->dealOverflow();
		if (n != nullptr) root = n;
	}
	entriesCount++;
	return isInserted;
}


//-------------------------------------------------------------------------------------------------
// Search value by key (binary search)
//-------------------------------------------------------------------------------------------------
VALUE BalancedTree::search(KEY key) {
	LeafNode* leaf = findLeafNode(key);
	size_t index = leaf->search(key);
	// FIXME: For non-pointer types returning nullptr is invalid 
	return (index == NOT_FOUND) ? 0 : leaf->getValueAt(index);
}


//-------------------------------------------------------------------------------------------------
// Delete key/value pair
//-------------------------------------------------------------------------------------------------
bool BalancedTree::erase(KEY key) {

	LeafNode* leaf = findLeafNode(key);

	if (leaf->deleteKey(key)) {
		entriesCount--;
		if (leaf->isUnderflow()) {
			Node* n = leaf->dealUnderflow();
			if (n != nullptr) {
				n->setParent(nullptr);
				root = n;
			}
		}
	}
	return true;
}


//-------------------------------------------------------------------------------------------------
// Search for LeafNode that contains specified key
//-------------------------------------------------------------------------------------------------
LeafNode* BalancedTree::findLeafNode(KEY key) {
	Node* node = root;
	InnerNode* innerNode;
	size_t index;
	while (node->getNodeType() == NodeType::INNER) {
		index = node->search(key);
		innerNode = (InnerNode*)node;
		node = innerNode->getChildAt(index);
	}
	return (LeafNode*)node;
}


//-------------------------------------------------------------------------------------------------
// Return tree order M
//-------------------------------------------------------------------------------------------------
size_t BalancedTree::getTreeOrder() {
	return TREE_ORDER;
}


//-------------------------------------------------------------------------------------------------
// Return tree height
//-------------------------------------------------------------------------------------------------
size_t BalancedTree::getTreeHeight() {
	size_t levelCounter = 0;
	Node* firstLeaf = root;
	while (firstLeaf->getNodeType() == NodeType::INNER) {
		firstLeaf = ((InnerNode*)firstLeaf)->getChildAt(0);
		levelCounter++;
	}
	return levelCounter;
}


//-------------------------------------------------------------------------------------------------
// Returns records count
//-------------------------------------------------------------------------------------------------
size_t BalancedTree::getEntriesCount() {
	return entriesCount;
}

//-------------------------------------------------------------------------------------------------
// Returns tree root
//-------------------------------------------------------------------------------------------------
Node* BalancedTree::getRoot() {
	return root;
}



void BalancedTree::printTree() {
	std::cout << "----------------------------------------" << std::endl;
	root->print(0);
}


void BalancedTree::printContent() {
	std::cout << "----------------------------------------" << std::endl;
	Node* firstLeaf = root;
	// go down tree
	while (firstLeaf->getNodeType() == NodeType::INNER) {
		firstLeaf = ((InnerNode*)firstLeaf)->getChildAt(0);
	}
	// print list
	while (firstLeaf != nullptr) {
		firstLeaf->print(0);
		firstLeaf = firstLeaf->getRightSibling();
	}
}
