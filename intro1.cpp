#include "xepl.h"

int main ( int, const char**, char** )
{
	XEPL::Brain brain ( "Amazing" );

	std::cout << "Hello, World!\n";

	new int;

	std::cout << "XEPL detects leaks\n";
}
// g++ -std=c++17 -L. -lxepl intro1.cpp -o intro1
