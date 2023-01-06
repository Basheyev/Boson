#include "BalancedTree.h"


using namespace Boson;


LeafNode::LeafNode() {
	values.reserve(NODE_CAPACITY);
}


//-------------------------------------------------------------------------------------------------
// Leaf Node Destructor
//-------------------------------------------------------------------------------------------------
LeafNode::~LeafNode() {
	values.clear();
}


//-------------------------------------------------------------------------------------------------
// Return value at specified index in this node
//-------------------------------------------------------------------------------------------------
VALUE LeafNode::getValueAt(size_t index) {
	return values[index];
}


//-------------------------------------------------------------------------------------------------
// Set value at specified index in this node
//-------------------------------------------------------------------------------------------------
void LeafNode::setValueAt(size_t index, VALUE value) {
	values[index] = value;
}


//-------------------------------------------------------------------------------------------------
// Search index of key in this node (binary search in sorted array)
//-------------------------------------------------------------------------------------------------
size_t LeafNode::search(KEY key) {

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
size_t LeafNode::searchPlaceFor(KEY key) {

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
bool LeafNode::insertKey(KEY key, VALUE value) {

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
void LeafNode::insertAt(size_t index, KEY key, VALUE value) {
	keys.insert(keys.begin() + index, 1, key);
	values.insert(values.begin() + index, 1, value);
}


//-------------------------------------------------------------------------------------------------
// Delete key/value pair by key in this node
//-------------------------------------------------------------------------------------------------
bool LeafNode::deleteKey(KEY key) {
	size_t deleteIndex = search(key);
	if (deleteIndex == NOT_FOUND) return false;
	deleteAt(deleteIndex);
	return true;
}


//-------------------------------------------------------------------------------------------------
// Delete key/value pair at specified index in this node
//-------------------------------------------------------------------------------------------------
void LeafNode::deleteAt(size_t index) {
	keys.erase(keys.begin() + index);
	values.erase(values.begin() + index);
}


//-------------------------------------------------------------------------------------------------
// Split this node by half and return new splitted node
//-------------------------------------------------------------------------------------------------
Node* LeafNode::split() {
	size_t midIndex = keys.size() / 2;
	LeafNode* newNode = new LeafNode();
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
void LeafNode::merge(KEY key, Node* sibling) {
	LeafNode* siblingLeaf = (LeafNode*)sibling;

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
Node* LeafNode::pushUpKey(KEY key, Node* leftChild, Node* rightChild) {
	throw std::runtime_error("Unsupported operation: leaf node can't push keys up.");
}


//-------------------------------------------------------------------------------------------------
// Unsupported operation for leaf node
//-------------------------------------------------------------------------------------------------
void LeafNode::borrowChildren(Node* borrower, Node* lender, size_t borrowIndex) {
	throw std::runtime_error("Unsupported operation: leaf node can't process children borrowing.");
}

//-------------------------------------------------------------------------------------------------
// Unsupported operation for leaf node
//-------------------------------------------------------------------------------------------------
Node* LeafNode::mergeChildren(Node* leftChild, Node* rightChild) {
	throw std::runtime_error("Unsupported operation: leaf node can't merge children.");
}


//-------------------------------------------------------------------------------------------------
// Merge this leaf node with right sibling
//-------------------------------------------------------------------------------------------------
void LeafNode::mergeWithSibling(KEY key, Node* rightSibling) {

	LeafNode* siblingLeaf = (LeafNode*)rightSibling;

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
KEY LeafNode::borrowFromSibling(KEY key, Node* sibling, size_t borrowIndex) {

	LeafNode* siblingNode = (LeafNode*)sibling;

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
NodeType LeafNode::getNodeType() {
	return NodeType::LEAF;
}



//-------------------------------------------------------------------------------------------------
// Prints this leaf node key/value pairs
//-------------------------------------------------------------------------------------------------
void LeafNode::print(int level) {
	//cout << "-------- " << keys.size() << " records -------" << endl;
	for (size_t i = 0; i < keys.size(); i++) {
		printTabs(level);
		std::cout << keys[i] << " - " << values[i] << std::endl;
	}
	//cout << "-------------------------------\n";
}


