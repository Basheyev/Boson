/*
* ==========================================================================
* Boson Database
* ==========================================================================
* - NoSQL database engine.
* - Single database file.
* - Standard Key/Value store.
* - Document store (JSON).
* - Support cursors for linear records traversal.
* - Support for on-disk as well in-memory databases.
* - Support Terabyte sized databases.
* ==========================================================================
* (C) Bolat Basheyev 2022
*/

#include "Boson.h"

#include "io/CachedFileIO.h"
#include "test/CachedFileIOTest.h"

#include "core/BalancedTreeIndex.h"
#include "BalancedTreeTest.h"

#include <iostream>
#include <ctime>

using namespace Boson;



/**
*  @brief Probability density function for normal distribution (Gauss)
* 
*  @param x - argument
*  @param sigma - standard deviation
*  @param mean - mean number 
* 
*/
double probabilityDensity(double x, double sigma, double mu) {
	using namespace std;
	double pi = 3.14159265358979323846;
	double e = 2.71828182845904523536;
	double power = -0.5 * pow((x - mu) / sigma, 2);
	double numerator = 1.0;
	double denominator = sigma * sqrt(2 * pi);
	return (numerator / denominator) * pow(e, power);
}



int main()
{
	using namespace std;
	//CachedFileIOTest cft("F:\\database.bin");
	//cft.generateFileData();
	
	// normalized random
	//
	
	//double f = random * dF;
	
	double random = ((double)std::rand()) / (double)RAND_MAX;
	double y;
	srand(time(NULL));
	for (double x = 0.0; x < 1; x += 0.025) {
		random = ((double)std::rand()) / (double)RAND_MAX;
		y = probabilityDensity(x, 0.1, 0.5); // *random;
		cout << "x=" << x;
		cout << setprecision(10) << "\t f(x)=" << y << "\t";
		for (int i = 0; i < (y / 0.1); i++) {
			cout << "#";
		}
		cout << endl;
	}

	return 0;
}
