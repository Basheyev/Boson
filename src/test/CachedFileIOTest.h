#pragma once

#include "CachedFileIO.h"

namespace Boson {



	class CachedFileIOTest {
	public:

		CachedFileIOTest();
		~CachedFileIOTest();

		void open(char* filename, size_t cacheSize = DEFAULT_CACHE_SIZE);
		
		double sequencialReadTest(size_t readBufferSize);

		double randomReadTest(size_t position, size_t bufferSize);
		void sequencialWriteTest(size_t bufferSize);
		void randomWriteTest();

		void close();


		double stdioSequencialRead(char* filename, size_t readBufferSize);
		double stdioRandomRead(char* filename, size_t position, size_t readBufferSize);

	private:

		CachedFileIO cf;

	};



}
