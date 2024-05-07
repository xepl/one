// SPDX: AGPL-3.0-only
/* XEPL Operating Environment - Copyright (c) 2024 Keith Edwin Robbins
	Project Name: XEPL Operating Environment
	File Name:    xepl_splicer_kit.hpp
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


bool XEPL::Split_ch_lhs ( String* _source_string, char _char, String* _lhs )
{
	if ( _source_string->empty() )
		return false;

	size_t pos = _source_string->find ( _char );
	if ( pos != String::npos )
	{
		_lhs->assign ( _source_string->c_str(), pos );
		_source_string->erase ( 0, pos+1 );
		return true;
	}
	return false;
}

/// split the string at character lhs/rhs
bool XEPL::Split_ch_rhs ( String* _source_string, char _char, String* _rhs )
{
	if ( _source_string->empty() )
		return false;

	size_t pos = _source_string->find ( _char );
	if ( pos != String::npos )
	{
		_rhs->assign ( _source_string->c_str()+pos+1 );
		_source_string->erase ( pos, _source_string->size()-pos );
		return true;
	}
	return false;
}

/// copy the string into two parts
bool XEPL::Split_ch_lhs_rhs ( Cord* _source_string, char _char, String* _lhs, String* _rhs )
{
	Text* start_position   = _source_string->c_str();
	Text* current_position = start_position;

	while ( *current_position && *current_position != _char )
		++current_position;

	if ( *current_position )
	{
		_lhs->assign ( start_position, current_position-start_position );
		_rhs->assign ( current_position+1 );
		return true;
	}
	_lhs->assign ( start_position );
	_rhs->assign ( start_position );
	return false;
}

/// remove embedded quotes and symbols
XEPL::String* XEPL::Escape_Quotes ( XEPL::Cord* _ptr, XEPL::String* _result )
{
	static Text* breakset    ="&<>\"";
	static Text* breakset_alt="&<>'";

	if ( !_ptr )
		return nullptr;

	if ( !_result )
		return nullptr;

	Text* start = _ptr->c_str();
	Text* pos = std::strpbrk ( start, breakset );

	if ( !pos )
	{
		_result->push_back (  '"'  );
		_result->append    ( *_ptr );
		_result->push_back (  '"'  );
		return _result;
	}

	if ( *pos == '"' )
	{
		Text* pos2 = std::strpbrk ( start, breakset_alt );

		if ( !pos2 )
		{
			_result->push_back (  '\'' );
			_result->append    ( *_ptr );
			_result->push_back (  '\'' );
			return _result;
		}
	}

	_result->push_back (  '"' );

	while ( pos )
	{
		_result->append ( start, pos-start );
		Text* conv = nullptr;

		switch ( *pos )
		{
			case '&':
				conv = "&amp;";
				break;

			case '<' :
				conv = "&lt;";
				break;

			case '>' :
				conv = "&gt;";
				break;

			case '\'' :
				conv = "&apos;";
				break;

			case '"' :
				conv = "&quot;";
				break;
		}

		_result->append ( conv );
		start = pos+1;
		pos = std::strpbrk ( start, breakset );
	}

	_result->append ( start );
	_result->push_back (  '"' );
	return _result;
}

/// What is hex 'a'?
int XEPL::hexCharToInt ( char ch )
{
	if ( ch >= '0' && ch <= '9' )
		return ch - '0';

	if ( ch >= 'A' && ch <= 'F' )
		return ch - 'A' + 10;

	if ( ch >= 'a' && ch <= 'f' )
		return ch - 'a' + 10;

	return 0;
}

/// Add commas
XEPL::String* XEPL::Long_Commafy ( long _number, XEPL::String* _into_string )
{
	const std::string number_string(std::to_string(_number));
	const size_t number_length = number_string.length();

	for ( size_t column_shift = 0; column_shift < number_length; ++column_shift )
	{
		*_into_string += number_string[column_shift];

		size_t position = number_length - column_shift - 1;
		
		if ( position % 3 == 0 && position != 0 )
			_into_string->push_back ( ',' );
	}

	return _into_string;
}

/// approximate byte count
XEPL::String* XEPL::Long_In_Bytes ( long _bytes, XEPL::String* _std_string )
{
	const long KB =      1024;
	const long MB = KB * 1024;
	const long GB = MB * 1024;

	std::ostringstream oss;
	oss << std::fixed << std::setprecision ( 2 );

	if ( _bytes >= GB )
		oss << static_cast<double> ( _bytes ) / ( GB ) << " GB";

	else if ( _bytes >= MB )
		oss << static_cast<double> ( _bytes ) / ( MB ) << " MB";

	else if ( _bytes >= KB )
		oss << static_cast<double> ( _bytes ) / ( KB ) << " KB";

	else
		oss << _bytes << " Bytes";

	*_std_string += oss.str();

	return _std_string;
}

std::string* XEPL::Long_In_Time ( long long milliseconds, std::string* result )
{
	int hours   = static_cast<int> (   milliseconds / ( 1000 * 60 * 60 ) );
	int minutes = static_cast<int> ( ( milliseconds / ( 1000 * 60 ) ) % 60 );
	int seconds = static_cast<int> ( ( milliseconds /   1000 )        % 60 );
	int ms      = static_cast<int> (   milliseconds % 1000 );

	std::ostringstream oss;
	oss << std::setfill ( '0' )
		<< std::setw ( 2 ) << hours   << ":"
		<< std::setw ( 2 ) << minutes << ":"
		<< std::setw ( 2 ) << seconds << "."
		<< std::setw ( 3 ) << ms;

	*result = oss.str();
	return result;
}

long XEPL::Into_Long( XEPL::Cord* _num )
{
	if ( _num->empty() )
		return 0L;
	
	return std::stol( _num->c_str(), nullptr, 10 );
}

