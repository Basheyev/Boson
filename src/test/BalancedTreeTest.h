#pragma once

#define _ITERATOR_DEBUG_LEVEL 0

#include "BalancedTreeIndex.h"

#include <chrono>
#include <iostream>
#include <string>



namespace Boson {

	class BalancedTreeTest {
	public:
		bool run(bool verbose = true);
	private:
		
		bool assert(char* msg, bool expr);
		
		bool testLeafNode(bool verbose = true);
		bool testInnerNode(bool verbose = true);
		bool testBalancedTree(bool verbose = true);
		bool testPerformance(bool verbose = true);
		bool testTextIndex(bool verbose = true);

		Research::BalancedTreeIndex<size_t, char*>* buildTree(CachedFileIO& cachedFile, bool verbose);
		bool deleteTree(Research::BalancedTreeIndex<size_t, char*>* bt, bool verbose);
	};

};

