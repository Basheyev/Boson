/******************************************************************************
*
*  Node class implementation
*
*  (C) Boson Database, Bolat Basheyev 2022-2024
*
******************************************************************************/


#include "BalancedIndex.h"
#include <ios>

using namespace Boson;

/*
* @brief Creates new index node in file
*/
Node::Node(BalancedIndex& bi, NodeType type) : index(bi) {   
    
    // initialize values    
    this->data.nodeType = type;
    
    // allocate space in file
    RecordFileIO& recordFile = index.getRecordsFile();
    uint64_t offset = recordFile.createRecord(&data, sizeof NodeData);
    if (offset == NOT_FOUND) {
        throw std::ios_base::failure("Can't write node data.");
    }
    this->position = offset;
    this->isPersisted = true;
}


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
* @brief Checks if node data is persisted
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


NodeType Node::getNodeType() {
    return data.nodeType;
}

/*
*  @brief Returns keys count inside Node
*  @return total keys count inside node
*/
uint32_t Node::getKeyCount() {
    return data.keysCount;
}


/*
*  @brief Returns whether node keys count > M-1
*  @return true if keys count more than MAX_DEGREE
*/
bool Node::isOverflow() {
    return data.keysCount > MAX_DEGREE;
}


/*
*  @brief Returns whether node keys count < M / 2
*  @return true if keys count less than MIN_DEGREE
*/
bool Node::isUnderflow() {
    return data.keysCount < MIN_DEGREE;
}


/*
*  @brief Returns whether node keys count > M / 2
*  @return true if keys count more than MIN_DEGREE
*/
bool Node::canLendAKey() {
    return data.keysCount > MIN_DEGREE;
}


/*
*  @brief Returns key at specified index
*/
uint64_t Node::getKeyAt(uint32_t index) {
    if (index >= data.keysCount) return NOT_FOUND;
    return data.keys[index];
}


/*
*  @brief Sets key at specified index
*/
void Node::setKeyAt(uint32_t index, uint64_t key) {
    if (index >= data.keysCount) return;
    data.keys[index] = key;
    this->isPersisted = false;
}


/*
*  @brief Returns Parent node position in file
*/
uint64_t Node::getParent() {
    return data.parent;
}


/*
*  @brief Sets Parent node position in file
*/
void Node::setParent(uint64_t parentPosition) {
    // TODO what if record position changed (not real?)
    data.parent = parentPosition;
    this->isPersisted = false;
}


uint64_t Node::getLeftSibling() {
    return data.leftSibling;
}


void Node::setLeftSibling(uint64_t siblingPosition) {
    data.leftSibling = siblingPosition;
    this->isPersisted = false;
}


uint64_t Node::getRightSibling() {
    return data.rightSibling;
}


void Node::setRightSibling(uint64_t siblingPosition) {
    data.rightSibling = siblingPosition;
    this->isPersisted = false;
}



uint64_t Node::dealOverflow() {
    
    // Get key at middle index for propagation to the parent node
    uint32_t midIndex = this->getKeyCount() / 2;
    uint64_t upKey = this->getKeyAt(midIndex);

    // Split this node by half (returns new splitted node)
    std::shared_ptr<Node> newRightNode = index.getNode(split());

    // if we are splitting the root node
    if (getParent() == NOT_FOUND) {
        // create new root node and set as parent to this node (grow at root)
        std::unique_ptr<InnerNode> newRootNode = std::make_unique<InnerNode>(index);        
        this->setParent(newRootNode->position);
        this->persist();
    }

    // Interconnect splitted node's parent and siblings
    newRightNode->setParent(getParent());
    newRightNode->setLeftSibling(this->position);
    newRightNode->setRightSibling(this->getRightSibling());
    newRightNode->persist();
    if (this->getRightSibling() != NOT_FOUND) {
        std::shared_ptr<Node> theRightSibling = index.getNode(getRightSibling());
        theRightSibling->setLeftSibling(newRightNode->position);
    }

    this->setRightSibling(newRightNode->position);

    // Push middle key up to parent the node (root node returned)
    std::shared_ptr<Node> theParent = index.getNode(getParent());
    uint64_t rootNode = theParent->pushUpKey(upKey, position, newRightNode->position);
    
    // Return current root node position
    return rootNode;

}



uint64_t Node::dealUnderflow() {

    // if this is the root node, then do nothing and return
    if (this->getParent() == NOT_FOUND) return NOT_FOUND;
    uint64_t leftSiblingPos = getLeftSibling();
    uint64_t rightSiblingPos = getRightSibling();

    // 1. Try to borrow top key from left sibling    
    if (leftSiblingPos != NOT_FOUND) {
        std::shared_ptr<Node> leftSibling = index.getNode(leftSiblingPos);
        if (leftSibling->canLendAKey() && leftSibling->getParent() == this->getParent()) {
            uint32_t keyIndex = leftSibling->getKeyCount() - 1;
            std::shared_ptr<Node> parent = index.getNode(this->getParent());
            parent->borrowChildren(position, leftSiblingPos, keyIndex);
            parent->persist();
            return NOT_FOUND;
        }
    }

    // 2. Try to borrow lower key from right sibling
    if (rightSiblingPos != NOT_FOUND) {
        std::shared_ptr<Node> rightSibling = index.getNode(rightSiblingPos);
        if (rightSibling->canLendAKey() && rightSibling->getParent() == this->getParent()) {
            uint32_t keyIndex = 0;
            std::shared_ptr<Node> parent = index.getNode(this->getParent());
            parent->borrowChildren(position, rightSiblingPos, keyIndex);
            parent->persist();
            return NOT_FOUND;
        }
    }

    // 3. Try to merge with left sibling
    if (leftSiblingPos != NOT_FOUND) {  
        std::shared_ptr<Node> leftSibling = index.getNode(leftSiblingPos); 
        if (leftSibling->getParent() == this->getParent()) {  
            std::shared_ptr<Node> parent = index.getNode(this->getParent());
            uint64_t rootNodePos = parent->mergeChildren(leftSiblingPos, this->position);
            parent->persist();
            return rootNodePos;
        }
    } 
    
    // 4. Try to merge with right sibling        
    std::shared_ptr<Node> parent = index.getNode(this->getParent());
    uint64_t rootNodePos = parent->mergeChildren(this->position, rightSiblingPos);
    parent->persist();
    return rootNodePos;

}

