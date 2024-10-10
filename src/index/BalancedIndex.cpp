/******************************************************************************
*
*  Balanced Index
*
*  (C) Boson Database, Bolat Basheyev 2022
*
******************************************************************************/



#include <memory>
#include <ios>

#include "BalancedIndex.h"


using namespace Boson;

// TODO: implementation

BalancedIndex::BalancedIndex(RecordFileIO& rf) : records(rf) {
    // check if file is open
    if (!rf.isOpen()) throw std::runtime_error("Can't open file.");
    // Check if file has its first record as DB header
    if (!records.first()) {
        records.createRecord(&rootPosition, sizeof rootPosition);
        // root record
        root = std::make_shared<InnerNode>(*this);
        uint64_t rootPos = root->persist();
    } else {
        // look up root position
        records.getRecordData(&rootPosition, sizeof rootPosition);
        // load root record
        root = std::dynamic_pointer_cast<InnerNode>
            (Node::loadNode(*this, rootPosition));
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
    std::shared_ptr<Node> node = root;
    std::shared_ptr<InnerNode> innerNode;
    uint32_t childIndex;
    while (node->getNodeType() == NodeType::INNER) {
        childIndex = node->search(key);
        innerNode = std::dynamic_pointer_cast<InnerNode>(node);
        node = Node::loadNode(*this, innerNode->getChildAt(childIndex));
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
    std::shared_ptr<LeafNode> leaf = findLeafNode(key);
    bool isInserted = leaf->insertKey(key, value);
    if (leaf->isOverflow()) {
        uint64_t newRootPos = leaf->dealOverflow();
        if (newRootPos != NOT_FOUND) updateRoot(newRootPos);
    }
    return isInserted;
}


bool BalancedIndex::update(uint64_t key, const std::string& value) {
    std::shared_ptr<LeafNode> leaf = findLeafNode(key);
    uint32_t valueIndex = leaf->search(key);
    if (valueIndex == KEY_NOT_FOUND) return false;
    leaf->setValueAt(valueIndex, value);
    return true;
}


bool BalancedIndex::search(uint64_t key, std::string& value) {
    return false;
}



bool BalancedIndex::erase(uint64_t key) {
    return false;
}

uint64_t BalancedIndex::getEntriesCount() {
    return 0;
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

bool BalancedIndex::getValue(std::string& value) {
    return false;
}


RecordFileIO& BalancedIndex::getRecordsFile() {
    return records;
}


