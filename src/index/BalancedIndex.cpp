/******************************************************************************
*
*  Balanced Index
*
*  (C) Boson Database, Bolat Basheyev 2022
*
******************************************************************************/


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



Node& BalancedIndex::getNode(uint64_t position) {

    // This method replaces loads Node from record file
    // to memory and returns reference. 
    
    // How to manage memory?
    // How to validate referneces if they been invalidated?

    // 
    records.setPosition(position);
    uint32_t dataLength = records.getDataLength();
    // 
    uint8_t* buffer = new uint8_t[dataLength];

    records.getRecordData(buffer, dataLength);
    // warning - it's just a stub, not working
    Node* node = (Node*) buffer;
    
    return *node;
}
