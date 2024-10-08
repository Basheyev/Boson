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
* @brief Inner Node Constructor (calls Node Constructor)
*/
InnerNode::InnerNode(BalancedIndex& bi, uint64_t offsetInFile) : Node(bi) {
    position = offsetInFile;
    isPersisted = true;
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


/*
*  @brief Split this inner node by half
*/
uint64_t InnerNode::split() {

    // Calculate mid index
    uint32_t midIndex = this->getKeyCount() / 2;

    // Create new node
    std::unique_ptr<InnerNode> newNode = std::make_unique<InnerNode>(this->index);

    // Copy keys from this node to new splitted node
    for (size_t i = midIndex + 1; i < data.keysCount; ++i) {
        // copy keys to new node
        newNode->data.pushBack(NodeArray::KEYS, data.keys[i]);
    }

    // truncate this node's keys list
    this->data.resize(NodeArray::KEYS, midIndex);

    // Copy childrens from this node to new splitted node
    for (size_t i = midIndex + 1; i < data.childrenCount; ++i) {
        // reattach children to new splitted node
        uint64_t childPos = data.children[i];
        std::unique_ptr<InnerNode> child = std::make_unique<InnerNode>(this->index, childPos);
        child->setParent(newNode->position);
        child->persist();
        // copy childrens to the new node
        newNode->data.pushBack(NodeArray::CHILDREN, childPos);
    }
    
    // truncate this node's children list
    this->data.resize(NodeArray::CHILDREN, midIndex + 1);

    // Persist changes in splitted nodes
    this->persist();
    newNode->persist();

    // return splitted node
    return newNode->position;
}


/*
* @brief Set key at specified index propogated from child
* @param key
* @param leftChild
* @param rightChild
*/
uint64_t InnerNode::pushUpKey(uint64_t key, uint64_t leftChild, uint64_t rightChild) {

    // search key index in this node
    uint32_t index = search(key);

    // insert key at specified index with left and right child
    insertAt(index, key, leftChild, rightChild);

    // if there is the node overflow
    if (isOverflow()) return dealOverflow();

    // if this is the root node then return this node's position in storage file
    if (getParent() == NOT_FOUND) return this->position;

    // return null pointer
    return NOT_FOUND;
}



/*
* @brief Borrow children by specifying Borrower, Lender and borrow index
* @param borrowerPos
* @param lender
* @param borrowIndex
*/
void InnerNode::borrowChildren(uint64_t borrowerPos, uint64_t lender, uint32_t borrowIndex) {
    uint32_t borrowerChildIndex = 0;

    std::unique_ptr<InnerNode> borrower = std::make_unique<InnerNode>(this->index, borrowerPos);

    // find borrower child index
    for (uint32_t i = 0; i < data.childrenCount; i++) {
        if (data.children[i] == borrowerPos) {
            borrowerChildIndex = i;
            break;
        }
    }

    // Process borrowing
    if (borrowIndex == 0) {
        // borrow from right sibling
        uint64_t theKey = data.keys[borrowerChildIndex];
        uint64_t upKey = borrower->borrowFromSibling(theKey, lender, borrowIndex);
        data.keys[borrowerChildIndex] = upKey;
        isPersisted = false;
    }
    else {
        // borrow from left sibling
        uint64_t theKey = data.keys[borrowerChildIndex - 1];
        uint64_t upKey = borrower->borrowFromSibling(theKey, lender, borrowIndex);
        data.keys[borrowerChildIndex - 1] = upKey;
        isPersisted = false;
    }
        
    persist();

}



/*
* @brief Borrow key with children from sibling node
* @param key
* @param sibling
* @param borrowIndex
*/
uint64_t InnerNode::borrowFromSibling(uint64_t key, uint64_t sibling, uint32_t borrowIndex) {
    
    std::unique_ptr<InnerNode> siblingNode = std::make_unique<InnerNode>(this->index, sibling);
    std::shared_ptr<Node> childNode;
    uint64_t childNodePos = 0;
    uint64_t upKey = 0;

    if (borrowIndex == 0) {
        // borrow the first key from right sibling, append it to tail	
        // get sibling child node
        childNodePos = siblingNode->getChildAt(borrowIndex);
        childNode = Node::loadNode(this->index, childNodePos);
        // reattach childNode to this node as parent
        childNode->setParent(this->position);
        // append borrowed key and child node to the tail of list
        data.pushBack(NodeArray::KEYS, key);
        data.pushBack(NodeArray::CHILDREN, childNodePos);
        // get key propogated to parent node
        upKey = siblingNode->getKeyAt(0);

        // delete the first key with children from sibling node
        siblingNode->data.deleteAt(NodeArray::KEYS, 0);
        siblingNode->data.deleteAt(NodeArray::CHILDREN, 0);        
    }
    else {
        // borrow the last key from left sibling, insert it to head
        childNodePos = siblingNode->getChildAt(borrowIndex + 1);
        childNode = Node::loadNode(this->index, childNodePos);
        // reattach childNode to this node as parent
        childNode->setParent(this->position);
        // insert borrowed key and child node to the list at beginning
        insertAt(0, key, childNodePos, data.children[0]);
        // get key propogated to parent node
        upKey = siblingNode->getKeyAt(borrowIndex);
        // delete key with children from sibling node
        siblingNode->deleteAt(borrowIndex);
    }

    this->persist();
    childNode->persist();

    return upKey;
}


uint64_t InnerNode::mergeChildren(uint64_t leftChild, uint64_t rightChild) {
    return NOT_FOUND;
}


void InnerNode::mergeWithSibling(uint64_t key, uint64_t rightSibling) {

}



NodeType InnerNode::getNodeType() {
    return NodeType::INNER;
}

