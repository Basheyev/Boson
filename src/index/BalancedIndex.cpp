/******************************************************************************
*
*  Balanced Index
*
*  (C) Boson Database, Bolat Basheyev 2022
*
******************************************************************************/





#include "BalancedIndex.h"

#include <algorithm>

using namespace Boson;



BalancedIndex::BalancedIndex(RecordFileIO& rf) : recordsFile(rf) {
    // check if file is open
    if (!rf.isOpen()) throw std::runtime_error("Can't open file.");
    // Check if file has its first record as DB header
    if (!recordsFile.first()) {
        memset(&indexHeader, 0, sizeof IndexHeader);
        uint64_t referencePos = recordsFile.createRecord(&indexHeader, sizeof indexHeader);
        // root record
        root = std::make_shared<LeafNode>(*this);      
        indexHeader.treeOrder = TREE_ORDER;
        indexHeader.rootPosition = root->persist();
        recordsFile.setPosition(referencePos);
        recordsFile.setRecordData(&indexHeader, sizeof indexHeader);
    } else {
        // look up root position
        recordsFile.getRecordData(&indexHeader, sizeof indexHeader);
        // load root record
        root = Node::loadNode(*this, indexHeader.rootPosition);
    }
}


BalancedIndex::~BalancedIndex() {
    root->persist();
    root.reset();
}


/*
*  @brief Searches LeafNode that contains key
* 
*/
std::shared_ptr<LeafNode> BalancedIndex::findLeafNode(uint64_t key) {
    std::vector<uint64_t> stack;
    std::shared_ptr<Node> node = root;
    std::shared_ptr<InnerNode> innerNode;
    uint32_t childIndex;

#ifdef _DEBUG
    std::cout << std::endl;
    std::cout << "Searching for a leaf node starting from root node (" << root->position << ")" << std::endl;
#endif

    while (node->getNodeType() == NodeType::INNER) {
        childIndex = node->search(key);
        innerNode = std::dynamic_pointer_cast<InnerNode>(node);
        uint64_t storagePos = innerNode->getChildAt(childIndex);
        if (std::find(stack.begin(), stack.end(), storagePos) != stack.end()) {
            std::stringstream ss;
            ss << "Cyclic references in index tree!\n";
            for (const auto& val : stack) ss << val << " -> ";
            ss << storagePos << std::endl;

            throw std::runtime_error(ss.str());
        }
        stack.push_back(storagePos);
        node = Node::loadNode(*this, storagePos);
#ifdef _DEBUG
        std::cout << "Drill down to the node (" << node->position << ")" << std::endl;
#endif
    }
#ifdef _DEBUG
    std::cout << "Leaf node found (" << node->position << ")!" << std::endl;
#endif
    return std::dynamic_pointer_cast<LeafNode>(node);
}


/*
*  @brief Set new index root InnerNode and update 
*/
void BalancedIndex::updateRoot(uint64_t newRootPosition) {
#ifdef _DEBUG
    std::cout << "Root node updated from " << root->position << " to " << newRootPosition << std::endl;
#endif    
    // update root position
    indexHeader.rootPosition = newRootPosition;
    // assign new root node
    root = Node::loadNode(*this, newRootPosition);            
    // persist header
    persistIndexHeader();
}


/*
*  @brief Persist balanced index header to storage
*/
void BalancedIndex::persistIndexHeader() {
#ifdef _DEBUG
    std::cout << "Index header persisted" << std::endl;
#endif
    // Header is first record in records file
    recordsFile.first();
    // Persist index header data
    recordsFile.setRecordData(&indexHeader, sizeof indexHeader);
}


/*
*  @brief Insert key/value pair
*/
bool BalancedIndex::insert(uint64_t key, const std::string& value) {

#ifdef _DEBUG
    std::cout << std::endl;
    std::cout << "Inserting key/value pair key=" << key << " value='" << value << "'" << std::endl;
#endif
    
    std::shared_ptr<LeafNode> leaf = findLeafNode(key);

    if (leaf->search(key) != KEY_NOT_FOUND) {
        return false;
    }

    bool isInserted = leaf->insertKey(key, value);
    if (leaf->isOverflow()) {        
        uint64_t rootPos = leaf->dealOverflow();
        // if this is root node position update it
        if (rootPos != NOT_FOUND) {
            updateRoot(rootPos);
        }
        
    }

    indexHeader.recordsCount++;
    persistIndexHeader();

    printTree();

    return isInserted;
}


/*
*  @brief Update key/value pair
*/
bool BalancedIndex::update(uint64_t key, const std::string& value) {
    std::shared_ptr<LeafNode> leaf = findLeafNode(key);
    uint32_t valueIndex = leaf->search(key);
    if (valueIndex == KEY_NOT_FOUND) return false;
    leaf->setValueAt(valueIndex, value);
    return true;
}


std::shared_ptr<std::string> BalancedIndex::search(uint64_t key) {
    std::shared_ptr<LeafNode> leaf = findLeafNode(key);    
    uint32_t index = leaf->search(key);
    // FIXME: For non-pointer types returning nullptr is invalid 
    return (index == KEY_NOT_FOUND) ? nullptr : leaf->getValueAt(index);
}



bool BalancedIndex::erase(uint64_t key) {

#ifdef _DEBUG
    std::cout << "Erasing key/value pair key=" << key << std::endl;
#endif

    std::shared_ptr<LeafNode> leaf = findLeafNode(key);

    if (leaf->deleteKey(key)) {
        if (leaf->isUnderflow()) {
            uint64_t newRootPos = leaf->dealUnderflow();
            if (newRootPos != NOT_FOUND) {
                std::shared_ptr<Node> newRoot = Node::loadNode(*this, newRootPos);
                newRoot->setParent(NOT_FOUND);
                updateRoot(newRootPos);
            }
        }
#ifdef _DEBUG
        std::cout << "Key/value pair deleted " << key << std::endl;
#endif
        indexHeader.recordsCount--;
        persistIndexHeader();
        return true;

    }    

    return false;
}


bool BalancedIndex::first() {
    return false;
}

bool BalancedIndex::last() {
    return false;
}

bool BalancedIndex::next() {
    return false;
}

bool BalancedIndex::previous() {
    return false;
}


RecordFileIO& BalancedIndex::getRecordsFile() {
    return recordsFile;
}



void BalancedIndex::printTree() {
    std::cout << "======================================================================================\n";
    std::cout << " TREE STATE\n";
    std::cout << "======================================================================================\n";
    std::cout << "Root node postion: " << indexHeader.rootPosition << " ";
    std::cout << "Records count: " << indexHeader.recordsCount << std::endl;
    printTreeLevel(root, 0);
}


void BalancedIndex::printTreeLevel(std::shared_ptr<Node> node, int level) {            
    for (int t = 0; t < level; t++) std::cout << "    ";
    uint64_t left = node->getLeftSibling();
    uint64_t right = node->getRightSibling();
    std::shared_ptr<std::string> nodeStr = node->toString();
    std::cout << *nodeStr;
    std::cout << " (LEFT: " << (left == NOT_FOUND ? 0 : left);
    std::cout << " RIGHT: " << (right == NOT_FOUND ? 0 : right) << ")";
    std::cout << " : " << node->position << "\n";
    if (node->getNodeType() == NodeType::INNER) {
        for (uint32_t i = 0; i < node->data.childrenCount; i++) {
            std::shared_ptr<Node> chld = Node::loadNode(*this, node->data.children[i]);
            printTreeLevel(chld, level + 1);
        }        
    }
}



