#pragma once

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

		BalancedTree* buildTree(bool verbose);
		BalancedTree* buildRandomTree(bool verbose);
		bool deleteTree(BalancedTree* bt, bool verbose);
		bool testPerformance(BalancedTree* bt, bool verbose);
	};

};

