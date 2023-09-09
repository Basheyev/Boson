


#include "BalancedIndex.h"
#include <ios>

using namespace Boson;


/*
* @brief Loads node data from specified position in file
*/
Node::Node(BalancedIndex& bi, uint64_t offsetInFile) 
    : index(bi), position(offsetInFile)
{    
    // load node data from specified offset in file
    RecordFileIO& recordsFile = index.getRecordsFile();
    recordsFile.setPosition(position);
    uint64_t offset = recordsFile.getRecordData(&data, sizeof NodeData);
    // Throw exception if file not open, can't read or checksum check failed
    if (offset == NOT_FOUND) {
        // TODO: maybe we can provide more useful information
        throw std::ios_base::failure("Can't read node data.");
    } 
    // Set flag that data is already persisted
    isPersisted = true;
}


/*
* @brief Checks if node data persisted
*/
Node::~Node() {
    if (!isPersisted) persist();
}


/*
*
* @brief Persists node data to the storage
*
* @return returns current offset of record or NOT_FOUND if fails
*
*/
void Node::persist() {
    // write node data to specified position
    RecordFileIO& recordsFile = index.getRecordsFile();
    recordsFile.setPosition(position);    
    // Throw exception if file not open or can't write
    uint64_t offset = recordsFile.setRecordData(&data, sizeof NodeData);
    if (offset == NOT_FOUND) {
        throw std::ios_base::failure("Can't persist node data.");
    }
    // Offset of record in the file could have been changed, so we update it
    position = offset;
    // Set flag that data is already persisted
    isPersisted = true;
}


uint32_t Node::getKeyCount() {
    return data.keysCount;
}


bool Node::isOverflow() {
    return data.keysCount > MAX_DEGREE;
}


bool Node::isUnderflow() {
    return data.keysCount < MIN_DEGREE;
}


bool Node::canLendAKey() {
    // TODO
    return false;
}


uint64_t Node::getKeyAt(uint32_t index) {
    // TODO
    return NOT_FOUND;
}


void Node::setKeyAt(uint32_t index, uint64_t key) {
    // TODO
}


uint64_t Node::getParent() {
    // TODO
    return data.parent;
}


void Node::setParent(uint64_t parentPosition) {
    // TODO
    // what if record position changed
}


uint64_t Node::getLeftSibling() {
    return NOT_FOUND;
}


void Node::setLeftSibling(uint64_t silbingPosition) {

}


uint64_t Node::getRightSibling() {
    return NOT_FOUND;
}


void Node::setRightSibling(uint64_t siblingPosition) {

}


uint64_t Node::dealOverflow() {
    return NOT_FOUND;
}


uint64_t Node::dealUnderflow() {
    return NOT_FOUND;
}

