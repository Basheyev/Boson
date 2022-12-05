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
		bool run(size_t samples = 1000000, size_t jsonSize = 384, double cacheRatio = 0.15, double sigma = 0.05);
	private:
		CachedFileIO cf;
		char* fileName;
		size_t samplesCount;
		size_t docSize;
		double cacheRatio;
		double sigma;

		double generateFileData();
		double randNormal(double mean, double stddev);
		double cachedRandomReads();
		double stdioRandomReads();
	};



}
