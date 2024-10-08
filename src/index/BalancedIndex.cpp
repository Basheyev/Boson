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


