#pragma once

#define _ITERATOR_DEBUG_LEVEL 0

//#include "BalancedTree.h"

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

		BalancedTree* buildTree(CachedFileIO& cachedFile, bool verbose);
		bool deleteTree(BalancedTree* bt, bool verbose);
	};

};

