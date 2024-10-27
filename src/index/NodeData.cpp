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

/*
* @brief Creates node data class and sets all fields to zero
*/
NodeData::NodeData() {
    memset(this, 0, sizeof NodeData);
}


/*
* @brief Adds new value to the end of specified array in the node
* @param mode type of array in the node
* @param value to add
*/
void NodeData::pushBack(NodeArray mode, uint64_t value) {
    uint64_t* values = (mode==NodeArray::KEYS) ? keys : children;
    uint32_t& length = (mode==NodeArray::KEYS) ? keysCount : childrenCount;
    uint32_t  max    = (mode==NodeArray::KEYS) ? MAX_DEGREE : TREE_ORDER;
    if (length < max) {
        values[length] = value;
        length++;
    }
    else std::cerr << "NodeData Push Back: can't add new value, array is full." << std::endl;
}


/*
* @brief Inserts new value at index position of specified array in the node
* @param mode type of array in the node
* @param index index of position in array
* @param value to add
*/
void NodeData::insertAt(NodeArray mode, uint32_t index, uint64_t value) {
    uint64_t* values = (mode == NodeArray::KEYS) ? keys : children;
    uint32_t& length = (mode == NodeArray::KEYS) ? keysCount : childrenCount;
    uint32_t  max = (mode == NodeArray::KEYS) ? MAX_DEGREE : TREE_ORDER;
    
    // check boundaries
    if (index < 0 || index > length || length > max) {
        // TODO: how to handle in the node or here?
        std::cerr << "NodeData Insert At: invalid index or array is full!" << std::endl;
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



/*
* @brief Deletes value at index position of specified array in the node
* @param mode type of array in the node
* @param index index of position in array
*/
void NodeData::deleteAt(NodeArray mode, uint32_t index) {
    uint64_t* values = (mode == NodeArray::KEYS) ? keys : children;
    uint32_t& length = (mode == NodeArray::KEYS) ? keysCount : childrenCount;
    uint32_t  max = (mode == NodeArray::KEYS) ? MAX_DEGREE : TREE_ORDER;

    // check boundaries
    if (index < 0 || index >= max) {
        std::cerr << "NodeData Delete At: Invalid index" << std::endl;
        return;
    }

    // Shift elements to the left
    for (uint32_t i = index; i < max - 1; ++i) {
        values[i] = values[i + 1];
    }

    // clear deleted value for debug purposes
    values[length - 1] = 0;

    // decrease length counter
    length--;
}


/*
* @brief Resizes specified array in the node
* @param mode type of the array in the node
* @param newSize new size of the array
*/
void NodeData::resize(NodeArray mode, uint32_t newSize) {
    if (mode == NodeArray::KEYS) {
        // clear deleted keys for debug purposes
        if (newSize < keysCount) {
            uint32_t gap = keysCount - newSize;
            memset(&keys[newSize], 0, gap * sizeof uint64_t);
        }
        keysCount = newSize;
    }
    else {
        // clear deleted children/values for debug purposes
        if (newSize < childrenCount) {
            uint32_t gap = childrenCount - newSize;
            memset(&children[newSize], 0, gap * sizeof uint64_t);
        }
        childrenCount = newSize;        
    }
}