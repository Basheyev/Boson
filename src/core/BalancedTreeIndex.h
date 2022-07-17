/*=================================================================================================
* 
*    B+ Tree Template
* 
*    Key/value store index template
* 
*    BOSON embedded database
*    (C) Bolat Basheyev 2022
*    
=================================================================================================*/
#pragma once

#include <vector>
#include <iostream>

namespace Boson {

	constexpr size_t MINIMAL_TREE_ORDER = 3;
	constexpr size_t DEFAULT_TREE_ORDER = 5;
	constexpr size_t NOT_FOUND = 0xFFFFFFFFFFFFFFFF;

	typedef enum { INNER = 1, LEAF = 2 } NodeType;

	//---------------------------------------------------------------------------------------------
	// Node abstract class
	//---------------------------------------------------------------------------------------------
	template <typename KEY>
	class Node {
	public:
		Node(size_t M);
		~Node();
		size_t      getKeyCount();
		bool        isOverflow();
		bool        isUnderflow();
		bool        canLendAKey();
		KEY         getKeyAt(size_t index);
		void        setKeyAt(size_t index, KEY key);
		Node<KEY>*  getParent();
		void        setParent(Node<KEY>* parent);
		Node<KEY>*  getLeftSibling();
		void        setLeftSibling(Node<KEY>*);
		Node<KEY>*  getRightSibling();
		void        setRightSibling(Node<KEY>*);
		Node<KEY>*  dealOverflow();
		Node<KEY>*  dealUnderflow();

		virtual NodeType getNodeType() = 0;
		virtual size_t search(KEY key) = 0;
		virtual Node<KEY>* split() = 0;
		virtual Node<KEY>* pushUpKey(KEY key, Node<KEY>* leftChild, Node<KEY>* rightChild) = 0;
		virtual Node<KEY>* mergeChildren(Node<KEY>* leftChild, Node<KEY>* rightChild) = 0;
		virtual void  mergeWithSibling(KEY key, Node<KEY>* rightSibling) = 0;
		virtual KEY   borrowFromSibling(KEY key, Node<KEY>* sibling, size_t borrowIndex) = 0;
		virtual void  borrowChildren(Node<KEY>* borrower, Node<KEY>* lender, size_t borrowIndex) = 0;
		virtual void  print(int level) = 0;

	protected:
		size_t treeOrder;
		size_t maxDegree;
		size_t minDegree;

		Node<KEY>* parent;
		Node<KEY>* leftSibling;
		Node<KEY>* rightSibling;
		std::vector<KEY> keys;

		void printTabs(size_t n);
	};


	//---------------------------------------------------------------------------------------------
	// Inner Node Class - index page
	//---------------------------------------------------------------------------------------------
	template <typename KEY>
	class InnerNode : public Node<KEY> {
	public:
		InnerNode(size_t m);
		~InnerNode();
		size_t     search(KEY key);
		Node<KEY>* getChildAt(size_t index);
		void       setChildAt(size_t index, Node<KEY>* childNode);
		void       insertAt(size_t index, KEY key, Node<KEY>* leftChild, Node<KEY>* rightChild);
		void       deleteAt(size_t index);
		Node<KEY>* split();
		Node<KEY>* pushUpKey(KEY key, Node<KEY>* leftChild, Node<KEY>* rightChild);
		void       borrowChildren(Node<KEY>* borrower, Node<KEY>* lender, size_t borrowIndex);
		Node<KEY>* mergeChildren(Node<KEY>* leftChild, Node<KEY>* rightChild);
		void       mergeWithSibling(KEY key, Node<KEY>* rightSibling);
		KEY        borrowFromSibling(KEY key, Node<KEY>* sibling, size_t borrowIndex);
		NodeType   getNodeType();
		void       print(int level);
	private:
		std::vector<Node<KEY>*> children;
	};


	//---------------------------------------------------------------------------------------------
	// Leaf Node Class - data page
	//---------------------------------------------------------------------------------------------
	template <typename KEY, typename VALUE>
	class LeafNode : public Node<KEY> {
	public:
		LeafNode(size_t m);
		~LeafNode();
		size_t search(KEY key);
		VALUE getValueAt(size_t index);
		void  setValueAt(size_t index, VALUE value);
		bool  insertKey(KEY key, VALUE value);
		void  insertAt(size_t index, KEY key, VALUE value);
		bool  deleteKey(KEY key);
		void  deleteAt(size_t index);
		Node<KEY>* split();
		void  merge(KEY key, Node<KEY>* siblingRight);
		Node<KEY>* pushUpKey(KEY key, Node<KEY>* leftChild, Node<KEY>* rightChild);
		void  borrowChildren(Node<KEY>* borrower, Node<KEY>* lender, size_t borrowIndex);
		Node<KEY>* mergeChildren(Node<KEY>* leftChild, Node<KEY>* rightChild);
		void  mergeWithSibling(KEY key, Node<KEY>* rightSibling);
		KEY   borrowFromSibling(KEY key, Node<KEY>* sibling, size_t borrowIndex);
		NodeType getNodeType();
		void  print(int level);
	private:
		std::vector<VALUE> values;
		size_t searchPlaceFor(KEY key);
	};



	//---------------------------------------------------------------------------------------------
	// Balanced Plus Tree class - KEY/VALUE store index
	//---------------------------------------------------------------------------------------------
	template <typename KEY, typename VALUE>
	class BalancedTreeIndex {
	public:
		BalancedTreeIndex(size_t M = DEFAULT_TREE_ORDER);
		~BalancedTreeIndex();
		
		bool   insert(KEY key, VALUE value);
		VALUE  search(KEY key);

		bool   erase(KEY key);
		size_t getEntriesCount();

		size_t getTreeOrder();
		size_t getTreeHeight();
		Node<KEY>*  getRoot();

		void   printTree();
		void   printContent();

	private:
		size_t treeOrder;
		size_t entriesCount;
		Node<KEY>* root;
		LeafNode<KEY,VALUE>* findLeafNode(KEY key);
	};




	//=================================================================================================
	//
	// Node Template
	//
	//=================================================================================================


	//-------------------------------------------------------------------------------------------------
	// Node Constructor
	// - M   - maximum Child nodes per Inner node
	// - M-1 - maximum Keys count per Inner node and Key-Value pairs per Leaf node
	// - M/2 - minimal keys count per Inner node and Key-Value pairs per Leaf node
	//-------------------------------------------------------------------------------------------------
	template <typename KEY> Node<KEY>::Node(size_t M) {
		this->treeOrder = M;
		this->maxDegree = M - 1;
		this->minDegree = M / 2;
		this->parent = nullptr;
		this->leftSibling = nullptr;
		this->rightSibling = nullptr;
		keys.reserve(M);
	}


	//-------------------------------------------------------------------------------------------------
	// Node Destructor
	//-------------------------------------------------------------------------------------------------
	template <typename KEY> Node<KEY>::~Node() {
		keys.clear();
	}


	//-------------------------------------------------------------------------------------------------
	// Returns Keys count inside Node
	//-------------------------------------------------------------------------------------------------
	template <typename KEY> size_t Node<KEY>::getKeyCount() {
		return keys.size();
	}


	//-------------------------------------------------------------------------------------------------
	// Returns whether node keys count > M-1
	//-------------------------------------------------------------------------------------------------
	template <typename KEY> bool Node<KEY>::isOverflow() {
		return keys.size() > maxDegree;
	}


	//-------------------------------------------------------------------------------------------------
	// Returns whether node keys count < M / 2
	//-------------------------------------------------------------------------------------------------
	template <typename KEY> bool Node<KEY>::isUnderflow() {
		return keys.size() < minDegree;

	}


	//-------------------------------------------------------------------------------------------------
	// Returns whether node keys count > M / 2
	//-------------------------------------------------------------------------------------------------
	template <typename KEY> bool Node<KEY>::canLendAKey() {
		return keys.size() > minDegree;
	}


	//-------------------------------------------------------------------------------------------------
	// Returns key at specified index
	//-------------------------------------------------------------------------------------------------
	template <typename KEY> KEY Node<KEY>::getKeyAt(size_t index) {
		return keys[index];
	}


	//-------------------------------------------------------------------------------------------------
	// Sets key at specified index
	//-------------------------------------------------------------------------------------------------
	template <typename KEY> void Node<KEY>::setKeyAt(size_t index, KEY key) {
		keys[index] = key;
	}


	//-------------------------------------------------------------------------------------------------
	// Returns parent node or nullptr if it is root node
	//-------------------------------------------------------------------------------------------------
	template <typename KEY> Node<KEY>* Node<KEY>::getParent() {
		return parent;
	}


	//-------------------------------------------------------------------------------------------------
	// Sets node parent node
	//-------------------------------------------------------------------------------------------------
	template <typename KEY> void Node<KEY>::setParent(Node<KEY>* parent) {
		this->parent = parent;
	}


	//-------------------------------------------------------------------------------------------------
	// Returns node's left sibling node
	//-------------------------------------------------------------------------------------------------
	template <typename KEY> Node<KEY>* Node<KEY>::getLeftSibling() {
		return leftSibling;
	}


	//-------------------------------------------------------------------------------------------------
	// Sets node's left sibling node
	//-------------------------------------------------------------------------------------------------
	template <typename KEY> void Node<KEY>::setLeftSibling(Node<KEY>* leftSibling) {
		this->leftSibling = leftSibling;
	}


	//-------------------------------------------------------------------------------------------------
	// Returns node right sibling node
	//-------------------------------------------------------------------------------------------------
	template <typename KEY> Node<KEY>* Node<KEY>::getRightSibling() {
		return rightSibling;
	}


	//-------------------------------------------------------------------------------------------------
	// Sets node's right sibling node
	//-------------------------------------------------------------------------------------------------
	template <typename KEY> void Node<KEY>::setRightSibling(Node<KEY>* rightSibling) {
		this->rightSibling = rightSibling;
	}


	//-------------------------------------------------------------------------------------------------
	// Process node overflow scenarios
	//-------------------------------------------------------------------------------------------------
	template <typename KEY> Node<KEY>* Node<KEY>::dealOverflow() {

		// Get key at middle index for propagation to the parent node
		size_t midIndex = this->getKeyCount() / 2;
		KEY upKey = this->getKeyAt(midIndex);

		// Split this node by half (returns new splitted node)
		Node<KEY>* newRightNode = this->split();

		// if we are splitting the root node
		if (getParent() == nullptr) {
			// create new root node and set as parent to this node (grow at root)
			Node<KEY>* newRootNode = new InnerNode<KEY>(treeOrder);
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
		Node<KEY>* rootNode = this->parent->pushUpKey(upKey, this, newRightNode);

		// Return current root node
		return rootNode;
	}


	//-------------------------------------------------------------------------------------------------
	// Process node underoverflow scenarios
	//-------------------------------------------------------------------------------------------------
	template <typename KEY> Node<KEY>* Node<KEY>::dealUnderflow() {

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
			Node<KEY>* rootNode = parent->mergeChildren(leftSibling, this);
			return rootNode;
		}
		else {
			// 4. Try to merge with right sibling
			Node<KEY>* rootNode = parent->mergeChildren(this, rightSibling);
			return rootNode;
		}

	}


	//-------------------------------------------------------------------------------------------------
	// Print N tabls to standart out
	//-------------------------------------------------------------------------------------------------
	template <typename KEY> void Node<KEY>::printTabs(size_t n) {
		for (size_t i = 0; i < n; i++) {
			std::cout << "\t";
		}
	}









	//=================================================================================================
	//
	// Inner Node Template
	//
	//=================================================================================================


	//-------------------------------------------------------------------------------------------------
	// Inner Node Constructor (calls Node Constructor)
	// - M   - maximum Child nodes per Inner node
	// - M-1 - maximum Keys count per Inner node
	// - M/2 - minimal keys count per Inner node
	//-------------------------------------------------------------------------------------------------
	template <typename KEY> InnerNode<KEY>::InnerNode(size_t m) : Node(m) {
		children.reserve(m);
	}


	//-------------------------------------------------------------------------------------------------
	// Inner Node Destructor
	//-------------------------------------------------------------------------------------------------
	template <typename KEY> InnerNode<KEY>::~InnerNode() {
		children.clear();
	}


	//-------------------------------------------------------------------------------------------------
	// Returns child node at specified index
	//-------------------------------------------------------------------------------------------------
	template <typename KEY> Node<KEY>* InnerNode<KEY>::getChildAt(size_t index) {
		return children[index];
	}


	//-------------------------------------------------------------------------------------------------
	// Sets child node at specified index
	//-------------------------------------------------------------------------------------------------
	template <typename KEY> void InnerNode<KEY>::setChildAt(size_t index, Node<KEY>* childNode) {
		children[index] = childNode;
	}


	//-------------------------------------------------------------------------------------------------
	// Return child node index for specified key
	//-------------------------------------------------------------------------------------------------
	template <typename KEY> size_t InnerNode<KEY>::search(KEY key) {
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
	template <typename KEY> void InnerNode<KEY>::insertAt(size_t index, KEY key, Node<KEY>* leftChild, Node<KEY>* rightChild) {

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
	template <typename KEY> Node<KEY>* InnerNode<KEY>::split() {

		// Calculate mid index
		size_t midIndex = this->getKeyCount() / 2;

		// Create new node
		InnerNode<KEY>* newNode = new InnerNode<KEY>(this->treeOrder);

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
	template <typename KEY> Node<KEY>* InnerNode<KEY>::pushUpKey(KEY key, Node<KEY>* leftChild, Node<KEY>* rightChild) {

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
	template <typename KEY> void InnerNode<KEY>::borrowChildren(Node<KEY>* borrower, Node<KEY>* lender, size_t borrowIndex) {
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
	template <typename KEY> Node<KEY>* InnerNode<KEY>::mergeChildren(Node<KEY>* leftChild, Node<KEY>* rightChild) {
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
	template <typename KEY> void InnerNode<KEY>::mergeWithSibling(KEY key, Node<KEY>* rightSiblingNode) {
		InnerNode<KEY>* rightSibling = (InnerNode<KEY>*)rightSiblingNode;
		Node<KEY>* siblingChild;

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
	template <typename KEY> KEY InnerNode<KEY>::borrowFromSibling(KEY key, Node<KEY>* sibling, size_t borrowIndex) {
		InnerNode<KEY>* siblingNode = (InnerNode<KEY>*)sibling;
		Node<KEY>* childNode;
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
	template <typename KEY> void InnerNode<KEY>::deleteAt(size_t index) {
		keys.erase(keys.begin() + index);
		children.erase(children.begin() + index + 1);
	}


	//-------------------------------------------------------------------------------------------------
	// Return node type NodeType::INNER
	//-------------------------------------------------------------------------------------------------
	template <typename KEY> NodeType InnerNode<KEY>::getNodeType() {
		return NodeType::INNER;
	}



	//-------------------------------------------------------------------------------------------------
	// Print inner node children
	//-------------------------------------------------------------------------------------------------
	template <typename KEY> void InnerNode<KEY>::print(int level) {
		Node<KEY>* leftChild;
		Node<KEY>* rightChild = nullptr;
		Node<KEY>* prevRightChild = nullptr;
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








	//=================================================================================================
	//
	// Leaf Node Template
	//
	//=================================================================================================


	//-------------------------------------------------------------------------------------------------
	// Leaf Node Constructor (calls Node Constructor)
	// - M-1 - maximum Key-Value pairs per Leaf node
	// - M/2 - minimal Key-Value pairs per Leaf node
	//-------------------------------------------------------------------------------------------------
	template <typename KEY, typename VALUE> LeafNode<KEY, VALUE>::LeafNode(size_t m) : Node(m) {
		values.reserve(m);
	}


	//-------------------------------------------------------------------------------------------------
	// Leaf Node Destructor
	//-------------------------------------------------------------------------------------------------
	template <typename KEY, typename VALUE> LeafNode<KEY, VALUE>::~LeafNode() {
		values.clear();
	}


	//-------------------------------------------------------------------------------------------------
	// Return value at specified index in this node
	//-------------------------------------------------------------------------------------------------
	template <typename KEY, typename VALUE> VALUE LeafNode<KEY, VALUE>::getValueAt(size_t index) {
		return values[index];
	}


	//-------------------------------------------------------------------------------------------------
	// Set value at specified index in this node
	//-------------------------------------------------------------------------------------------------
	template <typename KEY, typename VALUE> void LeafNode<KEY, VALUE>::setValueAt(size_t index, VALUE value) {
		values[index] = value;
	}


	//-------------------------------------------------------------------------------------------------
	// Search index of key in this node (binary search in sorted array)
	//-------------------------------------------------------------------------------------------------
	template <typename KEY, typename VALUE> size_t LeafNode<KEY, VALUE>::search(KEY key) {

		int64_t start = 0;                            // we need signed integers for
		int64_t end = keys.size() - 1;                // correct comparison in while loop
		size_t mid;                                   // middle index can not be negative
		KEY entry;                                    // variable to hold value

		while (start <= end) {                        // while start index <= end index
			mid = start + (end - start) / 2;          // calculate middle index between start & end
			entry = keys[mid];                        // get value in keys array at middle index
			if (entry == key) return mid;             // if value equals to key return index
			if (key < entry) end = mid - 1; else      // if key < value bound end index to middle-1
				if (key > entry) start = mid + 1;     // if key > value bound start index to middle+1 
		}

		return NOT_FOUND;                             // Key is definitely not found
	}


	//-------------------------------------------------------------------------------------------------
	// Search index for new key in sorted order (NOT_FOUND returned if key duplicate)
	//-------------------------------------------------------------------------------------------------
	template <typename KEY, typename VALUE> size_t LeafNode<KEY, VALUE>::searchPlaceFor(KEY key) {

		size_t insertIndex = keys.size();
		int64_t start = 0;                            // we need signed integers for
		int64_t end = keys.size() - 1;                // correct comparison in while loop
		size_t mid;                                   // middle index can not be negative
		KEY entry;                                    // variable to hold value

		while (start <= end) {                        // while start index <= end index
			mid = start + (end - start) / 2;          // calculate middle index between start & end
			entry = keys[mid];                        // get value in keys array at middle index
			if (entry == key) return NOT_FOUND;       // if value equals to key - key duplicate!
			if (key < entry) {                        // if key < value 
				end = mid - 1;                        // bound end index to middle-1
				insertIndex = mid;                    // save index where next entry is greater
			}
			else if (key > entry) start = mid + 1;    // if key > value bound start index to middle+1 
		}

		return insertIndex;
	}


	//-------------------------------------------------------------------------------------------------
	// Insert key/value pair to this node in sorted order
	//-------------------------------------------------------------------------------------------------
	template <typename KEY, typename VALUE> bool LeafNode<KEY, VALUE>::insertKey(KEY key, VALUE value) {

		// find index to insert new key/value pair in sorted order
		size_t insertIndex = searchPlaceFor(key);
		// 
		if (insertIndex == NOT_FOUND) return false;

		// insert key/value
		insertAt(insertIndex, key, value);

		return true;
	}


	//-------------------------------------------------------------------------------------------------
	// Insert key/value pair at specified index in this node
	//-------------------------------------------------------------------------------------------------
	template <typename KEY, typename VALUE> void LeafNode<KEY, VALUE>::insertAt(size_t index, KEY key, VALUE value) {
		keys.insert(keys.begin() + index, 1, key);
		values.insert(values.begin() + index, 1, value);
	}


	//-------------------------------------------------------------------------------------------------
	// Delete key/value pair by key in this node
	//-------------------------------------------------------------------------------------------------
	template <typename KEY, typename VALUE> bool LeafNode<KEY, VALUE>::deleteKey(KEY key) {
		size_t deleteIndex = search(key);
		if (deleteIndex == NOT_FOUND) return false;
		deleteAt(deleteIndex);
		return true;
	}


	//-------------------------------------------------------------------------------------------------
	// Delete key/value pair at specified index in this node
	//-------------------------------------------------------------------------------------------------
	template <typename KEY, typename VALUE> void LeafNode<KEY, VALUE>::deleteAt(size_t index) {
		keys.erase(keys.begin() + index);
		values.erase(values.begin() + index);
	}


	//-------------------------------------------------------------------------------------------------
	// Split this node by half and return new splitted node
	//-------------------------------------------------------------------------------------------------
	template <typename KEY, typename VALUE> Node<KEY>* LeafNode<KEY, VALUE>::split() {
		size_t midIndex = keys.size() / 2;
		LeafNode<KEY,VALUE>* newNode = new LeafNode(this->treeOrder);
		for (size_t i = midIndex; i < keys.size(); ++i) {
			newNode->insertKey(keys[i], values[i]);
		}
		keys.resize(midIndex);
		values.resize(midIndex);
		return newNode;
	}


	//-------------------------------------------------------------------------------------------------
	// Merges two leaf nodes
	//-------------------------------------------------------------------------------------------------
	template <typename KEY, typename VALUE> void LeafNode<KEY, VALUE>::merge(KEY key, Node<KEY>* sibling) {
		LeafNode<KEY,VALUE>* siblingLeaf = (LeafNode<KEY,VALUE>*)sibling;

		// copy keys and values from sibling node to this node
		for (size_t i = 0; i < siblingLeaf->getKeyCount(); i++) {
			keys.push_back(siblingLeaf->getKeyAt(i));
			values.push_back(siblingLeaf->getValueAt(i));
		}

		// interconnect siblings
		setRightSibling(siblingLeaf->rightSibling);
		if (siblingLeaf->rightSibling != nullptr) {
			siblingLeaf->rightSibling->setLeftSibling(this);
		}

		// Delete sibling node
		delete siblingLeaf;
	}


	//-------------------------------------------------------------------------------------------------
	// Unsupported operation for leaf node
	//-------------------------------------------------------------------------------------------------
	template <typename KEY, typename VALUE> Node<KEY>* LeafNode<KEY, VALUE>::pushUpKey(KEY key, Node<KEY>* leftChild, Node<KEY>* rightChild) {
		throw std::runtime_error("Unsupported operation: leaf node can't push keys up.");
	}


	//-------------------------------------------------------------------------------------------------
	// Unsupported operation for leaf node
	//-------------------------------------------------------------------------------------------------
	template <typename KEY, typename VALUE> void LeafNode<KEY, VALUE>::borrowChildren(Node<KEY>* borrower, Node<KEY>* lender, size_t borrowIndex) {
		throw std::runtime_error("Unsupported operation: leaf node can't process children borrowing.");
	}

	//-------------------------------------------------------------------------------------------------
	// Unsupported operation for leaf node
	//-------------------------------------------------------------------------------------------------
	template <typename KEY, typename VALUE> Node<KEY>* LeafNode<KEY, VALUE>::mergeChildren(Node<KEY>* leftChild, Node<KEY>* rightChild) {
		throw std::runtime_error("Unsupported operation: leaf node can't merge children.");
	}


	//-------------------------------------------------------------------------------------------------
	// Merge this leaf node with right sibling
	//-------------------------------------------------------------------------------------------------
	template <typename KEY, typename VALUE> void LeafNode<KEY, VALUE>::mergeWithSibling(KEY key, Node<KEY>* rightSibling) {

		LeafNode<KEY,VALUE>* siblingLeaf = (LeafNode<KEY,VALUE>*)rightSibling;

		// Copy keys and values at the tail of this node
		for (size_t i = 0; i < siblingLeaf->getKeyCount(); ++i) {
			keys.push_back(siblingLeaf->getKeyAt(i));
			values.push_back(siblingLeaf->getValueAt(i));
		}

		// Interconnect siblings
		this->setRightSibling(siblingLeaf->rightSibling);
		if (siblingLeaf->rightSibling != nullptr) {
			siblingLeaf->rightSibling->setLeftSibling(this);
		}

		// Delete sibling node
		delete siblingLeaf;
	}


	//-------------------------------------------------------------------------------------------------
	// Borrow child node at specified index from sibling and return new middle key
	//-------------------------------------------------------------------------------------------------
	template <typename KEY, typename VALUE> KEY LeafNode<KEY, VALUE>::borrowFromSibling(KEY key, Node<KEY>* sibling, size_t borrowIndex) {

		LeafNode<KEY,VALUE>* siblingNode = (LeafNode<KEY, VALUE>*)sibling;

		// insert borrowed key/value pair
		KEY borrowedKey = siblingNode->getKeyAt(borrowIndex);
		VALUE borrowedValue = siblingNode->getValueAt(borrowIndex);
		this->insertKey(borrowedKey, borrowedValue);

		// delete borrowed key/value pair in sibling node
		siblingNode->deleteAt(borrowIndex);

		// return new middle key
		if (borrowIndex == 0)
			return sibling->getKeyAt(0);
		else
			return this->getKeyAt(0);

	}


	//-------------------------------------------------------------------------------------------------
	// Return NodeType::LEAF
	//-------------------------------------------------------------------------------------------------
	template <typename KEY, typename VALUE> NodeType LeafNode<KEY, VALUE>::getNodeType() {
		return NodeType::LEAF;
	}


	//-------------------------------------------------------------------------------------------------
	// Prints this leaf node key/value pairs
	//-------------------------------------------------------------------------------------------------
	template <typename KEY, typename VALUE> void LeafNode<KEY, VALUE>::print(int level) {
		//cout << "-------- " << keys.size() << " records -------" << endl;
		for (size_t i = 0; i < keys.size(); i++) {
			printTabs(level);
			std::cout << keys[i] << " - " << values[i] << std::endl;
		}
		//cout << "-------------------------------\n";
	}





	
	
	//=================================================================================================
	//
	// Balanced Tree Index Template
	//
	//=================================================================================================


	//-------------------------------------------------------------------------------------------------
	// Balanced Tree Constructor
	// - M-1 - maximum Keys count per Inner node and Key-Value pairs per Leaf node
	// - M/2 - minimal keys count per Inner node and Key-Value pairs per Leaf node
	// - M   - maximum Child nodes per Inner node
	//-------------------------------------------------------------------------------------------------
	template <typename KEY, typename VALUE> BalancedTreeIndex<KEY,VALUE>::BalancedTreeIndex(size_t M) {
		if (M < MINIMAL_TREE_ORDER) M = MINIMAL_TREE_ORDER;
		this->treeOrder = M;
		this->entriesCount = 0;
		root = new LeafNode<KEY,VALUE>(M);
	}


	//-------------------------------------------------------------------------------------------------
	// Balanced Tree Desctructor
	//-------------------------------------------------------------------------------------------------
	template <typename KEY, typename VALUE> BalancedTreeIndex<KEY,VALUE>::~BalancedTreeIndex() {
		delete root;
	}


	//-------------------------------------------------------------------------------------------------
	// Insert key/value pair
	//-------------------------------------------------------------------------------------------------
	template <typename KEY, typename VALUE> bool BalancedTreeIndex<KEY,VALUE>::insert(KEY key, VALUE value) {
		LeafNode<KEY,VALUE>* leaf = findLeafNode(key);
		bool isInserted = leaf->insertKey(key, value);
		if (leaf->isOverflow()) {
			Node<KEY>* n = leaf->dealOverflow();
			if (n != nullptr) root = n;
		}
		entriesCount++;
		return isInserted;
	}


	//-------------------------------------------------------------------------------------------------
	// Search value by key (binary search)
	//-------------------------------------------------------------------------------------------------
	template <typename KEY, typename VALUE> VALUE BalancedTreeIndex<KEY,VALUE>::search(KEY key) {
		LeafNode<KEY,VALUE>* leaf = findLeafNode(key);
		size_t index = leaf->search(key);
		// FIXME: For non-pointer types returning nullptr is invalid 
		return (index == NOT_FOUND) ? 0 : leaf->getValueAt(index);
	}


	//-------------------------------------------------------------------------------------------------
	// Delete key/value pair
	//-------------------------------------------------------------------------------------------------
	template <typename KEY, typename VALUE> bool BalancedTreeIndex<KEY,VALUE>::erase(KEY key) {

		LeafNode<KEY,VALUE>* leaf = findLeafNode(key);

		if (leaf->deleteKey(key)) {
			entriesCount--;
			if (leaf->isUnderflow()) {
				Node<KEY>* n = leaf->dealUnderflow();
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
	template <typename KEY, typename VALUE> LeafNode<KEY,VALUE>* BalancedTreeIndex<KEY,VALUE>::findLeafNode(KEY key) {
		Node<KEY>* node = root;
		InnerNode<KEY>* innerNode;
		size_t index;
		while (node->getNodeType() == NodeType::INNER) {
			index = node->search(key);
			innerNode = (InnerNode<KEY>*)node;
			node = innerNode->getChildAt(index);
		}
		return (LeafNode<KEY,VALUE>*)node;
	}


	//-------------------------------------------------------------------------------------------------
	// Return tree order M
	//-------------------------------------------------------------------------------------------------
	template <typename KEY, typename VALUE> size_t BalancedTreeIndex<KEY,VALUE>::getTreeOrder() {
		return this->treeOrder;
	}


	//-------------------------------------------------------------------------------------------------
	// Return tree height
	//-------------------------------------------------------------------------------------------------
	template <typename KEY, typename VALUE> size_t BalancedTreeIndex<KEY,VALUE>::getTreeHeight() {
		size_t levelCounter = 0;
		Node<KEY>* firstLeaf = root;
		while (firstLeaf->getNodeType() == NodeType::INNER) {
			firstLeaf = ((InnerNode<KEY>*)firstLeaf)->getChildAt(0);
			levelCounter++;
		}
		return levelCounter;
	}


	//-------------------------------------------------------------------------------------------------
	// Returns records count
	//-------------------------------------------------------------------------------------------------
	template <typename KEY, typename VALUE> size_t BalancedTreeIndex<KEY,VALUE>::getEntriesCount() {
		return entriesCount;
	}

	template <typename KEY, typename VALUE> Node<KEY>* BalancedTreeIndex<KEY,VALUE>::getRoot() {
		return root;
	}


	template <typename KEY, typename VALUE> void BalancedTreeIndex<KEY,VALUE>::printTree() {
		std::cout << "----------------------------------------" << std::endl;
		root->print(0);
	}


	template <typename KEY, typename VALUE> void BalancedTreeIndex<KEY,VALUE>::printContent() {
		std::cout << "----------------------------------------" << std:endl;
		Node<KEY>* firstLeaf = root;
		// go down tree
		while (firstLeaf->getNodeType() == NodeType::INNER) {
			firstLeaf = ((InnerNode<KEY>*)firstLeaf)->getChildAt(0);
		}
		// print list
		while (firstLeaf != nullptr) {
			firstLeaf->print(0);
			firstLeaf = firstLeaf->getRightSibling();
		}
	}
}