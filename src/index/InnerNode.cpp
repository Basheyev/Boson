#include "BalancedTree.h"


using namespace Boson;


InnerNode::InnerNode() {
	children.reserve(NODE_CAPACITY);
}


//-------------------------------------------------------------------------------------------------
// Inner Node Destructor
//-------------------------------------------------------------------------------------------------
InnerNode::~InnerNode() {
	children.clear();
}


//-------------------------------------------------------------------------------------------------
// Returns child node at specified index
//-------------------------------------------------------------------------------------------------
Node* InnerNode::getChildAt(size_t index) {
	return children[index];
}


//-------------------------------------------------------------------------------------------------
// Sets child node at specified index
//-------------------------------------------------------------------------------------------------
void InnerNode::setChildAt(size_t index, Node* childNode) {
	children[index] = childNode;
}


//-------------------------------------------------------------------------------------------------
// Return child node index for specified key
//-------------------------------------------------------------------------------------------------
size_t InnerNode::search(KEY key) {
	size_t index = 0;
	KEY entry;

	// Look up for a child node index that contains the key
	for (index = 0; index < keys.size(); index++) {
		entry = keys[index];                   // get keys entry at specified index
		if (key == entry) return index + 1;    // if key is found then return right child index
		else if (key < entry) return index;    // if key is less than entry left child index
	}

	return index;
}


//-------------------------------------------------------------------------------------------------
// Insert key with left and right child at specified index
//-------------------------------------------------------------------------------------------------
void InnerNode::insertAt(size_t index, KEY key, Node* leftChild, Node* rightChild) {

	// Insert the key at specified index
	keys.insert(keys.begin() + index, key);

	// Insert left child at specified index
	children.insert(children.begin() + index, leftChild);

	// Right child index
	size_t rightChildIndex = index + 1;

	// If there is child at this index then
	if (rightChildIndex < children.size())
		children[rightChildIndex] = rightChild;   // overwrite child value to new rightChild
	else
		children.push_back(rightChild);       // add right child to the end of children list

}



//-------------------------------------------------------------------------------------------------
// Split this inner node by half
//-------------------------------------------------------------------------------------------------
Node* InnerNode::split() {

	// Calculate mid index
	size_t midIndex = this->getKeyCount() / 2;

	// Create new node
	InnerNode* newNode = new InnerNode();

	// Copy keys from this node to new splitted node
	for (size_t i = midIndex + 1; i < keys.size(); ++i) {
		// copy keys to new node
		newNode->keys.push_back(this->keys[i]);
	}
	// truncate this node's keys list
	this->keys.resize(midIndex);

	// Copy childrens from this node to new splitted node
	for (size_t i = midIndex + 1; i < children.size(); ++i) {
		// reattach children to new splitted node
		this->children[i]->setParent(newNode);
		// copy childrens to the new node
		newNode->children.push_back(this->children[i]);
	}
	// truncate this node's children list
	this->children.resize(midIndex + 1);

	// return splitted node
	return newNode;

}


//-------------------------------------------------------------------------------------------------
// Set key at specified index propogated from child
//-------------------------------------------------------------------------------------------------
Node* InnerNode::pushUpKey(KEY key, Node* leftChild, Node* rightChild) {

	// search key index in this node
	size_t index = search(key);

	// insert key at specified index with left and right child
	insertAt(index, key, leftChild, rightChild);

	// if there is a node overflow
	if (isOverflow()) return dealOverflow();

	// if this is the root node return this pointer
	if (getParent() == nullptr) return this;

	// return null pointer
	return nullptr;
}


//-------------------------------------------------------------------------------------------------
// Borrow children by specifying Borrower, Lender and borrow index
//-------------------------------------------------------------------------------------------------
void InnerNode::borrowChildren(Node* borrower, Node* lender, size_t borrowIndex) {
	size_t borrowerChildIndex = 0;

	// find borrower child index
	for (int i = 0; i < children.size(); i++) {
		if (children[i] == borrower) {
			borrowerChildIndex = i;
			break;
		}
	}

	// Process borrowing
	if (borrowIndex == 0) {
		// borrow from right sibling
		KEY theKey = keys[borrowerChildIndex];
		KEY upKey = borrower->borrowFromSibling(theKey, lender, borrowIndex);
		keys[borrowerChildIndex] = upKey;
	}
	else {
		// borrow from left sibling
		KEY theKey = keys[borrowerChildIndex - 1];
		KEY upKey = borrower->borrowFromSibling(theKey, lender, borrowIndex);
		keys[borrowerChildIndex - 1] = upKey;
	}
}


//-------------------------------------------------------------------------------------------------
// Merge children of InnerNode by specifying left and right child
//-------------------------------------------------------------------------------------------------
Node* InnerNode::mergeChildren(Node* leftChild, Node* rightChild) {
	size_t index = 0;

	// Find corresponding key index of left child

	// FIXME: What if left child doesnt belong to this node? Causes wrong pointer access
	while (index < children.size() - 1) {
		if (children[index] == leftChild) break;
		index++;
	}
	KEY key = keys[index];

	// Merge two children and push key into the left child node
	leftChild->mergeWithSibling(key, rightChild);

	// Remove the key, keep the left child and abandon the right child
	this->deleteAt(index);

	// If there is underflow propagate borrow or merge to parent
	if (this->isUnderflow()) {
		// If this node is root node (no parent)
		if (getParent() == nullptr) {
			// if this node is empy
			if (keys.size() == 0) {
				leftChild->setParent(nullptr);
				return leftChild;
			}
			else return nullptr;
		}
		return dealUnderflow();
	}
	return nullptr;
}


//-------------------------------------------------------------------------------------------------
// Merge with sibling InnerNode by specifying key and right sibling
//-------------------------------------------------------------------------------------------------
void InnerNode::mergeWithSibling(KEY key, Node* rightSiblingNode) {
	InnerNode* rightSibling = (InnerNode*)rightSiblingNode;
	Node* siblingChild;

	// Push key into keys
	this->keys.push_back(key);

	// Copy sibling keys
	for (size_t i = 0; i < rightSibling->getKeyCount(); ++i) {
		this->keys.push_back(rightSibling->getKeyAt(i));
	}

	// Copy sibling children
	for (size_t i = 0; i < rightSibling->getKeyCount() + 1; ++i) {
		// get sibling's child
		siblingChild = rightSibling->getChildAt(i);
		// reattach sibling's child to this node
		siblingChild->setParent(this);
		// copy sibling child to this node
		children.push_back(siblingChild);
	}

	// Interrconnect siblings
	this->setRightSibling(rightSibling->rightSibling);
	if (rightSibling->rightSibling != nullptr) {
		rightSibling->rightSibling->setLeftSibling(this);
	}

	// Clear and delete right sibling
	rightSibling->keys.clear();
	rightSibling->children.clear();
	delete rightSibling;

}


//-------------------------------------------------------------------------------------------------
// Borrow key with children from sibling node
//-------------------------------------------------------------------------------------------------
KEY InnerNode::borrowFromSibling(KEY key, Node* sibling, size_t borrowIndex) {
	InnerNode* siblingNode = (InnerNode*)sibling;
	Node* childNode;
	KEY upKey = 0;

	if (borrowIndex == 0) {
		// borrow the first key from right sibling, append it to tail	
		// get sibling child node
		childNode = siblingNode->getChildAt(borrowIndex);
		// reattach childNode to this node as parent
		childNode->setParent(this);
		// append borrowed key and child node to the tail of list
		keys.push_back(key);
		children.push_back(childNode);
		// get key propogated to parent node
		upKey = siblingNode->getKeyAt(0);
		// delete key with children from sibling node
		siblingNode->keys.erase(siblingNode->keys.begin());
		siblingNode->children.erase(siblingNode->children.begin());
	}
	else {
		// borrow the last key from left sibling, insert it to head
		childNode = siblingNode->getChildAt(borrowIndex + 1);
		// reattach childNode to this node as parent
		childNode->setParent(this);
		// insert borrowed key and child node to the list at beginning
		insertAt(0, key, childNode, children[0]);
		// get key propogated to parent node
		upKey = siblingNode->getKeyAt(borrowIndex);
		// delete key with children from sibling node
		siblingNode->deleteAt(borrowIndex);
	}

	return upKey;
}


//-------------------------------------------------------------------------------------------------
// Delete key and right children at specified index
//-------------------------------------------------------------------------------------------------
void InnerNode::deleteAt(size_t index) {
	keys.erase(keys.begin() + index);
	children.erase(children.begin() + index + 1);
}


//-------------------------------------------------------------------------------------------------
// Return node type NodeType::INNER
//-------------------------------------------------------------------------------------------------
NodeType InnerNode::getNodeType() {
	return NodeType::INNER;
}


//-------------------------------------------------------------------------------------------------
// Print inner node children
//-------------------------------------------------------------------------------------------------
void InnerNode::print(int level) {
	Node* leftChild;
	Node* rightChild = nullptr;
	Node* prevRightChild = nullptr;
	for (size_t i = 0; i < keys.size(); i++) {
		leftChild = children[i];
		if (leftChild != prevRightChild) leftChild->print(level + 1);
		printTabs(level);
		std::cout << keys[i] << std::endl;
		if (i + 1 < children.size()) rightChild = children[i + 1];
		if (rightChild != nullptr) {
			rightChild->print(level + 1);
		}
		prevRightChild = rightChild;
	}
}
