/******************************************************************************
*
*  B+ Tree Index header
*
*  BPTreeIndex is designed for quick search of value by key in the storage.
*  
*  Features:
*    - Insert/Search/Update/Delete key-value pairs (ID, Document data)
*    - Persist tree changes to storage file
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

	//--------------------------------------------------------------------------
	// Index Header
	//--------------------------------------------------------------------------
	typedef struct {
		uint64_t rootOffset;              // index root node offset
		uint64_t entriesCount;            // total entries count
		uint32_t nodeCapacity;            // node capacity
	} IndexHeader;

	//--------------------------------------------------------------------------
	// Node Entry
	//--------------------------------------------------------------------------
	typedef struct {
		uint64_t key;                      // Key - ID of document
		uint64_t offset;                   // Value/Children offset		
	} NodeEntry;
	
	typedef enum {
		INNER = 1,                         // Inner node (Index)
		LEAF = 2                           // Leaf node (Data)
	} NodeType;

	//--------------------------------------------------------------------------
	// Node capacity constants
	//--------------------------------------------------------------------------
	constexpr uint32_t NODE_PAYLOAD = 128; // node payload in bytes
	constexpr uint32_t NODE_CAPACITY = NODE_PAYLOAD / sizeof NodeType;
	constexpr uint32_t TREE_ORDER = NODE_CAPACITY;
	constexpr uint32_t MAX_DEGREE = NODE_CAPACITY - 1;
	constexpr uint32_t MIN_DEGREE = NODE_CAPACITY / 2;

	//--------------------------------------------------------------------------
	// Node Entry
	//--------------------------------------------------------------------------
	typedef struct {
		uint32_t  nodeType;                // Node type
		uint32_t  keysCount;               // Node keys count
		uint64_t  parent;                  // Parent node offset
		uint64_t  leftSibling;             // Left sibling offset
		uint64_t  rightSibling;            // Right sibling offset		
		NodeEntry entries[NODE_CAPACITY];  // Entries
	} NodeData;


	//-------------------------------------------------------------------------
	// Node pair: key / offset
	//-------------------------------------------------------------------------


	using KEY = uint64_t;
	using VALUE = std::string;
	using OFFSET = uint64_t;


	//-------------------------------------------------------------------------
	// Node base class
	//-------------------------------------------------------------------------
	class Node {
	friend class BalancedTree;
	public:
		Node();
		~Node();
		size_t  getKeyCount();
		bool    isOverflow();
		bool    isUnderflow();
		bool    canLendAKey();
		KEY     getKeyAt(size_t index);
		void    setKeyAt(size_t index, KEY key);
		OFFSET  getParent();
		void    setParent(OFFSET parent);
		OFFSET  getLeftSibling();
		void    setLeftSibling(OFFSET);
		OFFSET  getRightSibling();
		void    setRightSibling(OFFSET);
		OFFSET  dealOverflow();
		OFFSET  dealUnderflow();

		virtual NodeType getNodeType() = 0;
		virtual size_t search(KEY key) = 0;
		virtual OFFSET split() = 0;
		virtual OFFSET pushUpKey(KEY key, OFFSET leftChild, OFFSET rightChild) = 0;
		virtual OFFSET mergeChildren(OFFSET leftChild, OFFSET rightChild) = 0;
		virtual void  mergeWithSibling(KEY key, OFFSET rightSibling) = 0;
		virtual KEY   borrowFromSibling(KEY key, OFFSET sibling, size_t borrowIndex) = 0;
		virtual void  borrowChildren(OFFSET borrower, OFFSET lender, size_t borrowIndex) = 0;
		virtual void  print(int level) = 0;

	protected:

		OFFSET fileOffset;
		NodeData nodeData;

		void printTabs(size_t n);
	};


	//---------------------------------------------------------------------------------------------
	// Leaf Node Class
	//---------------------------------------------------------------------------------------------
	class LeafNode : public Node {
	public:
		LeafNode();
		~LeafNode();
		size_t search(KEY key);
		VALUE  getValueAt(size_t index);
		void   setValueAt(size_t index, VALUE value);
		bool   insertKey(KEY key, VALUE value);
		void   insertAt(size_t index, KEY key, VALUE value);
		bool   deleteKey(KEY key);
		void   deleteAt(size_t index);
		OFFSET split();
		void   merge(KEY key, OFFSET siblingRight);
		OFFSET pushUpKey(KEY key, OFFSET leftChild, OFFSET rightChild);
		void   borrowChildren(OFFSET borrower, OFFSET lender, size_t borrowIndex);
		OFFSET mergeChildren(OFFSET leftChild, OFFSET rightChild);
		void   mergeWithSibling(KEY key, OFFSET rightSibling);
		KEY    borrowFromSibling(KEY key, OFFSET sibling, size_t borrowIndex);
		NodeType getNodeType();
		void   print(int level);
	protected:
		//std::vector<VALUE> values;
		size_t searchPlaceFor(KEY key);
	};


	//---------------------------------------------------------------------------------------------
	// Inner Node Class
	//---------------------------------------------------------------------------------------------
	class InnerNode : public Node {
	public:
		InnerNode();
		~InnerNode();
		size_t search(KEY key);
		Node* getChildAt(size_t index);
		void  setChildAt(size_t index, OFFSET childNode);
		void  insertAt(size_t index, KEY key, OFFSET leftChild, OFFSET rightChild);
		void  deleteAt(size_t index);
		OFFSET split();
		OFFSET pushUpKey(KEY key, OFFSET leftChild, OFFSET rightChild);
		void  borrowChildren(OFFSET borrower, OFFSET lender, size_t borrowIndex);
		OFFSET mergeChildren(OFFSET leftChild, OFFSET rightChild);
		void  mergeWithSibling(KEY key, OFFSET rightSibling);
		KEY  borrowFromSibling(KEY key, OFFSET sibling, size_t borrowIndex);
		NodeType getNodeType();
		void  print(int level);
	protected:
		//std::vector<Node*> children;
	};


	//---------------------------------------------------------------------------------------------
	// B+ Tree class - KEY/VALUE store index
	//---------------------------------------------------------------------------------------------
	class BalancedTree {
	public:
		BalancedTree(RecordFileIO& file);
		~BalancedTree();

		bool   insert(KEY key, VALUE value);
		VALUE  search(KEY key);
		bool   erase(KEY key);
		size_t getEntriesCount();

		size_t getTreeOrder();
		size_t getTreeHeight();
		OFFSET getRoot();

		void   printTree();
		void   printContent();
	
	protected:

		OFFSET createLeafNode();
		OFFSET createInnerNode();
		bool   removeLeafNode(OFFSET pos);
		bool   removeInnerNode(OFFSET pos);

	private:
		RecordFileIO& storageFile;
		IndexHeader indexHeader;
		OFFSET findLeafNode(KEY key);
	};

}