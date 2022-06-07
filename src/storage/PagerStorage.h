#pragma once

#include <cstdint>

namespace Boson {


	typedef struct {
		int8_t  pageType;




	} PageHeader;


	typedef struct {
		PageHeader header;
		int64_t    keys[];
		int64_t    values[];		
	} DatabasePage;



	class PagerCache {
	public:



	private:


	};


	class PagerStorage {
	public:


	protected:
		PagerCache* cache;

	private:


	};



}