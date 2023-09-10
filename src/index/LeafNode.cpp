#include "BalancedIndex.h"


using namespace Boson;


LeafNode::LeafNode(BalancedIndex& bi) : Node(bi, NodeType::LEAF) {

}


LeafNode::~LeafNode() {

}


uint32_t LeafNode::search(uint64_t key) {
    return NOT_FOUND;
}


uint64_t LeafNode::getValueAt(uint32_t index) {
    return NOT_FOUND;
}


void LeafNode::setValueAt(uint32_t index, const std::string& value) {

}


bool LeafNode::insertKey(uint64_t key, const std::string& value) {
    return false;
}


void LeafNode::insertAt(uint32_t index, uint64_t key, const std::string& value) {

}


bool LeafNode::deleteKey(uint64_t key) {
    return false;
}


void LeafNode::deleteAt(uint32_t index) {

}


uint64_t LeafNode::split() {
    return NOT_FOUND;
}


void LeafNode::merge(uint64_t key, uint64_t siblingRight) {

}


uint64_t LeafNode::pushUpKey(uint64_t key, uint64_t leftChild, uint64_t rightChild) {
    return NOT_FOUND;
}


void LeafNode::borrowChildren(uint64_t borrower, uint64_t lender, uint32_t borrowIndex) {

}


uint64_t LeafNode::mergeChildren(uint64_t leftChild, uint64_t rightChild) {
    return NOT_FOUND;
}


void LeafNode::mergeWithSibling(uint64_t key, uint64_t rightSibling) {

}


uint64_t LeafNode::borrowFromSibling(uint64_t key, uint64_t sibling, uint32_t borrowIndex) {
    return NOT_FOUND;
}


NodeType LeafNode::getNodeType() {
    return NodeType::LEAF;
}

