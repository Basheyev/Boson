#pragma once

#include <filesystem>

#include "BalancedIndex.h"

namespace Boson {

	class BalancedIndexTest {
	public:
		BalancedIndexTest(char* path);
		~BalancedIndexTest();
		bool run(bool clearFile = false);
		void insertRecords(BalancedIndex* bi);
		void removeRecords(BalancedIndex* bi);
	private:
		const char* filename;
	};

}
