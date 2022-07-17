#pragma once

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

		BalancedTreeIndex<size_t, std::string>* buildTree(bool verbose);
		bool deleteTree(BalancedTreeIndex<size_t, std::string>* bt, bool verbose);
	};

};

