#include "BalancedIndex.h"


using namespace Boson;


InnerNode::InnerNode(BalancedIndex& bi) : Node(bi, NodeType::INNER) {

}


InnerNode::InnerNode(BalancedIndex& bi, uint64_t offsetInFile) : Node(bi, offsetInFile) {

}


InnerNode::~InnerNode() {

}


uint32_t InnerNode::search(uint64_t key) {
    return NOT_FOUND;
}


uint64_t InnerNode::getChildAt(uint32_t index) {
    return NOT_FOUND;
}


void InnerNode::setChildAt(uint32_t index, uint64_t childNode) {

}


void InnerNode::insertAt(uint32_t index, uint64_t key, uint64_t leftChild, uint64_t rightChild) {

}


void InnerNode::deleteAt(uint32_t index) {

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

