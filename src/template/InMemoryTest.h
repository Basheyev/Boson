#pragma once

#define _ITERATOR_DEBUG_LEVEL 0

#include "InMemoryBPTree.h"

#include <chrono>
#include <iostream>
#include <string>



namespace Research {


	using BPTree = Research::BalancedTreeIndex<uint64_t, char*>;
	using BPInner = Research::InnerNode<uint64_t>;
	using BPLeaf = Research::LeafNode<uint64_t, char*>;

	class InMemoryTest {
	public:
		bool run(bool verbose = true);
	private:
		
		bool assert(char* msg, bool expr);
		
		bool testLeafNode(bool verbose = true);
		bool testInnerNode(bool verbose = true);
		bool testBalancedTree(bool verbose = true);
		bool testPerformance(bool verbose = true);
		bool testTextIndex(bool verbose = true);

		BPTree* buildTree(bool verbose);
		bool deleteTree(BPTree* bt, bool verbose);
	};

};

