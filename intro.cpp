#include "xepl.hpp"

#include "../kits/xepl_all_kits.cc"
#include "../kits/PerformanceTimer.hpp"

int main ( int, char**, char** )
{
	bool reboot;
	do {
		XEPL::Cortex cortex ( "brain", std::cout );
		
		KITS::TIMER::Initialize(100);
		{
			KITS::OPERATORS::Register_Operator_Kit  ( &cortex );
			KITS::KEYWORDS::Register_Keyword_Kit    ( &cortex );
			KITS::SPLICERS::Register_Splicer_Kit    ( &cortex );
			KITS::TIMER::Register_Performance_Kit   ( &cortex );

			cortex.Register_Command ( "Trace",     [] ( XEPL::String* _opt ) { XEPL::Show_Trace          = _opt->compare("off"); });
			cortex.Register_Command ( "Counters",  [] ( XEPL::String* _opt ) { XEPL::Show_Counters       = _opt->compare("off"); });
			cortex.Register_Command ( "Memory",    [] ( XEPL::String* _opt ) { XEPL::Show_Memory_Counts  = _opt->compare("off"); });
			
			reboot = KITS::CLI::CliLoop(std::cin, std::cout, std::cerr);

			cortex.Close_Cortex();
		}
		KITS::TIMER::Shutdown();

	} while ( reboot );

	return 0;
}