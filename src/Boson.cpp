/*
* ==========================================================================
* Boson Database
* ==========================================================================
* - Serverless, NoSQL database engine.
* - Transactional (ACID) database.
* - Zero configuration.
* - Single database file, does not use temporary files.
* - Cross-platform file format.
* - Self-Contained library without dependency.
* - Standard Key/Value store.
* - Document store (JSON).
* - Support cursors for linear records traversal.
* - Pluggable run-time interchangeable storage engine.
* - Support for on-disk as well in-memory databases.
* - Built with a powerful disk storage engine which support O(1) lookup.
* - Thread safe and full reentrant.
* - Simple, Clean and easy to use API.
* - Support Terabyte sized databases.
* ==========================================================================
* (C) Bolat Basheyev 2022
*/

#include "Boson.h"
#include "BalancedTreeIndex.h"
#include "BalancedTreeTest.h"

#include <chrono>
#include <iostream>

using namespace Boson;

int main()
{
    BalancedTreeTest btt;
	btt.run(true);

	return 0;
}
