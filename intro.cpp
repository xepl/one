#include "xepl.cc"
#include "xepl_defaults.hpp"

#include "xepl_string_tools.hpp"
#include "xepl_file_kit.hpp"
#include "xepl_cli_kit.hpp"
#include "xepl_keyword_kit.hpp"
#include "xepl_operator_kit.hpp"
#include "xepl_splicer_kit.hpp"

#include "xepl_timer_kit.hpp"

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