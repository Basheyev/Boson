/******************************************************************************
*
*  Balanced Index
*
*  (C) Boson Database, Bolat Basheyev 2022
*
******************************************************************************/



#include <memory>
#include <algorithm>
#include <sstream>
#include <ios>

#include "BalancedIndex.h"


using namespace Boson;



BalancedIndex::BalancedIndex(RecordFileIO& rf) : records(rf) {
    // check if file is open
    if (!rf.isOpen()) throw std::runtime_error("Can't open file.");
    // Check if file has its first record as DB header
    if (!records.first()) {
        uint64_t referencePos = records.createRecord(&rootPosition, sizeof rootPosition);
        // root record
        root = std::make_shared<LeafNode>(*this);
        rootPosition = root->persist();
        records.setPosition(referencePos);
        records.setRecordData(&rootPosition, sizeof rootPosition);
    } else {
        // look up root position
        records.getRecordData(&rootPosition, sizeof rootPosition);
        // load root record
        root = Node::loadNode(*this, rootPosition);
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
    while (node->getNodeType() == NodeType::INNER) {
        childIndex = node->search(key);
        innerNode = std::dynamic_pointer_cast<InnerNode>(node);
        uint64_t storagePos = innerNode->getChildAt(childIndex);
        if (std::find(stack.begin(), stack.end(), storagePos) != stack.end()) {
            std::stringstream ss;
            ss << "Cyclic references in index tree!\n";
            for (const auto& val : stack) {
                ss << val << "\n";  // Add each integer to the stream
            }                        
            throw std::runtime_error(ss.str());
        }
        stack.push_back(storagePos);
        node = Node::loadNode(*this, storagePos);
    }
    return std::dynamic_pointer_cast<LeafNode>(node);
}


/*
*  @brief set new index root InnerNode and update 
*/
void BalancedIndex::updateRoot(uint64_t newRootPosition) {
    // assign new root and update its position
    root = std::dynamic_pointer_cast<InnerNode>(Node::loadNode(*this, newRootPosition));
    rootPosition = root->getPosition();
    // update header
    records.first();
    records.setRecordData(&rootPosition, sizeof rootPosition);
}

/*
*  @brief Insert key/value pair
*/
bool BalancedIndex::insert(uint64_t key, const std::string& value) {
    
    std::shared_ptr<LeafNode> leaf = findLeafNode(key); // FIXME: bug

    bool isInserted = leaf->insertKey(key, value);
    if (leaf->isOverflow()) {
        uint64_t newRootPos = leaf->dealOverflow();
        if (newRootPos != NOT_FOUND) {
            updateRoot(newRootPos);
        }
    }
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
    std::shared_ptr<LeafNode> leaf = findLeafNode(key);
    if (leaf->deleteKey(key)) {
        if (leaf->isUnderflow()) {
            uint64_t newRootPos = leaf->dealUnderflow();
            if (newRootPos != NOT_FOUND) {
                std::shared_ptr<LeafNode> newRoot =
                    std::dynamic_pointer_cast<LeafNode>(Node::loadNode(*this, newRootPos));
                newRoot->setParent(NOT_FOUND);
                updateRoot(newRootPos);
            }
        }
    }
    return true;
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
    return records;
}


