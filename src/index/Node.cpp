/******************************************************************************
*
*  Node class implementation
*
*  (C) Boson Database, Bolat Basheyev 2022-2024
*
******************************************************************************/


#include "BalancedIndex.h"

using namespace Boson;


Node::Node(BalancedIndex& bi) : index(bi) {
    position = NOT_FOUND;
    isPersisted = true;
}

/*
* @brief Creates new index node in file
* @param bi B+ Tree instance
* @param type required type of node
*/
Node::Node(BalancedIndex& bi, NodeType type) : index(bi) {   
    
    // initialize values    
    this->data.nodeType = type;
    this->data.parent = NOT_FOUND;
    this->data.leftSibling = NOT_FOUND;
    this->data.rightSibling = NOT_FOUND;        
    this->data.keysCount = 0;
    this->data.childrenCount = 0;
        
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
* @brief Loads node data from specified position in storage file
* @param bi B+ Tree instance
* @param offsetInFile offset of node position in storage file
*/
std::shared_ptr<Node> Node::loadNode(BalancedIndex& bi, uint64_t offsetInFile) {

    std::shared_ptr<Node> node;

    // load node data from specified offset in file
    RecordFileIO& recordsFile = bi.getRecordsFile();
    recordsFile.setPosition(offsetInFile);
    NodeData data;
    uint64_t offset = recordsFile.getRecordData(&data, sizeof NodeData);
    if (offset == NOT_FOUND) {
        std::stringstream ss;
        ss << "Can't read node data at " << offsetInFile << " ";
        throw std::ios_base::failure(ss.str());
    }

    // create required node
    if (data.nodeType == NodeType::INNER) {
        node = std::make_shared<InnerNode>(bi, offset, data);
#ifdef _DEBUG
     //   std::cout << "Inner Node loaded (" << node->position << ")" << std::endl;
#endif
    }
    else {
        node = std::make_shared<LeafNode>(bi, offset, data);
#ifdef _DEBUG
     //   std::cout << "Leaf Node loaded (" << node->position << ")" << std::endl;
#endif
    }


    return node;

}


/*
* @brief Deletes node data from specified position in storage file
* @param bi B+ Tree instance
* @param offsetInFile offset of node position in storage file
*/
void Node::deleteNode(BalancedIndex& bi, uint64_t offsetInFile) {    
    RecordFileIO& recordsFile = bi.getRecordsFile();
    recordsFile.setPosition(offsetInFile);
    recordsFile.removeRecord();
}



/*
* @brief Checks if node data is persisted
*/
Node::~Node() {
    if (!isPersisted) {
        persist();
#ifdef _DEBUG
        std::cout << "Node destructed and persisted (" << position << ")" << std::endl;
#endif        
    }
}



/*
* @brief Returns position of the node in the storage file
*/
uint64_t Node::getPosition() {
    return position;
}

/*
* @brief Persists node data to the storage
* @return returns current offset of record or NOT_FOUND if fails
*/
uint64_t Node::persist() {
    // write node data to specified position
    RecordFileIO& recordsFile = index.getRecordsFile();
    recordsFile.setPosition(position);        
    uint64_t offset = recordsFile.setRecordData(&data, sizeof NodeData);
    // Throw exception if file not open or can't write
    if (offset == NOT_FOUND) {
        std::stringstream ss;
        ss << "Can't persist node data at " << position;        
        throw std::ios_base::failure(ss.str());
    }


    // Offset of record in the file could have been changed, so we update it
    if (offset != position) {
        std::cout << "Node migrated in file from " << position << " to " << offset << std::endl;
        position = offset;

    }
    // Set flag that data is already persisted
    isPersisted = true;

#ifdef _DEBUG
    std::cout << "Node at " << position << " is persisted: " << *toString() << std::endl;
#endif

    return position;
}


/*
* @brief Returns type of node
* @return type of node
*/
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
*  @brief Returns if this node is Root
*  @return is root node
*/
bool Node::isRootNode() {
    return data.parent == NOT_FOUND;
}


/*
*  @brief Returns whether node keys or children count > M-1
*  @return true if keys or children count more than MAX_DEGREE
*/
bool Node::isOverflow() {
    return data.keysCount > MAX_DEGREE ||
           data.childrenCount > MAX_DEGREE;
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
*  @brief Returns key at the specified index
*  @param index index of the key in the node
*  @return key at the specified index
*/
uint64_t Node::getKeyAt(uint32_t index) {
    if (index >= data.keysCount) return NOT_FOUND;
    return data.keys[index];
}


/*
*  @brief Sets key at specified index
*  @param index index of the key in the node
*  @param key value
*/
void Node::setKeyAt(uint32_t index, uint64_t key) {
    if (index >= data.keysCount) return;
    data.keys[index] = key;
    this->isPersisted = false;
}


/*
*  @brief Returns Parent node position in storage file
*/
uint64_t Node::getParent() {
    return data.parent;
}


/*
*  @brief Sets Parent node position in storage file
*  @param parentPosition parent node position in storage file
*/
void Node::setParent(uint64_t parentPosition) {
    // TODO what if record position changed (not real if size is not changed?)
    data.parent = parentPosition;
    this->isPersisted = false;
}


/*
* @brief Returns left sibling node position in storage file
* @return left sibling node position in storage file
*/
uint64_t Node::getLeftSibling() {
    return data.leftSibling;
}


/*
*  @brief Sets left sibling node position in storage file
*  @param siblingPosition left sibling node position in storage file
*/
void Node::setLeftSibling(uint64_t siblingPosition) {
    data.leftSibling = siblingPosition;
    this->isPersisted = false;
}


/*
* @brief Returns right sibling node position in storage file
* @return right sibling node position in storage file
*/
uint64_t Node::getRightSibling() {
    return data.rightSibling;
}


/*
*  @brief Sets right sibling node position in storage file
*  @param siblingPosition right sibling node position in storage file
*/
void Node::setRightSibling(uint64_t siblingPosition) {
    data.rightSibling = siblingPosition;
    this->isPersisted = false;
}



/*
*  @brief Handles node overflow by splitting node and interconnecting new nodes
*  @return returns current root node position in storage file or NOT_FOUMD
*/
uint64_t Node::dealOverflow() {

#ifdef _DEBUG
    std::cout << std::endl;
    std::cout << "Overflow detected in the node (" << position << "):" << *toString() << std::endl;
#endif
    
    // Get key at middle index for propagation to the parent node
    uint32_t midIndex = this->getKeyCount() / 2;
    uint64_t upKey = this->getKeyAt(midIndex);

    // Split this node by half (returns new splitted node)
    uint64_t splittedRightNodePos = this->split();
    std::shared_ptr<Node> splittedRightNode = loadNode(index, splittedRightNodePos);

    // if we are splitting the root node
    if (isRootNode()) {
        // create new root node and set as parent to this node (grow at root)
        std::unique_ptr<InnerNode> newRootNode = std::make_unique<InnerNode>(index);                
        this->setParent(newRootNode->position);
        this->persist();
    }

    // Interconnect splitted node's parent and siblings
    splittedRightNode->setParent(this->getParent());
    splittedRightNode->setLeftSibling(this->position);
    splittedRightNode->setRightSibling(this->getRightSibling());
    splittedRightNode->persist();    
    if (this->getRightSibling() != NOT_FOUND) {
        std::shared_ptr<Node> theRightSibling = loadNode(index, getRightSibling());
        theRightSibling->setLeftSibling(splittedRightNode->position);
        theRightSibling->persist();
    }
    this->setRightSibling(splittedRightNode->position);
    // save changes
    this->persist();

    // Push middle key up to parent the node (root node returned)
    std::shared_ptr<Node> parent = loadNode(index, this->getParent());
    uint64_t parentPos = parent->pushUpKey(upKey, position, splittedRightNode->position);
//  already saved changes, don't call parent->persist();
    
    // Return current root node position
    return parentPos;

}


/*
*  @brief Handles node underflow by borrowing keys from left or right sibling
*  or by merging this node with left or right sibling
*  @return returns current root node position in storage file or NOT_FOUMD
*/
uint64_t Node::dealUnderflow() {

#ifdef _DEBUG
    std::cout << std::endl;
    std::cout << "Underflow detected in the node (" << position << "):" << *toString() << std::endl;
#endif

    // if this is the root node, then do nothing and return
    if (this->getParent() == NOT_FOUND) return NOT_FOUND;
    uint64_t leftSiblingPos = getLeftSibling();
    uint64_t rightSiblingPos = getRightSibling();

    // 1. Try to borrow top key from left sibling    
    if (leftSiblingPos != NOT_FOUND) {
        std::shared_ptr<Node> leftSibling = loadNode(index, leftSiblingPos);
        if (leftSibling->canLendAKey() && leftSibling->getParent() == this->getParent()) {
            uint32_t keyIndex = leftSibling->getKeyCount() - 1;
            std::shared_ptr<Node> parent = loadNode(index, this->getParent());
            parent->borrowChildren(position, leftSiblingPos, keyIndex);
            parent->persist();
            return NOT_FOUND;
        }
    }

    // 2. Try to borrow lower key from right sibling
    if (rightSiblingPos != NOT_FOUND) {
        std::shared_ptr<Node> rightSibling = loadNode(index, rightSiblingPos);
        if (rightSibling->canLendAKey() && rightSibling->getParent() == this->getParent()) {
            uint32_t keyIndex = 0;
            std::shared_ptr<Node> parent = loadNode(index, this->getParent());
            parent->borrowChildren(position, rightSiblingPos, keyIndex);
            parent->persist();
            return NOT_FOUND;
        }
    }

    // 3. Try to merge with left sibling
    if (leftSiblingPos != NOT_FOUND) {  
        std::shared_ptr<Node> leftSibling = loadNode(index, leftSiblingPos);
        if (leftSibling->getParent() == this->getParent()) {  
            std::shared_ptr<Node> parent = loadNode(index, this->getParent());
            uint64_t rootNodePos = parent->mergeChildren(leftSiblingPos, this->position);
            parent->persist();
            return rootNodePos;
        }
    } 
    
    // 4. Try to merge with right sibling        
    std::shared_ptr<Node> parent = loadNode(index, this->getParent());
    uint64_t rootNodePos = parent->mergeChildren(this->position, rightSiblingPos);
    // FIXME: parent object could be changed after merge children
    // this local pointer is rewrites correct valus with old ones
    // SOLUTION: may be we shoud enhance loadNode method to share 
    // same links on same position
    //parent->persist();
    return rootNodePos;

}

