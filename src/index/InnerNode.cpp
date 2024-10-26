/******************************************************************************
*
*  InnerNode class implementation
*
*  (C) Boson Database, Bolat Basheyev 2022-2024
*
******************************************************************************/


#include <sstream>

#include "BalancedIndex.h"


using namespace Boson;



/*
* @brief Inner Node Constructor (calls Node Constructor)
* @param bi BalancedIndex object
*/
InnerNode::InnerNode(BalancedIndex& bi) : Node(bi, NodeType::INNER) {
    
}


/*
* @brief Inner Node Constructor (used from Node::loadNode)
* @param bi BalancedIndex object
* @param offsetInFile position in the storage file
* @param loadedData NodeData object with loaded data
*/
InnerNode::InnerNode(BalancedIndex& bi, uint64_t offsetInFile, NodeData& loadedData) : Node(bi) {
    position = offsetInFile;
    memcpy(&(this->data), &loadedData, sizeof NodeData);
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
    //isPersisted = false;
    persist();
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

    persist();

}


/*
*  @brief Delete key and right children at specified index
*  @param index of key to delete
*/
void InnerNode::deleteAt(uint32_t index) {
    uint32_t childIndex = index + 1;
    uint64_t childrenPos = data.children[childIndex];
    data.deleteAt(NodeArray::KEYS, index);
    data.deleteAt(NodeArray::CHILDREN, childIndex);
    Node::deleteNode(this->index, childrenPos);
    persist();
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
        std::shared_ptr<Node> child = Node::loadNode(this->index, childPos);
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
    if (isRootNode()) return this->position;

    // return NOT_FOUND otherwise
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

    std::shared_ptr<Node> untypedBorrower = Node::loadNode(this->index, borrowerPos);
    
    // find borrower child index
    for (uint32_t i = 0; i < data.childrenCount; i++) {
        if (data.children[i] == borrowerPos) {
            borrowerChildIndex = i;
            break;
        }
    }

#ifdef _DEBUG
    std::cout << "Borrow key from " << (borrowIndex == 0 ? "right " : "left ");
    std::cout << ((untypedBorrower->data.nodeType == NodeType::INNER) ? "inner" : "leaf");
    std::cout << " node(" << lender << ") to node(" << borrowerPos << ")" << std::endl;
#endif

    if (untypedBorrower->data.nodeType == NodeType::INNER) {
        auto borrower = std::dynamic_pointer_cast<InnerNode>(untypedBorrower);
        // Process inner node borrowing
        if (borrowIndex == 0) {
            // borrow from right sibling
            uint64_t theKey = data.keys[borrowerChildIndex];
            uint64_t upKey = borrower->borrowFromSibling(theKey, lender, borrowIndex);
            data.keys[borrowerChildIndex] = upKey;
        }
        else {
            // borrow from left sibling
            uint64_t theKey = data.keys[borrowerChildIndex - 1];
            uint64_t upKey = borrower->borrowFromSibling(theKey, lender, borrowIndex);
            data.keys[borrowerChildIndex - 1] = upKey;
        }
    }
    else {
        auto borrower = std::dynamic_pointer_cast<LeafNode>(untypedBorrower);
        // Process leaf node borrowing
        if (borrowIndex == 0) {
            // borrow from right sibling
            uint64_t theKey = data.keys[borrowerChildIndex];
            uint64_t upKey = borrower->borrowFromSibling(theKey, lender, borrowIndex);
            data.keys[borrowerChildIndex] = upKey;
        }
        else {
            // borrow from left sibling
            uint64_t theKey = data.keys[borrowerChildIndex - 1];
            uint64_t upKey = borrower->borrowFromSibling(theKey, lender, borrowIndex);
            data.keys[borrowerChildIndex - 1] = upKey;
        }

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
    
    std::shared_ptr<Node> untypedSibling = Node::loadNode(this->index, sibling);
    std::shared_ptr<InnerNode> siblingNode = std::dynamic_pointer_cast<InnerNode>(untypedSibling);
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


/*
*  @brief Merge children of InnerNode by specifying left and right child
*  @param leftChild to merge
*  @param rightChild to merge
*/
uint64_t InnerNode::mergeChildren(uint64_t leftChildPos, uint64_t rightChildPos) {
    
    // Find corresponding key index of left child    
    uint32_t i = 0; 
    while (i < data.childrenCount - 1) {
        if (data.children[i] == leftChildPos) break;
        i++;
    }
    uint64_t key = data.keys[i];

#ifdef _DEBUG
    std::cout << "Merging children of inner node " << position 
        << " - [" << leftChildPos << ", " << rightChildPos << "]" << std::endl;
#endif

    // Merge two children and push key into the left child node
    std::shared_ptr<Node> leftChildNode = Node::loadNode(this->index, leftChildPos);
    leftChildNode->mergeWithSibling(key, rightChildPos);

    // Remove the key, keep the left child and abandon the right child
    this->deleteAt(i);
    
    // FIXME: underflow / overflow?

    // If there is underflow propagate borrow or merge to parent
    if (this->isUnderflow()) {
        // If this node is root node (no parent)
        if (isRootNode()) {
            // prevent overwrite of this actual root shared_ptr by other instances
            index.updateRoot(this->position);
            index.persistIndexHeader();
            // if this node is empty
            if (data.keysCount == 0) {
                leftChildNode->setParent(NOT_FOUND);
                leftChildNode->persist();
                return leftChildPos;
            } else return NOT_FOUND;
        } return dealUnderflow();
    }

    // If there is overflow propagate borrow or merge to parent
    if (this->isOverflow()) {
        // If this node is root node (no parent)
        if (isRootNode()) {
            
            // prevent overwrite of this actual root shared_ptr by other instances
            index.updateRoot(this->position);

            // if this node is empty
            if (data.keysCount == 0) {
                leftChildNode->setParent(NOT_FOUND);
                leftChildNode->persist();
                return leftChildPos;
            }
            else return NOT_FOUND;
        } return dealOverflow();
    }

    return NOT_FOUND;
}



/*
*  @brief Merge with sibling InnerNode by specifying key and right sibling
*  @param key
*  @param rightSiblingPos
*/
void InnerNode::mergeWithSibling(uint64_t key, uint64_t rightSiblingPos) {

    std::shared_ptr<Node> rightSiblingNode = Node::loadNode(this->index, rightSiblingPos);
    std::shared_ptr<InnerNode> rightSibling = std::dynamic_pointer_cast<InnerNode>(rightSiblingNode);
    std::shared_ptr<Node> siblingChild, afterRight;
    uint64_t siblingChildPos;

#ifdef _DEBUG
    std::cout << "Left sibling (" << position << "): " << *toString() << std::endl;
    std::cout << "Right sibling (" << rightSibling->position << "): " << *rightSibling->toString() << std::endl;
#endif

    // Push key into keys
    this->data.pushBack(NodeArray::KEYS, key);

    // Copy sibling keys
    for (uint32_t i = 0; i < rightSibling->data.keysCount; ++i) {
        this->data.pushBack(NodeArray::KEYS, rightSibling->getKeyAt(i));        
    }

    // Copy sibling children
    for (uint32_t i = 0; i < rightSibling->getKeyCount() + 1; ++i) {
        // get sibling's child
        siblingChildPos = rightSibling->getChildAt(i);
        siblingChild = Node::loadNode(this->index, siblingChildPos);
        // reattach sibling's child to this node
        siblingChild->setParent(this->position);
        siblingChild->persist();
        siblingChild.reset();
        // copy sibling child to this node
        this->data.pushBack(NodeArray::CHILDREN, siblingChildPos);        
    }

    // Interrconnect siblings
    this->setRightSibling(rightSibling->data.rightSibling);
    if (rightSibling->data.rightSibling != NOT_FOUND) {
        afterRight = Node::loadNode(this->index, rightSibling->data.rightSibling);
        afterRight->setLeftSibling(this->position);
        afterRight->persist();
    }
    
    // Persist this node
    this->persist();
#ifdef _DEBUG
    std::cout << "Merged inner node: " << *toString() << std::endl;
    std::cout << "Right sibling deleted at " << rightSiblingPos << std::endl;
#endif

    // Clear and delete right sibling from storage and memory
    Node::deleteNode(this->index, rightSiblingPos); 
}


/*
*  @brief returns node type
*  @return node type
*/
NodeType InnerNode::getNodeType() {
    return NodeType::INNER;
}


/*
*  @brief returns string of childrens of this node
*  @return string of childrens of this node
*/
std::shared_ptr<std::string> InnerNode::toString() {
    std::stringstream ss;
    ss << "Inner: Keys=[";
    for (uint32_t i = 0; i < data.keysCount; i++) {
        ss << data.keys[i] << ((i < data.keysCount - 1) ? ", " : "");
    }
    ss << "]";
    ss << " Children=[";
    for (uint32_t i = 0; i < data.childrenCount; i++) {
        ss << data.children[i] << ((i < data.childrenCount - 1) ? ", " : "");
    }    
    ss << "]: parent(";
    if (data.parent == NOT_FOUND) {
        ss << "no)";
    }
    else ss << data.parent << ")";
    return std::make_shared<std::string>(ss.str());
}

