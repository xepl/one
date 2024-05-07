// SPDX: AGPL-3.0-only
/* XEPL Operating Environment - Copyright (c) 2024 Keith Edwin Robbins
	Project Name: XEPL Operating Environment
	File Name:    xepl_file_kit.hpp
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
namespace KITS::FILES
{
	bool String_Into_File ( XEPL::Cord* _name_cord, XEPL::Cord* _contents_cord );
	bool File_Load_String ( XEPL::Cord* _cord, std::string& std_output_string );
	bool File_Load_String ( XEPL::Cord* _cord, XEPL::String* _string );
	bool File_Load_Gene   ( XEPL::Cord* _filename_cord, XEPL::Gene** _gene );

	void Keyword_FileWrite ( XEPL::Cortex* _cortex );
	void Keyword_FileRead  ( XEPL::Cortex* _cortex );
	void Keyword_Include   ( XEPL::Cortex* _cortex );
	void Keyword_Command   ( XEPL::Cortex* _cortex );

	void Register_File_Kit( XEPL::Cortex* _cortex );
}

bool KITS::FILES::String_Into_File ( XEPL::Cord* _name_cord, XEPL::Cord* _contents_cord )
{
	if ( !_name_cord || _name_cord->empty() )
		return false;

	if ( !_contents_cord || _contents_cord->empty() )
		return false;

	XEPL::String filename_string( _name_cord );
	filename_string.insert( 0, "resources/");

	std::ofstream output_stream ( filename_string.c_str(), std::ios::out );

	if ( !output_stream.is_open() )
		return false;

	output_stream << _contents_cord->c_str();

	if ( !output_stream.fail() )
		output_stream.close();

	return !output_stream.fail();
}

bool KITS::FILES::File_Load_String ( XEPL::Cord* _cord, std::string& std_output_string )
{
	if ( !_cord || _cord->empty() )
		return false;

	XEPL::String filename_string( _cord );

	XEPL::TRACE("Load_File", nullptr, &filename_string );

	int more_tries_int=2;
	while (more_tries_int)
	{
		std::ifstream std_input_stream ( filename_string.c_str() );

		if ( std_input_stream.is_open() )
		{
			std::stringstream std_string_stream;
			std_string_stream << std_input_stream.rdbuf();
			std_input_stream.close();

			std_output_string = std_string_stream.str();

			return true;
		}
		filename_string.insert( 0, "../");
		more_tries_int--;
	}

	std::cout << "cannot load " << _cord << '\n';

	return false;
}

bool KITS::FILES::File_Load_String ( XEPL::Cord* _cord, XEPL::String* _string )
{
	if ( !_string )
		return false;

	std::string std_string;

	if ( !File_Load_String ( _cord, std_string ) )
		return false;

	_string->assign ( std_string );

	return true;
}

bool KITS::FILES::File_Load_Gene ( XEPL::Cord* _filename_cord, XEPL::Gene** _gene )
{
	if ( !_filename_cord || _filename_cord->empty() )
		return false;

	XEPL::String file_contents_string;

	if ( File_Load_String ( _filename_cord, &file_contents_string ) )
	{
		XEPL::Gene* file_gene = new XEPL::Gene( nullptr, "file", nullptr );

		XEPL::XeplXml file_parser ( file_gene, &file_contents_string );

		if ( !file_parser.error_string )
		{
			file_gene->Trait_Set( "path", _filename_cord );
			*_gene = file_gene;
			return true;
		}
		file_gene->Release();
	}

	return false;
}


void KITS::FILES::Keyword_FileWrite ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Keyword ( "FileWrite", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* )
	{
		XEPL::String filename;
		if ( _call_gene->Trait_Get ( "filename", &filename ) )
		{
			XEPL::String content;
			if ( XEPL::tlsLobe->index_link->Copy_Content(&content) )
			{
				if ( KITS::FILES::String_Into_File ( &filename, &content ) )
				{
					_neuron->Process_Exact_Gene ( "Ok", _call_gene );
				}
				else
					_neuron->Process_Exact_Gene ( "Failed", _call_gene );
			}
		}
	} );
}

void KITS::FILES::Keyword_FileRead ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Keyword ( "FileRead", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* )
	{
		if ( auto filename = _call_gene->Trait_Raw("filename") )
		{
			XEPL::Gene* file_gene = new XEPL::Gene ( nullptr, "File", nullptr );

			if ( KITS::FILES::File_Load_String ( filename, file_gene->Make_Content() ) )
			{
				XEPL::ScopeIndex swap_index ( file_gene );

				_neuron->Process_Exact_Gene ( "Found", _call_gene );
				XEPL::tlsLobe->Set_Outdex ( file_gene );
				file_gene->Release();
				return;
			}
			file_gene->Release();
		}
		_neuron->Process_Exact_Gene ( "Missing", _call_gene );
	} );
}

void KITS::FILES::Keyword_Include ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Keyword ( "Include", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* )
	{
		XEPL::Gene* load = nullptr;
		{
			XEPL::Cord* filename = _call_gene->Trait_Raw("filename");

			if ( !filename || !KITS::FILES::File_Load_Gene ( filename, &load )  )
				return;

			XEPL::tlsLobe->shadows->Make_One("Include")->Add_Gene(load);
			load->Release();
		}
		XEPL::ShortTerms capture_parameters( _call_gene );
		_neuron->Process_Gene(load->First());
	} );
}

void KITS::FILES::Keyword_Command ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Keyword ( "Command", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* _param )
	{
		XEPL::cortex->Did_Command(_param->c_str());
		XEPL::ShortTerms capture_parameters( _call_gene );
		_neuron->Process_Inner_Genes(_call_gene);
	} );
}


void KITS::FILES::Register_File_Kit( XEPL::Cortex* _cortex )
{
	Keyword_FileRead     ( _cortex );
	Keyword_FileWrite    ( _cortex );
	Keyword_Include      ( _cortex );
	Keyword_Command      ( _cortex );
}

