/*=================================================================================================
* 
*    Balanced PLus Tree Header
* 
*    Indexed key/value store
* 
*    BOSON embedded database
*    (C) Bolat Basheyev 2022
*    
=================================================================================================*/
#pragma once

#include <vector>
#include <string>

namespace Boson {

	constexpr size_t MINIMAL_TREE_ORDER = 3;
	constexpr size_t DEFAULT_TREE_ORDER = 5;
	constexpr size_t NOT_FOUND = -1;

	typedef enum { INNER, LEAF } NodeType;
	typedef __int64     KEY;                     // Key type
	typedef char*       VALUE;                   // Value payload

	//---------------------------------------------------------------------------------------------
	// Node abstract class
	//---------------------------------------------------------------------------------------------
	class Node {
	public:
		Node(size_t M);
		~Node();
		size_t getKeyCount();
		bool   isOverflow();
		bool   isUnderflow();
		bool   canLendAKey();
		KEY    getKeyAt(size_t index);
		void   setKeyAt(size_t index, KEY key);
		Node*  getParent();
		void   setParent(Node* parent);
		Node*  getLeftSibling();
		void   setLeftSibling(Node*);
		Node*  getRightSibling();
		void   setRightSibling(Node*);
		Node*  dealOverflow();
		Node*  dealUnderflow();

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
		size_t treeOrder;
		size_t maxDegree;
		size_t minDegree;
		Node* parent;
		Node* leftSibling;
		Node* rightSibling;
		std::vector<KEY> keys;
		void printTabs(size_t n);
	};


	//---------------------------------------------------------------------------------------------
	// Inner Node Class - index page
	//---------------------------------------------------------------------------------------------
	class InnerNode : public Node {
	public:
		InnerNode(size_t m);
		~InnerNode();
		size_t search(KEY key);
		Node* getChild(size_t index);
		void  setChild(size_t index, Node* childNode);
		void  insertAt(size_t index, KEY key, Node* leftChild, Node* rightChild);
		void  deleteAt(size_t index);
		Node* split();
		Node* pushUpKey(KEY key, Node* leftChild, Node* rightChild);
		void  borrowChildren(Node* borrower, Node* lender, size_t borrowIndex);
		Node* mergeChildren(Node* leftChild, Node* rightChild);
		void  mergeWithSibling(KEY key, Node* rightSibling);
		KEY   borrowFromSibling(KEY key, Node* sibling, size_t borrowIndex);
		NodeType getNodeType();
		void print(int level);
	private:
		std::vector<Node*> children;
	};


	//---------------------------------------------------------------------------------------------
	// Leaf Node Class - data page
	//---------------------------------------------------------------------------------------------
	class LeafNode : public Node {
	public:
		LeafNode(size_t m);
		~LeafNode();
		size_t search(KEY key);
		VALUE getValueAt(size_t index);
		void  setValueAt(size_t index, VALUE value);
		void  insertKey(KEY key, VALUE value);
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
	private:
		std::vector<VALUE> values;
	};



	//---------------------------------------------------------------------------------------------
	// Balanced Plus Tree class - indexed KEY/VALUE store
	//---------------------------------------------------------------------------------------------
	class BalancedTree {
	public:
		BalancedTree(size_t M = DEFAULT_TREE_ORDER);
		~BalancedTree();
		void   insert(KEY key, VALUE value);
		VALUE  search(KEY key);
		VALUE  directSearch(KEY key);
		bool   erase(KEY key);
		size_t getTreeOrder();
		Node*  getRoot();
		void   printTree();
		void   printContent();
	private:
		size_t treeOrder;
		Node* root;
		LeafNode* findLeafNode(KEY key);
	};


}