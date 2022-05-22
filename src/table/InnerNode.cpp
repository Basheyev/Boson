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


Node* InnerNode::getChild(size_t index) {
	return children[index];
}


void InnerNode::setChild(size_t index, Node* childNode) {
	children[index] = childNode;
}


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




Node* InnerNode::split() {
	// When splits an internal node, the middle key is kicked out and pushed to parent node.

	size_t midIndex = this->getKeyCount() / 2;

	InnerNode* newNode = new InnerNode(this->keys.capacity());

	for (size_t i = midIndex + 1; i < keys.size(); ++i) {
		newNode->keys.push_back(this->keys[i]);                      // copy keys to new node
	}
	this->keys.resize(midIndex);                                     // truncate keys list

	// FIXME

	for (size_t i = midIndex + 1; i < children.size(); ++i) {
		newNode->children.push_back(this->children[i]);              // move children to the new node
		newNode->children[i - midIndex - 1]->setParent(newNode);     // reattach children to new parent
	}
	this->children.resize(midIndex + 1);                             // truncate children list

	return newNode;

}



Node* InnerNode::pushUpKey(KEY key, Node* leftChild, Node* rightChild) {
	size_t index = search(key);
	// fixme
	if (index == NOT_FOUND) index = 0;
	insertAt(index, key, leftChild, rightChild);
	if (isOverflow())
		return dealOverflow();
	else
		return getParent() == nullptr ? this : nullptr;
}


void InnerNode::transferChildren(Node* borrower, Node* lender, size_t borrowIndex) {
	size_t borrowerChildIndex = 0;

	// find borrower child index
	for (int i = 0; i < children.size(); i++) {
		if (children[i] == borrower) {
			borrowerChildIndex = i;
			break;
		}
	}

	if (borrowIndex == 0) {
		// borrow from right sibling
		// FIXME
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


Node* InnerNode::mergeChildren(Node* leftChild, Node* rightChild) {
	size_t index = 0;

	// FIXME BUGS
	
	while (index < children.size() - 1) {
		if (children[index] == leftChild) break;
		index++;
	}
	KEY key = keys[index];
	
	// merge two children and the sink key into the left child node
	leftChild->mergeWithSibling(key, rightChild);

	// remove the sink key, keep the left child and abandon the right child
	deleteAt(index);

	// check whether need to propagate borrow or fusion to parent
	if (isUnderflow()) {
		if (getParent() == nullptr) {
			if (keys.size() == 0) {
				leftChild->setParent(nullptr);
				return leftChild;
			}
			else {
				return nullptr;
			}

		}
		return dealUnderflow();
	}
	return nullptr;
}


void  InnerNode::mergeWithSibling(KEY key, Node* rightSiblingNode) {

	InnerNode* rightSibling = (InnerNode*)rightSiblingNode;
	Node* siblingChild;
	size_t j = keys.size();

	keys.push_back(key);
	for (size_t i = 0; i < rightSibling->getKeyCount(); ++i) {
		keys.push_back(rightSibling->getKeyAt(i));
	}
	for (size_t i = 0; i < rightSibling->getKeyCount() + 1; ++i) {
		siblingChild = rightSibling->getChild(i);
		children.push_back(siblingChild);
		siblingChild->setParent(this);
	}	

	this->setRightSibling(rightSibling->rightSibling);
	if (rightSibling->rightSibling != nullptr) rightSibling->rightSibling->setLeftSibling(this);

}


KEY InnerNode::borrowFromSibling(KEY key, Node* sibling, size_t borrowIndex) {
	InnerNode* siblingNode = (InnerNode*)sibling;

	KEY upKey = 0;

	if (borrowIndex == 0) {
		// borrow the first key from right sibling, append it to tail
		keys.push_back(key);
		children.push_back(siblingNode->getChild(borrowIndex));
		upKey = siblingNode->getKeyAt(0);
		siblingNode->deleteAt(0);
	}
	else {
		// borrow the last key from left sibling, insert it to head
		insertAt(0, key, siblingNode->getChild(borrowIndex + 1), children[0]);
		upKey = siblingNode->getKeyAt(borrowIndex);
		siblingNode->deleteAt(borrowIndex);
	}

	return upKey;
}


void InnerNode::deleteAt(size_t index) {
	keys.erase(keys.begin() + index);
	children.erase(children.begin() + index + 1);
}


NodeType InnerNode::getNodeType() {
	return NodeType::INNER;
}


void InnerNode::print(int level) {
	Node* leftChild;
	Node* rightChild;
	Node *prevRightChild = nullptr;
	for (size_t i = 0; i < keys.size(); i++) {
		leftChild = children[i];
		rightChild = children[i + 1];
		if (leftChild != prevRightChild) leftChild->print(level + 1);
		printTabs(level);
		cout << keys[i] << endl;
		rightChild->print(level + 1);
		prevRightChild = rightChild;
	}
}

