#include "xepl.h"

int main ( int, const char**, char** )
{
	XEPL::Brain brain ( "Intro" );

	brain.Register_Keyword ( "Hi", [] CORTEX_KEYWORD
	{
		std::cout << "Hello" << *_call_gene->Content() << std::endl;
	} );

	brain.Execute_Xml ( "<Hi>, World!</Hi>" );
}
// g++ -std=c++17 -L. -lxepl intro2.cpp -o intro2
