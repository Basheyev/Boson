


#include "BalancedIndex.h"

using namespace Boson;


Node::Node(BalancedIndex& bi, uint64_t position) : index(bi) {

}


Node::~Node() {

}


bool Node::persist() {
    return false;
}


uint32_t Node::getKeyCount() {
    return 0;
}


bool Node::isOverflow() {
    return false;
}


bool Node::isUnderflow() {
    return false;
}


bool Node::canLendAKey() {
    return false;
}


uint64_t Node::getKeyAt(uint32_t index) {
    return NOT_FOUND;
}


void Node::setKeyAt(uint32_t index, uint64_t key) {

}


uint64_t Node::getParent() {
    return NOT_FOUND;
}


void Node::setParent(uint64_t parentPosition) {

}


uint64_t Node::getLeftSibling() {
    return NOT_FOUND;
}


void Node::setLeftSibling(uint64_t silbingPosition) {

}


uint64_t Node::getRightSibling() {
    return NOT_FOUND;
}


void Node::setRightSibling(uint64_t siblingPosition) {

}


uint64_t Node::dealOverflow() {
    return NOT_FOUND;
}


uint64_t Node::dealUnderflow() {
    return NOT_FOUND;
}

