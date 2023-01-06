/******************************************************************************
*
*  B+ Tree Index classes header
*
*  BPTreeIndex is designed for quick search of value by key in the storage.
*  
*  Features:
*    - Insert/Search/Update/Delete key-value pairs (ID, Document data)
*    - Persist changes to storage file
*
*  (C) Boson Database, Bolat Basheyev 2022
*
******************************************************************************/
#pragma once

#include "CachedFileIO.h"
#include "RecordFileIO.h"

#include <vector>
#include <stdexcept>
#include <iostream>

namespace Boson {

	typedef uint64_t KEY;
	typedef char* VALUE;
	typedef uint64_t OFFSET;
	
	typedef enum { 
		INNER = 1, 
		LEAF = 2 
	} NodeType;

	//-------------------------------------------------------------------------
	// Node pair: key / offset
	//-------------------------------------------------------------------------
	typedef struct {
		KEY    key;                       // Key - ID of document
		OFFSET offset;                    // Children / Value position
	} NodePair;
	
	constexpr size_t NODE_PAYLOAD = (PAGE_SIZE - sizeof(RecordHeader)); // FIXME
	constexpr size_t NODE_CAPACITY = NODE_PAYLOAD / sizeof(NodePair);
	constexpr size_t TREE_ORDER = NODE_CAPACITY;
	constexpr size_t MAX_DEGREE = NODE_CAPACITY - 1;
	constexpr size_t MIN_DEGREE = NODE_CAPACITY / 2;

	//-------------------------------------------------------------------------
	// Node base class
	//-------------------------------------------------------------------------
	class Node {
	public:
		Node();
		~Node();
		size_t  getKeyCount();
		bool    isOverflow();
		bool    isUnderflow();
		bool    canLendAKey();
		KEY     getKeyAt(size_t index);
		void    setKeyAt(size_t index, KEY key);
		Node*   getParent();
		void    setParent(Node* parent);
		Node*   getLeftSibling();
		void    setLeftSibling(Node*);
		Node*   getRightSibling();
		void    setRightSibling(Node*);
		Node*   dealOverflow();
		Node*   dealUnderflow();

		virtual NodeType getNodeType() = 0;
		virtual size_t search(KEY key) = 0;
		virtual Node* split() = 0;
		virtual Node* pushUpKey(KEY key, Node* leftChild, Node* rightChild) = 0;
		virtual Node* mergeChildren(Node* leftChild, Node* rightChild) = 0;
		virtual void  mergeWithSibling(KEY key, Node* rightSibling) = 0;
		virtual KEY   borrowFromSibling(KEY key, Node* sibling, size_t borrowIndex) = 0;
		virtual void  borrowChildren(Node* borrower, Node* lender, size_t borrowIndex) = 0;
		virtual void  print(int level) = 0;

	protected:
		Node* parent;
		Node* leftSibling;
		Node* rightSibling;
		std::vector<KEY> keys;
		void printTabs(size_t n);
	};


	//---------------------------------------------------------------------------------------------
	// Inner Node Class
	//---------------------------------------------------------------------------------------------
	class InnerNode : public Node {
	public:
		InnerNode();
		~InnerNode();
		size_t   search(KEY key);
		Node*    getChildAt(size_t index);
		void     setChildAt(size_t index, Node* childNode);
		void     insertAt(size_t index, KEY key, Node* leftChild, Node* rightChild);
		void     deleteAt(size_t index);
		Node*    split();
		Node*    pushUpKey(KEY key, Node* leftChild, Node* rightChild);
		void     borrowChildren(Node* borrower, Node* lender, size_t borrowIndex);
		Node*    mergeChildren(Node* leftChild, Node* rightChild);
		void     mergeWithSibling(KEY key, Node* rightSibling);
		KEY      borrowFromSibling(KEY key, Node* sibling, size_t borrowIndex);
		NodeType getNodeType();
		void     print(int level);
	protected:
		std::vector<Node*> children;
	};



	//---------------------------------------------------------------------------------------------
	// Leaf Node Class
	//---------------------------------------------------------------------------------------------
	class LeafNode : public Node {
	public:
		LeafNode();
		~LeafNode();
		size_t search(KEY key);
		VALUE getValueAt(size_t index);
		void  setValueAt(size_t index, VALUE value);
		bool  insertKey(KEY key, VALUE value);
		void  insertAt(size_t index, KEY key, VALUE value);
		bool  deleteKey(KEY key);
		void  deleteAt(size_t index);
		Node* split();
		void  merge(KEY key, Node* siblingRight);
		Node* pushUpKey(KEY key, Node* leftChild, Node* rightChild);
		void  borrowChildren(Node* borrower, Node* lender, size_t borrowIndex);
		Node* mergeChildren(Node* leftChild, Node* rightChild);
		void  mergeWithSibling(KEY key, Node* rightSibling);
		KEY   borrowFromSibling(KEY key, Node* sibling, size_t borrowIndex);
		NodeType getNodeType();
		void  print(int level);
	protected:
		std::vector<VALUE> values;
		size_t searchPlaceFor(KEY key);
	};


	//---------------------------------------------------------------------------------------------
	// B+ Tree class - KEY/VALUE store index
	//---------------------------------------------------------------------------------------------
	class BalancedTree {
	public:
		BalancedTree();
		~BalancedTree();

		bool   insert(KEY key, VALUE value);
		VALUE  search(KEY key);

		bool   erase(KEY key);
		size_t getEntriesCount();

		size_t getTreeOrder();
		size_t getTreeHeight();
		Node*  getRoot();

		void   printTree();
		void   printContent();

	private:
		size_t entriesCount;
		Node* root;
		LeafNode* findLeafNode(KEY key);
	};

}