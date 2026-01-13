
// System Includes
#include <iostream>

// Includes
#include "ripemd160.h"

int main( int argc, char *argv[] )
{
	if ( argc <= 1 )
		return 1;

	const char *filepath = argv[ 1 ];

	RipeMD160Hash ripemd160Result = ripemd160( filepath );

	std::cout << ripemd160Result;

	return 0;
}