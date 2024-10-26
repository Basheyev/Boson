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
*  @brief Searches LeafNode that contains the key
*  @param key to search
*  @return leaf node that possibly contains the key
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
*  @param newRootPosition
*/
void BalancedIndex::updateRoot(uint64_t newRootPosition) {
    // update root position
    indexHeader.rootPosition = newRootPosition;
    // assign new root node or update from storage if content of root changed
    root = Node::loadNode(*this, newRootPosition);            
#ifdef _DEBUG
    std::cout << "Root node updated from " << root->position << " to " << newRootPosition << std::endl;
#endif    
}


/*
*  @brief Persist balanced index header to storage
*/
void BalancedIndex::persistIndexHeader() {
#ifdef _DEBUG
    std::cout << "Index header persisted: root=" 
        << indexHeader.rootPosition << ", records count="
        << indexHeader.recordsCount
        << std::endl;
#endif
    // Header is first record in records file
    recordsFile.first();
    // Persist index header data
    recordsFile.setRecordData(&indexHeader, sizeof indexHeader);
}


/*
*  @brief Insert key/value pair
*  @param key to insert
*  @param value to insert
*  @return true if succeeded or false otherwise
*/
bool BalancedIndex::insert(uint64_t key, const std::string& value) {

#ifdef _DEBUG    
    std::cout << "-----------------------------------------------------------------------" << std::endl;
    std::cout << "Inserting key/value pair key=" << key << " value='" << value << "'" << std::endl;
#endif
    // Traverse down the tree to a leaf node that can contain the key
    std::shared_ptr<LeafNode> leaf = findLeafNode(key);
    // if key found, then we can't insert duplicate - return false
    if (leaf->search(key) != KEY_NOT_FOUND) return false;    
    // Otherwise inser key to the leaf node    
    if (!leaf->insertKey(key, value)) return false;
    // If succeeded increment records counter
    indexHeader.recordsCount++;
    // if leaf node overflow detected then deal overflow
    if (leaf->isOverflow()) {        
        uint64_t rootPos = leaf->dealOverflow();
        // if this is root node position update it
        if (rootPos != NOT_FOUND) updateRoot(rootPos);
    }
    // Persist index header if root node possibly affected
    persistIndexHeader();
#ifdef _DEBUG   
    this->printTree();
#endif
    // return true because key/value pair successfuly inserted
    return true;
}


/*
*  @brief Update key/value pair
*  @param key to update
*  @param value new value assigned to key
*  @return true if succeeded or false otherwise
*/
bool BalancedIndex::update(uint64_t key, const std::string& value) {
    // Traverse down the tree to a leaf node that can contain the key
    std::shared_ptr<LeafNode> leaf = findLeafNode(key);
    // Get key index in the leaf node
    uint32_t valueIndex = leaf->search(key);
    // if key is not found, then we can't update it - return false
    if (valueIndex == KEY_NOT_FOUND) return false;
    // update value in the leaf node
    leaf->setValueAt(valueIndex, value);
    // persist leaf node
    leaf->persist();
    // return that everything is OK
    return true;
}


/*
*  @brief Searches and returns value by key
*  @param key requested
*  @return std::shared_ptr<string> if succeded or nullptr otherwise
*/
std::shared_ptr<std::string> BalancedIndex::search(uint64_t key) {
    // Traverse down the tree to a leaf node that can contain the key
    std::shared_ptr<LeafNode> leaf = findLeafNode(key);    
    // Get key index in the leaf node
    uint32_t index = leaf->search(key);
    // if key is not found, then we can't update it - return nullptr
    if (index == KEY_NOT_FOUND) return nullptr;
    // if key is found, then return value
    return leaf->getValueAt(index);
}



/*
*  @brief Deletes key/value pair
*  @param key requested
*/
bool BalancedIndex::erase(uint64_t key) {

#ifdef _DEBUG
    std::cout << "Erasing key/value pair key=" << key << std::endl;
#endif
    // Traverse down the tree to a leaf node that can contain the key
    std::shared_ptr<LeafNode> leaf = findLeafNode(key);
    // if key is successfuly deleted
    if (leaf->deleteKey(key)) {
        // if underflow appears
        if (leaf->isUnderflow()) {
            // deal underflow
            uint64_t newRootPos = leaf->dealUnderflow();
            // if root changed
            if (newRootPos != NOT_FOUND) {                
                updateRoot(newRootPos);
            } else {
                // update the root anyway if the data possibly changed
                root = Node::loadNode(*this, indexHeader.rootPosition);
            }
        }
#ifdef _DEBUG
        std::cout << "Key/value pair deleted " << key << std::endl;
#endif
        // decrease records counter
        indexHeader.recordsCount--;
        // persist index header
        persistIndexHeader();
#ifdef _DEBUG   
        this->printTree();
#endif
        return true;
    }    

    return false;
}


bool BalancedIndex::first() {
    // Traverse down the tree to a leaf node that can contain the first key
    std::shared_ptr<LeafNode> leaf = findLeafNode(0); // Zero is minimal key value

    // todo 

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
    std::cout << " : " << node->position << std::endl;
    if (node->getNodeType() == NodeType::INNER) {
        for (uint32_t i = 0; i < node->data.childrenCount; i++) {
            std::shared_ptr<Node> chld = Node::loadNode(*this, node->data.children[i]);
            printTreeLevel(chld, level + 1);
        }        
    }
}



