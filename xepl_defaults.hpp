// SPDX: AGPL-3.0-only
/* XEPL Operating Environment - Copyright (c) 2024 Keith Edwin Robbins
	Project Name: XEPL Operating Environment
	File Name:    xepl_defaults.hpp
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

#ifdef _WIN32
void XEPL::Set_Thread_Name ( XEPL::Thread*, Text*  _name )
{
	pthread_setname_np ( _name );
}
#elif __linux__
void XEPL::Set_Thread_Name ( XEPL::Thread* _thread, Text*  _name )
{
	pthread_setname_np ( _thread->std_thread, _name );
}
#else
void XEPL::Set_Thread_Name ( XEPL::Thread* _thread, Text*  _name )
{
	pthread_setname_np ( _name );
}
#endif


XEPL::ErrorReport::ErrorReport ( Text* _text )
	: String ( _text )

{}  // SET A BREAKPOINT HERE ... To Catch Errors


void XEPL::TRACE(Text* _action, XEPL::Neuron* _neuron, XEPL::Cord* _cord, XEPL::Cord* _cord2 )
{
	if(XEPL::Show_Trace)
	{
		std::unique_lock lock_output ( XEPL::output_lock );

		XEPL::String path_string;

		if ( _neuron )
			_neuron->Feature_Get("path", &path_string);

		std::cout
		<< std::setw(12) << ( XEPL::tlsLobe ? *XEPL::tlsLobe->cell_name : "" )
		<< ": " << std::setw(10) << _action
		<< ": " << path_string
		<< "."  << ( _cord ? *_cord : "" )
		<< " "  << ( _cord2 ? *_cord2 : "" ) << '\n';
	}
}
