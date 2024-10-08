/******************************************************************************
*
*  InnerNode class implementation
*
*  (C) Boson Database, Bolat Basheyev 2022-2024
*
******************************************************************************/

#include "BalancedIndex.h"


using namespace Boson;


/*
* @brief Inner Node Constructor (calls Node Constructor)
*/
InnerNode::InnerNode(BalancedIndex& bi) : Node(bi, NodeType::INNER) {
    
}


/*
* @brief Inner Node Constructor from storage file position (calls Node Constructor)
*/
InnerNode::InnerNode(BalancedIndex& bi, uint64_t offsetInFile) : Node(bi, offsetInFile) {

}


/*
* @brief Inner Node Destructor
*/
InnerNode::~InnerNode() {
    Node::~Node();
}


/*
* @brief Return child node index of specified key
* @param key required key
* @return index of child node of the specified key
*/
uint32_t InnerNode::search(uint64_t key) {
    uint32_t index = 0;
    uint64_t entry;

    // Look up for a child node index that contains the key
    for (index = 0; index < data.keysCount; index++) {
        entry = data.keys[index];              // get keys entry at specified index
        if (key == entry) return index + 1;    // if key is found then return right child index
        else if (key < entry) return index;    // if key is less than entry left child index
    }

    return index;
}


/*
* @brief Returns child node at specified index
* @param index required child node index
* @return child node position in storage file
*/
uint64_t InnerNode::getChildAt(uint32_t index) {
    return data.children[index];
}



/*
* @brief Sets child node at specified index
* @param index required child node index
* @param child node position in storage file
*/
void InnerNode::setChildAt(uint32_t index, uint64_t childNode) {
    data.children[index] = childNode;
    isPersisted = false;
}


/*
* @brief Inserts key with left and right child at specified index
* @param index position of new key
* @param key value
* @param leftChild storage file position of left child node
* @param rightChild storage file position of right child node
*/
void InnerNode::insertAt(uint32_t index, uint64_t key, uint64_t leftChild, uint64_t rightChild) {
    
    // Insert the key at specified index
    data.insertAt(NodeArray::KEYS, index, key);

    // Insert left child at specified index
    data.insertAt(NodeArray::CHILDREN, index, leftChild);

    // Right child index
    uint32_t rightChildIndex = index + 1;
    
    // If there is child at this index then
    if (rightChildIndex < data.childrenCount)
        // overwrite child value to new rightChild
        data.children[rightChildIndex] = rightChild;   
    else
        // add right child to the end of children list
        data.pushBack(NodeArray::CHILDREN, rightChild);  

}


/*
*  @brief Delete key and right children at specified index
*  @param index of key to delete
*/
void InnerNode::deleteAt(uint32_t index) {
    data.deleteAt(NodeArray::KEYS, index);
    data.deleteAt(NodeArray::CHILDREN, index + 1);
    isPersisted = false;
}



uint64_t InnerNode::split() {
    return NOT_FOUND;
}


uint64_t InnerNode::pushUpKey(uint64_t key, uint64_t leftChild, uint64_t rightChild) {
    return NOT_FOUND;
}


void InnerNode::borrowChildren(uint64_t borrower, uint64_t lender, uint32_t borrowIndex) {

}


uint64_t InnerNode::mergeChildren(uint64_t leftChild, uint64_t rightChild) {
    return NOT_FOUND;
}


void InnerNode::mergeWithSibling(uint64_t key, uint64_t rightSibling) {

}


uint64_t InnerNode::borrowFromSibling(uint64_t key, uint64_t sibling, uint32_t borrowIndex) {
    return NOT_FOUND;
}


NodeType InnerNode::getNodeType() {
    return NodeType::INNER;
}

