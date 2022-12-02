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

		void generateFileData();


		void open(char* path, size_t cacheSize = DEFAULT_CACHE_SIZE);
		
		double sequencialReadTest(size_t readBufferSize);

		double randomReadTest(size_t position, size_t bufferSize);
		void sequencialWriteTest(size_t bufferSize);
		void randomWriteTest();

		void close();


		double stdioSequencialRead(char* filename, size_t readBufferSize);
		double stdioRandomRead(char* filename, size_t position, size_t readBufferSize);

	private:

		CachedFileIO cf;
		char* fileName;

	};



}
