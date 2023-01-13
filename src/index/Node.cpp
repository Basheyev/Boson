#include "BalancedTree.h"


using namespace Boson;

//-------------------------------------------------------------------------------------------------
	// Node Constructor
	// - M   - maximum Child nodes per Inner node
	// - M-1 - maximum Keys count per Inner node and Key-Value pairs per Leaf node
	// - M/2 - minimal keys count per Inner node and Key-Value pairs per Leaf node
	//-------------------------------------------------------------------------------------------------
Node::Node() {
	//this->parent = nullptr;
    //this->leftSibling = nullptr;
	//this->rightSibling = nullptr;
	//keys.reserve(NODE_CAPACITY);

}


//-------------------------------------------------------------------------------------------------
// Node Destructor
//-------------------------------------------------------------------------------------------------
Node::~Node() {
	this->keysCount = 0;
	this->parent = NOT_FOUND;
	this->leftSibling = NOT_FOUND;
	this->rightSibling = NOT_FOUND;	
}


//-------------------------------------------------------------------------------------------------
// Returns Keys count inside Node
//-------------------------------------------------------------------------------------------------
size_t Node::getKeyCount() {
	return keysCount;
}


//-------------------------------------------------------------------------------------------------
// Returns whether node keys count > M-1
//-------------------------------------------------------------------------------------------------
bool Node::isOverflow() {
	return keysCount > MAX_DEGREE;
}


//-------------------------------------------------------------------------------------------------
// Returns whether node keys count < M / 2
//-------------------------------------------------------------------------------------------------
bool Node::isUnderflow() {
	return keysCount < MIN_DEGREE;

}


//-------------------------------------------------------------------------------------------------
// Returns whether node keys count > M / 2
//-------------------------------------------------------------------------------------------------
bool Node::canLendAKey() {
	return keysCount > MIN_DEGREE;
}


//-------------------------------------------------------------------------------------------------
// Returns key at specified index
//-------------------------------------------------------------------------------------------------
KEY Node::getKeyAt(size_t index) {
	return pairs[index].key;
}


//-------------------------------------------------------------------------------------------------
// Sets key at specified index
//-------------------------------------------------------------------------------------------------
void Node::setKeyAt(size_t index, KEY key) {
	pairs[index].key = key;
}


//-------------------------------------------------------------------------------------------------
// Returns parent node or nullptr if it is root node
//-------------------------------------------------------------------------------------------------
OFFSET Node::getParent() {
	return parent;
}


//-------------------------------------------------------------------------------------------------
// Sets node parent node
//-------------------------------------------------------------------------------------------------
void Node::setParent(OFFSET parent) {
	this->parent = parent;
}


//-------------------------------------------------------------------------------------------------
// Returns node's left sibling node
//-------------------------------------------------------------------------------------------------
OFFSET Node::getLeftSibling() {
	return leftSibling;
}


//-------------------------------------------------------------------------------------------------
// Sets node's left sibling node
//-------------------------------------------------------------------------------------------------
void Node::setLeftSibling(OFFSET leftSibling) {
	this->leftSibling = leftSibling;
}


//-------------------------------------------------------------------------------------------------
// Returns node right sibling node
//-------------------------------------------------------------------------------------------------
OFFSET Node::getRightSibling() {
	return rightSibling;
}


//-------------------------------------------------------------------------------------------------
// Sets node's right sibling node
//-------------------------------------------------------------------------------------------------
void Node::setRightSibling(OFFSET rightSibling) {
	this->rightSibling = rightSibling;
}


//-------------------------------------------------------------------------------------------------
// Process node overflow scenarios
//-------------------------------------------------------------------------------------------------
OFFSET Node::dealOverflow() {

	// Get key at middle index for propagation to the parent node
	size_t midIndex = this->getKeyCount() / 2;
	KEY upKey = this->getKeyAt(midIndex);

	// Split this node by half (returns new splitted node)
	Node* newRightNode = this->split();

	// if we are splitting the root node
	if (getParent() == NOT_FOUND) {
		// create new root node and set as parent to this node (grow at root)
		OFFSET newRootNode = new InnerNode();
		this->setParent(newRootNode);
	}

	// Interconnect splitted node's parent and siblings
	newRightNode->setParent(this->getParent());
	newRightNode->setLeftSibling(this);
	newRightNode->setRightSibling(this->rightSibling);
	if (this->getRightSibling() != nullptr) {
		this->getRightSibling()->setLeftSibling(newRightNode);
	}
	this->setRightSibling(newRightNode);

	// Push middle key up to parent the node (root node returned)
	Node* rootNode = this->parent->pushUpKey(upKey, this, newRightNode);

	// Return current root node
	return rootNode;
}


//-------------------------------------------------------------------------------------------------
// Process node underoverflow scenarios
//-------------------------------------------------------------------------------------------------
Node* Node::dealUnderflow() {

	// if this is the root node, then do nothing and return
	if (this->getParent() == nullptr) return nullptr;

	// 1. Try to borrow top key from left sibling
	if (leftSibling != nullptr && leftSibling->canLendAKey() && leftSibling->parent == parent) {
		size_t keyIndex = leftSibling->getKeyCount() - 1;
		this->parent->borrowChildren(this, leftSibling, keyIndex);
		return nullptr;
	}

	// 2. Try to borrow lower key from right sibling
	if (rightSibling != nullptr && rightSibling->canLendAKey() && rightSibling->parent == parent) {
		size_t keyIndex = 0;
		this->parent->borrowChildren(this, rightSibling, keyIndex);
		return nullptr;
	}

	if (leftSibling != nullptr && leftSibling->parent == parent) {
		// 3. Try to merge with left sibling
		Node* rootNode = parent->mergeChildren(leftSibling, this);
		return rootNode;
	}
	else {
		// 4. Try to merge with right sibling
		Node* rootNode = parent->mergeChildren(this, rightSibling);
		return rootNode;
	}

}


//-------------------------------------------------------------------------------------------------
// Print N tabls to standart out
//-------------------------------------------------------------------------------------------------
void Node::printTabs(size_t n) {
	for (size_t i = 0; i < n; i++) {
		std::cout << "\t";
	}
}
