/*=================================================================================================
*
*    Balanced Plus Tree Inner Node Implementation
*
*    Inner Node (Index Page) Implementation
*
*    BOSON embedded database
*    (C) Bolat Basheyev 2022
*
=================================================================================================*/
#include "BalancedTree.h"
#include <iostream>

using namespace Boson;
using namespace std;

//-------------------------------------------------------------------------------------------------
// Inner Node Constructor (calls Node Constructor)
// - M   - maximum Child nodes per Inner node
// - M-1 - maximum Keys count per Inner node
// - M/2 - minimal keys count per Inner node
//-------------------------------------------------------------------------------------------------
InnerNode::InnerNode(size_t m) : Node(m) {
	children.reserve(m);
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
Node* InnerNode::getChild(size_t index) {
	return children[index];
}


//-------------------------------------------------------------------------------------------------
// Sets child node at specified index
//-------------------------------------------------------------------------------------------------
void InnerNode::setChild(size_t index, Node* childNode) {
	children[index] = childNode;
}


//-------------------------------------------------------------------------------------------------
// Return index of specified key or return index before next greater key
//-------------------------------------------------------------------------------------------------
size_t InnerNode::search(KEY key) {
	size_t index = 0;
	for (index = 0; index < keys.size(); index++) {
		if (keys[index] == key) 
			return index + 1;
		else if (keys[index] > key) {
			return index;
		}
	}
	return index;
}


//-------------------------------------------------------------------------------------------------
// Insert key with left and right child at specified index
//-------------------------------------------------------------------------------------------------
void InnerNode::insertAt(size_t index, KEY key, Node* leftChild, Node* rightChild) {
	// FIXME
	keys.insert(keys.begin() + index, key);	
	children.insert(children.begin() + index, leftChild);
	if (index + 1 >= children.size()) {
		children.insert(children.begin() + index + 1, rightChild);
	} else {
		children[index + 1] = rightChild;
	}
	
}



//-------------------------------------------------------------------------------------------------
// Split this inner node by half
//-------------------------------------------------------------------------------------------------
Node* InnerNode::split() {

	// Calculate mid index
	size_t midIndex = this->getKeyCount() / 2;
	// Create new node
	InnerNode* newNode = new InnerNode(this->treeOrder);

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
	// if there is node overflow
	if (isOverflow())
		return dealOverflow();
	else
		return getParent() == nullptr ? this : nullptr;
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
	} else {
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
			} else return nullptr;
		}
		return dealUnderflow();
	}
	return nullptr;
}


//-------------------------------------------------------------------------------------------------
// Merge with sibling InnerNode by specifying key and right sibling
//-------------------------------------------------------------------------------------------------
void  InnerNode::mergeWithSibling(KEY key, Node* rightSiblingNode) {
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
		// get sibling child
		siblingChild = rightSibling->getChild(i);
		// reattach sibling child to this node
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
		childNode = siblingNode->getChild(borrowIndex);
		// reattach childNode to this node as parent
		childNode->setParent(this);
		// append borrowed key and child node to the tail of list
		keys.push_back(key);
		children.push_back(childNode);
		// get key propogated to parent node
		upKey = siblingNode->getKeyAt(0);
		// delete key with children from sibling node
		
	    // FIXME: deleteAt() always deletes right node!
		// siblingNode->deleteAt(0);        
		// Workaround:
		siblingNode->keys.erase(siblingNode->keys.begin());
		siblingNode->children.erase(siblingNode->children.begin());                   

	} else {
		// borrow the last key from left sibling, insert it to head
		childNode = siblingNode->getChild(borrowIndex + 1);
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
// Delete keys and right children at specified index
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
	Node* rightChild;
	Node *prevRightChild = nullptr;
	for (size_t i = 0; i < keys.size(); i++) {
		leftChild = children[i];
		if (leftChild != prevRightChild) leftChild->print(level + 1);
		printTabs(level);
		cout << keys[i] << endl;
		if (i+1 < children.size()) rightChild = children[i + 1];
		if (rightChild != nullptr) {
			rightChild->print(level + 1);
		}
		prevRightChild = rightChild;
	}
}

