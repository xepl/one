// SPDX: AGPL-3.0-only
/* XEPL Operating Environment - Copyright (c) 2024 Keith Edwin Robbins
	Project Name: XEPL Operating Environment
	File Name:    xepl_html_kit.hpp
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

namespace KITS::HTML
{
	void Scribble ( XEPL::Nucleus* _nucleus, XEPL::Gene* _gene, XEPL::Rendon* _rendon );
	void Output   ( XEPL::Nucleus* _nucleus, XEPL::Gene* _gene, XEPL::Rendon* _rendon );

	void Keyword_Html ( XEPL::Cortex* _cortex );

	void Register_Html_Kit( XEPL::Cortex* _cortex );
}

namespace KITS::HTML
{
	class RendonHtml : public XEPL::Rendon
	{
	public:
		explicit RendonHtml        ( XEPL::Neuron*,  XEPL::Gene*, XEPL::String* );
		virtual void Rendon_Markup ( XEPL::Gene*,    XEPL::Gene* ) override;
		virtual void Rendon_Render ( XEPL::Nucleus*, XEPL::Gene* ) override;
		void Scribble_Traits       ( XEPL::Nucleus*, XEPL::Gene* );
		void Scribble_Content      ( XEPL::Nucleus*, XEPL::Gene* );
		void Scribble_Gene         ( XEPL::Nucleus*, XEPL::Gene* );
	};
}

KITS::HTML::RendonHtml::RendonHtml ( XEPL::Neuron* _host, XEPL::Gene* _config, XEPL::String* _bag )
	: Rendon ( _host, _config, _bag )
{}

void KITS::HTML::RendonHtml::Rendon_Markup ( XEPL::Gene* _call_gene, XEPL::Gene* )
{
	XEPL::Bond* working_bond = nullptr;

	if ( _call_gene->inner_genes )
		 working_bond = _call_gene->inner_genes->head_bond;

	if ( auto tag = _call_gene->Trait_Raw ( "tag" ) )
	{
		XEPL::XmlBuilder x1 ( tag->c_str(), rendition );
		x1.Close_Attributes();

		while ( working_bond )
		{
			XEPL::Gene* gene   = static_cast<XEPL::Gene*> ( working_bond->atom );
			Markup ( this, gene, rendition );
			working_bond = working_bond->next_bond;
		}

		return;
	}

	XEPL::String content;
	if ( _call_gene->Copy_Content(&content) )
		XEPL::Script(Host(), _call_gene, &content, rendition, nullptr, nullptr, true );

	while ( working_bond )
	{
		XEPL::Gene* gene = static_cast<XEPL::Gene*> ( working_bond->atom );
		Markup ( this, gene, rendition );
		working_bond = working_bond->next_bond;
	}
}

void KITS::HTML::RendonHtml::Rendon_Render ( XEPL::Nucleus* _nucleus, XEPL::Gene* _gene )
{
	XEPL::XmlBuilder x1 ( "span", rendition );
	x1.Close_Attributes();
	Scribble_Gene ( _nucleus, _gene );
}

void KITS::HTML::RendonHtml::Scribble_Gene ( XEPL::Nucleus* _nucleus, XEPL::Gene* _element )
{
	if ( !_element )
		return;

	XEPL::StableGenes recall ( _element );

	{
		XEPL::XmlBuilder div_build ( "div", "class='gene'", rendition );
		{
			XEPL::XmlBuilder span_build ( "span", rendition );

			if ( recall.stable_gene_chain )
			{
				span_build.Attribute_Set ( "class", "geneName up" );
				span_build.Attribute_Set ( "onclick", "Debug(this,event);" );
			}
			else
				span_build.Attribute_Set ( "class", "geneName" );

			span_build.Close_Attributes();

			if ( _element->space_string )
			{
				rendition->append ( *_element->space_string );
				rendition->push_back ( ':' );
			}

			rendition->append ( *_element->cell_name );
		}

		Scribble_Traits   ( _nucleus, _element );
		Scribble_Content ( _nucleus, _element );
	}

	if ( recall.stable_gene_chain )
	{
		XEPL::XmlBuilder x1 ( "div", "class='subgenes' style='display:block'", rendition );

		XEPL::Gene* gene = nullptr;

		while ( recall.Next_Gene ( &gene ) )
			Scribble_Gene ( _nucleus, gene );
	}
}

void KITS::HTML::RendonHtml::Scribble_Traits ( XEPL::Nucleus*, XEPL::Gene* _element )
{
	if ( _element->traits )
	{
		XEPL::Cord*   name = nullptr;
		XEPL::String* term = nullptr;
		XEPL::StableTraits recall ( _element );

		while ( recall.Next_Trait ( &name, &term ) )
		{
			XEPL::XmlBuilder x2 ( "span", "class='trait'", rendition );
			{
				XEPL::XmlBuilder x3 ( "span", "class='traitName'", rendition );
				rendition->append ( *name );
			}
			rendition->append ( " = " );
			{
				XEPL::XmlBuilder x3 ( "span", "class='traitValue'", rendition );
				rendition->append ( *term );
			}
		}
	}
}

void KITS::HTML::RendonHtml::Scribble_Content ( XEPL::Nucleus*, XEPL::Gene* _element )
{
	if ( _element )
	{
		if ( _element->Has_Content() )
		{
			XEPL::XmlBuilder x1 ( "span", "class='geneWire'", rendition );
			_element->Copy_Content(rendition);
		}
	}
}

void KITS::HTML::Scribble ( XEPL::Nucleus* _nucleus, XEPL::Gene* _gene, XEPL::Rendon* _rendon )
{
	XEPL::Gene* source = nullptr;
	{
		XEPL::Cord* gene_name = _gene->Trait_Raw ( "neuron" );
		XEPL::Cord* form_name = _gene->Trait_Raw ( "form" );
		if ( !gene_name || !form_name )
		{
			XEPL::ErrorReport error_report ( "Scribble: No #form #gene found ", gene_name );
			return;
		}

		XEPL::Neuron* neuron = XEPL::Cortex::Locate_Neuron ( _nucleus, gene_name, '/' );
		if ( !neuron )
		{
			XEPL::ErrorReport error_report( "Scribble: #gene not found: ", gene_name );
			return;
		}

		XEPL::TRACE( "Scribble", neuron, form_name );

		if ( neuron->Nucleus_Rendered ( _rendon, form_name ) )
			return;

		source = XEPL::Cortex::Locate_Gene( neuron, form_name );

		if ( !source )
		{
			XEPL::xeplCantFind ( "Form", neuron, form_name );
			return;
		}
	}

	_rendon->Rendon_Render ( _nucleus, source );
}

void KITS::HTML::Output( XEPL::Nucleus* _nucleus, XEPL::Gene* _gene, XEPL::Rendon* _rendon )
{
	XEPL::String content;
	if ( _gene->Copy_Content(&content) )
	{
		XEPL::Evaluate_Inner_Scripts(_nucleus, nullptr, &content, _rendon->rendition );
		_rendon->rendition->push_back('\n');
	}
}

void KITS::HTML::Keyword_Html ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Keyword ( "Html", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* )
	{
		XEPL::String* output = XEPL::tlsLobe->output_string;

		KITS::HTML::RendonHtml renderer ( _neuron, _call_gene, output );

		XEPL::String content;
		if ( _call_gene->Copy_Content(&content) )
			XEPL::Script ( _neuron, _call_gene, &content, output, nullptr, nullptr, true );

		renderer.Process_Inner_Genes ( _call_gene );
	} );
}

void KITS::HTML::Register_Html_Kit( XEPL::Cortex* _cortex )
{
	_cortex->Register_Render  ( "Scribble", &Scribble );
	_cortex->Register_Render  ( "Output",   &Output );

	Keyword_Html  ( _cortex );
}

