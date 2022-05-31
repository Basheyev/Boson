/*
* Boson Database
*
*
* (C) Bolat Basheyev 2022
*/

#include "Boson.h"
#include "table/BalancedTree.h"
#include "test/BalancedTreeTest.h"

using namespace Boson;

int main()
{
	BalancedTreeTest btt;
	btt.run(false);

	return 0;
}
