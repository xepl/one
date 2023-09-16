#include "xepl.h"

int main ()
{
	XEPL::Brain brain ( "intro", false );

	brain.Register_Operator ("space",[] CORTEX_OPERATOR
	{
		_lhs->push_back( ' ' );
		_lhs->append   ( *_rhs );
	} );

	brain.Execute_Rna("  'Hello,'.space('World!')  ");
}
// g++ -std=c++17 -L. -lxepl intro3.cpp -o intro3
