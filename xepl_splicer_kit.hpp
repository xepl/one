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

namespace KITS::SPLICERS
{
	void Splicer_Get ( XEPL::Cortex* _cortex );
	void Splicer_Set ( XEPL::Cortex* _cortex );

	void Register_Splicer_Kit( XEPL::Cortex* _cortex );
}

void KITS::SPLICERS::Splicer_Get ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Operator ( "Get", [] ( XEPL::Script* _script, XEPL::Cord* _rhs )
	{
		if ( !_rhs )
			return;

		if ( auto found_gene = _script->gene->Get_First( _rhs->c_str() ) )
			_script->gene = found_gene;
	} );
}

void KITS::SPLICERS::Splicer_Set ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Operator ( "Set", [] ( XEPL::Script* _script, XEPL::Cord* _rhs )
	{
		if ( !_script->gene )
			return;

		if ( !_rhs )
			return;

		auto& ephemerals = XEPL::tlsLobe->ephemerals;

		if ( !ephemerals )
			ephemerals = new XEPL::Ephemerals();

		ephemerals->Set ( _rhs, _script->gene );
	} );
}

void KITS::SPLICERS::Register_Splicer_Kit( XEPL::Cortex* _cortex )
{
	Splicer_Set         ( _cortex );
	Splicer_Get         ( _cortex );
}
