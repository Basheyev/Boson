/******************************************************************************
*
*  LeafNode class implementation
*
*  (C) Boson Database, Bolat Basheyev 2022-2024
*
******************************************************************************/

#include "BalancedIndex.h"
#include <sstream>
#include <ios>

using namespace Boson;

/*
* @brief Leaf Node Constructor (calls Node Constructor)
* @param bi BalancedIndex object
*/
LeafNode::LeafNode(BalancedIndex& bi) : Node(bi, NodeType::LEAF) {

}


/*
* @brief Leaf Node Constructor (used from Node::loadNode)
* @param bi BalancedIndex object
* @param offsetInFile position in the storage file
* @param loadedData NodeData object with loaded data
*/
LeafNode::LeafNode(BalancedIndex& bi, uint64_t offsetInFile, NodeData& loadedData) : Node(bi) {
    position = offsetInFile;
    memcpy(&(this->data), &loadedData, sizeof NodeData);
    isPersisted = true;
}


/*
* @brief Leaf Node Destructor
*/
LeafNode::~LeafNode() {
    if (!isPersisted) persist();
}


/*
* @brief Search index of key in this node (binary search in sorted array)
* @param key required key
* @return index of child node of the specified key
*/
uint32_t LeafNode::search(uint64_t key) {
    int32_t start = 0;                            // we need signed integers for
    int32_t end = data.keysCount - 1;             // correct comparison in while loop
    uint32_t mid;                                 // middle index can not be negative
    uint64_t entry;                               // variable to hold value

    while (start <= end) {                        // while start index <= end index
        mid = start + (end - start) / 2;          // calculate middle index between start & end
        entry = data.keys[mid];                   // get value in keys array at middle index
        if (entry == key) return mid;             // if value equals to key return index
        if (key < entry) end = mid - 1; else      // if key < value bound end index to middle-1
            if (key > entry) start = mid + 1;     // if key > value bound start index to middle+1 
    }

    return KEY_NOT_FOUND;                             // Key is definitely not found
}


/*
*  @brief Return value at specified index in this node
*  @param index of value
*  @return returns value string
*/
std::shared_ptr<std::string> LeafNode::getValueAt(uint32_t index) {

    // Go to required position in storage file
    RecordFileIO& recordsFile = this->index.getRecordsFile();
    uint64_t offsetInFile = data.values[index];
    recordsFile.setPosition(offsetInFile);    

    // load data from storage file record
    uint32_t valueLength = recordsFile.getDataLength() + 1;
    
    char* cStr = new char[valueLength];

    uint64_t offset = recordsFile.getRecordData(cStr, valueLength);
    if (offset == NOT_FOUND) throw std::ios_base::failure("Can't read value.");

    // add null terminator to C style string
    cStr[valueLength - 1] = 0; 

    // convert to C++ string and return as shared pointer
    std::shared_ptr<std::string> cppStr = std::make_shared<std::string>();
    
    *cppStr = cStr;

    delete[] cStr;

    return cppStr;
}



/*
*  @brief Set value at specified index in this node
*  @param index of value
*  @param value string
*/
void LeafNode::setValueAt(uint32_t index, const std::string& value) {
    
    // Go to required position in storage file
    RecordFileIO& recordsFile = this->index.getRecordsFile();
    uint64_t offsetInFile = data.values[index];
    recordsFile.setPosition(offsetInFile);

    // Write value to the storage file
    uint32_t valueLength = (uint32_t) value.length() + 1;
    const char* cStr = value.c_str();
    uint64_t offset = recordsFile.setRecordData(cStr, valueLength);
    if (offset == NOT_FOUND) throw std::ios_base::failure("Can't write value.");

    // update offset if its changed
    data.values[index] = offset;
    isPersisted = false;
}



/*
*  @brief Search index for new key in sorted order (KEY_NOT_FOUND returned if key duplicate)
*  @param key
*  @return index for new key
*/
uint32_t LeafNode::searchPlaceFor(uint64_t key) {

    int32_t insertIndex = data.keysCount;
    int32_t start = 0;                            // we need signed integers for
    int32_t end = data.keysCount - 1;             // correct comparison in while loop
    int32_t mid;                                  // middle index can not be negative
    uint64_t entry;                               // variable to hold value

    while (start <= end) {                        // while start index <= end index
        mid = start + (end - start) / 2;          // calculate middle index between start & end
        entry = data.keys[mid];                   // get value in keys array at middle index
        if (entry == key) return KEY_NOT_FOUND;   // if value equals to key - key duplicate!
        if (key < entry) {                        // if key < value 
            end = mid - 1;                        // bound end index to middle-1
            insertIndex = mid;                    // save index where next entry is greater
        }
        else if (key > entry) start = mid + 1;    // if key > value bound start index to middle+1 
    }

    return insertIndex;
}



/*
*  @brief Insert key/value pair to this node in sorted order
*  @param key
*  @param value
*/
bool LeafNode::insertKey(uint64_t key, const std::string& value) {
    // find index to insert new key/value pair in sorted order
    uint32_t insertIndex = searchPlaceFor(key);
    if (insertIndex == KEY_NOT_FOUND) return false;
    // insert key/value
    insertAt(insertIndex, key, value);
    return true;
}


/*
*  @brief Insert key/value pair to this node in sorted order
*  @param key
*  @param value
*/
bool LeafNode::insertKey(uint64_t key, uint64_t valuePosition) {
    // find index to insert new key/value pair in sorted order
    uint32_t insertIndex = searchPlaceFor(key);
    if (insertIndex == KEY_NOT_FOUND) return false;
    // insert key/value
    insertAt(insertIndex, key, valuePosition);
    return true;
}


/*
*  @brief Insert key/value pair at specified index in this node
*  @param index
*  @param key
*  @param value
*/
void LeafNode::insertAt(uint32_t index, uint64_t key, const std::string& value) {
    // insert key
    data.insertAt(NodeArray::KEYS, index, key);

    // Create record in storage file
    RecordFileIO& recordsFile = this->index.getRecordsFile();
    uint32_t valueLength = (uint32_t) value.length() + 1;
    const char* cStr = value.c_str();    
    uint64_t offsetInFile = recordsFile.createRecord(cStr, valueLength);
    if (offsetInFile == NOT_FOUND) throw std::ios_base::failure("Can't write value.");
    
    // insert value pointer
    data.insertAt(NodeArray::VALUES, index, offsetInFile);
    
    //isPersisted = false;
    persist();
}


/*
*  @brief Insert key/value pair at specified index in this node
*  @param index
*  @param key
*  @param value
*/
void LeafNode::insertAt(uint32_t index, uint64_t key, uint64_t valuePosition) {
    // insert key
    data.insertAt(NodeArray::KEYS, index, key);
    // insert value pointer
    data.insertAt(NodeArray::VALUES, index, valuePosition);
    
    //isPersisted = false;
    persist();
}




/*
*  @brief Delete key/value pair by key in this node
*  @param key value
*  @return true if ok, or false if index not found
*/
bool LeafNode::deleteKey(uint64_t key) {
    uint32_t deleteIndex = search(key);
    if (deleteIndex == KEY_NOT_FOUND) return false;
    deleteAt(deleteIndex);
    return true;
}



/*
* @brief Delete key/value pair at specified index in this node
* @param index 
*/
void LeafNode::deleteAt(uint32_t index) {
    // get value position in storage file
    uint64_t offsetInFile = data.values[index];
    // Find record in storage file
    RecordFileIO& recordsFile = this->index.getRecordsFile();
    if (!recordsFile.setPosition(offsetInFile))
        throw std::ios_base::failure("Can't delete value.");
    // Delete value record in storage file
    recordsFile.removeRecord();
    // Delete key/value pair
    data.deleteAt(NodeArray::KEYS, index);
    data.deleteAt(NodeArray::VALUES, index);     
    //isPersisted = false;
    persist();
}



/*
* @brief Split this node by half and return new splitted node
* @return new node position in storage file
*/
uint64_t LeafNode::split() {
    uint32_t midIndex = data.keysCount / 2;
    std::unique_ptr<LeafNode> newNode = std::make_unique<LeafNode>(this->index);
    for (size_t i = midIndex; i < data.keysCount; ++i) {
        newNode->insertKey(data.keys[i], data.values[i]);        
    }
    data.resize(NodeArray::KEYS, midIndex);
    data.resize(NodeArray::VALUES, midIndex);
    
    isPersisted = false;
    
    newNode->persist();
    this->persist();

    return newNode->position;
}


/*
*  @brief Merge this leaf node with right sibling (why its equivalent to to merge)
*  @param key
*  @param sibling
*/
void LeafNode::mergeWithSibling(uint64_t key, uint64_t siblingPos) {
    std::shared_ptr<Node> siblingLeaf = Node::loadNode(index, siblingPos);
    // copy keys and values from sibling node to this node
    for (size_t i = 0; i < siblingLeaf->getKeyCount(); i++) {
        data.pushBack(NodeArray::KEYS, siblingLeaf->data.keys[i]);
        data.pushBack(NodeArray::VALUES, siblingLeaf->data.values[i]);
    }
    // interconnect siblings
    uint64_t rightSiblingPos = siblingLeaf->getRightSibling();
    setRightSibling(rightSiblingPos);
    if (rightSiblingPos != NOT_FOUND) {
        std::shared_ptr<Node> rightSibling = Node::loadNode(index, rightSiblingPos);
        rightSibling->setLeftSibling(this->position);
    }
    // Delete sibling node
    Node::deleteNode(index, siblingPos);
    persist();
    //isPersisted = false;
}


/*
*  @brief Borrow child node at specified index from sibling and return new middle key
*  @param key
*  @param siblingPos
*  @param borrowIndex
*/
uint64_t LeafNode::borrowFromSibling(uint64_t key, uint64_t siblingPos, uint32_t borrowIndex) {

    std::shared_ptr<LeafNode> siblingNode =
        std::dynamic_pointer_cast<LeafNode>(Node::loadNode(index, siblingPos));

    // insert borrowed key/value pair
    uint64_t borrowedKey = siblingNode->data.keys[borrowIndex];
    uint64_t borrowedValuePos = siblingNode->data.values[borrowIndex];
    this->insertKey(borrowedKey, borrowedValuePos);

    // delete borrowed key/value pair in sibling node
    siblingNode->deleteAt(borrowIndex);
    
    //isPersisted = false;
    persist();

    // return new middle key
    if (borrowIndex == 0)
        return siblingNode->getKeyAt(0);
    else
        return this->getKeyAt(0);
}



/*
*  @brief returns node type
*  @return node type
*/
NodeType LeafNode::getNodeType() {
    return NodeType::LEAF;
}


/*
*  @brief returns string of childrens of this node
*  @return string of childrens of this node
*/
std::shared_ptr<std::string> LeafNode::toString() {
    std::stringstream ss;
    ss << "Leaf: Values=[";
    for (uint32_t i = 0; i < data.valuesCount; i++) {
        bool isNotLast = (i < data.valuesCount - 1);        
        std::shared_ptr<std::string> value = this->getValueAt(i);
        ss << data.keys[i] << ":'" << *value << (isNotLast ? "', " : "'");
    }
    ss << "]";
    return std::make_shared<std::string>(ss.str());
}



uint64_t LeafNode::pushUpKey(uint64_t key, uint64_t leftChild, uint64_t rightChild) {
    throw std::runtime_error("Unsupported operation: leaf node can't push keys up.");
}


void LeafNode::borrowChildren(uint64_t borrower, uint64_t lender, uint32_t borrowIndex) {
    throw std::runtime_error("Unsupported operation: leaf node can't process children borrowing.");
}


uint64_t LeafNode::mergeChildren(uint64_t leftChild, uint64_t rightChild) {
    throw std::runtime_error("Unsupported operation: leaf node can't merge children.");
}

