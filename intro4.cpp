#include "xepl.h"

int main ( int, const char**, char** )
{
	XEPL::Brain brain ( "intro4" );

	brain.Register_Command ( "Hello", [] BRAIN_COMMAND
	{
		std::cout << "Hello, World!\n" << std::flush;
	} );

	brain.Executes_Command("Hello");
}
// g++ -std=c++17 -L. -lxepl intro4.cpp -o intro4
