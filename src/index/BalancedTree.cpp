#include "BalancedTree.h"


using namespace Boson;


//-------------------------------------------------------------------------------------------------
// Balanced+ Tree Constructor
//-------------------------------------------------------------------------------------------------
BalancedTree::BalancedTree(RecordFileIO& file) : storageFile(file) {
	
	bool newDatabase = false;

	// Go to first record
	if (storageFile.first()) {
		// Read index header
		uint64_t offset = storageFile.getRecordData(indexHeader, sizeof IndexHeader);
		if (offset != NOT_FOUND) {
			
			return;
		}
		newDatabase = true;
	} else {
		newDatabase = true;
	}

	if (newDatabase) {
		indexHeader.rootOffset = createLeafNode();
	}
		
}


//-------------------------------------------------------------------------------------------------
// Balanced+ Tree Desctructor
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
OFFSET BalancedTree::findLeafNode(KEY key) {
	OFFSET node = root;
	OFFSET innerNode;
	size_t index;
	while (node->getNodeType() == NodeType::INNER) {
		index = node->search(key);
		innerNode = node;
		node = innerNode->getChildAt(index);
	}
	return node;
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
	OFFSET firstLeaf = root;
	while (firstLeaf->getNodeType() == NodeType::INNER) {
		firstLeaf = firstLeaf->getChildAt(0);
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
OFFSET BalancedTree::getRoot() {
	return indexHeader.rootOffset;
}

//-------------------------------------------------------------------------------------------------

OFFSET BalancedTree::createLeafNode() {
	return NOT_FOUND;
}

OFFSET BalancedTree::createInnerNode() {
	return NOT_FOUND;
}

bool BalancedTree::removeLeafNode(OFFSET pos) {
	return false;
}

bool BalancedTree::removeInnerNode(OFFSET pos) {
	return false;
}

//-------------------------------------------------------------------------------------------------

void BalancedTree::printTree() {
	std::cout << "----------------------------------------" << std::endl;
	root->print(0);
}


void BalancedTree::printContent() {
	std::cout << "----------------------------------------" << std::endl;
	OFFSET firstLeaf = root;
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
