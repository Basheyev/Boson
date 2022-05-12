#include "BalancedTree.h"

#include <iostream>

using namespace Boson;
using namespace std;


LeafNode::LeafNode(int m) : Node(m) {
	values.reserve(m);
}


LeafNode::~LeafNode() {
	values.clear();
}



VALUE LeafNode::getValue(int index) {
	return values[index];
}


void LeafNode::setValue(int index, VALUE value) {
	values[index] = value;
}


int LeafNode::search(KEY key) {
	for (int i = 0; i < keys.size(); i++) {
		if (keys[i] == key) return i;
	}
	return NOT_FOUND;
}


void LeafNode::insertKey(KEY key, VALUE value) {
	// find index to insert new key/value pair in sorted order
	int insertIndex = keys.size();
	for (int i = 0; i < keys.size(); i++) {
		if (key < keys[i]) {
			insertIndex = i;
			break;
		}
	}
	// insert key/value
	insertKeyAt(insertIndex, key, value);

}


void LeafNode::insertKeyAt(int index, KEY key, VALUE value) {
	keys.insert(keys.begin() + index, 1, key);
	values.insert(values.begin() + index, 1, value);
}


bool LeafNode::deleteKey(KEY key) {
	int deleteIndex = search(key);
	if (deleteIndex == NOT_FOUND) return false;
	return deleteAt(deleteIndex);;
}


bool LeafNode::deleteAt(int index) {
	keys.erase(keys.begin() + index);
	values.erase(values.begin() + index);
	return true;
}


NodeType LeafNode::getNodeType() {
	return NodeType::LEAF;
}


void LeafNode::print() {
	cout << "-------  " << keys.size() << " records -------" << endl;
	for (int i = 0; i < keys.size(); i++) {
		cout << keys[i] << " - " << values[i] << endl;
	}
	cout << "-------------------------------\n";
}