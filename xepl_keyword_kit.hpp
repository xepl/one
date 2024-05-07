// SPDX: AGPL-3.0-only
/* XEPL Operating Environment - Copyright (c) 2024 Keith Edwin Robbins
	Project Name: XEPL Operating Environment
	File Name:    xepl_keyword_kit.hpp
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

#include <algorithm>

namespace KITS::KEYWORDS
{
	void Keyword_Axons      ( XEPL::Cortex* _cortex );
	void Keyword_Forms      ( XEPL::Cortex* _cortex );
	void Keyword_Genes      ( XEPL::Cortex* _cortex );
	void Keyword_Lobe       ( XEPL::Cortex* _cortex );
	void Keyword_Macros     ( XEPL::Cortex* _cortex );
	void Keyword_Methods    ( XEPL::Cortex* _cortex );
	void Keyword_Neuron     ( XEPL::Cortex* _cortex );
	void Keyword_Properties ( XEPL::Cortex* _cortex );
	void Keyword_Synapses   ( XEPL::Cortex* _cortex );

	void Keyword_Index      ( XEPL::Cortex* _cortex );
	void Keyword_New        ( XEPL::Cortex* _cortex );
	void Keyword_Using      ( XEPL::Cortex* _cortex );

	void Keyword_Modify  ( XEPL::Cortex* _cortex );
	void Keyword_Print   ( XEPL::Cortex* _cortex );
	void Keyword_Trigger ( XEPL::Cortex* _cortex );

	void Keyword_ForEach ( XEPL::Cortex* _cortex );
	void Keyword_IfNo    ( XEPL::Cortex* _cortex );
	void Keyword_IfYes   ( XEPL::Cortex* _cortex );
	void Keyword_Repeat  ( XEPL::Cortex* _cortex );
	void Keyword_Run     ( XEPL::Cortex* _cortex );
	void Keyword_When    ( XEPL::Cortex* _cortex );

	void Register_Keyword_Kit( XEPL::Cortex* _cortex );
}



void KITS::KEYWORDS::Keyword_Axons ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Keyword ( "Axons", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* )
	{
		if (  !_call_gene->inner_genes )
			return;

		_neuron->shadows->Make_One("Axons")->Absorb_Gene(_call_gene);

		XEPL::Bond* bond = _call_gene->inner_genes->head_bond;

		while ( bond )
		{
			XEPL::Gene* bonded_gene   = static_cast<XEPL::Gene*> ( bond->atom );

			XEPL::Axon* axon = nullptr;
			XEPL::Cord* name_string = bonded_gene->cell_name;

			if ( !_neuron->Hunt_Axon ( name_string, &axon ) )
				axon = new XEPL::Axon( _neuron, name_string );

			if ( bonded_gene->inner_genes )
				axon->Synapse ( _neuron, bonded_gene );

			bond = bond->next_bond ;
		}
	} );
}

void KITS::KEYWORDS::Keyword_Forms ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Keyword ( "Forms", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* )
	{
		if ( !_call_gene->inner_genes )
			return;

		XEPL::Bond* bond = _call_gene->inner_genes->head_bond;

		while ( bond )
		{
			XEPL::Gene* gene =  static_cast<XEPL::Gene*> ( bond->atom );
			_neuron->Register_Form ( gene );
			bond=bond->next_bond;
		}
	} );
}

void KITS::KEYWORDS::Keyword_Genes ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Keyword ( "Genes", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* )
	{
		if ( !_call_gene->inner_genes )
			return;

		XEPL::Bond* bond = _call_gene->inner_genes->head_bond;

		while ( bond )
		{
			XEPL::Gene* gene =  static_cast<XEPL::Gene*> ( bond->atom );
			gene->Absorb_Traits(_call_gene);
			_neuron->Register_Gene ( gene->cell_name, gene );
			bond = bond->next_bond;
		}
	} );
}

void KITS::KEYWORDS::Keyword_Lobe ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Keyword ( "Lobe", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* )
	{
		static std::atomic_long autonum = 0;
		if ( auto raw_name = _call_gene->Trait_Raw("name") )
		{
			if ( raw_name->compare("auto") == 0)
			{
				XEPL::String name( "Lobe" );
				name.append(std::to_string(++autonum));
				_call_gene->Trait_Set("name", &name);
			}
		}
		XEPL::Lobe* lobe = new XEPL::Lobe ( _neuron, _call_gene );

		lobe->Start_Lobe ();
	} );
}

void KITS::KEYWORDS::Keyword_Macros ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Keyword ( "Macros", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* )
	{
		if ( !_call_gene->inner_genes )
			return;

		XEPL::Bond* bond = _call_gene->inner_genes->head_bond;
		while ( bond )
		{
			XEPL::Gene* gene =  static_cast<XEPL::Gene*> ( bond->atom );

			XEPL::String content;
			if ( gene->Copy_Content(&content) )
				_neuron->Register_Macro ( gene->cell_name, &content );
			
			bond = bond->next_bond;
		}
	} );
}

void KITS::KEYWORDS::Keyword_Methods ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Keyword ( "Methods", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* )
	{
		if ( !_call_gene->inner_genes )
			return;

		XEPL::Bond* bond = _call_gene->inner_genes->head_bond;

		while ( bond )
		{
			XEPL::Gene* gene =  static_cast<XEPL::Gene*> ( bond->atom );
			_neuron->Register_Method ( gene->cell_name, &XEPL::Nucleus::Method_Execute, gene );
			bond = bond->next_bond;
		}
	} );
}

void KITS::KEYWORDS::Keyword_Neuron ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Keyword ( "Neuron", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* )
	{
		static std::atomic_long autonum = 0;
		if ( auto raw_name = _call_gene->Trait_Raw("name") )
		{
			if ( raw_name->compare("auto") == 0 )
			{
				XEPL::String name( "Neuron" );
				name.append(std::to_string(++autonum));
				_call_gene->Trait_Set("name", &name);
			}
		}
		auto neuron = new XEPL::Neuron ( _neuron, _call_gene );
		neuron->shadows->Add_Gene(_call_gene);
		neuron->Process_Inner_Genes(_call_gene);
	} );
}

void KITS::KEYWORDS::Keyword_Properties ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Keyword ( "Properties", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* )
	{
		if ( !_call_gene->inner_genes )
			return;

		XEPL::Bond* bond = _call_gene->inner_genes->head_bond;

		while ( bond )
		{
			XEPL::Gene* gene =  static_cast<XEPL::Gene*> ( bond->atom );

			XEPL::String evaluated_string;

			XEPL::Script ( _neuron, gene, &evaluated_string );
			_neuron->Property_Set ( gene->cell_name, &evaluated_string );

			bond = bond->next_bond;
		}
	} );
}

void KITS::KEYWORDS::Keyword_Synapses ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Keyword ( "Synapses", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* )
	{
		if ( !_call_gene->inner_genes )
			return;

		_neuron->shadows->Make_One("Synapses")->Absorb_Gene(_call_gene);

		XEPL::Bond* bond = _call_gene->inner_genes->head_bond;

		while ( bond )
		{
			XEPL::Gene* gene =  static_cast<XEPL::Gene*> ( bond->atom );
			XEPL::Axon* axon = XEPL::Cortex::Locate_Axon ( _neuron, gene->cell_name, '.' );

			if ( axon )
				_neuron->Synapse_Axon ( axon, ( XEPL::Receiver )&XEPL::Neuron::Receive_Axon, gene );
			else
				XEPL::xeplCantFind ( "Axon", _neuron, gene->cell_name );
			bond = bond->next_bond;
		}
	} );
}


void KITS::KEYWORDS::Keyword_New ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Keyword ( "New", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* _param )
	{
		auto lobe = XEPL::tlsLobe;

		XEPL::Gene* located_gene = lobe->index_link->Make_One(_param->c_str());

		if ( auto template_name = _call_gene->Trait_Raw("template") )
		{
			XEPL::Gene* template_gene = XEPL::Cortex::Locate_Gene( _neuron, template_name );

			located_gene->Absorb_Gene(template_gene);
		}

		XEPL::ScopeIndex restore ( located_gene );

		_neuron->Process_Inner_Genes ( _call_gene );

		lobe->Set_Outdex ( located_gene );

	} );
}

void KITS::KEYWORDS::Keyword_Using ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Keyword ( "Using", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* _param )
	{
		XEPL::Gene* using_gene = nullptr;
		{
			if ( !_call_gene->inner_genes )
				return;

			using_gene = XEPL::Cortex::Locate_Gene ( _neuron, _param );
		}

		if ( !using_gene )
			return;

		XEPL::StableGenes stable_gene ( using_gene );

		XEPL::Gene* working_gene = nullptr;
		while ( stable_gene.Next_Gene ( &working_gene ) )
		{
			XEPL::ScopeIndex swap_index ( working_gene );
			_neuron->Process_Inner_Genes ( _call_gene );
		}
	} );
}


void KITS::KEYWORDS::Keyword_Modify ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Keyword ( "Mod", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* )
	{
		XEPL::String content;
		if ( _call_gene->Copy_Content(&content) )
			XEPL::Script ( _neuron, XEPL::tlsLobe->index_link, &content, nullptr );

		if ( _call_gene->inner_genes )
			_neuron->Process_Inner_Genes ( _call_gene );
	} );
}

void KITS::KEYWORDS::Keyword_Print ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Keyword ( "Print", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* _param )
	{
		{
			std::unique_lock lock_output ( XEPL::output_lock );

			if ( XEPL::Show_Trace )
				std::cout << *XEPL::tlsLobe->cell_name << ": ";

			std::cout << *_param << '\n';
			std::cout << std::flush;
		}
		if ( _call_gene->inner_genes )
			_neuron->Process_Inner_Genes ( _call_gene );
	} );
}

void KITS::KEYWORDS::Keyword_Trigger ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Keyword ( "Trigger", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* _param )
	{
		XEPL::Axon* axon = nullptr;

		if ( _param && _neuron->Hunt_Axon ( _param, &axon ) )
		{
			XEPL::String gene_name_string;

			if ( !_call_gene->Trait_Get( "gene", &gene_name_string ) )
			{
				axon->Trigger ( XEPL::tlsLobe->index_link );
				return;
			}

			XEPL::Gene* located_gene = XEPL::Cortex::Locate_Gene(_neuron, &gene_name_string);
			if ( located_gene )
			{
				axon->Trigger ( located_gene );
				return;
			}
		}
		XEPL::xeplCantFind ( _param ? _param->c_str() : "missing", _neuron, "Trigger: Can'f find #axon" );
	});
}

void KITS::KEYWORDS::Keyword_ForEach ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Keyword ( "ForEach", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* _param )
	{
		if ( !_call_gene->inner_genes )
			return;

		XEPL::String lhs(_param);
		XEPL::String rhs;

		while ( XEPL::Split_ch_lhs(&lhs, ':',&rhs ) )
		{
			XEPL::ShortTerms press( "for", &rhs );
			_neuron->Process_Inner_Genes ( _call_gene );
		}

		XEPL::ShortTerms press( "for", &lhs );
		_neuron->Process_Inner_Genes ( _call_gene );
	} );
}


void KITS::KEYWORDS::Keyword_IfNo ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Keyword ( "No", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* _param )
	{
		{
			bool truth = true;
			XEPL::Script ( _neuron, XEPL::tlsLobe->index_link, _param, nullptr, &truth, nullptr, false );

			if ( truth )
				return;
		}
		_neuron->Process_Inner_Genes ( _call_gene );
	} );
}

void KITS::KEYWORDS::Keyword_IfYes ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Keyword ( "Yes", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* _param )
	{
		{
			bool truth = true;
			XEPL::Script ( _neuron, XEPL::tlsLobe->index_link, _param, nullptr, &truth, nullptr, false );

			if ( !truth )
				return;
		}
		_neuron->Process_Inner_Genes ( _call_gene );
	} );
}

void KITS::KEYWORDS::Keyword_Index ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Keyword ( "Index", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* _param )
	{
		if ( auto located_gene = XEPL::Cortex::Locate_Gene(_neuron, _param) )
		{
			XEPL::ScopeIndex swap_index  ( located_gene );
			_neuron->Process_Inner_Genes ( _call_gene );
			return;
		}
		XEPL::ErrorReport error_report("Can't replace index with: ", _param );
	} );
}


void KITS::KEYWORDS::Keyword_Repeat ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Keyword ( "Repeat", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* _param )
	{
		long repeat_long = std::strtol ( _param->c_str(), nullptr, 0 );

		if ( repeat_long < 1 || !_call_gene->inner_genes )
			return;

		auto* lobe = XEPL::tlsLobe;
		while ( repeat_long-- && !lobe->Test_Flags( XEPL::lysing_flag | XEPL::closed_flag) )
			_neuron->Process_Inner_Genes ( _call_gene );
	} );
}

void KITS::KEYWORDS::Keyword_Run ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Keyword ( "Run", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* _param )
	{
		XEPL::String content;
		if ( _call_gene->Copy_Content(&content) )
		{
			if ( *content.c_str() && *content.c_str() != '{' )
				XEPL::Script ( _neuron, _call_gene, _param );
		}

		if ( _call_gene->inner_genes )
			_neuron->Process_Inner_Genes ( _call_gene );
	} );
}

void KITS::KEYWORDS::Keyword_When ( XEPL::Cortex* _cortex )
{
	_cortex->Register_Keyword ( "When", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* )
	{
		if ( _call_gene->Has_Content() )
		{
			XEPL::String chosen_string;

			XEPL::Script ( _neuron, _call_gene, &chosen_string );

			if ( _neuron->Process_Exact_Gene ( &chosen_string, _call_gene ) )
				return;

			_neuron->Process_Exact_Gene ( "NotFound", _call_gene );
		}
	} );
}

void KITS::KEYWORDS::Register_Keyword_Kit( XEPL::Cortex* _cortex )
{
	Keyword_Axons        ( _cortex );
	Keyword_Forms        ( _cortex );
	Keyword_Genes        ( _cortex );
	Keyword_Lobe         ( _cortex );
	Keyword_Neuron       ( _cortex );
	Keyword_Macros       ( _cortex );
	Keyword_Methods      ( _cortex );
	Keyword_Properties   ( _cortex );
	Keyword_Synapses     ( _cortex );

	Keyword_Print        ( _cortex );
	Keyword_Trigger      ( _cortex );
	
	Keyword_Index        ( _cortex );
	Keyword_Using        ( _cortex );

	Keyword_Modify       ( _cortex );
	Keyword_New          ( _cortex );

	Keyword_ForEach      ( _cortex );
	Keyword_IfNo         ( _cortex );
	Keyword_IfYes        ( _cortex );
	Keyword_When         ( _cortex );
	Keyword_Repeat       ( _cortex );
	Keyword_Run          ( _cortex );
}

