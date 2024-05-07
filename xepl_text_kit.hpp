// SPDX: AGPL-3.0-only
/* XEPL Operating Environment - Copyright (c) 2024 Keith Edwin Robbins
	Project Name: XEPL Operating Environment
	File Name:    xepl_text_kit.hpp
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

namespace KITS::TEXT
{
	void Keyword_Text     ( XEPL::Cortex* _cortex );
	void Register_Text_Kit( XEPL::Cortex* _cortex );
}

namespace KITS::TEXT
{
	class RendonText : public XEPL::Rendon
	{
	public: 	
		explicit RendonText ( XEPL::Neuron* _parent, XEPL::Gene* _config, XEPL::String* _bag );
		virtual void Rendon_Markup ( XEPL::Gene* _gene, XEPL::Gene* ) override;
	};
}




 KITS::TEXT::RendonText::RendonText ( XEPL::Neuron* _parent, XEPL::Gene* _config, XEPL::String* _bag )
	: Rendon ( _parent, _config, _bag )
{}

 void KITS::TEXT::RendonText::Rendon_Markup ( XEPL::Gene* _gene, XEPL::Gene* ) 
{
	XEPL::Bond* bond = _gene->inner_genes->head_bond;

	while ( bond )
	{
		XEPL::Gene* gene =  static_cast<XEPL::Gene*> ( bond->atom );
		Markup ( this, gene, rendition );
		bond = bond->next_bond ;
	}
}



void KITS::TEXT::Keyword_Text ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Keyword ( "Text", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* )
	{
		XEPL::String* output = XEPL::tlsLobe->output_string;

		KITS::TEXT::RendonText renderer ( _neuron, _call_gene, output );

		XEPL::String content;
		if ( _call_gene->Copy_Content(&content) )
			XEPL::Script ( _neuron, _call_gene, &content, output, nullptr, nullptr, true );

		renderer.Process_Inner_Genes ( _call_gene );
	} );
}

void KITS::TEXT::Register_Text_Kit(XEPL::Cortex* _cortex )
{
	Keyword_Text ( _cortex );
}
