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



BalancedIndex::BalancedIndex(RecordFileIO& rf) : records(rf) {
    rootPosition = 0;
    cursorPosition = 0;
}


BalancedIndex::~BalancedIndex() {

}


bool BalancedIndex::insert(uint64_t key, const std::string& value) {
    return false;
}


bool BalancedIndex::update(uint64_t key, const std::string& value) {
    return false;
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


/**
* 
* 
* 
*/
std::shared_ptr<Node> BalancedIndex::getNode(uint64_t position) {

    NodeData data;
    // load node data from specified offset in file    
    records.setPosition(position);
    uint64_t offset = records.getRecordData(&data, sizeof NodeData);
    // Throw exception if file not open, can't read or checksum check failed
    if (offset == NOT_FOUND) {
        // TODO: maybe we can provide more useful information
        throw std::ios_base::failure("Can't read node data.");
    }

    std::shared_ptr<Node> node;
    // Allocated memory for requered type of node
    if (data.nodeType == NodeType::INNER) {            
        node = std::make_shared<InnerNode>(*this, position);
    } else node = std::make_shared<LeafNode>(*this, position);

    // copy loaded data and turn on persisted flag
    memcpy(&(node->data), &data, sizeof(NodeData));
    node->isPersisted = true;

    return node;
}
