/******************************************************************************
* 
*  CachedFileIO class test header
* 
*  (C) Bolat Basheyev 2022
* 
******************************************************************************/
#pragma once


#define _ITERATOR_DEBUG_LEVEL 0

#include <cstdio>
#include <iostream>
#include <locale>
#include <chrono>
#include <thread>
#include <filesystem>

#include "CachedFileIO.h"

namespace Boson {

	class CachedFileIOTest {
	public:
		CachedFileIOTest(char* path);
		~CachedFileIOTest();
		bool run(size_t samples = 1000000, size_t jsonSize = 479, double cacheRatio = 0.15, double sigma = 0.04);
	private:
		CachedFileIO cf;
		char* fileName;
		size_t samplesCount;
		size_t docSize;
		double cacheRatio;
		double sigma;

		double cachedRandomPageWrites();
		double cachedRandomWrites();
		double randNormal(double mean, double stddev);
		double cachedRandomReads();
		double stdioRandomReads();
		double cachedRandomPageReads();
		double stdioRandomPageReads();
	};

}
