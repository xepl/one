// SPDX: AGPL-3.0-only
/* XEPL Operating Environment - Copyright (c) 2024 Keith Edwin Robbins
	Project Name: XEPL Operating Environment
	File Name:    xepl_opereator_kit.hpp
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

namespace KITS::OPERATORS
{
	void Operator_After     ( XEPL::Cortex* _cortex );
	void Operator_AfterAny  ( XEPL::Cortex* _cortex );
	void Operator_AfterLast ( XEPL::Cortex* _cortex );
	void Operator_Before    ( XEPL::Cortex* _cortex );
	void Operator_BeforeAny ( XEPL::Cortex* _cortex );
	void Operator_NextAny   ( XEPL::Cortex* _cortex );

	void Operator_Append ( XEPL::Cortex* _cortex );
	void Operator_Cr     ( XEPL::Cortex* _cortex );
	void Operator_CrLf   ( XEPL::Cortex* _cortex );
	void Operator_Lf     ( XEPL::Cortex* _cortex );
	void Operator_Period ( XEPL::Cortex* _cortex );
	void Operator_Slash  ( XEPL::Cortex* _cortex );
	void Operator_Space  ( XEPL::Cortex* _cortex );
	void Operator_Tab    ( XEPL::Cortex* _cortex );

	void Operator_Empty ( XEPL::Cortex* _cortex );
	void Operator_Has   ( XEPL::Cortex* _cortex );
	void Operator_Is    ( XEPL::Cortex* _cortex );

	void Operator_Add      ( XEPL::Cortex* _cortex );
	void Operator_Multiply ( XEPL::Cortex* _cortex );
	void Operator_Subtract ( XEPL::Cortex* _cortex );
 
	void Operator_Equal ( XEPL::Cortex* _cortex );
	void Operator_Gt    ( XEPL::Cortex* _cortex );
	void Operator_Lt    ( XEPL::Cortex* _cortex );

	void Operator_DeAmp      ( XEPL::Cortex* _cortex );
	void Operator_Depercent  ( XEPL::Cortex* _cortex );
	void Operator_Lowercase  ( XEPL::Cortex* _cortex );
	void Operator_Percentify ( XEPL::Cortex* _cortex );

	void Register_Operator_Kit( XEPL::Cortex* _cortex );
}

void KITS::OPERATORS::Operator_After ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Operator ( "after", [] ( XEPL::Script* _script, XEPL::Cord* _rhs )
	{
		if ( _rhs )
		{
			auto idx  = _script->value->find ( *_rhs );
			if ( idx != XEPL::String::npos )
			{
				_script->value->erase ( 0, idx+_rhs->size() );
				_script->truth = true;
				return;
			}
		}
		_script->truth = false;
	} );
}

void KITS::OPERATORS::Operator_AfterAny ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Operator ( "afterAny", [] ( XEPL::Script* _script, XEPL::Cord* _rhs )
	{
		if ( _rhs )
		{
			auto idx  = _script->value->find_first_of ( *_rhs );
			if ( idx != XEPL::String::npos )
			{
				_script->value->erase ( 0, idx+1 );
				_script->truth = true;
				return;
			}
		}
		_script->value->erase();
		_script->truth = false;
	} );
}

void KITS::OPERATORS::Operator_AfterLast ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Operator ( "afterLast", [] ( XEPL::Script* _script, XEPL::Cord* _rhs )
	{
		if ( _rhs )
		{
			auto idx  = _script->value->rfind ( *_rhs );
			if ( idx != XEPL::String::npos )
			{
				_script->value->erase ( 0, idx+_rhs->size() );
				_script->truth = true;
				return;
			}
		}
		_script->truth = false;
	} );
}

void KITS::OPERATORS::Operator_Append ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Operator ( "append", [] ( XEPL::Script* _script, XEPL::Cord* _rhs )
	{
		if ( _rhs )
			_script->value->append( *_rhs );

		_script->truth = !_script->value->empty();
	} );
}

void KITS::OPERATORS::Operator_Before ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Operator ( "before", [] ( XEPL::Script* _script, XEPL::Cord* _rhs )
	{
		if ( _rhs )
		{
			auto idx  = _script->value->find ( *_rhs );
			if ( idx != XEPL::String::npos )
			{
				_script->value->erase ( idx );
				_script->truth = true;
				return;
			}
		}
		_script->truth = false;
	} );
}

void KITS::OPERATORS::Operator_BeforeAny ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Operator ( "beforeAny", [] ( XEPL::Script* _script, XEPL::Cord* _rhs )
	{
		if ( _rhs )
		{
			auto idx  = _script->value->find_first_of ( *_rhs );
			if ( idx != XEPL::String::npos )
			{
				_script->value->erase ( idx );
				_script->truth = true;
				return;
			}
		}
		_script->truth = false;
	} );
}

void KITS::OPERATORS::Operator_Cr ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Operator ( "cr", [] ( XEPL::Script* _script, XEPL::Cord* _rhs )
	{
		_script->value->push_back ( '\r' );

		if ( _rhs )
			_script->value->append ( *_rhs );
	} );
}

void KITS::OPERATORS::Operator_Lf ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Operator ( "lf", [] ( XEPL::Script* _script, XEPL::Cord* _rhs )
	{
		_script->value->push_back ( '\n' );

		if ( _rhs )
			_script->value->append ( *_rhs );
	} );
}

void KITS::OPERATORS::Operator_CrLf ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Operator ( "crlf", [] ( XEPL::Script* _script, XEPL::Cord* _rhs )
	{
		_script->value->append ( "\r\n" );

		if ( _rhs )
			_script->value->append ( *_rhs );
	} );
}

void KITS::OPERATORS::Operator_DeAmp ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Operator ( "deamp", [] ( XEPL::Script* _script, XEPL::Cord* _rhs )
	{
		const char amper='&';
		XEPL::Text*  start = _script->value->c_str();
		XEPL::Text*   pos1 = std::strchr ( start, amper );

		if ( !pos1 )
		{
			if ( _rhs )
				_script->value->append ( *_rhs );

			return;
		}

		XEPL::Text* pos2 = nullptr;
		XEPL::String scratch;
		scratch.reserve ( _script->value->size() );

		while ( pos1 )
		{
			long length = ( pos1-start );

			if ( length >=1 )
				scratch.append ( start, length );

			pos1++;
			pos2 = std::strpbrk ( pos1, "&;" );
			XEPL::Text* conv = nullptr;

			if ( !pos2 )
				break;

			size_t len = ( pos2-pos1 );

			if ( *pos2 == ';' )
			{
				start = pos2+1;
				switch ( len )
				{
					case 2:
						if ( !std::strncmp ( pos1, "gt", len ) )
							conv = ">";
						else if ( !std::strncmp ( pos1, "lt", len ) )
							conv = "<";
						break;
					case 3:
						if ( !std::strncmp ( pos1, "amp", len ) )
							conv = "&";
						break;
					case 4:
						if ( !std::strncmp ( pos1, "apos", len ) )
							conv = "'";
						else if ( !std::strncmp ( pos1, "quot", len ) )
							conv = "\"";
						break;
					default:
						start = pos1;
						break;
				}
			}

			if ( conv )
				scratch.append ( conv );
			else
			{
				scratch.append ( pos1-1, len+2 );
				start = pos2+1;
			}
			pos1 = std::strchr ( start, amper );
		}

		scratch.append ( start );

		_script->value->assign ( scratch );
		_script->truth = !_script->value->empty();

		if ( _rhs )
			_script->value->append ( *_rhs );
	} );
}

void KITS::OPERATORS::Operator_Percentify ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Operator ( "percentify", [] ( XEPL::Script* _script, XEPL::Cord* _rhs )
	{
		auto input = *_script->value;
		bool conversion_made = false;

		XEPL::String converted;

		for ( size_t i = 0; i < input.size(); ++i )
		{
			if ( input[i] == '%' && i + 2 < input.size() && std::isxdigit ( input[i + 1] ) && std::isxdigit ( input[i + 2] ) )
			{
				converted.push_back( static_cast<char> ( ( XEPL::hexCharToInt ( input[i + 1] ) << 4 ) + XEPL::hexCharToInt ( input[i + 2] ) ) );
				i += 2;
				conversion_made = true;
			}
			else if ( input[i] == '+' )
			{
				converted.push_back(' ');
				conversion_made = true;
			}
			else
				converted.push_back( input[i] );
		}

		if ( conversion_made )
			_script->value->assign( converted );
			
		_script->truth = !_script->value->empty();

		if ( _rhs )
			_script->value->append ( *_rhs );
		} );
}

void KITS::OPERATORS::Operator_Depercent ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Operator ( "depercent", [] ( XEPL::Script* _script, XEPL::Cord* _rhs )
	{
		auto input = *_script->value;
		bool conversion_made = false;

		XEPL::String converted;

		for ( size_t i = 0; i < input.size(); ++i )
		{
			if ( input[i] == '%' && i + 2 < input.size() && std::isxdigit ( input[i + 1] ) && std::isxdigit ( input[i + 2] ) )
			{
				converted.push_back( static_cast<char> ( ( XEPL::hexCharToInt ( input[i + 1] ) << 4 ) + XEPL::hexCharToInt ( input[i + 2] ) ) );
				i += 2;
				conversion_made = true;
			}
			else if ( input[i] == '+' )
			{
				converted.push_back(' ');
				conversion_made = true;
			}
			else
				converted.push_back( input[i] );
		}
		if ( conversion_made )
			_script->value->assign( converted );

		_script->truth = !_script->value->empty();

		if ( _rhs )
			_script->value->append ( *_rhs );
	} );
}

void KITS::OPERATORS::Operator_Empty ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Operator ( "empty", [] ( XEPL::Script* _script, XEPL::Cord* )
	{
		_script->truth = _script->value->empty();
	} );
}

void KITS::OPERATORS::Operator_Has ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Operator ( "has", [] ( XEPL::Script* _script, XEPL::Cord* _rhs )
	{
		if ( _rhs )
			_script->truth = _script->value->find ( *_rhs ) != XEPL::String::npos;
		else
			_script->truth = false;
	} );
}

void KITS::OPERATORS::Operator_Is ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Operator ( "is", [] ( XEPL::Script* _script, XEPL::Cord* _rhs )
	{
		if ( _rhs )
			_script->truth = _script->value->compare ( *_rhs ) == 0;
		else
			_script->truth = false;
	} );
}


void KITS::OPERATORS::Operator_Lowercase ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Operator ( "lower", [] ( XEPL::Script* _script, XEPL::Cord* _rhs )
	{
		std::transform ( _script->value->begin(), _script->value->end(), _script->value->begin(), ::tolower );

		if ( _rhs )
			_script->value->append ( *_rhs );

		_script->truth = !_script->value->empty();
	} );
}
long number_from( XEPL::Cord* _num )
{
	if ( _num->empty() )
		return 0L;
	
	return std::stol( _num->c_str(), nullptr, 10 );
}

void KITS::OPERATORS::Operator_Lt ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Operator ( "lt", [] ( XEPL::Script* _script, XEPL::Cord* _rhs )
	{
		if ( _rhs )
			_script->truth = number_from ( _script->value ) < number_from ( _rhs );
	} );
}

void KITS::OPERATORS::Operator_Gt ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Operator ( "gt", [] ( XEPL::Script* _script, XEPL::Cord* _rhs )
	{
		if ( _rhs )
			_script->truth = number_from ( _script->value ) > number_from ( _rhs );
	} );
}

void KITS::OPERATORS::Operator_Equal ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Operator ( "eq", [] ( XEPL::Script* _script, XEPL::Cord* _rhs )
	{
		if ( _rhs )
			_script->truth = number_from ( _script->value ) == number_from ( _rhs );
	} );
}

void KITS::OPERATORS::Operator_Add ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Operator ( "add", [] ( XEPL::Script* _script, XEPL::Cord* _rhs )
	{
		if ( _rhs )
			_script->value->assign ( std::to_string ( number_from ( _script->value ) + number_from ( _rhs ) ) );
	} );
}

void KITS::OPERATORS::Operator_Subtract ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Operator ( "sub", [] ( XEPL::Script* _script, XEPL::Cord* _rhs )
	{
		if ( _rhs )
			_script->value->assign ( std::to_string ( number_from ( _script->value ) - number_from ( _rhs ) ) );
	} );
}

void KITS::OPERATORS::Operator_Multiply ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Operator ( "mul", [] ( XEPL::Script* _script, XEPL::Cord* _rhs )
	{
		if ( _rhs )
			_script->value->assign ( std::to_string ( number_from ( _script->value ) * number_from ( _rhs ) ) );
	} );
}

void KITS::OPERATORS::Operator_NextAny ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Operator ( "nextAny", [] ( XEPL::Script* _script, XEPL::Cord* _rhs )
	{
		if ( _rhs )
		{
			auto idx  = _script->value->find_first_of ( *_rhs );
			if ( idx != XEPL::String::npos )
			{
				_script->value->erase ( idx+1 );
				_script->value->erase ( 0, idx );
				_script->truth = true;
				return;
			}
		}
		_script->value->erase();
		_script->truth = false;
	} );
}

void KITS::OPERATORS::Operator_Period ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Operator ( "period", [] ( XEPL::Script* _script, XEPL::Cord* _rhs )
	{
		_script->value->push_back ( '.' );

		if ( _rhs )
			_script->value->append ( *_rhs );
	} );
}

void KITS::OPERATORS::Operator_Slash ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Operator ( "slash", [] ( XEPL::Script* _script, XEPL::Cord* _rhs )
	{
		_script->value->push_back ( '/' );

		if ( _rhs )
			_script->value->append ( *_rhs );
	} );
}

void KITS::OPERATORS::Operator_Space ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Operator ( "space", [] ( XEPL::Script* _script, XEPL::Cord* _rhs )
	{
		_script->value->push_back ( ' ' );

		if ( _rhs )
			_script->value->append ( *_rhs );
	} );
}

void KITS::OPERATORS::Operator_Tab ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Operator ( "tab", [] ( XEPL::Script* _script, XEPL::Cord* _rhs )
	{
		_script->value->push_back ( '\t' );

		if ( _rhs )
			_script->value->append ( *_rhs );
	} );
}

void KITS::OPERATORS::Register_Operator_Kit( XEPL::Cortex* _cortex )
{
	Operator_Add        ( _cortex );
	Operator_Subtract   ( _cortex );
	Operator_Multiply   ( _cortex );
	Operator_Equal      ( _cortex );
	Operator_After      ( _cortex );
	Operator_AfterAny   ( _cortex );
	Operator_AfterLast  ( _cortex );
	Operator_Append     ( _cortex );
	Operator_Before     ( _cortex );
	Operator_BeforeAny  ( _cortex );
	Operator_Cr         ( _cortex );
	Operator_CrLf       ( _cortex );
	Operator_DeAmp      ( _cortex );
	Operator_Depercent  ( _cortex );
	Operator_Percentify ( _cortex );
	Operator_Empty      ( _cortex );
	Operator_Has        ( _cortex );
	Operator_Is         ( _cortex );
	Operator_Lf         ( _cortex );
	Operator_Lt         ( _cortex );
	Operator_Gt         ( _cortex );
	Operator_Tab        ( _cortex );
	Operator_Space      ( _cortex );
	Operator_Period     ( _cortex );
	Operator_Slash      ( _cortex );
	Operator_Lowercase  ( _cortex );
	Operator_NextAny    ( _cortex );
}
