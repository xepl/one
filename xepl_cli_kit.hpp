// SPDX: AGPL-3.0-only
/* XEPL Operating Environment - Copyright (c) 2024 Keith Edwin Robbins
	Project Name: XEPL Operating Environment
	File Name:    xepl_cli_kit.hpp
	Author:       Keith Edwin Robbins
	Release date: May 10, 2024
	Website:      https://xepl.com

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU Affero General Public License as published by
	the Free Software Foundation version 3 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Affero General Public License for more details.

	You should have received a copy of the GNU Affero General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.

	For more information about the AGPL, please visit:
 	https://www.gnu.org/licenses/agpl-3.0.html
*/

namespace KITS::CLI
{
	bool Execute_As_Xml    ( XEPL::Text* _chars );
	bool Execute_As_Rna    ( XEPL::Text* _chars );
	bool Execute_From_File ( XEPL::Text* _chars );
	bool Execute_Command   ( XEPL::Text* _chars );

	bool CliLoop ( std::istream& _input_stream, std::ostream& _output_stream, std::ostream& _error_stream );
}

bool KITS::CLI::Execute_As_Xml ( XEPL::Text* _chars )
{
	XEPL::Gene* gene = new XEPL::Gene ( _chars );

	XEPL::ShortTerms nesting ( gene );

	if ( !XEPL::tlsLobe->Process_Gene ( gene ) )
		XEPL::xeplCantFind ( "Cmd", XEPL::tlsLobe, gene->cell_name );

	gene->Release();

	return true;
}

bool KITS::CLI::Execute_As_Rna ( XEPL::Text* _chars )
{
	XEPL::Cord   rna_cord ( _chars );
	XEPL::String result_string;

	XEPL::ShortTerms nesting;

	auto lobe = XEPL::tlsLobe;

	XEPL::Script ( lobe, lobe->index_link, &rna_cord, &result_string );
	std::cout << result_string << std::endl;

	return true;
}

bool KITS::CLI::Execute_From_File ( XEPL::Text* _chars )
{
	XEPL::Gene* gene = nullptr;

	XEPL::String file_name( _chars );
	file_name.append(".xml");

	if ( KITS::FILES::File_Load_Gene( &file_name, &gene ) )
	{
		XEPL::tlsLobe->Process_Inner_Genes ( gene );
		gene->Release();
		return true;
	}
	return false;
}

bool KITS::CLI::Execute_Command ( XEPL::Text* _chars )
{
	switch ( *_chars )
	{
		case '\0' :
		case ';' :
			return true;

		case '<' :
			return Execute_As_Xml ( _chars );

		case '{' :
		case '!' :
		case '%' :
			return Execute_As_Rna ( _chars );

		case '}' :
			return Execute_From_File ( _chars+1 );

		case '~' :
			return XEPL::tlsLobe->Drop_Neuron ( _chars+1 );

		case '|' :
			return system ( _chars+1 ) == 0;

		default :
			return XEPL::cortex->Did_Command ( _chars );
	}
}

bool KITS::CLI::CliLoop ( std::istream& _input_stream, std::ostream& _output_stream, std::ostream& _error_stream )
{
	errno = 0;

	XEPL::Lobe* host_lobe = XEPL::tlsLobe;

	XEPL::ShortTerms short_terms( nullptr );

	bool show_prompt = true;

	while ( !host_lobe->Test_Flags( XEPL::lysing_flag ) && !std::cin.eof() )
	{
		if ( show_prompt )
		{
			std::unique_lock lock_output ( XEPL::output_lock );
			_output_stream << host_lobe->cell_name << "> " << std::flush;
		}
		show_prompt = true;

		XEPL::String input_string;
		std::getline ( _input_stream, input_string );

		if ( input_string.compare(";") == 0 )
		{
			host_lobe->Close_Dispatch();
			continue;
		}

		if ( input_string.compare(";;") == 0 )
			return true;

		if ( input_string.compare("quit") == 0 )
			return false;

		if ( errno == EINTR )
		{
			show_prompt = false;
			_input_stream.clear();
			errno = 0;
			continue;
		}

		while (	!host_lobe->Test_Flags( XEPL::lysing_flag ) && host_lobe->Dispatch_Action() )
			_output_stream.flush();

		if ( !Execute_Command ( input_string.c_str() ) )
			_error_stream << "Command Failed: " << input_string << '\n' << std::flush;
	}
	return false;
}
