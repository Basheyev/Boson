#pragma once

#include "CachedFile.h"

namespace Boson {



	class CachedFileTest {
	public:

		CachedFileTest();
		~CachedFileTest();

		void open(char* filename, size_t cacheSize = DEFAULT_CACHE_SIZE);
		
		void sequencialReadTest(size_t readBufferSize);

		void randomReadTest(size_t position, size_t bufferSize);
		void sequencialWriteTest();
		void randomWriteTest();

		void close();


		void stdioSequencialRead(char* filename, size_t readBufferSize);

	private:

		CachedFile cf;

	};



}
