/******************************************************************************
*
*  NodeData class implementation
*
*  (C) Boson Database, Bolat Basheyev 2022-2024
*
******************************************************************************/

#include "BalancedIndex.h"
#include <iostream>

using namespace Boson;


NodeData::NodeData() {
    memset(this, 0, sizeof NodeData);
}


void NodeData::pushBack(NodeArray mode, uint64_t value) {
    uint64_t* values = (mode==NodeArray::KEYS) ? keys : children;
    uint32_t& length = (mode==NodeArray::KEYS) ? keysCount : childrenCount;
    uint32_t  max    = (mode==NodeArray::KEYS) ? MAX_DEGREE : TREE_ORDER;
    if (length < max) {
        values[length] = value;
        length++;
    }
    else std::cerr << "NodeData: can't add new value, array is full." << std::endl;
}


void NodeData::insertAt(NodeArray mode, uint32_t index, uint64_t value) {
    uint64_t* values = (mode == NodeArray::KEYS) ? keys : children;
    uint32_t& length = (mode == NodeArray::KEYS) ? keysCount : childrenCount;
    uint32_t  max = (mode == NodeArray::KEYS) ? MAX_DEGREE : TREE_ORDER;
    
    // check boundaries
    if (index < 0 || index > length || length >= max) {
        std::cerr << "Error: invalid index or array is full!" << std::endl;
        return;
    }

    // shift right elements 
    for (uint32_t i = length; i > index; --i) {
        values[i] = values[i - 1];
    }
    
    // insert value
    values[index] = value;
    // increase length counter
    length++;
}


void NodeData::deleteAt(NodeArray mode, uint32_t index) {
    uint64_t* values = (mode == NodeArray::KEYS) ? keys : children;
    uint32_t& length = (mode == NodeArray::KEYS) ? keysCount : childrenCount;
    uint32_t  max = (mode == NodeArray::KEYS) ? MAX_DEGREE : TREE_ORDER;

    // check boundaries
    if (index < 0 || index >= max) {
        std::cerr << "Error: Invalid index" << std::endl;
        return;
    }

    // Shift elements to the left
    for (uint32_t i = index; i < max - 1; ++i) {
        values[i] = values[i + 1];
    }

    // decrease length counter
    length--;
}