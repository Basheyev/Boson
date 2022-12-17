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
		bool run(size_t samples = 1000000, size_t jsonSize = 384, double cacheRatio = 0.1, double sigma = 0.05);
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

	//------------------------------------------------------------------

	class NumberPunctuation : public std::numpunct<char>
	{
	protected:
		virtual char do_thousands_sep() const { return ','; }
		virtual std::string do_grouping() const { return "\03"; }
	};

	//------------------------------------------------------------------

}
