#pragma once

#include "../api/BosonAPI.h"
#include <iostream>


using namespace Boson;

namespace Boson {

	class BosonAPITest {
	public:
		BosonAPITest(char* path);
		~BosonAPITest();
		void run();
	private:
		void insertData();
		void eraseData();
		void traverseEntries();
		BosonAPI db;
	};

}