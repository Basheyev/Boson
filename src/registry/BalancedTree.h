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
		Node(int m);
		~Node();
		int    getKeyCount();
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
		virtual int search(KEY key) = 0;
	protected:
		int maxDegree;
		int minDegree;
		int degree;
		Node* parent;
		Node* leftSibling;
		Node* rightSibling;
		std::vector<KEY> keys;
	};



	class InnerNode : public Node {
	public:
		InnerNode(int m);
		~InnerNode();
		Node* getChild(int index);
		void  setChild(int index, Node* childNode);
		int   search(KEY key);
		void  insertAt(int index, Node*, Node*);
		void  deleteAt(int index);
		NodeType getNodeType();
	private:
		std::vector<Node*> children;
	};



	class LeafNode : public Node {
	public:
		LeafNode(int m);
		~LeafNode();
		VALUE getValue(int index);
		void  setValue(int index, VALUE value);
		int   search(KEY key);
		void  insertKey(KEY key, VALUE value);
		void  insertKeyAt(int index, KEY key, VALUE value);
		bool  deleteKey(KEY key);
		bool  deleteAt(int index);
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
		InnerNode* root;
	};














}