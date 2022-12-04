/******************************************************************************
* 
*  CachedFileIO class test header
* 
*  (C) Bolat Basheyev 2022
* 
******************************************************************************/
#pragma once

#include "CachedFileIO.h"

namespace Boson {

	class CachedFileIOTest {
	public:
		CachedFileIOTest(char* path);
		~CachedFileIOTest();
		bool run();
	private:
		CachedFileIO cf;
		char* fileName;
		double generateFileData();
		double randNormal(double mean, double stddev);
		double cachedRandomReads();
		double stdioRandomReads();
	};



}
