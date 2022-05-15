#pragma once

#include <vector>
#include <string>

namespace Boson {

	constexpr int MINIMAL_TREE_ORDER = 3;
	constexpr int DEFAULT_TREE_ORDER = 5;
	constexpr int NOT_FOUND = -1;

	typedef enum { INNER, LEAF } NodeType;


	typedef __int64     KEY;
	typedef char*       VALUE;


	class Node {
	public:
		Node(size_t m);
		~Node();
		size_t getKeyCount();
		bool   isOverflow();
		bool   isUnderflow();
		bool   canLendAKey();
		KEY    getKeyAt(int index);
		void   setKeyAt(int index, KEY key);
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
		virtual void merge(KEY sinkkey, Node* siblingRight) = 0;
		virtual Node* pushUpKey(KEY key, Node* leftChild, Node* rightChild) = 0;
		virtual Node* transferChildren(Node* borrower, Node* lender, int borrowIndex) = 0;
		virtual Node* mergeChildren(Node* leftChild, Node* rightChild) = 0;

	protected:
		size_t maxDegree;
		size_t minDegree;
		size_t degree;
		Node* parent;
		Node* leftSibling;
		Node* rightSibling;
		std::vector<KEY> keys;
	};



	class InnerNode : public Node {
	public:
		InnerNode(size_t m);
		~InnerNode();
		size_t search(KEY key);
		Node* getChild(int index);
		void  setChild(int index, Node* childNode);
		void  insertAt(int index, KEY key, Node* leftChild, Node* rightChild);
		void  deleteAt(int index);		
		Node* split();
		void  merge(KEY sinkkey, Node* siblingRight);
		Node* pushUpKey(KEY key, Node* leftChild, Node* rightChild);
		Node* transferChildren(Node* borrower, Node* lender, int borrowIndex);
		Node* mergeChildren(Node* leftChild, Node* rightChild);
		KEY   borrowFromSibling(KEY sinkKey, Node* sibling, int borrowIndex);
		NodeType getNodeType();
	private:
		std::vector<Node*> children;
	};



	class LeafNode : public Node {
	public:
		LeafNode(size_t m);
		~LeafNode();
		size_t search(KEY key);
		VALUE getValueAt(int index);
		void  setValueAt(int index, VALUE value);
		void  insertKey(KEY key, VALUE value);
		void  insertAt(size_t index, KEY key, VALUE value);
		bool  deleteKey(KEY key);
		bool  deleteAt(int index);		
		Node* split();
		void  merge(KEY sinkkey, Node* siblingRight);
		Node* pushUpKey(KEY key, Node* leftChild, Node* rightChild);
		Node* transferChildren(Node* borrower, Node* lender, int borrowIndex);
		Node* mergeChildren(Node* leftChild, Node* rightChild);
		KEY   borrowFromSibling(KEY sinkKey, Node* sibling, int borrowIndex);
		NodeType getNodeType();
		void  print();
	private:
		std::vector<VALUE> values;
	};



	class BalancedTree {
	public:
		BalancedTree(int order=DEFAULT_TREE_ORDER);
		~BalancedTree();
		void   insert(KEY key, VALUE value);
		VALUE  search(KEY key);
		bool   erase(KEY key);
		int    getTreeOrder();
		Node*  getRoot();
	private:
		int treeOrder;
		Node* root;
		LeafNode* findLeaf(KEY key);
	};














}