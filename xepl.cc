// SPDX License Identifier: DAZL-1.0
////////////////////////////////////////////////////////////////////////////////
// Copyright 2005, 2023 (c) Keith Edwin Robbins
// Licensed under the DAZL License (DAZL-1.0) (the License)
// You may not use this content except in compliance with the License
// Obtain the License at https://xepl.net/licenses/dazl-1.0.md
//
// DAZL Project: XEPL-1.0.0
//
// This Original Work is provided under this License on an **AS IS** BASIS
// and **WITHOUT WARRANTY**, either express or implied, including,
// without limitation, the warranties of non-infringement, merchantability
// or fitness for a particular purpose. THE ENTIRE RISK AS TO THE QUALITY OF
// THE ORIGINAL WORK IS WITH YOU. This DISCLAIMER OF WARRANTY constitutes an
// essential part of this License. No license to the Original Work is granted
// by this License except under this disclaimer.
//
// All contributions, notifications, and communications related to this project
// are to be directed to the designated repository on GitHub. This ensures a
// centralized location for the collection, review, and integration of
// contributions, while also maintaining a record of all project interactions
// and updates. Adherence to this process ensures the transparency and integrity
// of the project's evolution.
//
//+----------------------------Attribution Notice:-------------------------+
//|  o   o     o--o     o--o      o                  Original Work:        |
//|   \ /      |        |   |     |               Keith Edwin Robbins      |
//|    O       O-o      O--o      |     1.0.0     Project Repository:      |
//|   / \      |        |         |             https://github.com/xepl    |
//|  o   o     o--o     o         O---o           Notification Chain:      |
//|                                                   Pull Request         |
//+- visit: XEPL.Shop --- get: XEPL.Services --- join: XEPL.Tech ----------+
//
////////////////////////////////////////////////////////////////////////////////
#include "xepl.h"
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <netdb.h>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <termios.h>
#include <unordered_map>
#include <vector>


XEPL::SIGNAL::Action::~Action ( void )
{
	if ( receptor && --receptor->retain_count < 1 )
		receptor->Destroy();

	if ( trigger && --trigger->retain_count < 1 )
		trigger->Destroy();

	if ( tlsLobe->locals )
	{
		tlsLobe->locals->Release();
		tlsLobe->locals = nullptr;
	}
}

XEPL::SIGNAL::Action::Action ( SIGNAL::Receptor* _receptor, ATOM::Atom* _trigger )
	: receptor ( _receptor )
	, trigger                    ( _trigger )
	, action_next                ( nullptr )
{
	THREAD_COUNTER ( count_actions );

	if ( receptor )
		++receptor->retain_count;

	if ( trigger )
		++trigger->retain_count;
}


XEPL::SIGNAL::ActionList::~ActionList ( void )
{
	delete actions_lock;
}

XEPL::SIGNAL::ActionList::ActionList ( CORTEX::Lobe* _lobe )
	: actions_lock      ( new THREAD::Mutex() )
	, head              ( nullptr )
	, tail              ( nullptr )
	, tail_actions      ( nullptr )
	, tail_action_last  ( nullptr )
	, target_lobe       ( _lobe )
	, closed            ( false )
{}


void XEPL::SIGNAL::ActionList::Close()
{
	THREAD::MutexScope lock_actions ( actions_lock );
	closed = true;
}


void XEPL::SIGNAL::ActionList::Flush()
{
	while ( head )
	{
		SIGNAL::Action* action = head;
		head = head->action_next;

		if ( !head )
			tail = nullptr;

		delete action;
	}

	while ( tail_actions )
	{
		head = tail_actions->action_next;

		delete tail_actions;
		tail_actions = head;
	}

	tail_action_last = nullptr;
}


bool XEPL::SIGNAL::ActionList::Get_Next_Action ( SIGNAL::Action** __action )
{
	THREAD::MutexScope lock_actions ( actions_lock );

	if ( closed )
	{
		Flush();
		return false;
	}

	if ( head )
	{
		*__action = head;
		head = head->action_next;

		if ( !head )
			tail = nullptr;

		return true;
	}

	if ( !tail_actions )
		return true;

	*__action = tail_actions;

	tail_actions = tail_actions->action_next;

	if ( !tail_actions )
		tail_action_last = nullptr;

	return true;
}


// Place the _action at the head of the list
void XEPL::SIGNAL::ActionList::Press_Before_all ( SIGNAL::Action* _action )
{
	THREAD::MutexScope lock_actions ( actions_lock );

	if ( !closed )
	{
		_action->action_next = head;

		if ( !tail )
			tail = _action;

		head = _action;

		target_lobe->Lobe__Wake_Up();
	}
	else
		delete _action;
}


// Deliver the Action only once
void XEPL::SIGNAL::ActionList::Pulse_Only_Once ( SIGNAL::Action* _action )
{
	THREAD::MutexScope lock_actions ( actions_lock );

	if ( !closed && !_action->Already_Pending() )
	{
		_action->action_next = nullptr;

		if ( tail )
			tail->action_next = _action;
		else
		{
			head = _action;
			target_lobe->Lobe__Wake_Up();
		}

		tail = _action;
	}
	else
		delete _action;
}


// Just add the _action tot he list
void XEPL::SIGNAL::ActionList::Repeat_Multiple ( SIGNAL::Action* _action )
{
	THREAD::MutexScope lock_actions ( actions_lock );

	if ( closed )
	{
		delete _action;
		return;
	}

	if ( tail )
		tail->action_next = _action;
	else
	{
		head = _action;
		target_lobe->Lobe__Wake_Up();
	}

	tail = _action;
}


void XEPL::SIGNAL::ActionList::Tail_After ( SIGNAL::Action* _action )
{
	THREAD::MutexScope lock_actions ( actions_lock );

	if ( closed || _action->Already_Pending() )
	{
		delete _action;
		return;
	}

	if ( !tail_actions )
		tail_actions = _action;
	else
		tail_action_last->action_next = _action;

	tail_action_last = _action;

	target_lobe->Lobe__Wake_Up();
}


// Check if and Mark if this Action is in the List
bool XEPL::SIGNAL::Action::Already_Pending ( void )
{
	if ( !receptor )
		return false;

	if ( receptor->active_action )
		return true;

	receptor->active_action = this;

	return false;
}


void XEPL::SIGNAL::Action::Clear_Pending()
{
	if ( receptor )
		receptor->active_action = nullptr;
}


XEPL::SIGNAL::Action__Drop::Action__Drop ( CORTEX::Neuron* _neuron )
	: Action ( nullptr, nullptr )
	, neuron ( _neuron )
{
	if ( neuron )
		++neuron->retain_count;
}


void XEPL::SIGNAL::Action__Drop::Action__Execute ( void )
{
	if ( neuron )
	{
		ATOM::Scope_Release detach ( neuron );

		neuron->Nucleus__Dropped();
	}
}


XEPL::SIGNAL::Action__Signal::Action__Signal ( SIGNAL::Receptor* _receptor, ATOM::Atom* _trigger )
	: Action ( _receptor, _trigger )
{}


// Activate all the Receptor
void XEPL::SIGNAL::Action__Signal::Action__Execute ( void )
{
	receptor->Receptor__Activate ( trigger );

	Action::Clear_Pending();
}


XEPL::ATOM::Atom::Atom()
	: retain_count ( 1 )
{
	THREAD_COUNTER ( count_atoms )
}


void XEPL::ATOM::Atom::Destroy ( void )
{
	if ( retain_count != 0 )
		XEPL::SOMA::OffBalance();

	delete this;
}


void XEPL::ATOM::Atom::Release()
{
	const long retaining_count = --retain_count;

	if ( retaining_count > 0 )
		return;

	if ( retaining_count == 0 )
	{
		delete this;
		return;
	}

	XEPL::SOMA::OffBalance();
}


XEPL::SIGNAL::Axon::~Axon()
{
	delete receptor_chain;
	delete axon_name;
}

XEPL::SIGNAL::Axon::Axon ( CORTEX::Neuron* _owner, DNA::Gene* _config )
	: Atom::Atom()
	, axon_name         ( new TEXT::String( _config->Trait_Default ( "name", _config->cell_name->c_str() ) ) )
	, host_neuron       ( _owner )
	, receptor_chain    ( new SIGNAL::ReceptorChain() )
	, axon_type         ( _config->Trait_Get_bool ( "queue", true ) ? REPEATS : PULSE )
{
	host_neuron->Axon_Hold ( this );
}


XEPL::SIGNAL::Axon::Axon ( CORTEX::Neuron* _owner, const char* _name, AxonType _axon_type )
	: ATOM::Atom()
	, axon_name         ( new TEXT::String( _name ) )
	, host_neuron       ( _owner )
	, receptor_chain    ( new SIGNAL::ReceptorChain() )
	, axon_type         ( _axon_type )
{
	host_neuron->Axon_Hold ( this );
}

XEPL::SIGNAL::Axon::Axon ( CORTEX::Neuron* _owner, TEXT::Cord* _name, AxonType _axon_type )
	: ATOM::Atom()
	, axon_name         ( new TEXT::String( _name ) )
	, host_neuron       ( _owner )
	, receptor_chain    ( new SIGNAL::ReceptorChain() )
	, axon_type         ( _axon_type )
{
	host_neuron->Axon_Hold ( this );
}


XEPL::SIGNAL::AxonChain::~AxonChain ( )
{
	if ( head )
		Cancel_All_Receptors();
}

XEPL::SIGNAL::AxonChain::AxonChain ( void )
	: ATOM::Chain ( true )
{}


void XEPL::SIGNAL::AxonChain::Cancel_All_Receptors()
{
	SIGNAL::Axon* axon;

	while ( head && ( axon = static_cast<SIGNAL::Axon*> ( head->atom ) ) )
		axon->Cancel_Receptors();
}


XEPL::CORTEX::AxonLocator::~AxonLocator()
{
	delete axon_name;
}

XEPL::CORTEX::AxonLocator::AxonLocator ( CORTEX::Neuron* _neuron, TEXT::Cord* _cmd, char _sep1, char _sep2 )
	: owner_neuron     ( _neuron )
	, axon_name        ( new TEXT::String() )
	, axon             ( nullptr )
{
	if ( !_cmd )
		return;

	TEXT::String command_path;

	if ( _cmd->Split_ch_lhs_rhs ( _sep1, &command_path, axon_name ) )
	{
		owner_neuron = CORTEX::Neuron::Locator ( _neuron, &command_path, _sep2 );

		if ( owner_neuron )
			owner_neuron->Axon_Get ( axon_name, &axon );

		return;
	}

	owner_neuron->Axon_Hunt ( axon_name, &axon );
}


void XEPL::SIGNAL::Axon::Cancel_Receptors()
{
	receptor_chain->Detach_Receptors();
	host_neuron->Axon_Release ( this );
	Release();
}


void XEPL::SIGNAL::Axon::Subscribe ( CELL::Nucleus* _nucleus, DNA::Gene* _config )
{
	_nucleus->Nucleus__Host_Neuron()->Subscribe ( this, ( SIGNAL::Receiver )&CORTEX::Neuron::Axon_Synapse, _config );
}


// Deliver the _stimuli to the Receptor(s)
void XEPL::SIGNAL::Axon::Trigger ( ATOM::Atom* _stimuli ) const 
{
	if ( receptor_chain->head )
	{
		XEPL::TRACE( "Trigger", this->host_neuron, this->axon_name );
		receptor_chain->Deliver_Signal ( _stimuli );
	}
}


void XEPL::SIGNAL::Axon::Trigger_Wait ( ATOM::Atom* _stimuli ) const
{
	bool dissolved = false;
	SIGNAL::Signal__Rendezvous* rpc = new SIGNAL::Signal__Rendezvous ( _stimuli, &dissolved );

	std::unique_lock<std::mutex> lock ( *rpc->semaphore );
	{
		ATOM::Scope_Release detach ( rpc );
		receptor_chain->Deliver_Signal ( rpc );
	}

	if ( !dissolved )
		rpc->semaphore->std::condition_variable::wait ( lock );
}


XEPL::ATOM::Bond::Bond ( ATOM::Atom* _atom, ATOM::Bond* _left )
	: next   ( nullptr )
	, prev   ( _left )
	, atom   ( _atom )
{
	if ( atom )
		++atom->retain_count;
}

XEPL::ATOM::Bond::~Bond()
{
	if ( atom && --atom->retain_count < 1 )
		atom->Destroy();
}


XEPL::CELL::Cell::~Cell()
{
	delete cell_name;
}

XEPL::CELL::Cell::Cell ( TEXT::Cord* _name )
	: Atom()
	, cell_name    ( new TEXT::String ( _name ) )
{
	THREAD_COUNTER ( count_cells )
}

XEPL::CELL::Cell::Cell ( const char* _name )
	: Atom()
	, cell_name    ( new TEXT::String ( _name ) )
{
	THREAD_COUNTER ( count_cells )
}


XEPL::ATOM::Chain::~Chain()
{
	if ( chain_lock )
	{
		if ( head )
		{
			THREAD::MutexScope lock_chain ( chain_lock );

			ATOM::Bond* bond = head;

			while ( bond )
			{
				ATOM::Bond* next = bond->next;
				delete bond;
				bond = next;
			}
		}

		if ( is_my_lock )
			delete chain_lock;
	}
	else
	{
		ATOM::Bond* bond = head;

		while ( bond )
		{
			ATOM::Bond* next = bond->next;
			delete bond;
			bond = next;
		}
	}
}

XEPL::ATOM::Chain::Chain ( bool _locked )
	: is_my_lock ( _locked )
	, chain_lock ( nullptr )
	, head       ( nullptr )
	, tail       ( nullptr )
{
	if ( is_my_lock )
		chain_lock = new THREAD::Mutex();
}

XEPL::ATOM::Chain::Chain ( THREAD::Mutex* _mutex )
	: is_my_lock ( false )
	, chain_lock ( _mutex )
	, head       ( nullptr )
	, tail       ( nullptr )
{}


XEPL::ATOM::Bond* XEPL::ATOM::Chain::Add_Atom ( ATOM::Atom* _atom )
{
	LOCK_CHAIN

	ATOM::Bond* fresh_bond = new ATOM::Bond ( _atom, tail );

	if ( !head )
		head = fresh_bond;
	else
		tail->next = fresh_bond;

	tail = fresh_bond;

	return fresh_bond;
}


bool XEPL::ATOM::Chain::Pull_Atom ( ATOM::Atom** __atom )
{
	if ( ! head )
		return false;

	LOCK_CHAIN

	if ( head )
	{
		*__atom = head->atom;
		head->atom = nullptr;

		ATOM::Bond* return_node = head;
		head = return_node->next;

		delete return_node;

		if ( head )
			head->prev = nullptr;
		else
			tail = nullptr;

		return true;
	}

	return false;
}


bool XEPL::ATOM::Chain::Remove_Atom ( ATOM::Atom* _atom )
{
	LOCK_CHAIN

	ATOM::Bond* bond = head;

	while ( bond && bond->atom != _atom )
		bond=bond->next;

	if ( !bond )
		return !!head;

	if ( bond == head )
		head = bond->next;
	else
		bond->prev->next = bond->next;

	if ( bond == tail )
		tail = bond->prev;
	else
		bond->next->prev = bond->prev;

	delete bond;

	return !head;
}


void XEPL::ATOM::Chain::Remove_Bond ( ATOM::Bond* _bond )
{
	LOCK_CHAIN

	if ( _bond->prev )
		_bond->prev->next = _bond->next;
	else
		head = _bond->next;

	if ( _bond->next )
		_bond->next->prev = _bond->prev;
	else
		tail = _bond->prev;

	delete _bond;
}


bool XEPL::ATOM::Chain::Replicate_Atoms ( bool _locked, ATOM::Chain** _chain ) const
{
	if ( !head )
		return false;

	LOCK_CHAIN

	if ( !head )
		return false;

	ATOM::Chain* duplicate_chain = new ATOM::Chain ( _locked );
	ATOM::Bond* node = head;

	while ( node )
	{
		duplicate_chain->Add_Atom ( node->atom );
		node = node->next;
	}

	*_chain = duplicate_chain;

	return true;
}


XEPL::CORTEX::Cortex::~Cortex()
{
	alive = false;

	host_lobe->Lobe__Dying();

	if ( XEPL::Show_Final_Report )
	{
		TEXT::String report;
		SOMA::ErrorReport::finalReport ( &report );

		*output_duct << host_lobe->report_gene << std::flush;
		*output_duct << report << XEPL::EOL << std::flush;
	}

	SOMA::ErrorReport::Flush();

	delete keywords_map;
	delete operators_map;
	delete renkeys_map;
	delete render_map;
	delete splicer_map;
	delete short_terms;
	delete output_duct;
	delete chromos_map;

	system_gene->Release();
	host_lobe->Release();

	if ( XEPL::Show_Counters )
		final_counters.Final_Report();
}

XEPL::CORTEX::Cortex::Cortex ( const char* _name, std::ostream& _ostream )
	: report_memory_usage        ( &std::cout )
	, report_backpack_usage      ( &std::cout )
	, memory_backpack            ()
	, short_terms                ( nullptr )
	, chromos_map                ( new CORTEX::ChromosMap() )
	, keywords_map               ( new CORTEX::KeywordsMap() )
	, operators_map              ( new CORTEX::OperatorsMap() )
	, render_map                 ( new CORTEX::RenderMap() )
	, renkeys_map                ( new CORTEX::RenkeysMap() )
	, splicer_map                ( new CORTEX::SplicerMap() )
	, host_lobe                  ( new CORTEX::Lobe ( _name ) )
	, system_gene                ( new DNA::Gene ( nullptr, "System", nullptr ) )
	, output_duct                ( new ATOM::Duct ( _ostream, 2 ) )
	, final_counters             ()
{
	++host_lobe->retain_count;
	alive = true;
	tlsLobe = host_lobe;

	host_lobe->cortex = this;
	host_lobe->config_gene = new DNA::Gene ( nullptr, host_lobe->cell_name, nullptr );

	short_terms = new DNA::Scope_Terms();

	host_lobe->Lobe__Born();
}


bool XEPL::CORTEX::Cortex::Did_Operation ( TEXT::Cord* _name, TEXT::String* _seed, TEXT::String* _param, bool& _truth )
{
	auto it = operators_map->find ( *_name );

	if ( it == operators_map->end() )
		return false;

	THREAD_COUNTER ( count_operations );

	it->second ( _seed, _param, _truth );
	return true;
}


bool XEPL::CORTEX::Cortex::Did_Render ( TEXT::Cord* _name, CELL::Nucleus* _nucleus, DNA::Gene* _gene, XEPL::CORTEX::Rendron* _render )
{
	auto it  = render_map->find ( *_name );

	if ( it != render_map->end() )
	{
		Render render = it->second;

		if ( !_gene->traits )
		{
			render ( _nucleus, _gene, _render );
			return true;
		}

		DNA::Scope_Duplicate duplicate ( _gene );

		duplicate.gene->Traits_Evaluate ( _nucleus );

		render ( _nucleus, duplicate.gene, _render );

		return true;
	}

	if ( renkeys_map->find ( *_name ) != renkeys_map->end() )
		return  Executes_Keyword ( _name, _nucleus, _gene );

	return _nucleus->Nucleus__Host_Neuron()->Performed_Method( _name, _gene );
}


bool XEPL::CORTEX::Cortex::Did_Splice ( TEXT::Cord* _name, TEXT::String* _seed, CELL::Nucleus* _nucleus, DNA::Gene*& __gene, TEXT::String* _param, bool* _truth )
{
	auto it = splicer_map->find ( *_name );

	if ( it == splicer_map->end() )
		return false;

	THREAD_COUNTER ( count_splices );

	it->second ( _seed, _nucleus, __gene, _param, _truth );

	return true;
}


void XEPL::CORTEX::Cortex::Enable_Renkey ( const char* _name )
{
	if ( !_name )
		return;

	TEXT::String name ( _name );

	if ( keywords_map->find ( name ) != keywords_map->end() )
		renkeys_map->emplace ( name, true );
}


void XEPL::CORTEX::Cortex::Execute_Dna ( DNA::Gene* _gene )
{
	if ( !host_lobe->Process_Gene ( _gene ) )
		SOMA::ErrorReport ( "what is keyword: " )->append ( *_gene->cell_name );
}


bool XEPL::CORTEX::Cortex::Execute_Rna ( const char* _cmd )
{
	TEXT::String expr ( _cmd );
	return Execute_Rna( &expr );
}

bool XEPL::CORTEX::Cortex::Execute_Rna ( TEXT::Cord* _expr )
{
	TEXT::String script_result;

	bool truth=false;

	RNA::RnaScript ( host_lobe, tlsLobe->index, _expr, &script_result, &truth, nullptr );
	std::cout << ( truth ? '+' : '-' ) << script_result << XEPL::EOL << std::flush;

	return true;
}


bool XEPL::CORTEX::Cortex::Execute_Xml ( const char* _program )
{
	DNA::Gene* program = new DNA::Gene ( _program );
	ATOM::Scope_Release release ( program );

	if ( !program->cell_name )
		return false;

	program->Make_Gene_Safe ( true );

	DNA::Scope_Terms nesting ( program );

	if ( !host_lobe->Process_Gene ( program ) )
		SOMA::xeplCantFind ( "Cmd", host_lobe, program->cell_name->c_str() );

	return true;
}


bool XEPL::CORTEX::Cortex::Executes_Keyword ( TEXT::Cord* _name, CELL::Nucleus* _nucleus, DNA::Gene* _gene )
{
	auto it = keywords_map->find ( *_name );

	if ( it == keywords_map->end() )
		return false;

	THREAD_COUNTER ( count_keywords_executed )

	it->second ( _nucleus, _gene );

	return true;
}


void XEPL::CORTEX::Cortex::Register_Gene ( const char* _name, DNA::Chromosome _variable )
{
	TEXT::String name ( _name );
	return Register_Gene ( &name, _variable );
}

void XEPL::CORTEX::Cortex::Register_Gene ( TEXT::Cord* _name, DNA::Chromosome _variable )
{
	if ( !_name )
		return;

	chromos_map->emplace ( *_name, _variable );
}


void XEPL::CORTEX::Cortex::Register_Keyword ( const char* _name, CORTEX::Keyword _keyword )
{
	if ( !_name )
		return;

	TEXT::Cord name ( _name );

	keywords_map->emplace ( name, _keyword );
}


void XEPL::CORTEX::Cortex::Register_Operator ( const char* _name, CORTEX::Operator _operator )
{
	if ( !_name )
		return;

	TEXT::String name ( _name );

	operators_map->emplace ( name, _operator );
}


void XEPL::CORTEX::Cortex::Register_Render ( const char* _name, Render _renderer )
{
	TEXT::String name ( _name );
	return Register_Render ( &name, _renderer );
}

void XEPL::CORTEX::Cortex::Register_Render ( TEXT::Cord* _name, Render _renderer )
{
	if ( !_name )
		return;

	render_map->emplace ( *_name, _renderer );
}


void XEPL::CORTEX::Cortex::Register_Splicer ( const char* _name, DNA::Splicer _splicer )
{
	if ( !_name )
		return;

	TEXT::String name ( _name );

	splicer_map->emplace ( name, _splicer );
}


bool XEPL::CORTEX::Cortex::Cortex__Can_Execute_Cmd ( const char* bufSpot )
{
	switch ( *bufSpot )
	{
		case '\0' :
		case ';' :
			return true;

		case '<' :
			return Execute_Xml ( bufSpot );

		case '{' :
		case '!' :
		case '%' :
			return Execute_Rna ( bufSpot );
	}

	return false;
}


void XEPL::CORTEX::Cortex::Cortex__Initialize()
{}

void XEPL::CORTEX::Cortex::Cortex__Shutdown()
{
	host_lobe->healthy = false;
	host_lobe->Lobe__Wake_Up();
}


void XEPL::DNA::Ephemerals::Set ( XEPL::TEXT::Cord* _name, XEPL::DNA::Gene* _gene )
{
	auto it = find ( *_name );

	if ( it != end() )
	{
		auto its = it->second;
		its->Release();
	}

	++_gene->retain_count;
	(*this)[ *_name ] = _gene;
}


XEPL::ATOM::Duct::Duct ( std::ostream& _ostream, int _indent )
	: std::ostream ( _ostream.rdbuf() )
	, indent       ( _indent )
	, num_spaces   ( 0 )
{}

XEPL::ATOM::Duct& XEPL::ATOM::operator<< ( Duct& _lhs, Denter _rhs )
{
	_rhs ( _lhs );
	return _lhs;
}


namespace XEPL
{
	const char blanks[]="                                                    ";
}

void XEPL::ATOM::Duct::Indent ( Duct& _lhs )
{
	_lhs.num_spaces += _lhs.indent;
}

void XEPL::ATOM::Duct::Undent ( Duct& _lhs )
{
	_lhs.num_spaces -= _lhs.indent;
}

void XEPL::ATOM::Duct::Dent ( Duct& _lhs )
{
	if ( _lhs.indent )
		_lhs << XEPL::EOL;

	if ( _lhs.num_spaces <= 0 )
		return;

	int needed = _lhs.num_spaces;

	const int blanks_available  = sizeof ( XEPL::blanks )-1;
	while ( needed > blanks_available )
	{
		_lhs.write ( blanks, blanks_available );
		needed -= blanks_available;
	}

	_lhs.write ( XEPL::blanks, needed );
}


XEPL::TEXT::String* XEPL::SOMA::Escape_Quotes ( TEXT::Cord* _ptr, TEXT::String* _result )
{
	static const char* breakset    ="&<>\"";
	static const char* breakset_alt="&<>'";

	if ( !_ptr )
		return nullptr;

	if ( !_result )
		return nullptr;

	const char* start = _ptr->c_str();
	const char* pos = std::strpbrk ( start, breakset );

	if ( !pos )
	{
		_result->push_back (  '"'  );
		_result->append    ( *_ptr );
		_result->push_back (  '"'  );
		return _result;
	}

	if ( *pos == '"' )
	{
		const char* pos2 = std::strpbrk ( start, breakset_alt );

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
		const char* conv = nullptr;

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


void XEPL::SOMA::Escape_Quotes_Remove ( TEXT::Cord* _ptr, TEXT::String* _results )
{
	static
	const std::unordered_map<std::string, char> escape_map =
	{
		{"&lt;", '<'},
		{"&gt;", '>'},
		{"&amp;", '&'},
		{"&apos;", '\''},
		{"&quot;", '\"'}
	};

	TEXT::Cord&    input = *_ptr;
	TEXT::String& output = *_results;

	size_t offset = 0;

	while ( offset < input.length() )
	{
		if ( input[offset] == '&' )
		{
			bool replaced = false;

			for ( const auto& pair : escape_map )
			{
				const std::string& escape_sequence = pair.first;

				if ( input.compare ( offset, escape_sequence.length(), escape_sequence ) == 0 )
				{
					output.push_back ( pair.second );
					offset += escape_sequence.length();
					replaced = true;
					break;
				}
			}

			if ( !replaced )
			{
				output.push_back ( input[offset] );
				++offset;
			}
		}
		else
		{
			output.push_back ( input[offset] );
			++offset;
		}
	}
}


XEPL::DNA::Gene::~Gene()
{
	if ( !is_just_a_copy )
	{
		delete inner_genes;
		delete content_mutex;

		if ( content_wire && --content_wire->retain_count < 1 )
			content_wire->Destroy();
	}

	if ( watch_axon && --watch_axon->retain_count < 1 )
		watch_axon->Destroy();

	delete watch_expr;

	delete space_string;
	delete traits;
	delete traits_mutex;
}

XEPL::DNA::Gene::Gene ( const char* _xml )
	: CELL::Cell ( "")
	, content_wire               ( nullptr )
	, traits                     ( nullptr )
	, inner_genes                ( nullptr )
	, space_string               ( nullptr )
	, content_mutex              ( nullptr )
	, traits_mutex               ( nullptr )
	, gene_owner                 ( nullptr )
	, watch_axon                 ( nullptr )
	, watch_expr                 ( nullptr )
	, is_just_a_copy             ( false )
	, thread_safe                ( false )
{
	THREAD_COUNTER ( count_genes );

	TEXT::Cord text ( _xml );
	XEPL::XML::XmlParser parseit ( &text );

	if ( !parseit.error_string )
	{
		cell_name->assign( parseit.root_gene->cell_name->c_str() );

		this->Gene_Absorb ( parseit.root_gene );
	}
}

XEPL::DNA::Gene::Gene ( DNA::Gene* _parent, TEXT::Cord* _name, TEXT::Cord* _space )
	: CELL::Cell ( _name )
	, content_wire               ( nullptr )
	, traits                     ( nullptr )
	, inner_genes                ( nullptr )
	, space_string               ( _space ? new TEXT::String ( _space ) : nullptr )
	, content_mutex              ( nullptr )
	, traits_mutex               ( nullptr )
	, gene_owner                 ( _parent )
	, watch_axon                 ( nullptr )
	, watch_expr                 ( nullptr )
	, is_just_a_copy             ( false )
	, thread_safe                ( false )
{
	THREAD_COUNTER ( count_genes );

	if ( _parent )
		_parent->Add_Gene ( this );
}

XEPL::DNA::Gene::Gene ( DNA::Gene* _parent, const char* _name, TEXT::Cord* _space )
	: CELL::Cell ( _name )
	, content_wire               ( nullptr )
	, traits                     ( nullptr )
	, inner_genes                ( nullptr )
	, space_string               ( _space ? new TEXT::String ( _space ) : nullptr )
	, content_mutex              ( nullptr )
	, traits_mutex               ( nullptr )
	, gene_owner                 ( _parent )
	, watch_axon                 ( nullptr )
	, watch_expr                 ( nullptr )
	, is_just_a_copy             ( false )
	, thread_safe                ( false )
{
	THREAD_COUNTER ( count_genes );

	if ( _parent )
		_parent->Add_Gene ( this );
}

void XEPL::DNA::Gene::Watch( SIGNAL::Axon * _notify, TEXT::String* _expr )
{
	if ( watch_axon )
		watch_axon->Release();

	if ( watch_expr )
	{
		delete watch_expr;
		watch_expr = nullptr;
	}
	watch_axon = _notify;

	XEPL::TRACE( "Watch", tlsLobe->host_lobe, _expr );

	if ( _notify )
	{
		watch_axon->retain_count++;
		if ( _expr )
			watch_expr = new TEXT::String( _expr );
	}
}


XEPL::DNA::GeneChain::~GeneChain()
{
	if ( current_gene && --current_gene->retain_count )
		current_gene->Destroy();

	ATOM::Atom* atom;

	while ( Pull_Atom ( &atom ) )
		atom->Release();
}

XEPL::DNA::GeneChain::GeneChain ( const ATOM::Chain* _chain )
	: ATOM::Chain   ( false )
	, current_gene               ( nullptr )
{
	if ( !_chain )
		return;

	if ( !_chain->head )
		return;

	THREAD::MutexScope lock_chain ( _chain->chain_lock );

	ATOM::Bond* prev = nullptr;
	ATOM::Bond* bond = _chain->head;

	while ( bond )
	{
		ATOM::Bond* bond_clone = new ATOM::Bond ( bond->atom, prev );

		if ( prev )
			prev->next = bond_clone;
		else
			head = bond_clone;

		prev = bond_clone;
		bond = bond->next;
	}

	tail = prev;
}


bool XEPL::DNA::GeneChain::Next ( DNA::Gene** __gene )
{
	if ( current_gene && --current_gene->retain_count < 1 )
		current_gene->Destroy();

	ATOM::Atom* atom;

	if ( Pull_Atom ( &atom ) )
	{
		current_gene = static_cast<DNA::Gene*> ( atom );
		*__gene = current_gene;
		return true;
	}

	current_gene = nullptr;
	return false;
}


bool XEPL::SOMA::GeneLocator::Hunt ( CELL::Nucleus* _nucleus, TEXT::Cord* _name )
{
	// Neuron Variable
	auto* host = _nucleus;

	while ( host )
	{
		auto* chromosomes = host->chromo_gene;
		if ( chromosomes && chromosomes->inner_genes && ( chromosomes->Get_First_Gene ( _name, &gene ) ) )
			return true;

		if ( host == _nucleus->parent_neuron )
			break;

		host = _nucleus->parent_neuron;
	}

	// Cortex Chromosomes
	const auto& lobe = tlsLobe;
	{
		auto it  = lobe->cortex->chromos_map->find ( *_name );
		if ( it != lobe->cortex->chromos_map->end() )
		{
			gene = it->second ( _nucleus );
			return true;
		}
	}

	// Lobe Register
	if ( lobe->ephemerals )
	{
		auto it  = lobe->ephemerals->find ( *_name );
		if ( it != lobe->ephemerals->end() )
		{
			gene = it->second;
			return true;
		}
	}

	return false;
}


void XEPL::DNA::Gene::Gene_Absorb ( DNA::Gene* _gene )
{
	if ( !_gene )
		return;

	if ( _gene->inner_genes )
	{
		DNA::StableRecall recall ( _gene, true );

		if ( !thread_safe && !content_mutex )
		{
			THREAD::MutexResource resource ( &content_mutex );
		}

		LOCK_CONTENTS

		if ( !inner_genes )
			inner_genes = new DNA::Genes ();

		DNA::Gene* gene;

		while ( recall.Next_Gene ( &gene ) )
			inner_genes->Add_Gene ( gene );

		Content_Append ( _gene->content_wire );
	}
	else
	{
		LOCK_CONTENTS

		Content_Append ( _gene->content_wire );
	}

	if ( _gene->traits )
	{
		THREAD::MutexScope lockTheirAttributes ( _gene->traits_mutex );
		Traits_Absorb ( _gene );
	}
}


XEPL::TEXT::String* XEPL::DNA::Gene::Content ( void ) const
{
	LOCK_CONTENTS
	
	return content_wire ? content_wire->wire_string : nullptr;
}


bool XEPL::DNA::Gene::Content_Append ( TEXT::Wire* _string )
{
	if ( !_string )
		return false;

	if ( !thread_safe && !content_mutex )
	{
		THREAD::MutexResource resource ( &content_mutex );
	}

	LOCK_CONTENTS

	if ( !content_wire )
		content_wire = new TEXT::Wire ();

	return content_wire->Append ( _string );
}

bool XEPL::DNA::Gene::Content_Append ( TEXT::Cord* _string )
{
	if ( !_string )
		return false;

	if ( !thread_safe && !content_mutex )
	{
		THREAD::MutexResource resource ( &content_mutex );
	}

	LOCK_CONTENTS

	if ( !content_wire )
		content_wire = new TEXT::Wire ();

	return content_wire->Append ( _string );
}


void XEPL::DNA::Gene::Content_Assign ( TEXT::Cord* _text )
{
	Make_Content_Wire();
	LOCK_CONTENTS
	content_wire->Assign ( _text );
}


void XEPL::DNA::Gene::Content_Attach ( TEXT::Wire* _wire )
{
	if ( !thread_safe && !content_mutex )
	{
		THREAD::MutexResource resource( &content_mutex );
	}

	LOCK_CONTENTS

	if ( content_wire )
		content_wire->Release();

	content_wire = _wire;

	if ( content_wire )
		++content_wire->retain_count;
}


void XEPL::DNA::Gene::Content_Drop ( void )
{
	LOCK_CONTENTS

	if ( content_wire && --content_wire->retain_count < 1 )
		content_wire->Destroy();

	content_wire=nullptr;
}


XEPL::TEXT::String* XEPL::DNA::Gene::Make_Content_Wire ( void )
{
	if ( !thread_safe && !content_mutex )
	{
		THREAD::MutexResource resource( &content_mutex );
	}

	LOCK_CONTENTS

	if ( !content_wire )
		content_wire=new TEXT::Wire();

	return content_wire->wire_string;
}


void XEPL::DNA::Gene::Gene_Deflate ( void )
{
	if ( content_wire || inner_genes )
	{
		LOCK_CONTENTS

		delete inner_genes;
		inner_genes = nullptr;

		if ( content_wire && --content_wire->retain_count < 1 )
		{
			content_wire->Destroy();
			content_wire=nullptr;
		}
	}

	if ( traits )
	{
		LOCK_TRAITS

		delete traits;
		traits=nullptr;
	}

	delete content_mutex;
	content_mutex=nullptr;

	delete traits_mutex;
	traits_mutex=nullptr;
}


// Duplicatge this gene into __gene
void XEPL::DNA::Gene::Gene_Duplicate ( DNA::Gene** __gene )
{
	if ( !traits )
	{
		++this->retain_count;
		*__gene = this;
		return;
	}

	DNA::Gene* cloned = new Gene ( nullptr, cell_name, space_string );
	
	cloned->is_just_a_copy = true;
	cloned->gene_owner     = gene_owner ;
	cloned->content_wire   = content_wire;
	cloned->content_mutex  = content_mutex;
	cloned->inner_genes    = inner_genes;

	*__gene = cloned;

	if ( thread_safe )
	{
		traits->Duplicate_Into ( &cloned->traits );
		cloned->Make_Gene_Safe ( false );
		return;
	}

	cloned->traits_mutex = new THREAD::Mutex();

	LOCK_TRAITS
	traits->Duplicate_Into ( &cloned->traits );
}


void XEPL::DNA::Gene::Genes_Absorb ( DNA::Gene* _rhs )
{
	if ( !_rhs->inner_genes )
		return;

	if ( !thread_safe && !content_mutex )
	{
		THREAD::MutexResource resource ( &content_mutex );
	}

	DNA::StableRecall right ( _rhs, true );

	LOCK_CONTENTS

	if ( !inner_genes )
		inner_genes = new DNA::Genes();

	DNA::Gene*        gene;

	while ( right.Next_Gene ( &gene ) )
		inner_genes->Add_Gene ( gene );
}


void XEPL::DNA::Gene::Add_Gene ( DNA::Gene* _gene )
{
	if ( !_gene )
		return;

	if ( !thread_safe && !content_mutex )
	{
		THREAD::MutexResource resource ( &content_mutex );
	}

	LOCK_CONTENTS

	if ( !inner_genes )
		inner_genes = new DNA::Genes();

	inner_genes->Add_Gene ( _gene );
}


void XEPL::DNA::Gene::Copy_Genes_Into ( DNA::GeneChain** __chain ) const
{
	if ( !inner_genes )
	{
		__chain = nullptr;
		return;
	}

	LOCK_CONTENTS

	*__chain = new DNA::GeneChain ( inner_genes );
}


void XEPL::DNA::Gene::Get_Gene_Chain ( TEXT::Cord* _name, DNA::GeneChain** __chain ) const
{
	delete *__chain;
	*__chain=nullptr;

	if ( !inner_genes )
		return;

	LOCK_CONTENTS
	ATOM::Chain* atom_chain = nullptr;

	if ( !inner_genes->Clone_Tree ( _name, &atom_chain ) )
		return;

	*__chain = new DNA::GeneChain ( atom_chain );
	delete atom_chain;
}


bool XEPL::DNA::Gene::Get_First_Gene ( TEXT::Cord* _name, DNA::Gene** __gene ) const
{
	if ( !inner_genes )
		return false;

	LOCK_CONTENTS

	if ( !inner_genes )
		return false;

	return inner_genes->Find_Gene ( _name, __gene );
}

bool XEPL::DNA::Gene::Get_First_Gene ( const char* _name, DNA::Gene** __gene ) const
{
	if ( !inner_genes )
		return false;

	TEXT::String name ( _name );

	LOCK_CONTENTS

	if ( !inner_genes )
		return false;

	return inner_genes->Find_Gene ( &name, __gene );
}


void XEPL::DNA::Gene::Keep_Common_Genes ( DNA::Gene* _rhs )
{
	if ( !_rhs->inner_genes )
		return;

	if ( !thread_safe && !content_mutex )
	{
		THREAD::MutexResource resource ( &content_mutex );
	}

	DNA::StableRecall left ( this, true );

	LOCK_CONTENTS

	if ( !inner_genes )
		inner_genes = new DNA::Genes ();
	else
		inner_genes->Flush();

	DNA::Gene* candidate;

	while ( left.Next_Gene ( &candidate ) )
	{
		if ( _rhs->inner_genes->Has_Gene ( candidate ) )
			inner_genes->Add_Gene ( candidate );
	}
}


void XEPL::DNA::Gene::Make_New_Gene ( TEXT::Cord* _child, DNA::Gene** _gene )
{
	if ( !_child )
		*_gene = this;
	else
		-- ( *_gene = new Gene ( this, _child, nullptr ) )->retain_count;
}

void XEPL::DNA::Gene::Make_New_Gene ( const char* _child, DNA::Gene** _gene )
{
	if ( !_child )
		*_gene = this;
	else
		-- ( *_gene = new Gene ( this, _child, nullptr ) )->retain_count;
}


bool XEPL::DNA::Gene::Make_One_Gene ( const char* _name, DNA::Gene** _gene )
{
	if ( !_name )
	{
		*_gene = this;
		return false;
	}
	else
	{
		TEXT::String name ( _name );
		return Make_One_Gene ( &name, _gene );
	}
}

bool XEPL::DNA::Gene::Make_One_Gene ( TEXT::Cord* _into, DNA::Gene** __gene )
{
	if ( !_into )
	{
		*__gene = this;
		return false;
	}
	else
	{
		LOCK_CONTENTS

		if ( Get_First_Gene ( _into, __gene ) )
			return false;

		-- ( *__gene = new Gene ( this, _into, nullptr ) )->retain_count;

		return true;
	}
}


void XEPL::DNA::Gene::Remove_Common_Genes ( DNA::Gene* _rhs )
{
	if ( !_rhs->inner_genes )
		return;

	if ( !thread_safe && !content_mutex )
	{
		THREAD::MutexResource resource ( &content_mutex );

	}

	DNA::StableRecall left ( this, true );

	LOCK_CONTENTS
	
	if ( !inner_genes )
		inner_genes = new DNA::Genes();
	else
		inner_genes->Flush();

	DNA::Gene* candidate;

	while ( left.Next_Gene ( &candidate ) )
	{
		if ( ! _rhs->inner_genes->Has_Gene ( candidate ) )
			inner_genes->Add_Gene ( candidate );
	}
}


void XEPL::DNA::Gene::Remove_Gene ( DNA::Gene* _gene )
{
	if ( !inner_genes || !_gene  )
		return;

	LOCK_CONTENTS

	ATOM::Bond* bond;

	auto it  = inner_genes->bond_map->find ( _gene );
	if ( it == inner_genes->bond_map->end() )
		return;

	bond = it->second;

	inner_genes->Remove_Bond ( bond );

	if ( !inner_genes->bond_map )
	{
		delete inner_genes;
		inner_genes=nullptr;
	}
}


bool XEPL::DNA::Gene::Replace_Gene ( DNA::Gene* _gene )
{
	LOCK_CONTENTS

	DNA::Gene* gene = nullptr;

	if ( Get_First_Gene ( _gene->cell_name, &gene ) )
		Remove_Gene ( gene );

	Add_Gene ( _gene );

	return gene != nullptr;
}


void XEPL::DNA::Gene::Make_Gene_Safe ( bool _follow )
{
	if ( !thread_safe )
	{
		thread_safe = true;

		if ( !is_just_a_copy )
		{
			delete content_mutex;
			content_mutex=nullptr;
		}

		delete traits_mutex;
		traits_mutex=nullptr;
	}

	if ( _follow && inner_genes )
		inner_genes->Morph_Safe();
}

void XEPL::DNA::Gene::Make_Gene_UnSafe ( bool _follow )
{
	if ( thread_safe )
	{
		thread_safe = false;

		if ( !is_just_a_copy && ( inner_genes || content_wire ) )
			content_mutex = new THREAD::Mutex();

		if ( traits )
			traits_mutex = new THREAD::Mutex();
	}

	if ( _follow && inner_genes )
		inner_genes->Morph_Unsafe();
}


bool XEPL::DNA::Gene::Gene_Print_duct ( bool _inTag, ATOM::Duct& _lhs )
{
	if ( _inTag )
	{
		LOCK_TRAITS

		if ( traits )
			_lhs << traits;

		return ( inner_genes || ( content_wire && content_wire->Avail() > 0 ) );
	}


	if ( content_wire )
	{
		LOCK_CONTENTS

		if ( content_wire )
			_lhs << content_wire->wire_string->c_str();

	}

	if ( inner_genes )
	{
		_lhs << ATOM::Duct::Indent
		<< inner_genes
		<< ATOM::Duct::Undent
		<< ATOM::Duct::Dent;
	}

	return false;
}

XEPL::ATOM::Duct& XEPL::DNA::operator<< ( ATOM::Duct& _lhs, DNA::Gene* _rhs )
{
	if ( _rhs )
	{
		TEXT::Cord* gene_name =_rhs->cell_name;
		TEXT::Cord* gene_space=_rhs->space_string;

		_lhs << &ATOM::Duct::Dent;
		_lhs << '<';

		if ( gene_space )
			_lhs << *gene_space << ':';

		_lhs << *gene_name;

		if ( _rhs->Gene_Print_duct ( true, _lhs ) )
		{
			_lhs.put ( '>' );
			_rhs->Gene_Print_duct ( false, _lhs );
			_lhs.write ( "</", 2 );

			if ( gene_space )
			{
				_lhs << *gene_space;
				_lhs.put ( ':' );
			}

			_lhs << *gene_name;
			_lhs.put ( '>' );
		}
		else
			_lhs << "/>";
	}

	return _lhs;
}


void XEPL::DNA::Gene::Gene_Print_rope ( TEXT::String* _bag, int _depth ) const
{
	if ( !_bag )
		return;

	XML::XmlBuilder x1 ( cell_name, _bag, space_string );

	if ( traits )
	{
		LOCK_TRAITS
		traits->Print_Into_Bag ( _bag );
	}

	if ( content_wire )
	{
		x1.Close_Traits();

		LOCK_CONTENTS

		if ( ( content_wire && content_wire->Avail()>0 ) )
			content_wire->Print ( _bag );
	}

	if ( --_depth )
	{
		if ( inner_genes )
		{
			x1.Close_Traits();
			inner_genes->Print_Into_Bag ( _bag, _depth );
		}
	}
}


void XEPL::DNA::Gene::Gene_Seal_Up()
{
	if ( gene_owner )
	{
		if  ( inner_genes || traits || ( content_wire && content_wire->Avail()>0 )  )
		{
			gene_owner->Make_Content_Wire();
			XEPL::THREAD::MutexScope lock_theirs ( gene_owner->content_wire->wire_lock );
			Gene_Print_rope ( gene_owner->content_wire->wire_string );
		}

		Gene_Deflate();

		gene_owner->Remove_Gene ( this );
	}
}


const char* XEPL::DNA::Gene::Trait_Default ( const char* _name, const char* _defaultTo ) const
{
	if ( !traits )
		return _defaultTo;

	TEXT::String name ( _name );

	return Trait_Get ( &name, _defaultTo );
}

const char* XEPL::DNA::Gene::Trait_Default ( TEXT::Cord* _name, const char* _defaultTo ) const
{
	if ( !traits )
		return _defaultTo;

	return Trait_Get ( _name, _defaultTo );
}


const char* XEPL::DNA::Gene::Trait_Get ( TEXT::Cord* _name, const char* _defaultTo ) const
{
	if ( !traits )
		return nullptr;

	TEXT::Cord* term = nullptr;
	Trait_Get ( _name, &term );

	if ( !term )
		return _defaultTo;

	return term->c_str();
}

XEPL::TEXT::Cord* XEPL::DNA::Gene::Trait_Get ( const char* _name, TEXT::String* _into ) const
{
	if ( !traits )
		return nullptr;

	TEXT::String name ( _name );
	return Trait_Get ( &name, _into );
}

XEPL::TEXT::Cord* XEPL::DNA::Gene::Trait_Get ( TEXT::Cord* _name, TEXT::String* _into ) const
{
	if ( !traits )
		return nullptr;

	LOCK_TRAITS

	if ( !traits )
		return nullptr;

	auto it  = traits->map_of_traits->find ( *_name );
	if ( it == traits->map_of_traits->end() )
		return nullptr;

	_into->assign ( * it->second->trait_term );
	return _into;
}

bool XEPL::DNA::Gene::Trait_Get ( TEXT::Cord* _name, TEXT::Cord** __term ) const
{
	if ( !traits )
		return false;

	LOCK_TRAITS

	if ( !traits )
		return false;

	auto it  = traits->map_of_traits->find ( *_name );
	if ( it == traits->map_of_traits->end() )
		return false;

	*__term = it->second->trait_term;
	return true;
}


bool XEPL::DNA::Gene::Trait_Get_bool ( const char* _name, bool _defaultTo ) const
{
	if ( !traits )
		return _defaultTo;

	TEXT::String name ( _name );
	return Trait_Get_bool ( &name, _defaultTo );
}

bool XEPL::DNA::Gene::Trait_Get_bool ( TEXT::Cord* _name, bool _defaultTo ) const
{
	if ( !traits )
		return _defaultTo;

	LOCK_TRAITS

	if ( !traits )
		return _defaultTo;

	auto it  = traits->map_of_traits->find ( *_name );
	if ( it == traits->map_of_traits->end() )
		return _defaultTo;

	TEXT::String lowerit ( it->second->trait_term );

	std::transform ( lowerit.begin(), lowerit.end(), lowerit.begin(), ::tolower );

	return lowerit.compare ( "true" ) == 0;
}


long XEPL::DNA::Gene::Trait_Get_long ( const char* _name, long _defaultTo ) const
{
	TEXT::String name ( _name );
	return Trait_Get_long ( &name, _defaultTo );
}

long XEPL::DNA::Gene::Trait_Get_long ( TEXT::Cord* _name, long _defaultTo ) const
{
	if ( !traits )
		return _defaultTo;

	LOCK_TRAITS

	if ( !traits )
		return _defaultTo;

	auto it  = traits->map_of_traits->find ( *_name );
	if ( it == traits->map_of_traits->end() )
		return _defaultTo;

	return strtol ( it->second->trait_term->c_str(), nullptr, 10 );
}


XEPL::TEXT::String* XEPL::DNA::Gene::Trait_Raw ( TEXT::Cord* _name )
{
	if ( !traits )
		return nullptr;

	LOCK_TRAITS

	if ( !traits )
		return nullptr;

	auto it  = traits->map_of_traits->find ( *_name );
	if ( it != traits->map_of_traits->end() )
		return it->second->trait_term;

	return nullptr;
}


XEPL::TEXT::String* XEPL::DNA::Gene::Trait_Raw ( const char* _name )
{
	TEXT::Cord name ( _name );
	return Trait_Raw ( &name );
}


void XEPL::DNA::Gene::Trait_Set ( const char* _name, const char* _term )
{
	TEXT::String name ( _name );
	TEXT::String val ( _term );
	Trait_Set ( &name, &val );
}

void XEPL::DNA::Gene::Trait_Set ( const char* _name, TEXT::Cord* _term )
{
	TEXT::String name ( _name );
	Trait_Set ( &name, _term );
}

void XEPL::DNA::Gene::Trait_Set ( TEXT::Cord* _name, const char* _term )
{
	TEXT::String val ( _term );
	Trait_Set ( _name, &val );
}

void XEPL::DNA::Gene::Trait_Set ( TEXT::Cord* _name, TEXT::Cord* _term )
{
	if ( !thread_safe && !traits_mutex )
	{
		THREAD::MutexResource resource( &traits_mutex );
	}

	LOCK_TRAITS

	if ( !traits )
		traits=new DNA::Traits();

	traits->Set_Trait ( _name, _term );

	if ( watch_axon )
	{
		if ( watch_expr )
		{
			bool truth;

			RNA::Script_Truth(tlsLobe->host_lobe, this, watch_expr, &truth );
			
			if ( !truth )
				return;
		}

		XEPL::TRACE( "Express", tlsLobe->host_lobe, _term );

		tlsLobe->gene_actions->Add( watch_axon, this );
	}
}


XEPL::TEXT::String* XEPL::DNA::Gene::Trait_Tap ( const char* _name, const char* _default )
{
	if ( !thread_safe && !traits )
	{
		THREAD::MutexResource resource( &traits_mutex );
	}

	TEXT::Cord name ( _name );

	LOCK_TRAITS

	if ( !traits )
		traits=new DNA::Traits();

	auto it  = traits->map_of_traits->find ( name );
	if ( it == traits->map_of_traits->end() )
	{
		Trait_Set ( &name, _default );
		return Trait_Raw ( name.c_str() );
	}

	return it->second->trait_term;
}


void XEPL::DNA::Gene::Traits_Absorb ( DNA::Gene* _gene )
{
	if ( !_gene || !_gene->traits )
		return;

	if ( !thread_safe && !traits_mutex )
	{
		THREAD::MutexResource resource( &traits_mutex );
	}

	THREAD::MutexScope lock_their_Traits ( _gene->traits_mutex );

	LOCK_TRAITS

	if ( !traits )
		traits = new DNA::Traits();

	DNA::Trait* their_trait = _gene->traits->first_trait;

	while ( their_trait )
	{
		traits->first_trait = new DNA::Trait ( their_trait, traits->first_trait );

		(*traits->map_of_traits)[*their_trait->trait_name] = traits->first_trait;

		their_trait = their_trait->next_trait;
	}
}


bool XEPL::DNA::Gene::Traits_Duplicate ( DNA::Traits** __traits ) const
{
	if ( !traits )
		return false;

	LOCK_TRAITS

	if ( !traits )
		return false;

	traits->Duplicate_Into ( __traits );

	return true;
}


void XEPL::DNA::Gene::Traits_Evaluate ( CELL::Nucleus* _nucleus )
{
	if ( !traits )
		return;

	LOCK_TRAITS

	if ( !traits )
		return;

	traits->Evaluate ( this, _nucleus );
}


void XEPL::DNA::Gene::Traits_Release()
{
	if ( !traits )
		return;

	LOCK_TRAITS

	delete traits;
	traits=nullptr;
}


XEPL::DNA::Genes::~Genes()
{
	if ( !head )
		return;

	delete chain_map;
	delete bond_map;
}

XEPL::DNA::Genes::Genes ()
	: GeneChain                  (    this )
	, bond_map                   ( nullptr )
	, chain_map                  ( nullptr )
{}


void XEPL::DNA::Genes::Add_Gene ( DNA::Gene* _gene )
{
	if ( !_gene )
		return;

	if ( !bond_map )
	{
		bond_map  = new DNA::BondMap();
		chain_map = new DNA::ChainMap();
	}

	if ( bond_map->find ( _gene ) == bond_map->end() )
	{
		auto it  = chain_map->find ( *_gene->cell_name );

		if ( it == chain_map->end() )
		{
			ATOM::Chain* chain = new ATOM::Chain ( nullptr );
			chain_map->emplace ( _gene->cell_name, chain );
			chain->Add_Atom ( _gene );
		}
		else
			it->second->Add_Atom ( _gene );

		ATOM::Bond* bond = ATOM::Chain::Add_Atom ( _gene );
		bond_map->emplace ( _gene, bond );
	}
}


bool XEPL::DNA::Genes::Clone_Tree ( TEXT::Cord* _name, ATOM::Chain** __chain ) const
{
	if ( !_name )
		return false;

	const auto it = chain_map->find ( *_name );

	if ( it == chain_map->end() )
		return false;

	it->second->Replicate_Atoms ( false, __chain );

	return true;
}


bool XEPL::DNA::Genes::Find_Gene ( TEXT::Cord* _name, XEPL::DNA::Gene** __gene ) const
{
	if ( !chain_map )
		return false;

	auto it  = chain_map->find ( *_name );

	if ( it == chain_map->end() )
		return false;

	*__gene = static_cast<DNA::Gene*> ( it->second->head->atom );

	return true;
}


void XEPL::DNA::Genes::Flush ( void )
{
	delete chain_map;
	chain_map = nullptr;

	delete bond_map;
	bond_map = nullptr;
}


bool XEPL::DNA::Genes::Has_Gene ( DNA::Gene* _gene ) const
{
	return bond_map->find ( _gene ) != bond_map->end();
}


void XEPL::DNA::Genes::Morph_Safe ( void )
{
	delete chain_lock;
	chain_lock = nullptr;

	auto ptr = head;

	while ( ptr )
	{
		auto gene = ( DNA::Gene* )ptr->atom;
		gene->Make_Gene_Safe ( true );
		ptr = ptr->next;
	}
}


void XEPL::DNA::Genes::Morph_Unsafe ( void )
{
	if ( !chain_lock )
	{
		is_my_lock = true;
		chain_lock = new THREAD::Mutex();
	}

	DNA::GeneChain memos ( this );
	DNA::Gene* gene = nullptr;

	while ( memos.Next ( &gene ) )
		gene->Make_Gene_UnSafe ( true );
}


void XEPL::DNA::Genes::Print_Into_Bag ( TEXT::String* _bag, int _depth ) const
{
	if ( !_bag )
		return;

	DNA::GeneChain memos ( this );
	DNA::Gene* gene = nullptr;

	while ( memos.Next ( &gene ) )
		gene->Gene_Print_rope ( _bag, _depth );
}

XEPL::ATOM::Duct& XEPL::DNA::operator<< ( XEPL::ATOM::Duct& lhs, XEPL::DNA::Genes* rhs )
{
	if ( rhs )
	{
		DNA::GeneChain memos ( rhs );
		DNA::Gene* gene = nullptr;

		while ( memos.Next ( &gene ) )
			lhs << gene;
	}

	return lhs;
}


void XEPL::DNA::Genes::Remove_Bond ( ATOM::Bond* _bond )
{
	if ( !_bond )
		return;

	DNA::Gene* gene = static_cast<DNA::Gene*> ( _bond->atom );

	gene->gene_owner = nullptr;
	bond_map->erase ( _bond->atom );

	auto it  = chain_map->find ( *gene->cell_name );

	if ( it != chain_map->end() )
	{
		if ( it->second->Remove_Atom ( _bond->atom ) )
		{
			delete it->second;
			chain_map->erase ( it );
		}
	}

	GeneChain::Remove_Bond ( _bond );

	if ( bond_map->empty() )
		Flush();
}


XEPL::CORTEX::Lobe::~Lobe()
{
	if ( cortex )
		cortex->final_counters.Add ( &counters );

	if ( outdex )
		outdex->Release();

	if ( locals )
		locals->Release();

	if ( fileData )
		fileData->Release();

	delete gene_actions;
	delete ephemerals;
	delete pending_actions;
	delete semaphore_rest;
	delete cpp_thread;
}

XEPL::CORTEX::Lobe::Lobe ( const char* _name )
	: Neuron ( _name )
	, index                      ( nullptr )
	, outdex                     ( nullptr )
	, locals                     ( nullptr )
	, ephemerals                 ( nullptr )
	, current_stimulus           ( nullptr )
	, parent_lobe                ( nullptr )
	, cortex                     ( nullptr )
	, pending_actions            ( new SIGNAL::ActionList ( this ) )
	, semaphore_rest             ( new THREAD::Semaphore() )
	, semaphore_birth            ( nullptr )
	, semaphore_loaded           ( nullptr )
	, cpp_thread                 ( new THREAD::Thread ( this, semaphore_rest ) )
	, short_term_stack           ( nullptr )
	, renderer                   ( nullptr )
	, healthy                    ( true )
	, started                    ( false )
	, counters                   ()
	, fileData                   ( nullptr)
	, gene_actions               ( new GeneActions() )
{
	THREAD_COUNTER ( count_lobes )

	Register_Method ( "Terminate", ( XEPL::CELL::Function )&Lobe::Method_Terminate, nullptr );
}

XEPL::CORTEX::Lobe::Lobe ( CORTEX::Neuron* _parent, DNA::Gene* _config )
	: Neuron ( _parent, _config )
	, index                      ( nullptr )
	, outdex                     ( nullptr )
	, locals                     ( nullptr )
	, ephemerals                 ( nullptr )
	, current_stimulus           ( nullptr )
	, parent_lobe                ( tlsLobe )
	, cortex                     ( parent_lobe->cortex )
	, pending_actions            ( new SIGNAL::ActionList ( this ) )
	, semaphore_rest             ( new THREAD::Semaphore() )
	, semaphore_birth            ( nullptr )
	, semaphore_loaded           ( nullptr )
	, cpp_thread                 ( new THREAD::Thread ( this, semaphore_rest ) )
	, short_term_stack           ( nullptr )
	, renderer                   ( nullptr )
	, healthy                    ( true )
	, started                    ( false )
	, counters                   ()
	, fileData                   ( nullptr)
	, gene_actions               ( new GeneActions() )
{
	THREAD_COUNTER ( count_lobes )
}


// Grab the next action from the list, and execute it
bool XEPL::CORTEX::Lobe::Dispatch_Action ( void )
{
	if ( !healthy )
		return false;

	XEPL::MEMORY::Scope_Delete<SIGNAL::Action> action;

	healthy = pending_actions->Get_Next_Action ( &action.ptr );

	if ( action.ptr && healthy && XEPL::alive )
	{
		locals = nullptr;

		action.ptr->Action__Execute();

		gene_actions->Notify();

		if ( ephemerals )
		{
			delete ephemerals;
			ephemerals = nullptr;
		}

		return true;
	}

	return false;
}


XEPL::CORTEX::GeneAction::~GeneAction()
{
	axon->Release();
	gene->Release();
}

XEPL::CORTEX::GeneAction::GeneAction( SIGNAL::Axon* _axon, DNA::Gene* _gene)
	: axon ( _axon )
	, gene ( _gene )
{
	axon->retain_count++;
//	gene->retain_count++;
}


XEPL::CORTEX::GeneActions::~GeneActions()
{
	delete action_map;
	delete gene_vector;
}

XEPL::CORTEX::GeneActions::GeneActions()
	: action_map( new GeneActionMap() )
	, gene_vector( new GeneVector() )
	{}

void XEPL::CORTEX::GeneActions::Add( XEPL::SIGNAL::Axon* _watch, XEPL::DNA::Gene* _touched )
{
	DNA::Gene*  signal_gene;
	GeneAction* signal_action;

	auto it  = action_map->find ( _watch );
	if ( it == action_map->end() )
	{
		signal_gene   = new XEPL::DNA::Gene(nullptr, _watch->axon_name );
		signal_action = new GeneAction( _watch, signal_gene );
		action_map->emplace( _watch, signal_action );
	}
	else
	{
		signal_action = it->second;
		signal_gene = signal_action->gene;
	}

	gene_vector->push_back( signal_action);
	signal_gene->Add_Gene(_touched);
}

void XEPL::CORTEX::GeneActions::Notify()
{
	if ( action_map->empty() )
		return;

	auto it = gene_vector->begin();
	while ( it != gene_vector->end() )
	{
		GeneAction* gene_action = *it;
		gene_action->axon->Trigger(gene_action->gene);
		delete gene_action;
		++it;
	}
	action_map->clear();
	gene_vector->clear();
}


void XEPL::CORTEX::Lobe::Lobe__Born ( void )
{
	alias = new XEPL::TEXT::String ( "Lobe" );
	
	Make_Locals();

	config_gene->Make_Gene_Safe ( true );

	Register_Method ( "Terminate", ( CELL::Function )&Lobe::Method_Terminate, nullptr );

	if ( const char* fromFile = config_gene->Trait_Default ( "filename", nullptr ) )
	{
		XEPL::TEXT::Cord filename(fromFile);

		KITS::FILE::CodeFile fullname(&filename);

		if ( KITS::FILE::File_Load_Gene ( &fullname, &fileData ) )
		{
			fileData->gene_owner = nullptr;
			fileData->Make_Gene_Safe ( true );
			Process_Inner_Genes ( fileData );
		}
	}
	Register_Gene( "upload", fileData );

	ATOM::Scope_Release release ( locals );

	Process_Inner_Genes ( config_gene );

	locals = nullptr;
}


void XEPL::CORTEX::Lobe::Lobe__Dying ( void )
{
	Neuron::Nucleus__Dropped();
}


// Waiting for the ActionList to have a Signal
void XEPL::CORTEX::Lobe::Lobe__Rest_Now ( void )
{
	std::unique_lock<std::mutex> lock ( *cpp_thread->semaphore_rest );

	if ( pending_actions->head || pending_actions->closed )
		return;

	THREAD_COUNTER ( count_rests )

	cpp_thread->semaphore_rest->wait ( lock );
}


// Wake up the other lobe
void XEPL::CORTEX::Lobe::Lobe__Wake_Up ( void )
{
	THREAD_COUNTER ( count_wakes )
	semaphore_rest->Give();
}


// This is the heart, of the lobe ... so to say
void XEPL::CORTEX::Lobe::Main_Processing_Loop ( void )
{
	TEXT::String path;
	TEXT::String slash ( "/" );
	Nucleus__Path ( &path, &slash );
	SOMA::Lobe_Set_Thread_Name ( cpp_thread, path.c_str() );

	++this->retain_count;

	Make_Index();

	semaphore_birth->Give();

	Lobe__Born();

	if ( semaphore_loaded )
		semaphore_loaded->Give();

	while ( healthy )
	{
		while ( Dispatch_Action() )
		{}

		if ( healthy )
			Lobe__Rest_Now();
	}

	Lobe__Dying();

	if ( locals )
		locals->Release();

	index->Release();
}


void XEPL::CORTEX::Lobe::Make_Index ( void )
{
	if ( !index )
		index = new DNA::Gene ( nullptr, "Index", cell_name );
}


void XEPL::CORTEX::Lobe::Make_Locals ( void )
{
	if ( !locals )
		locals = new DNA::Gene ( nullptr, "Locals", nullptr );
}


void XEPL::CORTEX::Lobe::Method_Terminate NUCLEUS_METHOD
{
	Neuron__Dismiss();
}


void XEPL::CORTEX::Lobe::Neuron__Build_Relay ( SIGNAL::Axon* _axon, SIGNAL::Receptor* _receptor, SIGNAL::Relay** __relay )
{
	*__relay = new SIGNAL::Synapse ( this, _axon,
	                                  ( SIGNAL::Receiver )&Lobe::Relay_Nop, _receptor );
	_axon->receptor_chain->Add_Atom ( *__relay );
}


void XEPL::CORTEX::Lobe::Neuron__Dismiss ( void )
{
	if ( !dropped )
	{
		if ( parent_lobe )
			parent_lobe->pending_actions->Pulse_Only_Once ( new SIGNAL::Action__Drop ( this ) );
		else
		{
			Nucleus__Dropped();
			healthy = false;
		}
	}
	else
		SOMA::xeplError ( "Program", this, "Double Suicide" );
}


void XEPL::CORTEX::Lobe::Neuron__Drop_Relay ( SIGNAL::Relay* _relay )
{
	ATOM::Scope_Release detach ( _relay );
	_relay->host_axon->receptor_chain->Remove_Atom ( _relay );
}


void XEPL::CORTEX::Lobe::Nucleus__Dropped ( void )
{
	if ( dropped )
		return;

	if ( started )
		Stop_Lobe();
	else
		Neuron::Nucleus__Dropped();

	Release();
}


void XEPL::CORTEX::Lobe::Relay_Nop ( CELL::Cell*, SIGNAL::Relay* )
{
//	assert ( false );
}


void XEPL::CORTEX::Lobe::Set_Outdex ( DNA::Gene* _gene )
{
	if ( outdex )
		outdex->Release();

	outdex = _gene;

	if ( outdex )
		++outdex->retain_count;
}


void XEPL::CORTEX::Lobe::Start_Lobe ( bool _waitingForLoad )
{
	semaphore_birth  = new THREAD::Semaphore();

	if ( _waitingForLoad )
	{
		semaphore_loaded = new THREAD::Semaphore();
		std::unique_lock<std::mutex> lockLoaded ( *semaphore_loaded );
		{
			std::unique_lock<std::mutex> lockBirth ( *semaphore_birth );
			cpp_thread->Conception();
			semaphore_birth->wait ( lockBirth );
		}
		semaphore_loaded->wait ( lockLoaded );

		delete semaphore_loaded;
		semaphore_loaded=nullptr;
	}
	else
	{
		std::unique_lock<std::mutex> lockBirth ( *semaphore_birth );
		cpp_thread->Conception();
		semaphore_birth->wait ( lockBirth );
	}

	started = true;

	delete semaphore_birth;
	semaphore_birth=nullptr;
}

void XEPL::CORTEX::Lobe::Stop_Lobe ( void )
{
	pending_actions->Close();
	Lobe__Wake_Up();
	cpp_thread->Burial();
}


XEPL::SOMA::LocateIndex::LocateIndex ( XEPL::CELL::Nucleus* _nucleus, XEPL::TEXT::Cord* _name )
	: GeneLocator ( _nucleus, _name )
{
	// Default to index
	if ( !gene && !_name )
		gene = tlsLobe->index;
}


std::string* XEPL::SOMA::Long_Commafy ( long _number, std::string* result )
{
	std::ostringstream oss;
	oss << _number;

	const std::string numberStr = oss.str();
	const size_t length = numberStr.length();

	std::string commafiedStr;

	for ( size_t i = 0; i < length; ++i )
	{
		commafiedStr += numberStr[i];

		const size_t posFromRight = length - i - 1;

		if ( posFromRight % 3 == 0 && posFromRight != 0 )
			commafiedStr.push_back ( ',' );
	}

	result->assign ( commafiedStr );

	return result;
}


std::string* XEPL::SOMA::Long_In_Bytes ( long _bytes, std::string* _result )
{
	std::ostringstream oss;
	oss << std::fixed << std::setprecision ( 2 );

	if ( _bytes >= 1024 * 1024 * 1024 )
		oss << static_cast<double> ( _bytes ) / ( 1024 * 1024 * 1024 ) << " GB";

	else if ( _bytes >= 1024 * 1024 )
		oss << static_cast<double> ( _bytes ) / ( 1024 * 1024 ) << " MB";

	else if ( _bytes >= 1024 )
		oss << static_cast<double> ( _bytes ) / ( 1024 ) << " KB";

	else
		oss << _bytes;

	*_result = oss.str();

	return _result;
}


XEPL::MEMORY::MemoryBackpack::~MemoryBackpack()
{
	heap->~HeapOfPools();
	free ( heap );

	tlsHeap = nullptr;
}

XEPL::MEMORY::MemoryBackpack::MemoryBackpack()
	: heap ( nullptr )
{
	void* ptr = malloc ( sizeof ( HeapOfPools ) );
	heap = ( new ( ptr ) HeapOfPools() );

	tlsHeap = heap;
}


thread_local class XEPL::MEMORY::HeapOfPools* XEPL::MEMORY::tlsHeap = nullptr;

XEPL::MEMORY::HeapOfPools::~HeapOfPools()
{
	COUNTERS::finalCount_biggies += count_to_big_to_pool;

	if ( COUNTERS::finalCount_largest < largest_block_requested )
		COUNTERS::finalCount_largest = largest_block_requested; // may overwrite on race

	for ( long index = 1; index <= CONSTANTS::maxPoolIndex; ++index )
	{
		pools[index]->~PoolOfBlocks();
		free ( pools[index] );
	}
}

XEPL::MEMORY::HeapOfPools::HeapOfPools()
	: pools  ()
	, count_to_big_to_pool      ( 0 )
	, largest_block_requested   ( 0 )
{
	for ( long index = 1; index <= CONSTANTS::maxPoolIndex; ++index )
	{
		void* ptr = malloc ( sizeof ( PoolOfBlocks ) );
		pools[index] = ( new ( ptr ) PoolOfBlocks ( index ) );
	}

	pools[0] = nullptr;
}


XEPL::MEMORY::RecycleCounts::~RecycleCounts()
{
	RecycleCounts_Report ( std__ostream );
}

XEPL::MEMORY::RecycleCounts::RecycleCounts ( std::ostream* _ostream )
	: std__ostream ( _ostream )
{
	RecycleCounts_Reset();
}


int  XEPL::MEMORY::RecycleCounts_Report ( std::ostream* _report )
{
	unsigned long needed  = COUNTERS::finalCount_needed;
	unsigned long discard = COUNTERS::finalCount_discard;
	unsigned long cached  = COUNTERS::finalCount_cached;
	unsigned long holding = COUNTERS::finalCount_holding;
	unsigned long biggies = COUNTERS::finalCount_biggies;
	unsigned long largest = COUNTERS::finalCount_largest;

	long leaking = needed - discard - holding;

	const long width = 12;

	if ( _report && ( XEPL::Show_Memory_Counts || leaking ) )
	{
		*_report << "Sizes:   " << std::setw ( width+13 ) << 2*CONSTANTS::poolWidth << " ";

		for ( long index = 2; index <= CONSTANTS::maxPoolIndex; ++index )
			*_report
			        << std::setw ( width )
			        << ( index+1 )*CONSTANTS::poolWidth
			        << " ";

		RecycleCounts_Show_Entry ( _report, "Cached:   ",  cached, COUNTERS::cached,   width );
		RecycleCounts_Show_Entry ( _report, "Needed:   ",  needed, COUNTERS::needed,   width );
		RecycleCounts_Show_Entry ( _report, "Discards: ", discard, COUNTERS::discards, width );
		RecycleCounts_Show_Entry ( _report, "Holding:  ", holding, COUNTERS::holding,  width );

		std::string counts;
		*_report
		        << XEPL::EOL
		        << "Biggies:  " << *XEPL::SOMA::Long_Commafy ( biggies, &counts )
		        << " : "
		        << "Largest:  " << *XEPL::SOMA::Long_In_Bytes ( largest, &counts )
		        << XEPL::EOL << std::flush;
	}

	if ( leaking )
	{
		std::cerr
		        << " ***LEAKING allocations: " << leaking
		        << XEPL::EOL
		        << std::flush;
	}

	return ( int )leaking;
}


void XEPL::MEMORY::RecycleCounts_Reset ( void )
{
	COUNTERS::finalCount_needed  = 0;
	COUNTERS::finalCount_discard = 0;
	COUNTERS::finalCount_cached  = 0;
	COUNTERS::finalCount_holding = 0;
	COUNTERS::finalCount_biggies = 0;
	COUNTERS::finalCount_largest = 0;
}


void XEPL::MEMORY::RecycleCounts_Show_Entry ( std::ostream* _report, const char* _label, long _total, std::atomic_size_t* _array, int _width )
{
	long mega_sum = 0;

	std::string counts;

	*_report
	        << XEPL::EOL
	        << _label
	        << std::setw ( _width )
	        << *SOMA::Long_Commafy ( _total, &counts )
	        << " : ";

	for ( long index = 1; index <= CONSTANTS::maxPoolIndex; ++index )
	{
		*_report
		        << std::setw ( _width )
		        << *SOMA::Long_Commafy ( _array[index], &counts )
		        << " ";

		mega_sum += ( index+1 )*CONSTANTS::poolWidth * _array[index];
	}

	*_report << std::setw ( _width+3 ) << *SOMA::Long_In_Bytes ( mega_sum, &counts );
}


std::atomic_size_t XEPL::MEMORY::COUNTERS::needed[]  = {0};
std::atomic_size_t XEPL::MEMORY::COUNTERS::cached[]  = {0};
std::atomic_size_t XEPL::MEMORY::COUNTERS::discards[]= {0};
std::atomic_size_t XEPL::MEMORY::COUNTERS::holding[] = {0};
std::atomic_size_t XEPL::MEMORY::COUNTERS::finalCount_needed  ( 0 );
std::atomic_size_t XEPL::MEMORY::COUNTERS::finalCount_cached  ( 0 );
std::atomic_size_t XEPL::MEMORY::COUNTERS::finalCount_discard ( 0 );
std::atomic_size_t XEPL::MEMORY::COUNTERS::finalCount_holding ( 0 );
std::atomic_size_t XEPL::MEMORY::COUNTERS::finalCount_biggies ( 0 );
std::atomic_size_t XEPL::MEMORY::COUNTERS::finalCount_largest ( 0 );


void* XEPL::MEMORY::Recycler::operator new ( size_t size )
{
	if ( auto heap = tlsHeap )
		return heap->Get_Size ( size );

	return ::operator new ( size );
}

void XEPL::MEMORY::Recycler::operator delete ( void* p )
{
	if ( auto heap = tlsHeap )
		heap->Captures_ptr ( p );
	else
		::operator delete ( p );
}


XEPL::CELL::Method::Method ( CELL::Function _function, TEXT::Cord* _name, DNA::Gene* _method_gene )
	: cell_function  ( _function )
	, soft_method    ( nullptr )
{
	if ( _method_gene )
	{
		soft_method = new DNA::Gene ( nullptr, _name, _method_gene->space_string );
		soft_method->Gene_Absorb ( _method_gene );

		// All XML Genes are thread-safe
		soft_method->Make_Gene_Safe ( true );
	}
}

XEPL::CELL::Method::~Method()
{
	if ( soft_method )
		soft_method->Release();
}


// Duplicate the _call_gene, and execute the cell_function on the _target
void XEPL::CELL::Method::Perform ( CELL::Nucleus* _target, DNA::Gene* _call_gene )
{
	THREAD_COUNTER ( count_methods_performed )

	DNA::Scope_Duplicate duplicate ( soft_method );

	if ( _call_gene && _call_gene->content_wire )
	{
		RNA::RnaScript ( _target, duplicate.gene );
		duplicate.gene->Content_Drop();
	}

	( _target->*cell_function ) ( _call_gene, duplicate.gene );
}


XEPL::THREAD::MutexResource::MutexResource ( THREAD::Mutex** __mutex )
	: mutex_at ( nullptr )
{
	if ( !__mutex )
		return;

	mutex_at = __mutex;

	if ( !*mutex_at )
		Consolidate ( ( *mutex_at = new THREAD::Mutex() ) );
}

void XEPL::THREAD::MutexResource::Consolidate ( THREAD::Mutex* _candidate )
{
	if ( _candidate != *mutex_at )
		delete ( _candidate );
}


XEPL::CORTEX::Neuron::~Neuron()
{
	XEPL::TRACE( "Delete", parent_neuron, cell_name );

	delete neuron_chain;
	delete neuron_map;
	delete relay_map;
	delete axon_chain;
	delete axon_map;
	delete receptor_chain;
	delete alias;
}

XEPL::CORTEX::Neuron::Neuron ( const char* _name )
	: CELL::Cell                ( _name )
	, Nucleus                   ( nullptr, this )
	, host_lobe                 ( nullptr)
	, receptor_chain            ( nullptr )
	, axon_chain                ( nullptr )
	, axon_map                  ( nullptr )
	, relay_map                 ( nullptr )
	, neuron_map                ( nullptr )
	, neuron_chain              ( nullptr )
	, alias                     ( nullptr )
	, output                    ( nullptr )
	, dropped                   ( false )
	, process_now               ( false )
{
	THREAD_COUNTER ( count_neurons )

//	assert ( !tlsLobe );

	Nucleus::parent_neuron = nullptr;

	report_gene = new DNA::Gene ( nullptr, "Report", cell_name );
}

XEPL::CORTEX::Neuron::Neuron ( Neuron*  _parent, DNA::Gene* _config )
	: CELL::Cell                 ( _config->Trait_Default ( "name", _config->cell_name->c_str() ) )
	, Nucleus                    ( _config, this )
	, host_lobe                  ( XEPL::tlsLobe )
	, receptor_chain             ( nullptr )
	, axon_chain                 ( nullptr )
	, axon_map                   ( nullptr )
	, relay_map                  ( nullptr )
	, neuron_map                 ( nullptr )
	, neuron_chain               ( nullptr )
	, alias                      ( nullptr )
	, output                     ( nullptr )
	, dropped                    ( false )
	, process_now                ( _config->Trait_Get_bool ( "simple", false ) )
{
	THREAD_COUNTER ( count_neurons )

	Nucleus::parent_neuron = _parent;

	++_parent->retain_count;

	if ( _parent->neuron_map )
	{
		TEXT::Cord* name = _config->Trait_Raw ( "name" );

		if ( !name )
			name = _config->cell_name;

		_parent->Drop_Neuron ( name );
	}

	_parent->Register_Neuron ( this );

	report_gene = new DNA::Gene ( _parent->report_gene, "Report", cell_name );
}


bool XEPL::CORTEX::Neuron::Axon_Get ( TEXT::Cord* _name, SIGNAL::Axon** __axon )
{
	if ( !axon_map )
		return false;

	auto it  = axon_map->find ( *_name );

	if ( it == axon_map->end() )
		return false;

	*__axon = it->second;

	return true;
}


void XEPL::CORTEX::Neuron::Axon_Hold ( SIGNAL::Axon* _axon )
{
	if ( !axon_chain )
		axon_chain = new SIGNAL::AxonChain ();

	axon_chain->Add_Atom ( _axon );
	Axon_Register ( _axon );

	XEPL::TRACE( "New_Axon", this, _axon->axon_name );
}


bool XEPL::CORTEX::Neuron::Axon_Hunt ( const char* _name, SIGNAL::Axon** __axon )
{
	TEXT::String name ( _name );
	return Axon_Hunt ( &name, __axon );
}

bool XEPL::CORTEX::Neuron::Axon_Hunt ( TEXT::Cord* _name, SIGNAL::Axon** __axon )
{
	if ( Axon_Get ( _name, __axon ) )
		return __axon;

	if ( parent_neuron )
		return parent_neuron->Axon_Hunt ( _name, __axon );

	return false;
}


void XEPL::CORTEX::Neuron::Axon_Register ( SIGNAL::Axon* _axon )
{
	if ( !axon_map )
		axon_map = new CORTEX::AxonMap();

	auto it = axon_map->find( *_axon->axon_name);
	if ( it != axon_map->end() )
		axon_map->erase( it );

	axon_map->emplace( *_axon->axon_name, _axon );
}


void XEPL::CORTEX::Neuron::Axon_Release ( SIGNAL::Axon* _axon )
{
	Axon_Unregister ( _axon );
	axon_chain->Remove_Atom ( _axon );
}


// This Axon delivered a Signal
void XEPL::CORTEX::Neuron::Axon_Synapse SIGNAL_RECEIVER
{
	if ( !_memento )
		return;

	DNA::Scope_Index restore ( static_cast<DNA::Gene*> ( _signal ) );

	Process_Inner_Genes ( static_cast<DNA::Gene*> ( _memento ) );
}


void XEPL::CORTEX::Neuron::Axon_Unregister ( SIGNAL::Axon* _axon )
{
	if ( axon_map )
	{
		auto it  = axon_map->find ( *_axon->axon_name );

		if ( it != axon_map->end() )
		{
			axon_map->erase ( it );

			if ( axon_map->empty() )
			{
				delete axon_map;
				axon_map=nullptr;
			}
		}
	}
}


void XEPL::CORTEX::Neuron::Drop_My_Axons()
{
	if ( axon_chain )
	{
		if ( axon_chain->head )
			axon_chain->Cancel_All_Receptors();

		delete axon_chain;
		axon_chain=nullptr;
	}
}


void XEPL::CORTEX::Neuron::Drop_My_Neurons()
{
	if ( neuron_chain )
	{
		while ( Neuron* neuron = neuron_chain->Last() )
			neuron->Nucleus__Dropped();

		// got stuck here once, when a lobe excepted ... and didn't remove itself
	}
}


void XEPL::CORTEX::Neuron::Drop_My_Receptors()
{
	if ( receptor_chain )
	{
		receptor_chain->Detach_Receptors();

		delete receptor_chain;
		receptor_chain=nullptr;
	}
}


void XEPL::CORTEX::Neuron::Drop_Neuron ( TEXT::Cord* _name )
{
	auto it  = neuron_map->find ( *_name );

	if ( it != neuron_map->end() )
		return it->second->Nucleus__Dropped();
}


bool XEPL::CORTEX::Neuron::Find_Neuron ( TEXT::Cord* _name, CORTEX::Neuron** __neuron  )
{
	if ( _name->empty() )
	{
		*__neuron = XEPL::tlsLobe->cortex->host_lobe;
		return true;
	}

	if ( Hunt_Neuron ( _name, __neuron ) )
		return true;

	if ( _name->compare("this") == 0 )
	{
		*__neuron = this;
		return true;
	}

	if ( _name->compare("parent") == 0 )
	{
		*__neuron = parent_neuron;
		return true;
	}

	return false;
}

bool XEPL::CORTEX::Neuron::Find_Neuron ( const char* _name, CORTEX::Neuron** __neuron  )
{
	TEXT::String name ( _name );
	return  Find_Neuron ( &name, __neuron );
}


bool XEPL::CORTEX::Neuron::Get_Neuron ( TEXT::Cord* _name, CORTEX::Neuron** __neuron )
{
	if ( !neuron_map )
		return false;

	auto it  = neuron_map->find ( *_name );

	if ( it == neuron_map->end() )
		return false;

	*__neuron = it->second;

	return true;
}


bool XEPL::CORTEX::Neuron::Hunt_Neuron ( TEXT::Cord* _name, CORTEX::Neuron** __neuron )
{
	if ( Get_Neuron ( _name, __neuron ) )
		return true;

	if ( _name->compare( *cell_name ) == 0 )
	{
		*__neuron = this;
		return true;
	}

	if ( alias && ( *_name == *alias ) )
	{
		*__neuron = this;
		return true;
	}

	if ( parent_neuron )
		return parent_neuron->Hunt_Neuron ( _name, __neuron );

	return false;
}


XEPL::CORTEX::Neuron* XEPL::CORTEX::Neuron::Locator ( CELL::Nucleus* _nucleus, TEXT::String* _cmd, char _sep )
{
	CORTEX::Neuron* neuron = nullptr;

	size_t spot = _cmd->find ( _sep );

	if ( spot == _cmd->npos )
	{
		_nucleus->Nucleus__Host_Neuron()->Find_Neuron ( _cmd, &neuron );
		return neuron;
	}

	TEXT::String rhs;
	TEXT::String lhs;
	_cmd->Split_at_lhs_rhs ( spot+1, &lhs, &rhs );

	neuron = tlsLobe->cortex->host_lobe;

	while ( neuron && _cmd->Split_ch_lhs_rhs ( _sep, &lhs, &rhs ) )
	{
		if ( neuron->Find_Neuron ( &lhs, &neuron ) )
			_cmd = &rhs;
		else
			break;
	}

	if ( neuron )
		neuron->Find_Neuron ( &rhs, &neuron );

	return neuron;
}


void XEPL::CORTEX::Neuron::Method_Terminate NUCLEUS_METHOD
{
	Nucleus__Dropped();
}


void XEPL::CORTEX::Neuron::Neuron__Build_Relay ( SIGNAL::Axon* _axon, SIGNAL::Receptor* _receptor, SIGNAL::Relay** __relay )
{
	*__relay = new SIGNAL::Relay ( this, _axon, ( SIGNAL::Receiver )&Neuron::Undeliverable, _receptor );
	parent_neuron->Receptor_Build ( _axon, *__relay );
}


void XEPL::CORTEX::Neuron::Neuron__Dismiss()
{
	if ( !dropped )
		Nucleus__Dropped();
}


void XEPL::CORTEX::Neuron::Nucleus__Dropped()
{
	if ( dropped )
		return;

	dropped = true;

	Performed_Method ( "Finished", nullptr );

	Drop_My_Receptors();
	Drop_My_Neurons();
	Drop_My_Axons();

	if ( parent_neuron )
		parent_neuron->Unregister_Neuron ( this );

	if ( output )
	{
		report_gene->Make_Content_Wire();
		report_gene->content_wire->wire_string->append ( *output );
		delete output;
		output = nullptr;
	}

	Nucleus::Nucleus__Dropped();

	Release();
}


XEPL::CORTEX::Neuron* XEPL::CORTEX::Neuron::Nucleus__Host_Neuron ( void )
{
	return this;
}


void XEPL::CORTEX::Neuron::Nucleus__Path ( TEXT::String* _into, TEXT::Cord* _separator ) const
{
	if ( parent_neuron )
	{
		parent_neuron->Nucleus__Path ( _into, _separator );

		if ( !_into->empty() )
			_into->append ( *_separator );
	}

	_into->append ( *cell_name );
}


void XEPL::CORTEX::Neuron::Receptor_Build ( SIGNAL::Axon* _axon, SIGNAL::Receptor* _receptor )
{
	SIGNAL::Relay* relay  = nullptr;

	if ( !relay_map )
		relay_map = new CORTEX::RelayMap();

	const auto it = relay_map->find ( _axon );

	if ( it == relay_map->end() )
	{
		Neuron__Build_Relay ( _axon, _receptor, &relay );
		relay_map->emplace ( _axon, relay );
	}
	else
		relay = it->second;

	relay->receptor_chain->Add_Atom ( _receptor );
}


void XEPL::CORTEX::Neuron::Receptor_Detach ( SIGNAL::Receptor* _receptor )
{
	const auto it = relay_map->find ( _receptor->host_axon );
	SIGNAL::Relay* relay = it->second;

	if ( relay->receptor_chain->Remove_Atom ( _receptor ) )
	{
		relay_map->erase ( it );

		if ( relay_map->empty() )
		{
			delete relay_map;
			relay_map=nullptr;
		}

		Neuron__Drop_Relay ( relay );
	}

	receptor_chain->Remove_Atom ( _receptor );
}


void XEPL::CORTEX::Neuron::Register_Neuron ( Neuron* _neuron )
{
	if ( !neuron_chain )
	{
		neuron_chain = new CORTEX::NeuronChain();
		neuron_map   = new CORTEX::NeuronMap();
	}

	XEPL::TRACE( "New_Neuron", this, _neuron->cell_name );

	neuron_chain->Add_Atom ( _neuron );

	TEXT::Cord* name = _neuron->cell_name;

	auto it = neuron_map->find ( *name );

	if ( it != neuron_map->end() )
		it->second->Release();

	neuron_map->emplace ( *name, _neuron );
}


void XEPL::CORTEX::Neuron::Relay_Detach ( SIGNAL::Relay* _relay )
{
	const auto it = relay_map->find ( _relay->host_axon );

	if ( it == relay_map->end() )
		return;

	SIGNAL::Relay* relay = it->second;

	if ( relay )
	{
		ATOM::Scope_Release detach ( _relay );

		if ( relay->receptor_chain->Remove_Atom ( _relay ) )
		{
			relay_map->erase ( it );

			if ( relay_map->empty() )
			{
				delete relay_map;
				relay_map=nullptr;
			}

			Neuron__Drop_Relay ( relay );
		}
	}
}


int XEPL::CORTEX::Neuron::Show_Neurons( TEXT::String* _string )
{
	if ( !neuron_map )
		return 0;

	int count = 0;
	auto it = neuron_map->begin();
	while ( it != neuron_map->end() )
	{
		CORTEX::Neuron* inner_neuron = it->second;
		XML::XmlBuilder x1( inner_neuron->cell_name->c_str(), _string );

		if ( inner_neuron->neuron_map )
			count += inner_neuron->Show_Neurons(x1.Close_Traits());

		++count;
		++it;
	}
	return count;
}


void XEPL::CORTEX::Neuron::Subscribe ( SIGNAL::Axon* _axon, SIGNAL::Receiver _routine, CELL::Cell* _memento )
{
	XEPL::TRACE("Subscribe", this, _axon->axon_name );

	if ( !receptor_chain )
		receptor_chain = new SIGNAL::ReceptorChain();

	SIGNAL::Receptor* receptor = new SIGNAL::Receptor ( this, _axon, _routine, _memento );
	Receptor_Build ( _axon, receptor );
	receptor_chain->Add_Atom ( receptor );
}


void XEPL::CORTEX::Neuron::Undeliverable ( CELL::Cell*, SIGNAL::Relay* )
{
//	assert ( nullptr );
}


void XEPL::CORTEX::Neuron::Unregister_Neuron ( Neuron* _neuron )
{
	if ( neuron_chain )
	{
		const auto it = neuron_map->find ( *_neuron->cell_name );

		if ( it != neuron_map->end() )
		{
			neuron_map->erase ( it );
			neuron_chain->Remove_Atom ( _neuron );
		}
	}
}


void XEPL::CORTEX::Neuron::Neuron__Drop_Relay ( SIGNAL::Relay* _relay )
{
	parent_neuron->Relay_Detach ( _relay );
}


XEPL::CELL::Nucleus::~Nucleus()
{
	if ( parent_neuron )
		parent_neuron->Release();

	if ( config_gene )
		config_gene->Release();

	if ( report_gene )
		report_gene->Release();

	if ( properties )
		properties->Release();

	if ( forms_gene )
		forms_gene->Release();

	if ( macros_gene )
		macros_gene->Release();

	if ( methods_gene )
		methods_gene->Release();

	if ( chromo_gene )
		chromo_gene->Release();

	delete method_map;
}

XEPL::CELL::Nucleus::Nucleus ( DNA::Gene* _config, CELL::Cell* _host )
	: parent_neuron    ( nullptr )
	, host_cell        ( _host )
	, config_gene      ( _config )
	, report_gene      ( nullptr )
	, properties       ( nullptr )
	, chromo_gene      ( nullptr )
	, forms_gene       ( nullptr )
	, macros_gene      ( nullptr )
	, methods_gene     ( nullptr )
	, method_map       ( nullptr )
{
	if ( config_gene )
		++config_gene->retain_count;
}


// This is the XEPL Entry point for an XML method
void XEPL::CELL::Nucleus::Execute_Method_Method ( DNA::Gene* _call, DNA::Gene* _definition )
{
	DNA::Gene* src = nullptr;

	DNA::Scope_Terms nesting ( _definition );

	if ( _call && _call->traits )
	{
		nesting.Nest_Gene_Terms ( _call );

		XEPL::SOMA::LocateIndex source ( this, _call->space_string );
		src = source.gene;
	}
	else
		src = tlsLobe->index;

	if ( !_definition->inner_genes )
		return;

	DNA::Scope_Index reindex ( src );

	ATOM::Bond* next = _definition->inner_genes->head;
	bool executed;

	while ( next )
	{
		DNA::Gene* gene = static_cast<DNA::Gene*> ( next->atom );

		if ( gene->traits )
		{
			DNA::Scope_Duplicate duplicate ( gene );
			executed = Process_Gene ( duplicate.gene );
		}
		else
			executed = Process_Gene ( gene );

		if ( !executed )
			SOMA::xeplCantFind ( "Statement", this, gene->cell_name->c_str() );

		next = next->next;
	}
}


XEPL::TEXT::String* XEPL::CELL::Nucleus::Feature_Get ( TEXT::Cord* _as, TEXT::String* _into )
{
	static const XEPL::TEXT::String SEPERATOR ( "/" );
	
	if ( _as->compare ( "path" ) == 0 )
	{
		_into->append ( SEPERATOR );
		Nucleus__Path ( _into, &SEPERATOR );
		return _into;
	}

	if ( _as->compare( "neurons" ) == 0 )
	{
		Nucleus__Host_Neuron()->Show_Neurons( _into );
		return _into;
	}

	return nullptr;
}

XEPL::TEXT::String* XEPL::CELL::Nucleus::Feature_Get ( const char* _as, TEXT::String* _into )
{
	TEXT::String name ( _as );
	return Feature_Get ( &name, _into );
}


bool XEPL::CELL::Nucleus::Form_Get ( TEXT::Cord* _name, DNA::Gene** __gene )
{
	if ( !forms_gene )
		return false;

	return forms_gene->Get_First_Gene ( _name, __gene );
}


bool XEPL::CELL::Nucleus::Macro_Hunt ( TEXT::Cord* _name, TEXT::String* _into )
{
	if ( macros_gene )
	{
		DNA::Gene* gene = nullptr;

		if ( macros_gene->Get_First_Gene ( _name, &gene ) )
		{
			XEPL::THREAD::MutexScope lock_theirs ( gene->content_wire->wire_lock );
			_into->assign ( *gene->content_wire->wire_string );
			return true;
		}
	}

	if ( !parent_neuron )
		return false;

	return parent_neuron->Macro_Hunt ( _name, _into );
}


void XEPL::CELL::Nucleus::Make_Genes ( void )
{
	if ( !chromo_gene )
		chromo_gene = new DNA::Gene ( nullptr, "Genes", host_cell->cell_name );
}


void XEPL::CELL::Nucleus::Path ( TEXT::String* _into, const char* _ch ) const
{
	TEXT::String sep ( _ch );
	Nucleus__Path ( _into, &sep );
}


bool XEPL::CELL::Nucleus::Performed_Macro ( TEXT::Cord* _opcode, TEXT::String* _seed,  bool& _truth, XEPL::TEXT::String* _result )
{
	TEXT::String expr;

	if ( Macro_Hunt ( _opcode, &expr ) )
	{
		RNA::RnaScript ( this, nullptr, &expr, _result, &_truth, _seed );
		return true;
	}

	return false;
}


// Scope into the _call_genes traits, and then have the Method perform the Function
bool XEPL::CELL::Nucleus::Performed_Method ( TEXT::Cord* _name, DNA::Gene* _call_gene )
{
	if ( !method_map )
		return false;

	auto it  = method_map->find ( *_name );

	if ( it == method_map->end() )
		return false;

	XEPL::TRACE( "ENTR_Method", Nucleus__Host_Neuron(), _name );

	DNA::Scope_Terms nesting ( _call_gene );

	it->second->Perform ( this, _call_gene );

	XEPL::TRACE( "EXIT_Method", Nucleus__Host_Neuron(), _name );

	return true;
}

bool XEPL::CELL::Nucleus::Performed_Method ( const char* _name, DNA::Gene* _call_gene )
{
	TEXT::Cord name ( _name );
	return Performed_Method ( &name, _call_gene );
}


bool XEPL::CELL::Nucleus::Process_Inner_Gene ( const char* _name, DNA::Gene* _program )
{
	TEXT::Cord name ( _name );
	return Process_Inner_Gene ( &name, _program );
}

bool XEPL::CELL::Nucleus::Process_Inner_Gene ( TEXT::Cord* _name, DNA::Gene* _program )
{
	if ( !_program )
		return false;

	DNA::Gene* dna_gene = nullptr;

	if ( !_program->Get_First_Gene ( _name, &dna_gene ) )
		return false;

	if ( dna_gene->content_wire )
		RNA::RnaScript ( this, dna_gene );

	Process_Inner_Genes ( dna_gene );

	return true;
}


// Hunt for the processor of the _call_gene, and invoke the tag
bool XEPL::CELL::Nucleus::Process_Gene ( DNA::Gene* _call_gene )
{
	if ( !_call_gene )
		return false;

	TEXT::String* name = _call_gene->cell_name;

	if ( _call_gene->traits )
		_call_gene->Traits_Evaluate ( this );

	auto& lobe = tlsLobe;

	if ( lobe->cortex->Executes_Keyword ( name, this, _call_gene ) )
		return true;

	if ( Nucleus::Performed_Method ( _call_gene->cell_name, _call_gene ) )
		return true;

	if ( lobe->cortex->Cortex__Can_Process_Tag ( name, this, _call_gene ) )
		return true;

	if ( lobe->renderer && this != lobe->renderer )
		return lobe->renderer->Nucleus__Processed_Gene ( this, _call_gene );

	return false;
}


bool XEPL::CELL::Nucleus::Property_Get ( TEXT::Cord* _as, TEXT::String* _into )
{
	if ( properties && properties->Trait_Get ( _as, _into ) )
		return true;

	return false;
}

bool XEPL::CELL::Nucleus::Property_Get ( const char* _as, TEXT::String* _into )
{
	TEXT::String name ( _as );
	return Property_Get ( &name, _into );
}


bool XEPL::CELL::Nucleus::Property_Hunt ( TEXT::Cord* _as, TEXT::String* _into )
{
	if ( properties && properties->Trait_Get ( _as, _into ) )
		return true;

	if ( parent_neuron )
		return parent_neuron->Property_Hunt ( _as, _into );

	return false;
}


void XEPL::CELL::Nucleus::Property_Set ( TEXT::Cord* _name, TEXT::Cord* _val )
{
	if ( !properties )
	{
		properties = new DNA::Gene ( nullptr, "Properties", host_cell->cell_name );
		properties->Make_Gene_Safe ( false );
	}

	properties->Trait_Set ( _name, _val );
}

void XEPL::CELL::Nucleus::Property_Set ( const char* _name, const char* _val )
{
	TEXT::Cord name ( _name );
	TEXT::Cord val ( _val );

	return Property_Set ( &name, &val );
}

void XEPL::CELL::Nucleus::Property_Set ( const char* _name, TEXT::Cord* _val )
{
	TEXT::Cord name ( _name );
	return Property_Set ( &name, _val );
}


void XEPL::CELL::Nucleus::Register_Form ( DNA::Gene* _form )
{
	if ( !_form )
		return;

	const char* name = _form->Trait_Default ( "name", nullptr );

	if ( !name )
		name = _form->cell_name->c_str();

	if ( !forms_gene )
		forms_gene = new DNA::Gene ( nullptr, "Forms", host_cell->cell_name );

	DNA::Gene* gene = nullptr;
	forms_gene->Make_One_Gene ( name, &gene );
	gene->Gene_Deflate();
	gene->Gene_Absorb ( _form );
}


void XEPL::CELL::Nucleus::Register_Gene ( const char* _name, DNA::Gene* _memento )
{
	TEXT::Cord name ( _name );
	return Register_Gene ( &name, _memento );
}

void XEPL::CELL::Nucleus::Register_Gene ( TEXT::Cord* _name, DNA::Gene* _memento )
{
	Make_Genes();

	DNA::Gene* var = nullptr;

	if ( !chromo_gene->Make_One_Gene ( _name, &var ) )
		var->Gene_Deflate();

	var->Gene_Absorb ( _memento );
	var->Make_Gene_UnSafe ( true );

	XEPL::TRACE( "Name_Gene", Nucleus__Host_Neuron(), _name );

	if ( !_memento || !_memento->traits )
		return;

	TEXT::String* axon_info = _memento->Trait_Raw( "watch" );

	if ( !axon_info )
		return;

	TEXT::String* expr = nullptr;
	TEXT::String axon_name( axon_info );
	TEXT::String axon_expr;

	if ( axon_name.Split_ch_rhs(':', &axon_expr ) )
		expr = &axon_expr;

	SIGNAL::Axon* axon;
	if ( !Nucleus__Host_Neuron()->Axon_Hunt( &axon_name, &axon ) )
		return;

	var->Watch(axon, expr);
}


void XEPL::CELL::Nucleus::Register_Macro ( TEXT::Cord* _name, TEXT::String* _macro )
{
	if ( !_name )
		return;

	if ( !macros_gene )
		macros_gene = new DNA::Gene ( nullptr, "Macros",  host_cell->cell_name );

	DNA::Gene* gene = nullptr;
	macros_gene->Make_One_Gene ( _name, &gene );
	gene->Make_Gene_Safe ( true );
	gene->Content_Assign ( _macro );
}

void XEPL::CELL::Nucleus::Register_Macro ( const char* _name, TEXT::String* _macro )
{
	TEXT::Cord name ( _name );
	return Register_Macro ( &name, _macro );
}


void XEPL::CELL::Nucleus::Register_Method ( TEXT::Cord* _name, CELL::Function _function, DNA::Gene* _gene )
{
	if ( !_name )
		return;

	if ( !method_map )
		method_map = new CELL::MethodMap();

	auto it  = method_map->find ( *_name );

	if ( it != method_map->end() )
	{
		method_map->erase ( it );
		delete it->second;
	}

	CELL::Method* method = new CELL::Method ( _function, _name, _gene );

	method_map->emplace ( *_name, method );

	XEPL::TRACE( "New_Method", Nucleus__Host_Neuron(), _name );

	if ( method->soft_method )
	{
		if ( !methods_gene )
			methods_gene = new DNA::Gene ( nullptr, "Methods", host_cell->cell_name );

		methods_gene->Replace_Gene ( method->soft_method );
	}
}

void XEPL::CELL::Nucleus::Register_Method ( const char* _name, CELL::Function _function, DNA::Gene* _gene )
{
	TEXT::String name ( _name );
	return Register_Method ( &name, _function, _gene );
}


// Enter the _program traits into short_term and then process all its tags
void XEPL::CELL::Nucleus::Process_Inner_Genes ( DNA::Gene* _program )
{
	if ( !_program )
		return;

	if ( !_program->inner_genes )
		return;

	DNA::Scope_Terms nesting ( _program );

	ATOM::Bond*  bond = _program->inner_genes->head;

	while ( bond )
	{
		bool    executed = false;
		DNA::Gene*  gene = static_cast<DNA::Gene*> ( bond->atom );

		if ( gene->traits )
		{
			DNA::Scope_Duplicate duplicate ( gene );
			executed = Process_Gene ( duplicate.gene );
		}
		else
			executed = Process_Gene ( gene );

		if ( !executed )
			SOMA::xeplCantFind ( "Statement", this, gene->cell_name->c_str() );

		bond = bond->next;
	}
}


void XEPL::CELL::Nucleus::Nucleus__Dropped()
{
	if ( report_gene )
		report_gene->Gene_Seal_Up();
}


bool XEPL::CELL::Nucleus::Nucleus__Processed_Gene ( CELL::Nucleus* _nucleus, DNA::Gene* _program )
{
	if ( !_nucleus )
		return false;

	return _nucleus->Process_Gene ( _program );
}


bool XEPL::CELL::Nucleus::Nucleus__Rendered_Form ( CORTEX::Rendron* _renderer, TEXT::Cord* _form )
{
	if ( !_renderer || !_form || this == _renderer )
		return true;

	TEXT::String form ( _form );

	if ( !form.empty() )
	{
		CORTEX::Neuron* target = Nucleus__Host_Neuron();

		TEXT::String tgt;

		if ( form.Split_ch_rhs ( '!', &tgt ) )
		{
			target->Find_Neuron ( &form, &target );
			form.assign ( tgt );
		}

		if ( target )
		{
			DNA::Gene* gene;

			if ( target->Form_Get ( &form, &gene ) )
			{
				_renderer->Payload ( target, gene );
				return true;
			}
		}
	}

	return false;
}


XEPL::TEXT::Parser::~Parser()
{
	delete error_string;
}

XEPL::TEXT::Parser::Parser()
	: parser_bag    ( nullptr )
	, error_string  ( nullptr )
{}


XEPL::TEXT::ParserBag::ParserBag ( TEXT::Cord* _bag, TEXT::Parser* _parser )
	: start_of_data    ( _bag->c_str() )
	, end_of_data      ( start_of_data+_bag->size() )
	, parser_host      ( _parser )
	, current_position ( start_of_data )
	, tip_char         ( *current_position )
{
	if ( *end_of_data )
		Parse_Error ( "buffer is not terminated" );
}


bool XEPL::TEXT::ParserBag::Consume ( char _c1 )
{
	return ( tip_char==_c1 && Nextt() );
}

bool XEPL::TEXT::ParserBag::Consume ( char _c1, char _c2 )
{
	return
	    tip_char == _c1 &&
	    * ( current_position+1 ) == _c2 &&
	    Nudget ( 2 );
}

bool XEPL::TEXT::ParserBag::Consume ( char _c1, char _c2, char _c3 )
{
	return
	    tip_char == _c1 &&
	    * ( current_position+1 ) == _c2 &&
	    * ( current_position+2 ) == _c3 &&
	    Nudget ( 3 );
}

bool XEPL::TEXT::ParserBag::Consume ( char _c1, char _c2, char _c3, char _c4 )
{
	return
	    tip_char == _c1 &&
	    * ( current_position+1 ) == _c2 &&
	    * ( current_position+2 ) == _c3 &&
	    * ( current_position+3 ) == _c4 &&
	    Nudget ( 4 );
}


bool XEPL::TEXT::ParserBag::Discard_Char ( char _c1 )
{
	if ( tip_char == _c1 )
	{
		tip_char = *++current_position;
		return true;
	}

	TEXT::String msg ( "Expected: " );
	msg.push_back ( _c1 );
	parser_host->Record_Error ( msg.c_str(), current_position );
	return false;
}


void XEPL::TEXT::ParserBag::Next ( void  )
{
	tip_char = *++current_position;
}

bool XEPL::TEXT::ParserBag::Nextt ( void  )
{
	Next();
	return true;
}


void XEPL::TEXT::ParserBag::Nudge ( int _offset )
{
	current_position += _offset;
	tip_char = *current_position;
}

bool XEPL::TEXT::ParserBag::Nudget ( int _offset )
{
	Nudge ( _offset );
	return true;
}


void XEPL::TEXT::ParserBag::Parse_Error ( const char* _reason )
{
	TEXT::String range;
	range.push_back (  '-' );
	range.append ( _reason );
	SOMA::ErrorReport ( _reason )->append ( range );
	parser_host->Record_Error ( range.c_str(), "" );
}


long XEPL::TEXT::ParserBag::Remaining ( void ) const
{
	return ( end_of_data-current_position );
}


void XEPL::TEXT::ParserBag::Rewrite ( char _as )
{
	// Skip_Whitespace will miss this
	tip_char = _as;
	--current_position;
}


void XEPL::TEXT::ParserBag::Skip_Whitespace()
{
	while ( isspace ( *current_position ) )
		++current_position;

	tip_char = *current_position;
}


XEPL::TEXT::ParserChoices::ParserChoices()
	: choices()
{}


XEPL::TEXT::ParserSelect::~ParserSelect( void )
{
	MEMORY::Scope_Delete<ParserChoices> drop ( parser_choices );

	while ( !parser->error_string )
	{
		if ( parser->Empty() )
			parser->Record_Error ( "unexpected EOF", static_cast<char*> ( nullptr ) );

		ParserChoice* choice = &parser_choices->choices[0];

		while ( choice && !parser->error_string )
		{
			if ( choice->parser_option && ( *choice->parser_option ) ( parser ) )
			{
				if ( choice->parser_flags & Completes )
					return;

				if ( ! ( choice->parser_flags & Can_Repeat )  )
					choice->parser_option = nullptr;

				break;
			}

			choice++;
		}

		if ( !choice )
			parser->Record_Error ( "not claimed", "parser" );
	}
}

XEPL::TEXT::ParserSelect::ParserSelect ( TEXT::Parser* _parser )
	: parser            ( _parser )
	, parser_choices    ( new ParserChoices() )
	, number_of_options ( 0 )
{}


void XEPL::TEXT::ParserSelect::Add_Option ( int _flags, TEXT::ParserOption _option )
{
	parser_choices->choices[number_of_options++] = { _option, _flags };
}


bool XEPL::TEXT::Parser::Empty() const
{
	return !parser_bag->Remaining();
}


void XEPL::TEXT::Parser::Record_Error ( const char* _reason, const char* _explain )
{
	SOMA::ErrorReport report ( _reason );

	if ( _explain )
		report.append ( _explain );

	report.report = false;

	if ( !error_string )
		error_string=new TEXT::String;

	error_string->append ( report );
	error_string->append ( XEPL::EOL );
}

void XEPL::TEXT::Parser::Record_Error ( const char* _reason, TEXT::String* _explain )
{
	const char* ptr = _explain ? _explain->c_str() : nullptr;
	Record_Error ( _reason, ptr );
}


XEPL::XML::PrintXml::~PrintXml()
{
	if ( !returned )
		std::cout << XEPL::EOL;

	std::cout << std::flush;
}

XEPL::XML::PrintXml::PrintXml ( TEXT::String* _string )
	: XmlParser    ( _string, true )
	, depth       ( 0 )
	, skip        ( 2 )
	, returned    ( true )
	, had_content ( false )
{
	ParseIt();
}

bool XEPL::XML::PrintXml::Xml__New_Element ( DNA::Gene* _cell )
{
	if ( !returned )
		std::cout << XEPL::EOL;

	returned = false;

	for ( int i=0; i<depth; i++ )
		std::cout << ' ';

	if ( _cell->space_string )
		std::cout << *_cell->space_string << ':';

	std::cout << *_cell->cell_name;
	depth += skip;
	return false;
}

bool XEPL::XML::PrintXml::Xml__End_Element ( DNA::Gene* _cell )
{
	depth -= skip;

	returned = !_cell->inner_genes && had_content && !_cell->traits;
	had_content = false;

	return false;
}

bool XEPL::XML::PrintXml::Xml__New_Attribute ( TEXT::String* _key, TEXT::String* _term, char _quote )
{
	std::cout << ' ';
	std::cout << *_key << "=" << _quote << *_term << _quote;
	return false;
}

bool XEPL::XML::PrintXml::Xml__New_Comment ( TEXT::String* )
{
//	std::cout << "COMMENT " << *_term << std::endl;
	return true;
}

bool XEPL::XML::PrintXml::Xml__New_Content ( TEXT::String* _term )
{
	std::cout << " | " << *_term << XEPL::EOL;
	had_content = true;
	return false;
}


XEPL::SIGNAL::Receptor::~Receptor()
{
	if ( memento && --memento->retain_count < 1 )
		memento->Destroy();

	if ( host_axon && --host_axon->retain_count < 1 )
		host_axon->Destroy();

	if ( target_neuron && --target_neuron->retain_count < 1 )
		target_neuron->Destroy();
}

XEPL::SIGNAL::Receptor::Receptor ( CORTEX::Neuron* _neuron, SIGNAL::Axon* _axon, SIGNAL::Receiver _response, ATOM::Atom* _memento )
	: ATOM::Atom()
	, host_axon         ( _axon )
	, target_neuron     ( _neuron )
	, active_action     ( nullptr )
	, signal_receiver   ( _response )
	, memento           ( _memento )
{
	if ( host_axon )
		++host_axon->retain_count;

	if ( target_neuron )
		++target_neuron->retain_count;

	if ( memento )
		++memento->retain_count;
}


XEPL::SIGNAL::ReceptorChain::~ReceptorChain ()
{
	Detach_Receptors();
}

XEPL::SIGNAL::ReceptorChain::ReceptorChain ()
	:  ATOM::Chain ( true )
{}


// Synapse with the other neuron to start a new _signal
void XEPL::SIGNAL::ReceptorChain::Deliver_Signal ( ATOM::Atom* _signal ) const
{
	THREAD::MutexScope lock_synapse ( chain_lock );

	ATOM::Bond* next_bond = head;

	while ( next_bond )
	{
		SIGNAL::Receptor* receptor = static_cast<SIGNAL::Receptor*> ( next_bond->atom ) ;

		receptor->Receptor__Activate ( _signal );

		next_bond = next_bond->next;
	}
}


void  XEPL::SIGNAL::ReceptorChain::Detach_Receptors()
{
	SIGNAL::Receptor* receptor;

	while ( head && ( receptor = static_cast<SIGNAL::Receptor*> ( head->atom ) ) )
		receptor->Receptor__Cancel();
}


void XEPL::SIGNAL::Receptor::Receptor__Cancel()
{
	target_neuron->Receptor_Detach ( this );
	Release();
}


// activate the response
void XEPL::SIGNAL::Receptor::Receptor__Activate ( ATOM::Atom* _stimuli ) const
{
	XEPL::TRACE( "ENTR_Axon", target_neuron, host_axon->axon_name );

	tlsLobe->current_stimulus = _stimuli;
	
	( target_neuron->*signal_receiver ) ( _stimuli, static_cast<DNA::Gene*> ( memento ) );

	XEPL::TRACE( "EXIT_Axon", target_neuron, host_axon->axon_name );
}


XEPL::SIGNAL::Relay::~Relay()
{
	delete receptor_chain;
}

XEPL::SIGNAL::Relay::Relay ( CORTEX::Neuron* _neuron, SIGNAL::Axon* _axon, SIGNAL::Receiver _response, SIGNAL::Receptor* _memento )
	: Receptor       ( _neuron, _axon, _response, _memento )
	, receptor_chain ( new SIGNAL::ReceptorChain() )
{}


// Deliver the Signal to the Receptor chain
void XEPL::SIGNAL::Relay::Receptor__Activate ( ATOM::Atom* _stimuli ) const
{
	receptor_chain->Deliver_Signal ( _stimuli );
}


XEPL::CORTEX::Rendron::Rendron ( CORTEX::Neuron* _owner, DNA::Gene* _config, TEXT::String* _bag )
	: Cell           ( _config->cell_name )
	, Nucleus        ( _config, this )
	, push_renderer  ( this )
	, rendition ( _bag )
{
	parent_neuron = _owner;
	++parent_neuron->retain_count;

	Register_Method ( "Markup", ( CELL::Function )&Rendron::Rendron__Method_Markup, nullptr );
}


void XEPL::CORTEX::Rendron::Markup ( CELL::Nucleus* _nucleus, DNA::Gene* _gene, TEXT::String* _results )
{
	SOMA::LocateIndex source ( _nucleus, _gene->space_string );

	DNA::Gene* src = source.gene;

	DNA::Scope_Index restore ( src );

	if ( tlsLobe->cortex->Did_Render ( _gene->cell_name, _nucleus, _gene, this ) )
		return;

	SOMA::ErrorReport ( "renderer missed tag: " )->append( *_gene->cell_name );

	XML::XmlBuilder x1 ( _gene->cell_name, _results, _gene->space_string );
	x1.Absorb_Attributes ( _nucleus, _gene );
	x1.Close_Traits();

	RNA::Evaluate_Script ( _nucleus, _gene, _gene->Content(), _results );

	if ( !_gene->inner_genes )
		return;

	ATOM::Bond* bond = _gene->inner_genes->head;

	while ( bond )
	{
		DNA::Gene* gene =  static_cast<DNA::Gene*> ( bond->atom );
		Markup ( _nucleus, gene, rendition );
		bond = bond->next ;
	}
}


XEPL::CORTEX::Neuron* XEPL::CORTEX::Rendron::Nucleus__Host_Neuron ( void )
{
	return Nucleus::parent_neuron;
}


void XEPL::CORTEX::Rendron::Nucleus__Path ( TEXT::String*, TEXT::Cord* ) const
{
}


bool XEPL::CORTEX::Rendron::Nucleus__Processed_Gene ( CELL::Nucleus* _nucleus, DNA::Gene* _config )
{
	Markup ( _nucleus, _config, rendition );
	return true;
}


void XEPL::CORTEX::Rendron::Payload ( CELL::Nucleus* _nucleus, DNA::Gene* _gene )
{
	RNA::Evaluate_Script ( _nucleus, _gene, _gene->Content(), rendition );

	if ( !_gene->inner_genes )
		return;

	DNA::Scope_Terms nest ( _gene );

	ATOM::Bond* bond = _gene->inner_genes->head;

	while ( bond )
	{
		DNA::Gene* gene =  static_cast<DNA::Gene*> ( bond->atom );
		Markup ( _nucleus, gene, rendition );
		bond = bond->next ;
	}
}


void XEPL::CORTEX::Rendron::Rendron__Render_Gene ( CELL::Nucleus*, DNA::Gene* _gene )
{
	_gene->Gene_Print_rope ( rendition );
}


XEPL::RNA::RnaBag::RnaBag ( TEXT::Cord* _bag, RNA::RnaScript* _script )
	: ParserBag      ( _bag, _script )
	, script         ( _script )
{}


void XEPL::RNA::RnaBag::Change_Variable ( void )
{
	if ( !isalpha ( tip_char ) && tip_char != '*' )
	{
		auto lobe = tlsLobe;

		if ( Consume ( '$' ) )
			Process_Gene ( lobe->outdex );
		else
			Process_Gene ( lobe->index );

		return;
	}

	TEXT::String variable_tag;
	Extract_Tag ( &variable_tag );

	SOMA::GeneLocator switch_into ( script->neuron, &variable_tag );

	if ( switch_into.gene )
	{
		Process_Gene ( switch_into.gene );
		return;
	}

	script->Record_Error ( "Gene not found", &variable_tag );
}


void XEPL::RNA::RnaBag::Declare_Content()
{
	if ( tip_char != '=' && tip_char != '+' )
	{
		script->gene->Make_Content_Wire();
		script->gene->content_wire->Print ( script->value );
		return;
	}

	bool append_content = tip_char == '+';
	tip_char = *++current_position;
	Skip_Whitespace();
	script->rna_bag->Get_Next_Value();

	if ( !append_content )
	{
		script->gene->Content_Assign ( script->value );
		return;
	}

	script->gene->Content_Append ( script->value );
	script->value->assign ( *script->gene->Content() );
}


void XEPL::RNA::RnaBag::Enter_Child_Gene()
{
	TEXT::String tag;
	Extract_Tag ( &tag );

	DNA::Gene* gene = nullptr;

	if ( script->gene->Get_First_Gene ( &tag, &gene ) )
	{
		DNA::Scope_Index active ( gene );
		Process_Gene ( gene );
	}
	else
		script->Record_Error ( "Child_Gene not found: ", tag.c_str() );
}


void XEPL::RNA::RnaBag::Enter_Inner_Block()
{
	while ( Extract_Value() )
	{}

	if ( Discard_Char ( '}' ) && tip_char )
		Skip_Whitespace();
}


void XEPL::RNA::RnaBag::Enter_Neuron()
{
	switch ( tip_char )
	{
		case '\'' :
			Extract_Neuron_Attribute();
			break;

		case '\"' :
			Extract_Neuron_Property();
			break;

		case '`' :
			Extract_Neuron_Feature();
			break;

		default:
			{
				TEXT::String neuron_tag;
				Extract_Tag ( &neuron_tag );

				CORTEX::Neuron* target = nullptr;

				if ( script->neuron->Find_Neuron ( &neuron_tag, &target ) )
				{
					CORTEX::Neuron* was = script->neuron;

					script->neuron = target;
					Enter_Neuron();
					script->neuron = was;
				}
			}
	}

	if ( tip_char == '.' )
		Mutate_Value();
}


void XEPL::RNA::RnaBag::Extract_Neuron_Attribute()
{
	tip_char = *++current_position;

	TEXT::String attribute_tag;
	Extract_Tag ( &attribute_tag );

	if ( tip_char == '\'' )
		tip_char = *++current_position;
	else
		parser_host->Record_Error ( "Attribute Missing closing quote", current_position );

	Skip_Whitespace();

	if ( attribute_tag.empty() )
		script->value->assign ( *script->neuron->cell_name );
	else if ( script->gene )
		script->gene->Trait_Get ( &attribute_tag, script->value );
	else
		script->value->erase();
}


void XEPL::RNA::RnaBag::Extract_Neuron_Feature()
{
	const char wastip = tip_char;
	tip_char = *++current_position;

	TEXT::String property_tag;
	Extract_Tag ( &property_tag );
	Discard_Char ( wastip );

	script->truth = script->neuron->Feature_Get ( &property_tag, script->value );
}


void XEPL::RNA::RnaBag::Extract_Next_Value()
{
	while ( !script->error_string && Get_Next_Value() )
	{}
}


void XEPL::RNA::RnaBag::Extract_Neuron_Property()
{
	const char wastip = tip_char;
	tip_char = *++current_position;

	TEXT::String property_tag;
	Extract_Tag ( &property_tag );
	Discard_Char ( wastip );

	script->truth = script->neuron->Property_Hunt ( &property_tag, script->value );
}


void XEPL::RNA::RnaBag::Extract_Tag ( TEXT::String* _string )
{
	Skip_Whitespace();

	int reinterpret_count = 0;

	while ( tip_char == '*' )
	{
		++reinterpret_count;
		tip_char = *++current_position;
	}

	const char* start_position = current_position;
	const char*       position = current_position;

	while ( isalnum ( *position ) || *position == '_' )
		++position;

	_string->assign ( start_position, position-start_position );

	while ( isspace ( *position ) )
		++position;

	current_position =  position;
	tip_char         = *position;

	while ( reinterpret_count-- && Revalue_Tag ( _string ) )
		;
}


bool XEPL::RNA::RnaBag::Extract_Value()
{
	switch ( tip_char )
	{
		case ' ' :
		case '\t' :
		case '\n' :
			++current_position;
			Skip_Whitespace();
			return Extract_Value();

		case '\'' :
		case '`' :
		case '"' :
			Pull_String();
			return true;

		case '$' :
			tip_char = *++current_position;
			Change_Variable();
			return  true;

		case '.' :
			Mutate_Value();
			return  true;

		case '?' :
			tip_char = *++current_position;
			Ternary_Choice();
			return  true;

		case '{' :
			tip_char = *++current_position;
			Enter_Inner_Block();
			return  true;

		case '@' :
			tip_char = *++current_position;
			Enter_Neuron();
			return  true;

		case '!' :
			tip_char = *++current_position;
			Pull_Or_Set_Property();
			return  true;

		case '#' :
			tip_char = *++current_position;
			Pull_Index();
			return  true;

		case ';' :
			tip_char = *++current_position;
			Pull_Short_Term();
			return  true;

		case '%' :
			tip_char = *++current_position;
			Pull_Or_Set_Local();
			return  true;

		case ')' :
		case '}' :
		case '\0' :
			return false;
	}

	return Pull_Number() || Hunt_Property();
}


bool XEPL::RNA::RnaBag::Get_Next_Value()
{
	if ( tip_char && Extract_Value() && tip_char )
	{
		Mutate_Value();
		Skip_Whitespace();
		return true;
	}

	return false;
}


bool XEPL::RNA::RnaBag::Hunt_Property()
{
	TEXT::String property_tag;
	Extract_Tag ( &property_tag );

	auto lobe = tlsLobe;

	if ( lobe->locals && lobe->locals->Trait_Get ( &property_tag, script->value ) )
		return true;

	if ( lobe->short_term_stack->Get ( &property_tag, script->value ) )
		return true;

	if ( script->gene && script->gene->Trait_Get( &property_tag, script->value ) )
		return true;

	if ( script->neuron && script->neuron->Property_Hunt ( &property_tag, script->value ) )
		return true;

	return false;
}


void XEPL::RNA::RnaBag::Mutate_Value ( void  )
{
	while ( tip_char == '.' )
	{
		tip_char = *++current_position;
		script->Do_Operation();
	}
}


void XEPL::RNA::RnaBag::Process_Gene ( DNA::Gene*& __gene )
{
	if ( !__gene )
		return;

	DNA::Scope_Gene hold_active   ( script->gene, __gene );

	Process_Gene2 ( __gene );
}

void XEPL::RNA::RnaBag::Process_Gene2 ( DNA::Gene*& __gene )
{
	switch ( tip_char )
	{
		case ' ' :
		case '\t':
		case '\n':
			++current_position;
			Skip_Whitespace();
			Process_Gene2 ( __gene );
			break;

		case '.' :
			do
			{
				tip_char = *++current_position;
				script->Do_Splice();
			}
			while ( *current_position == '.' );

			Process_Gene2 ( __gene );
			break;

		case '>' :
			tip_char = *++current_position;
			Serialize();
			break;

		case '/' :
			tip_char = *++current_position;
			Enter_Child_Gene();
			break;

		case '\'' :
			Select_Attribute();
			break;

		case '|' :
			tip_char = *++current_position;
			Declare_Content();
			break;
	}
}


bool XEPL::RNA::RnaBag::Process_Parameter ( TEXT::String** _param )
{
	TEXT::String* was = script->value;
	script->value = new TEXT::String();

	if ( Extract_Value() && tip_char )
	{
		Mutate_Value();
		Skip_Whitespace();
	}

	if ( tip_char == ')' )
		tip_char = *++current_position;
	else
		parser_host->Record_Error ( "Parameter not closed ", script->value );

	*_param = script->value;
	script->value = was;

	return true;
}


void XEPL::RNA::RnaBag::Pull_Index ( void )
{
	ScriptTagValue tag ( this );

	if ( tag.term )
	{
		if ( script->gene )
			script->gene->Trait_Set ( &tag, script->value );

		return;
	}

	if ( script->gene && script->gene->Trait_Get ( &tag, script->value ) )
		return;

	if ( script->value )
		script->value->clear();

	return;
}


bool XEPL::RNA::RnaBag::Pull_Number()
{
	if ( !isdigit ( tip_char ) )
		return false;

	const char* start_position=current_position;
	const char* position = start_position+1;

	while ( isdigit ( *position ) )
		++position;

	script->value->assign ( start_position, position-start_position );

	current_position = position;
	tip_char         = *current_position;

	return true;
}


void XEPL::RNA::RnaBag::Pull_Or_Set_Local ( void )
{
	ScriptTagValue tag ( this );

	auto* lobe = tlsLobe;

	auto& locals = lobe->locals;

	if ( tag.term )
	{
		if ( !locals )
			lobe->Make_Locals();

		locals->Trait_Set ( &tag, script->value );

		return;
	}

	if ( locals && locals->Trait_Get ( &tag, script->value ) )
		return;

	script->value->clear();
}


void XEPL::RNA::RnaBag::Pull_Or_Set_Property ( void )
{
	ScriptTagValue tag ( this );

	if ( tag.term )
	{
		script->neuron->Property_Set ( &tag, script->value );
		return;
	}

	if ( script->neuron->Property_Get ( &tag, script->value ) )
		return;

	script->value->clear();
}


void XEPL::RNA::RnaBag::Pull_Short_Term ( void )
{
	ScriptTagValue tag ( this );

	auto nest = tlsLobe->short_term_stack;

	if ( tag.term )
	{
		nest->Set ( &tag, script->value );
		return;
	}

	if ( nest->Get ( &tag, script->value ) )
		return;

	script->value->clear();
}


void XEPL::RNA::RnaBag::Pull_String ( void )
{
	char quote = tip_char;
	const char* start_position = ++current_position;
	const char* position = start_position;

	while ( *position && *position != quote )
		++position;

	script->value->assign ( start_position, position-start_position );

	if ( !*position )
		SOMA::ErrorReport ( "Non-Terminated string: " )->append ( start_position );
	else
		++position;

	current_position = position;
	tip_char = *current_position;
}


bool XEPL::RNA::RnaBag::Revalue_Tag ( TEXT::String* _tag_name )
{
	auto lobe = tlsLobe;

	if ( lobe->locals && lobe->locals->Trait_Get ( _tag_name, _tag_name ) )
		return true;

	if ( lobe->short_term_stack->Get ( _tag_name, _tag_name ) )
		return true;

	if ( script->neuron && script->neuron->Property_Get ( _tag_name, _tag_name ) )
		return true;

	return false;
}


void XEPL::RNA::RnaBag::Select_Attribute()
{
	const char wastip = tip_char;
	tip_char = *++current_position;

	TEXT::String attribute_tag;
	Extract_Tag ( &attribute_tag );

	if ( tip_char == wastip )
		tip_char = *++current_position;
	else
		SOMA::ErrorReport ( "Non-Terminated string: " )->append ( start_of_data );

	if ( attribute_tag.empty() )
		script->value->assign ( *script->gene->cell_name );
	else
	{
		if ( tip_char == '=' )
		{
			Get_Next_Value();
			script->gene->Trait_Set ( &attribute_tag, script->value );
			return;
		}

		script->truth = !!script->gene->Trait_Get ( &attribute_tag, script->value );
	}
}


void XEPL::RNA::RnaBag::Serialize()
{
	int depth=0;

	if ( tip_char == '>' )
	{
		depth = 1;

		while ( tip_char == '>' )
		{
			depth += 1;
			tip_char = *++current_position;
		}
	}

	script->gene->Gene_Print_rope ( script->value, depth );
}


void XEPL::RNA::RnaBag::Ternary_Choice()
{
	Skip_Whitespace();
	bool keepTruth = script->truth;
	bool resultTruth = script->truth;
	TEXT::String trueValue;
	TEXT::String preValue ( script->value );

	if ( tip_char && tip_char != ':' )
	{
		script->rna_bag->Get_Next_Value();

		if ( keepTruth )
		{
			resultTruth = script->truth;
			trueValue.assign ( *script->value );
		}
		else
			script->value->assign ( preValue );
	}

	if ( tip_char && Consume ( ':' ) )
	{
		script->value->assign ( preValue );
		script->rna_bag->Get_Next_Value();

		if ( keepTruth )
		{
			script->truth = resultTruth;
			script->value->assign ( trueValue );
		}
	}
}


XEPL::RNA::RnaScript::RnaScript ( CELL::Nucleus* _nucleus, DNA::Gene* _gene )
	: Parser  ()
	, neuron                     ( _nucleus->Nucleus__Host_Neuron() )
	, rna_bag                    ( nullptr )
	, value                      ( nullptr )
	, gene                       ( _gene )
	, truth                      ( false )
{
	if ( !_gene )
		return;

	if ( !_gene->content_wire )
		return;

	TEXT::String sValue;
	value = &sValue;

	THREAD_COUNTER ( count_scripts )

	if ( !gene->thread_safe )
	{
		TEXT::String safe_expr;
		{
			XEPL::THREAD::MutexScope lock_theirs ( _gene->content_wire->wire_lock );
			safe_expr.assign( *_gene->content_wire->wire_string );
		}
		parser_bag = rna_bag = new RnaBag ( &safe_expr, this );
		THREAD::MutexScope lock_traits ( gene->traits_mutex );
		rna_bag->Extract_Next_Value();
		rna_bag->Skip_Whitespace();
		delete rna_bag;
	}
	else
	{
		parser_bag = rna_bag = new RnaBag ( _gene->content_wire->wire_string, this );
		rna_bag->Extract_Next_Value();
		rna_bag->Skip_Whitespace();
		delete rna_bag;
	}

	if ( error_string )
		SOMA::ErrorReport ( "Script: " ) + error_string ;
}

XEPL::RNA::RnaScript::RnaScript ( CELL::Nucleus*  _nucleus,
                             DNA::Gene*      _gene,
                             TEXT::Cord*     _expr,
                             TEXT::String*   _result,
                             bool*           _truth,
                             TEXT::Cord*     _seed,
                             bool            _append
                           )
	: Parser  ()
	, neuron                     ( nullptr )
	, rna_bag                        ( nullptr )
	, value                      ( nullptr )
	, gene                       ( _gene )
	, truth                      ( false )
{
	TEXT::String* safe_expr = nullptr;

	if ( !_expr && _gene && _gene->content_wire )
	{
		XEPL::THREAD::MutexScope lock_theirs ( _gene->content_wire->wire_lock );

		_expr = _gene->content_wire->wire_string;

		if ( !_gene->thread_safe )
		{
			safe_expr = new TEXT::String ( _expr );
			_expr = safe_expr;
		}
	}

	if ( !_expr )
		return;

	rna_bag = new RnaBag ( _expr, this );
	parser_bag = rna_bag;

	THREAD_COUNTER ( count_scripts )

	TEXT::String sValue( _seed );
	value = &sValue;

	if ( _nucleus )
		neuron = _nucleus->Nucleus__Host_Neuron();

	if ( _truth )
		truth = *_truth;


	if ( gene && !gene->thread_safe )
	{
		THREAD::MutexScope lock_traits ( gene->traits_mutex );
		rna_bag->Extract_Next_Value();
	}
	else
		rna_bag->Extract_Next_Value();

	if ( _truth )
		*_truth = truth;

	rna_bag->Skip_Whitespace();

	if ( _result && !error_string )
	{
		if ( _append )
			_result->append ( *value );
		else
			_result->assign ( *value );
	}

	if ( error_string )
		SOMA::ErrorReport ( "Script: " ) + error_string ;

	delete rna_bag;
	delete safe_expr;
}


void XEPL::RNA::RnaScript::Do_Operation ( void )
{
	TEXT::String opcode;
	rna_bag->Extract_Tag ( &opcode );

	TEXT::String* param = nullptr;

	if ( rna_bag->tip_char == '(' )
	{
		rna_bag->tip_char = *++rna_bag->current_position;
		rna_bag->Process_Parameter ( &param );
	}

	MEMORY::Scope_Delete<TEXT::String> release ( param );

	if ( tlsLobe->cortex->Did_Operation ( &opcode, value, param, truth ) )
	{
		THREAD_COUNTER ( count_operations )
		return;
	}

	if ( neuron->Performed_Macro ( &opcode, value, truth, value ) )
	{
		THREAD_COUNTER ( count_macros )
		return;
	}

	Record_Error ( "Operator/Macro not found: ", &opcode );
}


void XEPL::RNA::RnaScript::Do_Splice ( void )
{
	TEXT::String opcode;
	rna_bag->Extract_Tag ( &opcode );

	TEXT::String* param = nullptr;

	if ( rna_bag->tip_char == '(' )
	{
		rna_bag->tip_char = *++rna_bag->current_position;
		rna_bag->Process_Parameter ( &param );
	}

	MEMORY::Scope_Delete<TEXT::String> release ( param );

	if ( tlsLobe->cortex->Did_Splice ( &opcode, value, neuron, gene,  param, &truth ) )
		return;

	Record_Error ( "Splicer not found: ", &opcode );
}


void XEPL::RNA::Evaluate_Script ( CELL::Nucleus* _nucleus, DNA::Gene* _gene, TEXT::Cord* _expr, TEXT::String* _into )
{
	if ( !_expr )
		return;

	const char* cur_pos = _expr->c_str();
	const char* bo_script;

	while ( ( bo_script = strstr ( cur_pos, "{{" ) ) != nullptr )
	{
		_into->append ( cur_pos, bo_script - cur_pos );
		const char* eo_script = strstr ( bo_script, "}}" );

		if ( eo_script )
		{
			TEXT::String inner_expr;
			inner_expr.assign ( bo_script + 2, eo_script - bo_script - 2 );
			cur_pos = eo_script + 2;

			RNA::Script_Append ( _nucleus, _gene, &inner_expr, _into );
		}
	}

	_into->append ( cur_pos );
}


XEPL::CORTEX::Scope_Render::~Scope_Render()
{
	r_renderer = was_renderer;
}

XEPL::CORTEX::Scope_Render::Scope_Render ( CORTEX::Rendron* _render )
	: r_renderer ( tlsLobe->renderer )
{
	was_renderer = r_renderer;
	r_renderer = _render;
}


XEPL::DNA::Scope_Terms::~Scope_Terms()
{
	restore_nest = previous_nest;

	if ( pushed )
		delete terms;
}

XEPL::DNA::Scope_Terms::Scope_Terms ( void )
	: restore_nest               ( tlsLobe->short_term_stack )
	, previous_nest              ( restore_nest )
	, hot_nest                   ( previous_nest )
	, terms                      ( nullptr )
	, pushed                     ( false )
{
	restore_nest  = this;

	if ( previous_nest )
		SOMA::xeplError ( "scShortTerm", nullptr, "Double Initialization" );
}

XEPL::DNA::Scope_Terms::Scope_Terms ( DNA::Gene* _traits_gene )
	: restore_nest               ( tlsLobe->short_term_stack )
	, previous_nest              ( restore_nest )
	, hot_nest                   ( previous_nest->hot_nest )
	, terms                      ( previous_nest->terms )
	, pushed                     ( false )
{
	if ( _traits_gene && _traits_gene->traits )
		Nest_Gene_Terms ( _traits_gene );

	restore_nest = this;
}

XEPL::DNA::Scope_Terms::Scope_Terms ( TEXT::Cord* _query_string )
	: restore_nest               ( tlsLobe->short_term_stack )
	, previous_nest              ( restore_nest )
	, hot_nest                   ( previous_nest->hot_nest )
	, terms                      ( previous_nest->terms )
	, pushed                     ( false )
{
	Nest_From_Str ( _query_string );
	restore_nest = this;
}

XEPL::DNA::Scope_Terms::Scope_Terms ( const char* _trait, TEXT::String* _value )
	: restore_nest               ( tlsLobe->short_term_stack )
	, previous_nest              ( restore_nest )
	, hot_nest                   ( previous_nest->hot_nest )
	, terms                      ( previous_nest->terms )
	, pushed                     ( false )
{
	Set( _trait, _value );
	restore_nest = this;
}


bool XEPL::DNA::Scope_Terms::Get ( TEXT::Cord* _trait_name, TEXT::String* _into_string ) const
{
	const Scope_Terms* checkNest = pushed ? this : hot_nest;

	while ( checkNest )
	{
		if ( checkNest->pushed )
		{
			auto it = checkNest->terms->find ( *_trait_name );

			if ( it != checkNest->terms->end() )
			{
				_into_string->assign ( ( *it ).second );
				return true;
			}
		}

		checkNest = checkNest->previous_nest->hot_nest;
	}

	return false;
}

void XEPL::DNA::Scope_Terms::Report ( DNA::Gene* _into_gene )
{
	const Scope_Terms* checkNest = pushed ? this : hot_nest;

	if ( checkNest->previous_nest )
		checkNest->previous_nest->Report( _into_gene );

	auto it = checkNest->terms->begin();

	while ( it != checkNest->terms->end() )
	{
		_into_gene->Trait_Set( &(*it).first, &(*it).second );
		++it;
	}
}


void XEPL::DNA::Scope_Terms::Nest_From_Str ( TEXT::Cord* _query_string )
{
	if ( _query_string->empty() )
		return;

	TEXT::Cord* query = _query_string;
	TEXT::String lhs, rhs;

	while ( query->Split_ch_lhs_rhs ( '&', &lhs, &rhs ) )
	{
		TEXT::String llhs, rrhs;

		if ( lhs.Split_ch_lhs_rhs ( '=', &llhs, &rrhs ) )
			Set ( &llhs, &rrhs );

		query = &rhs;
	}

	if ( rhs.Split_ch_lhs_rhs ( '=', &lhs, &rhs ) )
		Set ( &lhs, &rhs );
}


void XEPL::DNA::Scope_Terms::Nest_Gene_Terms ( DNA::Gene* _traits_gene )
{
	if ( !pushed )
	{
		terms = new TermMap();
		pushed = true;
	}

	DNA::TraitMap* traitsMap = _traits_gene->traits->map_of_traits;

	auto    it  = traitsMap->begin();
	while ( it != traitsMap->end() )
	{
		auto& its = *it++;
		(*terms)[its.first].assign( *its.second->trait_term );
	}
}


void XEPL::DNA::Scope_Terms::Set ( TEXT::Cord* _trait_name, TEXT::Cord* _to_string )
{
	if ( !pushed )
	{
		terms = new TermMap();
		pushed = true;
	}

	hot_nest = this;
	(*terms)[ *_trait_name ] = *_to_string ;
}

void XEPL::DNA::Scope_Terms::Set ( const char* _trait_name, TEXT::Cord* _to_string )
{
	TEXT::Cord trait_name( _trait_name );
	Set( &trait_name, _to_string );
}


XEPL::THREAD::Semaphore::Semaphore()
	: std::condition_variable()
	, std::mutex()
{}


void XEPL::THREAD::Semaphore::Give ( void )
{
	std::unique_lock<std::mutex> lock ( *this );
	notify_one();
}


void XEPL::THREAD::Semaphore::Give_As_Owner ( void )
{
	notify_one();
}


XEPL::CORTEX::Sensor::Sensor ( CORTEX::Neuron*  _parent, DNA::Gene* _config )
	: Neuron           ( _parent, _config )
	, sensor_wire      ( new TEXT::Wire() )
	, sensor_is_closed ( false )
{
	Register_Method ( "Sensor_Is_Closed",   ( CELL::Function )&Sensor::Method_Sensor_Closed,   nullptr );
	Register_Method ( "Sensor_Has_Input", ( CELL::Function )&Sensor::Method_Sensor_Received, nullptr );
}

XEPL::CORTEX::Sensor::~Sensor()
{
	if ( sensor_wire )
		sensor_wire->Release();
}


void XEPL::CORTEX::Sensor::Method_Sensor_Closed NUCLEUS_METHOD
{
	if ( sensor_is_closed )
		return;

	sensor_is_closed = true;

	Sensor__Closed();
}


void XEPL::CORTEX::Sensor::Method_Sensor_Received NUCLEUS_METHOD
{
	if ( sensor_is_closed )
		return;

	DNA::Gene* trigger = static_cast<DNA::Gene*> ( tlsLobe->current_stimulus );

	if ( trigger  )
	{
		sensor_wire->Append ( trigger->content_wire );
		Sensor__Scan();
	}
}


XEPL::SIGNAL::Signal::~Signal()
{
	if ( stimulus && --stimulus->retain_count < 1 )
		stimulus->Destroy();
}

XEPL::SIGNAL::Signal::Signal ( ATOM::Atom* _stimulus )
	: ATOM::Atom()
	, stimulus  ( _stimulus )
{
	if ( stimulus )
		++stimulus->retain_count;
}


XEPL::SIGNAL::Signal__Rendezvous::Signal__Rendezvous ( ATOM::Atom* _stimuli, bool* _dissolved )
	: Signal        ( _stimuli )
	, lobe          ( tlsLobe )
	, semaphore     ( new THREAD::Semaphore() )
	, is_dissolved  ( _dissolved )
{}

XEPL::SIGNAL::Signal__Rendezvous::~Signal__Rendezvous()
{
	*is_dissolved = true;

	if ( lobe == tlsLobe )
		semaphore->Give_As_Owner();
	else
		semaphore->Give();

	delete semaphore;
}


XEPL::DNA::StableRecall::~StableRecall ( void )
{
	delete stable_genes;
	delete stable_traits;
}

XEPL::DNA::StableRecall::StableRecall ( DNA::Gene* _gene, bool _elements )
	: stable_genes               ( nullptr )
	, stable_traits              ( nullptr )
	, current_trait              ( nullptr )
{
	if ( _gene )
	{
		if ( _elements )
			_gene->Copy_Genes_Into ( &stable_genes );
		else
			_gene->Traits_Duplicate ( &stable_traits );

		current_trait = stable_traits ? stable_traits->first_trait : nullptr;
	}
}

XEPL::DNA::StableRecall::StableRecall ( DNA::Gene* _gene, TEXT::Cord* _name )
	: stable_genes               ( nullptr )
	, stable_traits              ( nullptr )
	, current_trait              ( nullptr )
{
	if ( !_gene )
		return;

	if ( _gene->traits )
	{
		_gene->Traits_Duplicate ( &stable_traits );

		current_trait = stable_traits->first_trait;
	}

	if ( _name && !_name->empty() )
	{
		_gene->Get_Gene_Chain ( _name, &stable_genes );
		return;
	}

	_gene->Copy_Genes_Into ( &stable_genes );
}


bool XEPL::DNA::StableRecall::Has_Genes ( void ) const
{
	if ( !stable_genes )
		return false;

	return stable_genes->head != nullptr;
}


bool XEPL::DNA::StableRecall::Next_Trait ( const char** __name, const char** __term )
{
	if ( !current_trait )
	{
		current_trait = stable_traits ? stable_traits->first_trait : nullptr;
		__name = nullptr;
		__term = nullptr;
		return false;
	}

	*__name = current_trait->trait_name->c_str();
	*__term = current_trait->trait_term->c_str();
	current_trait = current_trait->next_trait;
	return true;
}

bool XEPL::DNA::StableRecall::Next_Trait ( TEXT::Cord** __name, TEXT::String** __term )
{
	if ( !current_trait )
	{
		current_trait = stable_traits ? stable_traits->first_trait : nullptr;
		__name = nullptr;
		__term = nullptr;
		return false;
	}

	*__name = current_trait->trait_name;
	*__term = current_trait->trait_term;
	current_trait = current_trait->next_trait;
	return true;
}


bool XEPL::DNA::StableRecall::Next_Gene ( DNA::Gene** __gene )
{
	if ( !stable_genes )
		return false;

	if ( stable_genes->Next ( __gene ) )
		return true;

	delete stable_genes;
	stable_genes=nullptr;

	return false;
}


XEPL::TEXT::String::String ( void )
	: TEXT::CppString()
{
	THREAD_COUNTER ( count_strings )
}

XEPL::TEXT::String::String ( const TEXT::CppString* _string )
	: TEXT::CppString()
{
	THREAD_COUNTER ( count_strings )

	if ( _string )
		assign ( *_string );
}

XEPL::TEXT::String::String ( const TEXT::String* _string )
	: TEXT::CppString()
{
	THREAD_COUNTER ( count_strings )

	if ( _string )
		assign ( *_string );
}

XEPL::TEXT::String::String ( const char* _string )
	: TEXT::CppString()
{
	THREAD_COUNTER ( count_strings )

	if ( _string )
		assign ( _string );
}

XEPL::TEXT::String::String ( const char* _string, size_t _length )
	: TEXT::CppString()
{
	THREAD_COUNTER ( count_strings )

	if ( _string && _length )
		assign ( _string, _length );
}


std::ostream& XEPL::TEXT::operator<< (std::ostream& os, const XEPL::TEXT::String& str)
{
	os << str.c_str();
	return os;
}

std::istream& XEPL::TEXT::operator>> (std::istream& is, XEPL::TEXT::String& str)
{
	std::string temp;
	is >> temp;
	str = temp.c_str();
	return is;
}


bool XEPL::TEXT::String::Split_at_lhs_rhs ( size_t _pos, TEXT::String* _lhs, TEXT::String* _rhs ) const
{
	_lhs->assign ( c_str(),     c_str()+_pos-1 );
	_rhs->assign ( c_str()+_pos, size()-_pos );
	return true;
}


bool XEPL::TEXT::String::Split_ch_lhs ( char _char, TEXT::String* _lhs )
{
	if ( empty() )
		return false;

	size_t pos = find ( _char );

	if ( pos != npos )
	{
		_lhs->assign ( c_str(), pos );
		erase ( 0, pos+1 );
		return true;
	}

	return false;
}


bool XEPL::TEXT::String::Split_ch_lhs_rhs ( char _char, TEXT::String* _lhs, TEXT::String* _rhs ) const
{
	const char* rhs = c_str();

	while ( *rhs && *rhs != _char )
		++rhs;

	if ( *rhs )
	{
		_lhs->assign ( c_str(), rhs-c_str() );
		_rhs->assign ( rhs+1 );
		return true;
	}

	_lhs->assign ( c_str() );
	_rhs->assign ( c_str() );
	return false;
}


bool XEPL::TEXT::String::Split_ch_rhs ( char _char, TEXT::String* _into )
{
	if ( empty() )
		return false;

	size_t pos = find ( _char );

	if ( pos != npos )
	{
		_into->assign ( c_str()+pos+1 );
		erase ( pos, size()-pos );
		return true;
	}

	return false;
}


static XEPL::SIGNAL::DeliverAction delivery_actions[] =
{
	&XEPL::SIGNAL::ActionList::Press_Before_all,
	&XEPL::SIGNAL::ActionList::Pulse_Only_Once,
	&XEPL::SIGNAL::ActionList::Repeat_Multiple,
	&XEPL::SIGNAL::ActionList::Tail_After
};

XEPL::SIGNAL::Synapse::~Synapse()
{}

XEPL::SIGNAL::Synapse::Synapse ( CORTEX::Lobe*         _active_lobe,
								 SIGNAL::Axon*         _target_axon,
								 SIGNAL::Receiver      _routine,
								 SIGNAL::Receptor*     _memento
                                 )
	: Relay             ( _active_lobe, _target_axon, _routine, _memento )
	, action_list       ( _active_lobe->pending_actions )
	, delivery_function ( delivery_actions[ _target_axon->axon_type ] )
{
}


// Deliver the _signal to the other Lobe
void XEPL::SIGNAL::Synapse::Receptor__Activate ( ATOM::Atom* _stimuli ) const
{
	THREAD::MutexScope lock_receptors ( receptor_chain->chain_lock );

	ATOM::Bond* next = receptor_chain->head;

	while ( next )
	{
		SIGNAL::Receptor* target_receptor = static_cast<SIGNAL::Receptor*> ( next->atom );

		( action_list->*delivery_function ) ( new SIGNAL::Action__Signal ( target_receptor, _stimuli ) );

		next = next->next;
	}
}


XEPL::THREAD::Thread::Thread ( CORTEX::Lobe* _lobe, THREAD::Semaphore* _restWakeSem )
	: std__thread     ( nullptr )
	, lobe            ( _lobe )
	, semaphore_rest  ( _restWakeSem )
{}


void XEPL::SOMA::Lobe_Set_Thread_Name ( THREAD::Thread*, const char* _name )
{
	pthread_setname_np ( _name );
	// OR  pthread_setname_np ( _thread, _name );
}


void XEPL::THREAD::Thread::Burial()
{
	if ( std__thread )
	{
		std__thread->join();
		std__thread->~thread();

		delete std__thread;
		std__thread=nullptr;

		lobe = nullptr;
	}
}


// CPP17 Thread Birthplace for our new Lobe
void XEPL::THREAD::Thread::Conception()
{
	std__thread = ::new std::thread ( [] ( Thread* _this )
	{
		tlsLobe = _this->lobe;

		MEMORY::MemoryBackpack recycler;

		DNA::Scope_Terms startNest;

		tlsLobe->Main_Processing_Loop();
	}, this );
}


namespace XEPL::COUNTERS
{
	const char* msg = "\
rec_news:\
rec_dels:\
raw_news:\
raw_dels:\
strings:\
atoms:\
cells:\
genes:\
traits:\
lobes:\
neurons:\
rests:\
actions:\
wakes:\
keywords:\
scripts:\
operations:\
splices:\
macros:\
methods:\
exceptions:\
";
}
void XEPL::COUNTERS::TlsCounters::Add ( TlsCounters* _from )
{
	long record_size = sizeof ( TlsCounters );
	long number_of_counters = record_size/sizeof ( long );
	Counter* scounter = (Counter*) ( _from );
	Counter* tcounter = (Counter*) ( this );

	while ( number_of_counters-- )
		*tcounter+++=*scounter++;
}

void XEPL::COUNTERS::TlsCounters::Report ( XEPL::TEXT::CppString* _into )
{
	XEPL::TEXT::String long_label ( XEPL::COUNTERS::msg );
	XEPL::TEXT::String lhs;
	Counter* tcounter = (Counter*) ( this );

	while ( long_label.Split_ch_lhs ( ':', &lhs ) )
	{
		_into->append ( lhs ).append ( " " );
		_into->append ( std::to_string ( *tcounter ) ).append ( " : " );
	}
}

void XEPL::COUNTERS::TlsCounters::Final_Report()
{
	XEPL::TEXT::String long_label ( XEPL::COUNTERS::msg );
	XEPL::TEXT::String lhs;
	Counter* tcounter = (Counter*) ( this );

	std::string count;

	while ( long_label.Split_ch_lhs ( ':', &lhs ) )
	{
		SOMA::Long_Commafy ( *tcounter++, &count );
		std::cout << std::setw ( 12 ) << lhs << std::setw ( 15 )
		          << count << XEPL::EOL;
	}

	std::cout << std::flush;
}


XEPL::DNA::Trait::~Trait( void )
{
	delete trait_name;
	delete trait_term;
}

XEPL::DNA::Trait::Trait ( TEXT::Cord* _name, TEXT::Cord* _term, DNA::Trait* _next )
	: trait_name ( new TEXT::Cord   ( _name ) )
	, trait_term ( new TEXT::String ( _term ) )
	, next_trait ( _next )
{
	THREAD_COUNTER ( count_traits );
}

XEPL::DNA::Trait::Trait ( DNA::Trait* _trait, DNA::Trait* _next )
	: trait_name ( new TEXT::Cord   ( _trait->trait_name ) )
	, trait_term ( new TEXT::String ( _trait->trait_term ) )
	, next_trait ( _next )
{
	THREAD_COUNTER ( count_traits );
}


void XEPL::DNA::Trait::Print_Into_Bag ( TEXT::String* _into )
{
	if ( !_into )
		return;

	_into->push_back ( ' ' );
	_into->append ( *trait_name );
	_into->push_back ( '=' );
	SOMA::Escape_Quotes ( trait_term, _into );
}


XEPL::ATOM::Duct& XEPL::DNA::operator<< ( ATOM::Duct& _lhs, DNA::Trait* _rhs )
{
	if ( _rhs )
	{
		TEXT::String term;
		_lhs << ' ' << *_rhs->trait_name << '=' << *SOMA::Escape_Quotes ( _rhs->trait_term, &term );
	}

	return _lhs;
}


XEPL::DNA::Traits::~Traits()
{
	delete map_of_traits;

	DNA::Trait* was = first_trait;

	while ( was )
	{
		DNA::Trait* next = was->next_trait;
		delete was;
		was = next;
	}
}

XEPL::DNA::Traits::Traits()
	: first_trait    ( nullptr )
	, map_of_traits  ( new DNA::TraitMap() )
{}


void XEPL::DNA::Traits::Duplicate_Into ( DNA::Traits** __traits )
{
	DNA::Traits* clone = new Traits();
	DNA::Trait*   node = first_trait;

	while ( node )
	{
		clone->first_trait = new Trait ( node, clone->first_trait );
		(*clone->map_of_traits)[ *node->trait_name ] = clone->first_trait;

		node = node->next_trait;
	}

	*__traits = clone;
}


void XEPL::DNA::Traits::Evaluate ( DNA::Gene* _gene, CELL::Nucleus* _nucleus )
{
	DNA::Trait* node = first_trait;

	while ( node )
	{
		if ( node->trait_term->front() == '{' )
			RNA::Script_Assign ( _nucleus, _gene, node->trait_term, node->trait_term );

		node = node->next_trait;
	}
}
 

void XEPL::DNA::Traits::Print_Into_Bag ( TEXT::String* _bag )
{
	DNA::Trait* node = first_trait;

	while ( node )
	{
		node->Print_Into_Bag ( _bag );
		node = node->next_trait;
	}
}
 

XEPL::ATOM::Duct& XEPL::DNA::operator<< ( ATOM::Duct& _lhs, DNA::Traits* _rhs )
{
	if ( _rhs )
	{
		DNA::Trait* node = _rhs->first_trait;

		while ( node )
		{
			_lhs << node;
			node = node->next_trait;
		}
	}

	return _lhs;
}


void XEPL::DNA::Traits::Set_Trait ( TEXT::Cord* _name, TEXT::Cord* _term )
{
	auto it  = map_of_traits->find ( *_name );

	if ( it != map_of_traits->end() )
		it->second->trait_term->assign ( *_term );
	else
	{
		first_trait = new Trait ( _name, _term, first_trait );
		map_of_traits->emplace ( *_name, first_trait );
	}
}


XEPL::TEXT::Wire::~Wire( void )
{
	delete wire_lock;
	delete wire_string;
}

XEPL::TEXT::Wire::Wire()
	: ATOM::Atom()
	, wire_lock     ( new THREAD::Mutex() )
	, wire_string ( new TEXT::String() )
{}

XEPL::TEXT::Wire::Wire ( TEXT::Cord* _string )
	: ATOM::Atom()
	, wire_lock     ( new THREAD::Mutex() )
	, wire_string ( new TEXT::String ( _string ) )
{}


bool XEPL::TEXT::Wire::Append ( TEXT::Wire* _bag )
{
	THREAD::MutexScope lock_string ( wire_lock );

	if ( !_bag )
		return false;

	bool was_empty = wire_string->size() == 0;

	THREAD::MutexScope lockTheirsToo ( _bag->wire_lock );

	wire_string->append ( *_bag->wire_string );

	return was_empty && wire_string->size() > 0;
}

bool XEPL::TEXT::Wire::Append ( const char* _text, size_t _size )
{
	THREAD::MutexScope lock_string ( wire_lock );

	bool was_empty = wire_string->size() == 0;

	wire_string->append ( _text, _size );

	return was_empty && wire_string->size() > 0;
}

bool XEPL::TEXT::Wire::Append ( TEXT::Cord* _str )
{
	THREAD::MutexScope lock_string ( wire_lock );

	bool was_empty = wire_string->size() == 0;

	wire_string->append ( *_str );

	return was_empty && wire_string->size() > 0;
}


void XEPL::TEXT::Wire::Assign ( TEXT::Cord* _str )
{
	if ( !_str )
		return;

	THREAD::MutexScope lock_string ( wire_lock );
	wire_string->assign ( *_str );
}

void XEPL::TEXT::Wire::Assign ( const char* _str )
{
	if ( !_str )
		return;

	THREAD::MutexScope lock_string ( wire_lock );
	wire_string->assign ( _str );
}


size_t XEPL::TEXT::Wire::Avail ( void ) const
{
	THREAD::MutexScope lock_string ( wire_lock );
	return wire_string->size();
}


void XEPL::TEXT::Wire::Erase()
{
	THREAD::MutexScope lock_string ( wire_lock );
	wire_string->erase();
}


bool XEPL::TEXT::Wire::Expire ( size_t _length )
{
	THREAD::MutexScope lock_string ( wire_lock );
	wire_string->erase ( 0, _length );
	return wire_string->empty();
}


bool XEPL::TEXT::Wire::Extract_Line ( XEPL::TEXT::String* _into )
{
	XEPL::THREAD::MutexScope lock ( wire_lock );

	if ( wire_string->empty() )
		return false;

	size_t offset = wire_string->find ( '\n', 0 );

	if ( offset != XEPL::TEXT::String::npos )
	{
		size_t eol = offset;

		if ( wire_string->at ( eol-1 )=='\r' )
			eol--;

		_into->append ( wire_string->c_str(), eol );
		wire_string->erase ( 0, offset+1 );
	}
	else
	{
		_into->append ( wire_string->c_str() );
		wire_string->erase ( 0, wire_string->size() );
	}

	return true;
}


void XEPL::TEXT::Wire::Print ( TEXT::String* _str ) const
{
	if ( !_str )
		return;

	THREAD::MutexScope lock_string ( wire_lock );
	_str->append ( *wire_string );
}


XEPL::XML::XeplXml::XeplXml ( TEXT::String* _string )
	: XmlParser ( _string, true )
{
	ParseIt();
}

bool XEPL::XML::XeplXml::Xml__New_Element ( DNA::Gene* _gene )
{
	return _gene->cell_name->front() != '_';
}


XEPL::XML::XmlBag::XmlBag ( TEXT::Cord* _bag, XML::XmlParser* _parser )
	: TEXT::ParserBag ( _bag, _parser )
	, xml_parser      ( _parser )
{}


void XEPL::XML::XmlBag::Discard_Shell_Directive()
{
	if ( tip_char=='#' )
	{
		Next();

		while ( !xml_parser->error_string
		        && Remaining() >= 1
		        && ( tip_char != '\n' && tip_char != '\r' ) )
			Next();
	}
}


void XEPL::XML::XmlBag::Extract_Attribute_Name ( TEXT::String* _into )
{
	const char* start = current_position;
	const char* position = start;

	if ( at_xmlTag() )
	{
		++position;

		while ( in_xmlTag ( *position ) )
			++position;

		_into->append ( start, position-start );

		current_position = position;
		tip_char         = *position;
	}
	else
		xml_parser->Record_Error ( "Not at attribute tag: ", start );
}


void XEPL::XML::XmlBag::Extract_CData ( TEXT::String* _into )
{
	long length = 0;
	const char* start = nullptr;

	if ( tip_char != 'C'          ||
	        * ( current_position+1 ) != 'D' ||
	        * ( current_position+2 ) != 'A' ||
	        * ( current_position+3 ) != 'T' ||
	        * ( current_position+4 ) != 'A' ||
	        * ( current_position+5 ) != '[' )
		xml_parser->Record_Error ( "Invalid CDATA start", current_position );
	else
		Nudge ( 6 );

	start = current_position+1;

	while
	( !xml_parser->error_string && Remaining() >= 2 &&
	        ! ( * ( ++current_position ) == ']' &&
	            * ( current_position+1 ) == ']' &&
	            * ( current_position+2 ) == '>' ) )
		++length;

	Nudge ( 3 );
	Skip_Whitespace();

	if ( length && _into )
	{
		_into->assign ( start, length );
		TEXT::String scratch;

		if ( SOMA::Escape_Quotes ( _into, &scratch ) != _into )
			_into->assign ( scratch );
	}
}


void XEPL::XML::XmlBag::Extract_Comment ( TEXT::String* _into )
{
	const char* start = current_position;

	while ( tip_char && !Consume ( '-', '-', '>' ) )
		Next();

	if ( _into )
		_into->append ( start, current_position-start-3 );
}


void XEPL::XML::XmlBag::Extract_DocType ( TEXT::String* _into )
{
	if ( tip_char != 'O'           ||
	        * ( current_position+1 ) != 'C' ||
	        * ( current_position+2 ) != 'T' ||
	        * ( current_position+3 ) != 'Y' ||
	        * ( current_position+4 ) != 'P' ||
	        * ( current_position+5 ) != 'E' )
		xml_parser->Record_Error ( "Invalid DOCTYPE start", current_position );

	Nudge ( 6 );
	const char* start = current_position;
	int nest  = 1;

	while ( tip_char && !xml_parser->error_string && nest )
	{
		if ( tip_char == '<' )
			++nest;
		else if ( tip_char == '>' )
		{
			if ( !--nest )
				break;
		}

		Next();
	}

	if ( _into )
		_into->assign ( start, current_position-start );

	Skip_Whitespace();
}


void XEPL::XML::XmlBag::Extract_Instruction ( TEXT::String* _into )
{
	const char* start = current_position;

	while ( !xml_parser->error_string && Remaining() >= 2 && !Consume ( '?', '>' ) )
		Next();

	if ( _into )
	{
		_into->append ( start, current_position-start );
		xml_parser->Perform_Instruction ( _into );
	}
}


void XEPL::XML::XmlBag::Extract_PCData ( TEXT::String* _into )
{
	const char* start    = current_position;
	const char* position = start;

	while ( in_xmlPCData ( *position ) )
		++position;

	current_position = position;
	tip_char         = *position;

	const char* tail = position-1;

	while ( iswspace ( *tail ) )
		--tail;

	if ( _into )
		_into->append ( start, tail-start+1 );

}


char XEPL::XML::XmlBag::Extract_Quoted_Value ( TEXT::String* _into )
{
	char quote           = tip_char;
	const char* start    = ++current_position;
	const char* position = start;

	while ( !xml_parser->error_string && *position && *position != quote )
		++position;

	if ( *position == quote )
	{
		_into->append ( start, position-start );

		current_position = position;
		tip_char = *++current_position;

		return quote;
	}

	current_position = position;
	tip_char = *current_position;
	xml_parser->Record_Error ( "Missing closing quote: ", start );
	return 0;
}


XEPL::TEXT::Cord* XEPL::XML::XmlBag::Extract_Space_Tag ( TEXT::String* _name )
{
	if ( !at_xmlTag() )
	{
		xml_parser->Record_Error ( "Not at tag", current_position );
		return nullptr;
	}

	const char* start = current_position;
	const char* colonpos = nullptr;
	const char* position = start;

	while ( in_xmlTag ( *position ) )
	{
		if ( *position == ':' )
			colonpos = position;

		++position;
	}

	current_position = position;
	tip_char         = *position;

	if ( colonpos )
	{
		_name->append ( colonpos + 1, position - colonpos - 1 );
		return new TEXT::Cord ( start, colonpos - start );
	}
	else
	{
		_name->append ( start, position - start );
		return nullptr;
	}
}


XEPL::XML::XmlBuilder::~XmlBuilder()
{
	if ( attributes_closed )
	{
		build_string->append ( "</" );
		build_string->append ( *tag_n_space );
	}
	else
		build_string->push_back ( '/' );

	build_string->push_back (  '>' );
	delete tag_n_space;
}

XEPL::XML::XmlBuilder::XmlBuilder ( const char* _name, const char* _attrs, TEXT::String* _bag )
	: tag_n_space         ( new TEXT::String ( _name ) )
	, build_string  ( _bag )
	, attributes_closed   ( true )
{
	build_string->push_back (  '<' );
	build_string->append ( *tag_n_space );
	build_string->push_back ( ' ' );
	build_string->append ( _attrs );
	build_string->push_back (  '>' );
}

XEPL::XML::XmlBuilder::XmlBuilder ( const char* _name, TEXT::String* _bag )
	: tag_n_space         ( new TEXT::String ( _name ) )
	, build_string  ( _bag )
	, attributes_closed   ( false )
{
	build_string->push_back (  '<' );
	build_string->append ( *tag_n_space );
}

XEPL::XML::XmlBuilder::XmlBuilder ( TEXT::Cord* _name, TEXT::String* _bag, TEXT::Cord* _space )
	: tag_n_space         ( new TEXT::String ( _space ) )
	, build_string  ( _bag )
	, attributes_closed   ( false )
{
	if ( _space )
		tag_n_space->push_back (  ':' );

	tag_n_space->append ( *_name );
	build_string->push_back (  '<' );
	build_string->append ( *tag_n_space );
}


void XEPL::XML::XmlBuilder::Absorb_Attributes ( CELL::Nucleus* _nucleus, DNA::Gene* _gene )
{
	if ( !_gene )
		return;

	if ( !_gene->traits )
		return;

	TEXT::Cord*   name = nullptr;
	TEXT::String* term = nullptr;
	TEXT::String result;

	DNA::StableRecall stable_gene ( _gene, false );

	while ( stable_gene.Next_Trait ( &name, &term ) )
	{
		result.clear();
		RNA::Evaluate_Script ( _nucleus, _gene, term, &result );
		Attribute_Set ( name, &result );
	}
}


void XEPL::XML::XmlBuilder::Attribute_Set ( const char* _name, const char* _term )
{
//	assert ( !attributes_closed );
	build_string->push_back ( ' ' );
	build_string->append ( _name );
	build_string->push_back ( '=' );

	TEXT::String term ( _term );
	SOMA::Escape_Quotes ( &term, build_string );
}

void XEPL::XML::XmlBuilder::Attribute_Set ( TEXT::Cord* _name, TEXT::Cord* _term )
{
//	assert ( !attributes_closed );
	build_string->push_back (  ' ' );
	build_string->append ( *_name );
	build_string->push_back (  '=' );

	SOMA::Escape_Quotes ( _term, build_string );
}


XEPL::TEXT::String* XEPL::XML::XmlBuilder::Close_Traits()
{
	if ( attributes_closed )
		return build_string;

	attributes_closed = true;

	build_string->push_back ( '>' );
	return build_string;
}


XEPL::XML::XmlNode::~XmlNode()
{
	xml_parser->active_node = parent_node;

	if ( element_gene )
	{
		if ( --element_gene->retain_count < 1 )
			element_gene->Destroy();

		parser_wants_it = xml_parser->Xml__End_Element ( element_gene ) && parser_wants_it;

		if ( !parser_wants_it )
			xml_parser->Xml__Undo_Element ( element_gene );
	}
}

XEPL::XML::XmlNode::XmlNode ( XML::XmlParser* _parser )
	: xml_parser      ( _parser )
	, parent_node     ( nullptr )
	, element_gene    ( nullptr )
	, parser_wants_it ( true )
{
	DNA::Gene* parent = nullptr;

	if ( xml_parser->active_node )
		parent = xml_parser->active_node->element_gene;

	TEXT::String tag;
	TEXT::Cord* spacePtr = xml_parser->xml_bag->Extract_Space_Tag ( &tag );

	xml_parser->Xml__Make_Element ( parent, &tag, spacePtr, &element_gene );

	delete spacePtr;

	parser_wants_it  = xml_parser->Xml__New_Element ( element_gene );

	if ( !parent )
		xml_parser->root_gene = element_gene;

	parent_node = xml_parser->active_node;
	xml_parser->active_node = this;

	xml_parser->xml_bag->Skip_Whitespace();
}


XEPL::XML::XmlParser::XmlParser ( TEXT::Cord* _bag, bool _wait )
	: Parser()
	, xml_bag                        ( new XML::XmlBag ( _bag, this ) )
	, active_node                ( nullptr )
	, instructions               ( nullptr )
	, root_gene                  ( nullptr )
{
	parser_bag = xml_bag;

	if ( !_wait )
		ParseIt();
}

XEPL::XML::XmlParser::~XmlParser()
{
	if ( root_gene )
		root_gene->Release();

	delete xml_bag;
	delete instructions;
}


bool XEPL::XML::XmlParser::do_BeginNode()
{
	if ( xml_bag->Consume ( '<' ) )
	{
		xml_bag->Skip_Whitespace();
		XmlNode fresh ( this );

		while ( !error_string && xml_bag->at_xmlTag() )
		{
			TEXT::String spacetag;
			xml_bag->Extract_Attribute_Name ( &spacetag );
			xml_bag->Skip_Whitespace();

			if ( xml_bag->Discard_Char ( '=' ) )
			{
				xml_bag->Skip_Whitespace();
				TEXT::String quoted_term;
				char quote = xml_bag->Extract_Quoted_Value ( &quoted_term );

				if ( !error_string )
				{
					xml_bag->Skip_Whitespace();

					if ( Xml__New_Attribute ( &spacetag, &quoted_term, quote ) )
						fresh.element_gene->Trait_Set ( &spacetag, &quoted_term );
				}
			}
		}

		if ( !error_string && do_CloseNode() )
			return true;

		if ( !error_string && do_NodeSplit() )
			return true;

		Record_Error ( "open tag error: ", fresh.element_gene->cell_name->c_str() );
	}

	return false;
}


bool XEPL::XML::XmlParser::do_CData()
{
	if ( xml_bag->Consume ( '<', '!', '[' ) )
	{
		TEXT::String CData;
		xml_bag->Extract_CData ( &CData );
		active_node->element_gene->Content_Append ( &CData );
		return true;
	}

	return false;
}


bool XEPL::XML::XmlParser::do_CloseNode()
{
	if ( xml_bag->Consume ( '/', '>' ) )
	{
		xml_bag->Skip_Whitespace();
		return true;
	}

	return false;
}


bool XEPL::XML::XmlParser::do_CloseSplit()
{
	if ( xml_bag->Consume ( '<', '/' ) )
	{
		if ( xml_bag->Consume ( '>' ) )
			return true;

		if ( xml_bag->tip_char == '/' )
		{
			xml_bag->Rewrite ( '<' );
			return true;
		}

		xml_bag->Skip_Whitespace();

		TEXT::String name;
		TEXT::Cord* spaceptr = xml_bag->Extract_Space_Tag ( &name );

		xml_bag->Skip_Whitespace();
		xml_bag->Discard_Char ( '>' );
		xml_bag->Skip_Whitespace();

		if ( active_node->element_gene->cell_name->compare ( name ) != 0
		        || ( active_node->element_gene->space_string
		             && active_node->element_gene->space_string->compare ( *spaceptr ) != 0 ) )
			Record_Error ( "Bad closing tag:", name.c_str() );

		delete spaceptr;

		return true;
	}

	return false;
}


bool XEPL::XML::XmlParser::do_Comment()
{
	if ( xml_bag->Consume ( '<', '!', '-', '-' ) )
	{
		TEXT::String comment;
		xml_bag->Extract_Comment ( &comment );
		Xml__New_Comment ( &comment );
		xml_bag->Skip_Whitespace();
		return true;
	}

	return false;
}


bool XEPL::XML::XmlParser::do_Content()
{
	if ( xml_bag->at_xmlPCData() )
	{
		active_node->element_gene->Make_Content_Wire();
		xml_bag->Extract_PCData ( active_node->element_gene->content_wire->wire_string );

		if ( !Xml__New_Content ( active_node->element_gene->content_wire->wire_string ) )
			active_node->element_gene->Content_Drop();

		return true;
	}

	return false;
}


bool XEPL::XML::XmlParser::do_DocType()
{
	if ( xml_bag->Consume ( '<', '!', 'D' ) )
	{
		TEXT::String doctype;
		xml_bag->Extract_DocType ( &doctype );
		Xml__DocType ( &doctype );
		return true;
	}

	return false;
}


bool XEPL::XML::XmlParser::do_Instruction()
{
	if ( xml_bag->Consume ( '<', '?' ) )
	{
		TEXT::String instruction;
		xml_bag->Extract_Instruction ( &instruction );
		Perform_Instruction ( &instruction );
		xml_bag->Skip_Whitespace();
		return true;
	}

	return false;
}


bool XEPL::XML::XmlParser::do_NodeSplit ( void )
{
	if ( xml_bag->Consume ( '>' ) )
	{
		xml_bag->Skip_Whitespace();
		TEXT::ParserSelect choices ( this );
		choices.Add_Option (  TEXT::ParserSelect::Completes, [] ( TEXT::Parser* _p )
		                      /* "</tag >" */
		{
			return static_cast<XmlParser*> ( _p )->do_CloseSplit();
		}   );

		choices.Add_Option (  TEXT::ParserSelect::Can_Repeat, [] ( TEXT::Parser* _p )
		                      /* "<!-- -->" */
		{
			return static_cast<XmlParser*> ( _p )->do_Comment();
		}      );

		choices.Add_Option (  TEXT::ParserSelect::Can_Repeat, [] ( TEXT::Parser* _p )
		                      /* "<?.?>" */
		{
			return static_cast<XmlParser*> ( _p )->do_Instruction();
		}  );

		choices.Add_Option (  TEXT::ParserSelect::No_Options, [] ( TEXT::Parser* _p )
		                      /* "<![CDATA[ ]]>" */
		{
			return static_cast<XmlParser*> ( _p )->do_CData();
		}        );

		choices.Add_Option (  TEXT::ParserSelect::Can_Repeat, [] ( TEXT::Parser* _p )
		                      /* "<tag>" */
		{
			return static_cast<XmlParser*> ( _p )->do_BeginNode();
		}    );

		choices.Add_Option (  TEXT::ParserSelect::Can_Repeat, [] ( TEXT::Parser* _p )
		                      /* >text< */
		{
			return static_cast<XmlParser*> ( _p )->do_Content();
		}      );

		return true;
	}

	return false;
}


void XEPL::XML::XmlParser::Xml__Make_Element ( DNA::Gene* _host, TEXT::Cord* _name, TEXT::Cord* _space, DNA::Gene** __gene )
{
	DNA::Gene* gene = new DNA::Gene ( _host, _name, _space );

	if ( !_host )
		++gene->retain_count;

	*__gene = gene;
}

void XEPL::XML::XmlParser::Xml__Undo_Element ( DNA::Gene* _gene )
{
	if ( _gene && _gene->gene_owner )
		_gene->gene_owner->Remove_Gene ( _gene );
}


void XEPL::XML::XmlParser::ParseIt ( void )
{
	if ( !xml_bag->xml_parser )
		return;

	xml_bag->Skip_Whitespace();
	xml_bag->Discard_Shell_Directive();
	xml_bag->Skip_Whitespace();

	while ( !error_string && ( do_Comment () || do_Instruction () ) )
	{}

	do_DocType ();

	while ( !error_string && ( do_Comment () || do_Instruction () ) )
	{}

	do_BeginNode();

	while ( !error_string && ( do_Comment () || do_Instruction () ) )
	{}

	if ( error_string )
		SOMA::ErrorReport ( this->error_string );
}


void XEPL::XML::XmlParser::Perform_Instruction ( TEXT::Cord* _instr )
{
	if ( !instructions )
		return;

	TEXT::String name ( _instr );
	name.Split_ch_rhs ( ' ', nullptr );

	const auto it = instructions->find ( name );

	if ( it !=  instructions->end() )
	{
		XML::xmlInstruction instruction= it->second;
		( *instruction ) ( &name, _instr );
	}
}


void XEPL::XML::XmlParser::Register_Instruction ( TEXT::Cord* _name, xmlInstruction _instruction )
{
	if ( !instructions )
		instructions = new XmlIntsructionMap();

	( *instructions )[ *_name ] = _instruction;
}


static void MemoryCounts_Reset  ( void );
static int  MemoryCounts_Report ( std::ostream* );

XEPL::MEMORY::MemoryCounts::~MemoryCounts()
{
	MemoryCounts_Report ( std__ostream );
}

XEPL::MEMORY::MemoryCounts::MemoryCounts ( std::ostream* _ostream )
	: std__ostream ( _ostream )
{
	MemoryCounts_Reset();
}

void MemoryCounts_Reset ( void )
{
	XEPL::MEMORY::COUNTERS::num_total_dels = 0;
	XEPL::MEMORY::COUNTERS::num_total_news = 0;
}

int MemoryCounts_Report ( std::ostream* _report )
{
	std::string new_count;
	std::string del_count;

	size_t _news = XEPL::MEMORY::COUNTERS::num_total_news;
	size_t _dels = XEPL::MEMORY::COUNTERS::num_total_dels;

	long leaking = _news-_dels;

	if ( _report && _news && ( XEPL::Show_Memory_Counts || leaking ) )
	{
		XEPL::SOMA::Long_Commafy(_news, &new_count);
		XEPL::SOMA::Long_Commafy(_dels, &del_count);

		*_report << "Other news "<< new_count << " deletes " << del_count << XEPL::EOL << std::flush;
	}

	if ( leaking )
		std::cerr << " ---LEAKING new/delete: " << leaking << XEPL::EOL << std::flush;

	return ( int )leaking;
}


std::atomic_size_t XEPL::MEMORY::COUNTERS::num_total_news = 0;
std::atomic_size_t XEPL::MEMORY::COUNTERS::num_total_dels = 0;

#ifdef COUNT_NEWS

void* operator new ( size_t _size )
{
	THREAD_COUNTER ( count_news )

	++XEPL::MEMORY::COUNTERS::num_total_news;
	return malloc ( _size );
}

void operator delete ( void* _ptr ) throw()
{
	if ( !_ptr )
		return;

	THREAD_COUNTER ( count_dels )

	++XEPL::MEMORY::COUNTERS::num_total_dels;
	free ( _ptr );
}

#endif // COUNT_NEWS


namespace XEPL::SOMA
{
	using mapOfCounts = MAPS::MapT<TEXT::Cord, long> ;

	static THREAD::Mutex   counts_lock;
	static mapOfCounts     error_counts;
}

bool XEPL::SOMA::ErrorReport::capture = false;

XEPL::SOMA::ErrorReport::ErrorReport ( const char* _text )
	: String ( _text )
	, report ( true )
{}

XEPL::SOMA::ErrorReport::ErrorReport ( TEXT::Cord* _text )
	: String ( _text )
	, report ( true )
{}

XEPL::SOMA::ErrorReport::~ErrorReport()
{
	if ( !report )
		return;

	if ( capture )
	{
		THREAD::MutexScope lock_error_counts ( &counts_lock );

		auto it  = error_counts.find ( *this );

		if ( it != error_counts.end() )
		{
			++ it->second;
			return;
		}

		error_counts[*this] = 1;
	}

	SOMA::ezError ( c_str() );
}

void XEPL::SOMA::ErrorReport::finalReport ( TEXT::String* _into )
{
	THREAD::MutexScope lock_error_counts ( &counts_lock );

	if ( error_counts.empty() )
		return;

	TEXT::String count;
	auto in = error_counts.begin();

	while ( in != error_counts.end() )
	{
		count.assign ( std::to_string ( ( *in ).second ) );
		_into->append (  ( *in ).first );
		_into->push_back ( ',' );
		_into->push_back ( ' ' );
		_into->append ( count );
		_into->append ( XEPL::EOL );
		++in;
	}
}

void XEPL::SOMA::ErrorReport::Flush()
{
	error_counts.clear();
}

XEPL::TEXT::String* XEPL::SOMA::ErrorReport::operator ->()
{
	return this;
}

XEPL::SOMA::ErrorReport& XEPL::SOMA::ErrorReport::operator+ ( const char* _txt )
{
	if ( _txt )
		append ( _txt );

	return *this;
}

XEPL::SOMA::ErrorReport& XEPL::SOMA::ErrorReport::operator+ ( TEXT::String* _str )
{
	append ( *_str );
	return *this;
}


namespace XEPL::KEYWORDS
{
	void Keyword_Lobe CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			if ( !_nucleus || !_call_gene )
				return;

			if ( _call_gene->content_wire )
				XEPL::RNA::RnaScript ( _nucleus->Nucleus__Host_Neuron(), _call_gene );

			XEPL::CORTEX::Lobe* lobe = new XEPL::CORTEX::Lobe ( _nucleus->Nucleus__Host_Neuron(), _call_gene );

			lobe->Start_Lobe ( _call_gene->Trait_Get_bool ( "wait", true ) );
		} );
	}
}


bool XEPL::Show_Collisions     = false;
bool XEPL::Show_Trace          = false;
bool XEPL::Show_Memory_Counts  = false;
bool XEPL::Show_Counters       = false;
bool XEPL::Show_Final_Report   = false;
bool XEPL::alive               = false;

thread_local XEPL::CORTEX::Lobe* XEPL::tlsLobe = nullptr;

const char* XEPL::EOL   = "\r\n";


void XEPL::TRACE(const char* action, XEPL::CORTEX::Neuron* neuron, XEPL::TEXT::Cord* name)
{
	if(XEPL::Show_Trace)
	{
		XEPL::TEXT::String path;
		std::string dur;
		std::cout
		<< *KITS::TIMER::runtime->Time(&dur)
		<< std::setw(12) << ( XEPL::tlsLobe ? *XEPL::tlsLobe->cell_name : "" )
		<< ": " << std::setw(10) << action
		<< ": " << ( neuron ? neuron->Feature_Get("path", &path)->c_str()  : "" )
		<< "."  << *name << XEPL::EOL;

	}
}


static std::atomic_long errorCount ( 0 );

void XEPL::SOMA::OffBalance ( void )
{
	SOMA::ErrorReport ( "Detach off balance" );
}

void XEPL::SOMA::ezError ( const char* _text )
{
	++errorCount;
	std::cout << "ERROR:" << errorCount << ": " << _text << XEPL::EOL << std::flush;
}

void XEPL::SOMA::xeplError ( const char* _type, CELL::Nucleus* _nucleus, const char* _name )
{
	TEXT::String temp;

	if ( _nucleus )
	{
		_nucleus->Path ( &temp, "/" );
		temp.push_back (  ' ' );
	}

	SOMA::ErrorReport ( "Error " )+_type+": " +&temp+_name;
}

void XEPL::SOMA::xeplCantFind ( const char* _type, CELL::Nucleus* _nucleus, const char* _name )
{
	TEXT::String temp;

	if ( _nucleus )
	{
		_nucleus->Path ( &temp, "/" );
		temp.push_back (  ' ' );
	}

	SOMA::ErrorReport ( "Can't find " )+_type+": "+&temp +_name;
}


XEPL::OS::osFd::~osFd()
{
	delete mutex;
	delete socket_name;

	if ( receiving )
		receiving->Release();

	if ( backpressure )
		backpressure->Release();
}

XEPL::OS::osFd::osFd ( void )
	: ATOM::Atom()
	, socket_name      ( nullptr )
	, receiving        ( nullptr )
	, eReceived        ( nullptr )
	, socket_manager   ( nullptr )
	, backpressure     ( nullptr )
	, mutex            ( nullptr )
{}


XEPL::OS::osFdPair::osFdPair()
	: descriptor_set             ( new SetInts() )
	, handler_map                ( new XEPL::OS::HandlerMap() )
	, watch_fds                  ( new XEPL::OS::osFdSelect() )
	, active_fds                 ( new XEPL::OS::osFdSelect() )
	, num_fds                    ( 0 )
	, max_fd                     ( 0 )
{}

XEPL::OS::osFdPair::~osFdPair()
{
	delete watch_fds;
	delete active_fds;
	delete descriptor_set;
	delete handler_map;
}


fd_set* XEPL::OS::osFdPair::Active_Fds() const
{
	if ( !num_fds )
		return nullptr;

	watch_fds->Duplicate_Into ( active_fds );

	return active_fds->os_fd_set;
}


int XEPL::OS::osFdPair::Deliver_Fd ( int result )
{
	if ( !result )
		return 0;

	return active_fds->Deliver_Fds ( this, result );
}


void XEPL::OS::osFdPair::Ignore_Fd ( XEPL::OS::osFd* _socket )
{
	int fd = _socket->osFd__Descriptor();

	const auto it = handler_map->find ( fd );

	if ( it != handler_map->end() )
	{
		--num_fds;

		descriptor_set->erase ( fd );
		watch_fds->Clear_Fd ( fd );
		active_fds->Clear_Fd ( fd );
		handler_map->erase ( it );
	}

	if ( num_fds )
		max_fd = * ( descriptor_set->rbegin() );
	else
		max_fd = 0;
}


int XEPL::OS::osFdPair::Max_Fd ( int _fd ) const
{
	return max_fd > _fd ? max_fd : _fd;
}


void XEPL::OS::osFdPair::Set_Fd_Handler ( XEPL::OS::osFd* _os_fd, XEPL::OS::fdHandler _handler )
{
	int fd = _os_fd->osFd__Descriptor();

	if ( fd < 0 )
		return;

	const auto it = handler_map->find ( fd );

	if ( it != handler_map->end() )
		return;

	( *handler_map )[ fd ] = std::make_pair ( _os_fd, _handler );
	watch_fds->Set_Fd ( fd );
	descriptor_set->insert ( fd );

	++num_fds;
	max_fd = * ( descriptor_set->rbegin() );
}


XEPL::OS::osFdSelect::~osFdSelect ()
{
	delete os_fd_set;
}

XEPL::OS::osFdSelect::osFdSelect ( void )
	: os_fd_set ( new struct fd_set() )
{
	FD_ZERO ( os_fd_set );
}


void XEPL::OS::osFdSelect::Clear_Fd ( int _fd )
{
	FD_CLR ( _fd, os_fd_set );
}


int XEPL::OS::osFdSelect::Deliver_Fds ( osFdPair* _pair, int _num_fds )
{
	int delivered = 0;
	int fd = _pair->max_fd;

	while ( _num_fds && ( fd >= 0 ) )
	{
		if ( _pair->active_fds->Is_Fd_Ready ( fd ) )
		{
			--_num_fds;

			auto it  = _pair->handler_map->find ( fd );
			if ( it != _pair->handler_map->end() )
			{
				if ( ! ( it->second.first->*it->second.second )() )
					_pair->Ignore_Fd ( it->second.first );

				++delivered;
			}
			else
				; // hmmm?
		}

		fd--;
	}

	return delivered;
}


void XEPL::OS::osFdSelect::Duplicate_Into ( osFdSelect* _set ) const
{
	memcpy ( _set->os_fd_set->fds_bits, os_fd_set->fds_bits, sizeof ( os_fd_set->fds_bits ) );
}


bool XEPL::OS::osFdSelect::Is_Fd_Ready ( int _fd ) const
{
	return FD_ISSET ( _fd, os_fd_set ) != 0;
}


void XEPL::OS::osFdSelect::Set_Fd ( int _fd )
{
	FD_SET ( _fd, os_fd_set );
}


XEPL::OS::osFdSet::osFdSet()
	: read_pair   ( new XEPL::OS::osFdPair() )
	, write_pair  ( new XEPL::OS::osFdPair() )
	, except_pair ( new XEPL::OS::osFdPair() )
{}

XEPL::OS::osFdSet::~osFdSet()
{
	delete except_pair;
	delete write_pair;
	delete read_pair;
}


void XEPL::OS::osFdSet::Ignore ( XEPL::OS::osFd* _fd, XEPL::OS::osFdPair* _pair )
{
	_pair->Ignore_Fd ( _fd );
}


void XEPL::OS::osFdSet::Wait_On_Selected()
{
	int max_fd = read_pair->Max_Fd ( write_pair->Max_Fd ( except_pair->Max_Fd ( 0 ) ) );
	int result = ::select ( max_fd+1, read_pair->Active_Fds(), write_pair->Active_Fds(), except_pair->Active_Fds(), 0 );

	if ( result == -1 )
		return;

	result -= read_pair->Deliver_Fd ( result );
	result -= write_pair->Deliver_Fd ( result );
	result -= except_pair->Deliver_Fd ( result );

//	assert ( !result );
}


bool XEPL::OS::osFd::Has_Backpressure() const
{
	if ( !mutex )
		return false;

	XEPL::THREAD::MutexScope lock_backpressure ( mutex );

	if ( !backpressure )
		return false;

	return backpressure->Avail() > 0;
}


void XEPL::OS::osFd::Hold_Data_Back ( XEPL::TEXT::Wire* _wire )
{
	if ( !mutex )
		mutex = new XEPL::THREAD::Mutex;

	XEPL::THREAD::MutexScope lock_backpressure ( mutex );

	if ( !backpressure )
		backpressure = new XEPL::TEXT::Wire();

	backpressure->wire_string->append ( *_wire->wire_string );

	_wire->wire_string->clear ();
}


void XEPL::OS::osFd::Receive_From_Fd ( const char* _buffer, long _length, bool _more )
{
	if ( !receiving )
	{
		if ( !socket_name )
		{
			socket_name = new XEPL::TEXT::String ( "fd_" );
			socket_name->append ( std::to_string ( osFd__Descriptor() ) );
		}

		receiving = new XEPL::DNA::Gene ( nullptr, socket_name );
		receiving->Make_Content_Wire();
	}

	receiving->content_wire->Append ( _buffer, _length );

	if ( !_more )
	{
		XEPL::ATOM::Scope_Release detach ( receiving );

		eReceived->Trigger ( receiving );
		receiving = nullptr;
	}
}


void XEPL::OS::osFd::Set_Data_Axon ( XEPL::SIGNAL::Axon* _axon )
{
	eReceived = _axon;
}


bool XEPL::OS::osFd::Set_Manager ( XEPL::CORTEX::Neuron* _neuron )
{
	XEPL::CORTEX::Neuron* found = nullptr;

	if ( _neuron->Find_Neuron ( "SocketMan", &found ) )
		socket_manager = static_cast<KITS::SOCKET::SocketMan*> ( found );
	else
		SOMA::ErrorReport ("Can't locate SocketMan");

	return found != nullptr;
}


XEPL::OS::osSocket::~osSocket()
{
	Close_Socket();
	delete socket_address;
}

XEPL::OS::osSocket::osSocket ( KITS::SOCKET::Socket* _socket )
	: osFd ()
	, socket_fd      ( 0 )
	, socket_address ( nullptr )
	, socket_neuron  ( _socket )
{}

XEPL::OS::osSocket::osSocket ( KITS::SOCKET::Socket* _socket, int _fd, XEPL::OS::osSocketAddress* _address )
	: osFd ()
	, socket_fd      ( _fd )
	, socket_address ( _address )
	, socket_neuron  ( _socket )
{}


XEPL::OS::osSocketAddress::~osSocketAddress()
{
	delete sockAddr;
	delete ip_string;
}

XEPL::OS::osSocketAddress::osSocketAddress ( void )
	: sockAddr                   ( new sockaddr_in() )
	, ip_string                  ( new XEPL::TEXT::String() )
{
	sockAddr->sin_family  = AF_INET;
	sockAddr->sin_port = 0;
	sockAddr->sin_addr.s_addr = 0;
	::memset ( sockAddr->sin_zero, 0, sizeof ( sockAddr->sin_zero ) );
}

XEPL::OS::osSocketAddress::osSocketAddress ( XEPL::DNA::Gene* _config )
	: sockAddr                   ( new sockaddr_in() )
	, ip_string                  ( new XEPL::TEXT::String() )
{
	sockAddr->sin_family  = AF_INET;
	sockAddr->sin_port = 0;
	sockAddr->sin_addr.s_addr = 0;
	memset ( sockAddr->sin_zero, 0, sizeof ( sockAddr->sin_zero ) );

	XEPL::TEXT::String* node = _config->Trait_Tap ( "node", "" );

	if ( node->empty() )
		Hostname ( node );

	XEPL::TEXT::String host ( node );
	XEPL::TEXT::String service;
	host.Split_ch_rhs ( ':', &service );

	if ( host.empty() )
		host.assign ( "127.0.0.1" );

	::in_addr os_in_addr;

	if ( ::inet_pton ( AF_INET, host.c_str(), &os_in_addr ) == 1 )
	{
		sockAddr->sin_family = AF_INET;
		sockAddr->sin_port  = htons ( static_cast<int> ( std::strtol ( service.c_str(), nullptr, 0 ) ) );
		sockAddr->sin_addr = os_in_addr;
		return;
	}

	::hostent* host_entry = ::gethostbyname ( host.c_str() );

	if ( host_entry )
	{
		::in_addr** addrList = reinterpret_cast<::in_addr**> ( host_entry->h_addr_list );
		sockAddr->sin_family = AF_INET;
		sockAddr->sin_port  = htons ( std::strtol ( service.c_str(), nullptr, 0 ) );
		::memcpy ( &sockAddr->sin_addr, *addrList, sizeof ( ::in_addr ) );
		return;
	}
}


::sockaddr* XEPL::OS::osSocketAddress::Get ( void ) const
{
	return reinterpret_cast<::sockaddr*> ( sockAddr );
}


void XEPL::OS::osSocketAddress::Hostname ( XEPL::TEXT::String* _hostname )
{
	if ( !_hostname )
		return;

	char selfNode[_POSIX_HOST_NAME_MAX] = {0};
	::gethostname ( selfNode, sizeof ( selfNode ) );
	_hostname->assign ( selfNode );
}


XEPL::TEXT::Cord* XEPL::OS::osSocketAddress::IpString ( void )
{
	if ( ip_string->empty() )
	{
		ip_string->assign ( ::inet_ntoa ( sockAddr->sin_addr ) );
		ip_string->append ( ":" );
		ip_string->append ( std::to_string ( Port() ) );
	}

	return ip_string;
}


long XEPL::OS::osSocketAddress::Length ( void ) const
{
	return sizeof ( ::sockaddr );
}


long XEPL::OS::osSocketAddress::Port ( void ) const
{
	return static_cast<long> ( ntohs ( sockAddr->sin_port ) );
}


#define SOC_MSG "wake"
#define SOC_LEN 4

XEPL::OS::osSocketControl::osSocketControl ( KITS::SOCKET::SocketControl* _socket )
	: osSocket      ( _socket )
	, controlSocket ( _socket )
{
	Build_Socket ( SOCK_DGRAM );
	Bind_Socket ( false );
}

void XEPL::OS::osSocketControl::Send()
{
	const XEPL::OS::osSocketAddress* addr = controlSocket->SocketAddress();

	::socklen_t len = static_cast<::socklen_t> ( addr->Length() );
	::sendto ( socket_fd, SOC_MSG, SOC_LEN, 0, addr->Get(), len );
}

bool XEPL::OS::osSocketControl::Fd_Receive()
{
	char  recv_msg[2*SOC_LEN] = {0};
	const XEPL::OS::osSocketAddress* addr = controlSocket->SocketAddress();

	::socklen_t len = ( ::socklen_t )addr->Length();
	::recvfrom ( socket_fd, &recv_msg, SOC_LEN, 0, addr->Get(), &len );

	return true;
}

XEPL::OS::fdHandler XEPL::OS::osSocketControl::osFd__OnFdRead() const
{
	return ( XEPL::OS::fdHandler )&osSocketControl::Fd_Receive;
}


void XEPL::OS::osSocket::Bind_Socket ( bool _reuseAddr )
{
	int optValue =	_reuseAddr ? 1 : 0;
	::socklen_t  length = static_cast<::socklen_t> ( socket_address->Length() );

	::setsockopt  ( socket_fd, SOL_SOCKET, SO_REUSEADDR, &optValue, sizeof ( int ) );
	::bind        ( socket_fd, socket_address->Get(), length );
	::getsockname ( socket_fd, socket_address->Get(), &length );
}


void XEPL::OS::osSocket::Build_Socket ( int _sockType )
{
	if ( !socket_address )
	{
		socket_neuron->config_gene->Trait_Tap ( "node", "127.0.0.1:0" );
		socket_address = new XEPL::OS::osSocketAddress ( socket_neuron->config_gene );
	}

	socket_fd = ::socket ( AF_INET, _sockType, 0 );

	::fcntl ( socket_fd, F_SETFL, O_NONBLOCK |
	          ::fcntl ( socket_fd, F_GETFL, 0 ) );
}


void XEPL::OS::osSocket::Close_Socket ( void )
{
	if ( socket_fd )
	{
		::close ( socket_fd );
		socket_fd = 0;
	}
}


int XEPL::OS::osSocket::osFd__Descriptor ( void ) const
{
	return socket_fd;
}


XEPL::OS::osTcpClient::osTcpClient ( KITS::SOCKET::TcpClient* _client )
	: osTcpSocket     ( _client )
	, tcp_client      ( _client )
	, server_address  ( new XEPL::OS::osSocketAddress( _client->config_gene ) )
{}

XEPL::OS::osTcpClient::~osTcpClient ( void )
{
	delete server_address;
}

void XEPL::OS::osTcpClient::osFd_Connect ( void )
{
	const XEPL::OS::osSocketAddress* socketAddress = server_address;

	server_address->IpString();

	::connect ( socket_fd, socketAddress->Get(), ( ::socklen_t )socketAddress->Length() );
}

bool XEPL::OS::osTcpClient::osFd_Connected ( void )
{
	::socklen_t addressLength = ( ::socklen_t )server_address->Length();

	int result = ::getsockname ( osFd__Descriptor(), server_address->Get(), &addressLength );

	if ( !result )
		return tcp_client->Fd_Connecting();
	else
		return tcp_client->Fd_Excepting();
}

XEPL::OS::fdHandler XEPL::OS::osTcpClient::osFd__OnFdWrite() const
{
	return ( XEPL::OS::fdHandler )&osTcpClient::osFd_Connected;
}

bool XEPL::OS::osTcpClient::osFd_Excepted ( void )
{
	return tcp_client->Fd_Excepting();
}

XEPL::OS::fdHandler XEPL::OS::osTcpClient::osFd__OnFdExcept ( void ) const
{
	return ( XEPL::OS::fdHandler )&osTcpClient::osFd_Excepted;
}


XEPL::OS::osTcpServer::~osTcpServer()
{}

XEPL::OS::osTcpServer::osTcpServer ( KITS::SOCKET::TcpServer* _server )
	: osTcpSocket ( _server )
	, tcp_server  ( _server )
{}


bool XEPL::OS::osTcpServer::Fd_Is_Connecting ( void )
{
	XEPL::OS::osSocketAddress* socketAddress = new XEPL::OS::osSocketAddress();
	::socklen_t addrLen = static_cast<::socklen_t> ( socketAddress->Length() );

	int socket = ::accept ( socket_fd, socketAddress->Get(), &addrLen );

	if ( socket == -1 )
	{
		delete socketAddress;
		return true;
	}

	return tcp_server->SocketMan_Connecting ( socket, socketAddress );
}


void XEPL::OS::osTcpServer::Listen_For_Connections ( long _backlog )
{
	Bind_Socket ( true );
	::listen ( socket_fd, static_cast<int> ( _backlog ) );
}


XEPL::OS::osTcpSocket::osTcpSocket ( KITS::SOCKET::SocketTcp* _socket )
	: osSocket ( _socket )
	, soc_tcp                    ( _socket )
	, messages_sent              ( 0 )
	, messages_received          ( 0 )
	, bytes_sent                 ( 0 )
	, bytes_received             ( 0 )
{}

XEPL::OS::osTcpSocket::osTcpSocket ( KITS::SOCKET::SocketTcp* _socket, int _fd, XEPL::OS::osSocketAddress* _address )
	: osSocket  ( _socket, _fd, _address )
	, soc_tcp                    ( nullptr )
	, messages_sent              ( 0 )
	, messages_received          ( 0 )
	, bytes_sent                 ( 0 )
	, bytes_received             ( 0 )
{}


void XEPL::OS::osTcpSocket::Create_Tcp_Socket ( void )
{
	Build_Socket ( SOCK_STREAM );
}


bool XEPL::OS::osTcpSocket::Read_Available ( void )
{
	if ( !soc_tcp )
		return false;

	int  read_length = (int)MEMORY::Block8K;
	char read_buffer[read_length];

	while ( true )
	{
		int num_bytes_received = ( int )recv ( socket_fd, read_buffer, read_length, 0 );

		if ( num_bytes_received > 0 )
		{
			++messages_received;
			bytes_received += num_bytes_received;

			std::string scratch;
			
			SOMA::Long_Commafy(bytes_received, &scratch);
			socket_neuron->Property_Set( "b_recd", scratch.c_str() );

			SOMA::Long_Commafy(messages_received, &scratch);
			socket_neuron->Property_Set( "m_recd", scratch.c_str() );

			Receive_From_Fd ( read_buffer, num_bytes_received, num_bytes_received == read_length );

			if ( num_bytes_received < read_length )
				return true;
		}
		else
		{
			if ( !num_bytes_received )
			{
				soc_tcp->Closed_Normal();
				return false;
			}

			if ( errno == EAGAIN )
				return true;

			soc_tcp->Closed_Aborted();
			return false;
		}
	}
}


bool XEPL::OS::osTcpSocket::Send_Available ( void )
{
	if ( Has_Backpressure() )
	{
		XEPL::THREAD::MutexScope lock_backpressure ( mutex );

		const char* payload = backpressure->wire_string->data();
		size_t  send_length = backpressure->wire_string->size();

		int  num_bytes_sent = ( int )::send ( socket_fd, payload, send_length, 0 );

		if ( num_bytes_sent == -1 )
		{
			if ( errno == EAGAIN )
			{
				errno = 0;
				return true;
			}

			soc_tcp->Closed_Aborted();
			return false;
		}

		if ( backpressure->Expire ( num_bytes_sent ) )
		{
			backpressure->Release();
			backpressure = nullptr;
			return false;
		}

		return true;
	}

	return false;
}


void XEPL::OS::osTcpSocket::Send_Data ( XEPL::TEXT::Wire* _buffer )
{
	const char*   payload = _buffer->wire_string->data();
	size_t payload_length = _buffer->wire_string->size();

	int num_bytes_sent = ( int )::send ( socket_fd, payload, payload_length, 0 );

	if ( num_bytes_sent == -1 )
	{
		if ( errno != EAGAIN )
			return soc_tcp->Closed_Aborted();

		errno = 0;
		Hold_Data_Back ( _buffer );
		socket_manager->eWrite->Trigger ( this );
	}

	messages_sent++;
	bytes_sent+=num_bytes_sent;
	
	std::string scratch;

	SOMA::Long_Commafy(bytes_sent, &scratch);
	socket_neuron->Property_Set( "b_sent", scratch.c_str() );

	SOMA::Long_Commafy(messages_sent, &scratch);
	socket_neuron->Property_Set( "m_sent", scratch.c_str() );

	_buffer->wire_string->erase ( 0, num_bytes_sent );
}


void XEPL::OS::OS_Initialize ( void )
{
	errno = 0;
	signal ( SIGPIPE, SIG_IGN );
}

int XEPL::OS::OS_Shutdown ( void )
{
	if ( errno )
		perror ( "shutdown" );

	return 0;
}


XEPL::Brain::~Brain()
{
	delete commands_map;
	delete spine_map;
	delete organ_map;
}

XEPL::Brain::Brain ( const char* _name, bool _standard )
	: XEPL::CORTEX::Cortex       ( _name, std::cout )
	, organ_map                  ( new OrganMap() )
	, spine_map                  ( new SpineMap() )
	, commands_map               ( new CommandsMap() )
{
	if ( !_standard )
		return;

	XEPL::Register_Std_Brain_Words ( this );
	XEPL::Register_Std_Genes       ( this );
	XEPL::Register_Std_Keywords    ( this );
	XEPL::Register_Std_Operators   ( this );
	XEPL::Register_Std_Neurons     ( this );
	XEPL::Register_Std_Splicers    ( this );
	XEPL::Register_Std_Renkeys     ( this );

	Register_Organ ( "brain", host_lobe );

}


int XEPL::Brain::CliLoop ( void )
{
	if (!host_lobe->healthy)
		return false;
	
	auto lobe = XEPL::tlsLobe;

	lobe->Make_Locals();
	lobe->Make_Index ();

	XEPL::ATOM::Scope_Release release_locals ( lobe->locals );
	XEPL::ATOM::Scope_Release release_index  ( lobe->index  );

	lobe->Make_Genes();

	XEPL::TEXT::String thePrompt ( lobe->cell_name );
	thePrompt.append ( "> " );

	errno = 0;
	XEPL::TEXT::String buffer;

	while ( host_lobe->healthy && !std::cin.eof() && buffer[0] != ';' )
	{
		std::cout << thePrompt << std::flush;
		std::getline ( std::cin, buffer );

		while ( errno != 0 )
		{
			//std::cout << "cli: " << strerror ( errno ) << XEPL::EOL << std::flush;
			errno = 0;
			std::cin.clear();
			std::getline ( std::cin, buffer );
		}

		if ( buffer.size() )
		{
			if ( !Cortex__Can_Execute_Cmd ( buffer.c_str() ) )
				std::cerr << "Command Failed: " << buffer << XEPL::EOL << std::flush;
		}

		while (	host_lobe->healthy && host_lobe->Dispatch_Action() )
			;
	}

	lobe->locals = nullptr;
	return 0;
}


void XEPL::Brain::Come_Alive()
{
	host_lobe->semaphore_birth  = new XEPL::THREAD::Semaphore();

	host_lobe->Lobe__Born();

	delete host_lobe->semaphore_birth ;
	host_lobe->semaphore_birth =nullptr;
}

int XEPL::Brain::CliLoop3 ( void )
{
	if (!host_lobe->healthy)
		return false;
	
	XEPL::DNA::Gene* cfg = new XEPL::DNA::Gene ( 0, "SocketMan", nullptr );
	XEPL::CORTEX::Lobe* socket2 =  new KITS::SOCKET::SocketMan ( this->host_lobe, cfg );
	cfg->Release();

	Register_Organ ( "Socket2", socket2 );

	socket2->Start_Lobe ( true );

	Execute_Xml ( "<Cli/>" );
	KITS::TERMINAL::ScopeRaw stayInRaw;

	Come_Alive();

	while ( host_lobe->healthy )
		if ( !host_lobe->Dispatch_Action() )
			host_lobe->Lobe__Rest_Now();

	XEPL::tlsLobe->locals = nullptr;

	return 0;
}


bool XEPL::Brain::Cortex__Can_Execute_Cmd ( const char* bufSpot )
{
	if ( Cortex::Cortex__Can_Execute_Cmd ( bufSpot ) )
		return true;

	switch ( *bufSpot )
	{
		case '}' :
			return Include_File ( bufSpot+1 );

		case '>' :
			return Upload_Neuron ( bufSpot+1 );

		case '~' :
			return Drop_Neuron ( bufSpot+1 );

		case '$' :
			{
				const char* position = ++bufSpot;

				XEPL::TEXT::String varname;

				while ( isalnum ( *bufSpot ) )
					++bufSpot;

				if ( bufSpot != position )
					varname.assign ( position, bufSpot-position );
				else
					host_lobe->Make_Index();

				XEPL::SOMA::GeneLocator source ( host_lobe, &varname );

				if ( ! source.gene )
				{
					source.gene = new XEPL::DNA::Gene ( nullptr, &varname, nullptr );
					XEPL::ATOM::Scope_Release release ( source.gene );

					if ( !XEPL::tlsLobe->ephemerals )
						XEPL::tlsLobe->ephemerals = new XEPL::DNA::Ephemerals();

					XEPL::tlsLobe->ephemerals->Set ( &varname, source.gene );
				}

				XEPL::TEXT::String rescript ( "$" );
				rescript.append ( bufSpot );

				XEPL::DNA::Scope_Index inside ( source.gene );

				return Execute_Rna ( &rescript );
			}

		case '|' :
			return system ( bufSpot+1 ) == 0;

		case ':' :
			{
				XEPL::TEXT::String varname ( bufSpot+1 ), path;
				varname.Split_ch_rhs ( ' ', &path );
				KITS::FILE::Set_File_Path ( varname.c_str(), path.c_str() );
				return true;
			}

		default :
			return Executes_Command ( bufSpot );
	}
}


namespace XEPL
{
	bool Process_Dot_Tag ( XEPL::Brain* _cortex, XEPL::CELL::Nucleus* _nucleus, XEPL::DNA::Gene* _call_gene, size_t dotspot )
	{
		XEPL::TEXT::String    neuron_name;
		XEPL::TEXT::String    method_name;
		XEPL::CORTEX::Neuron* neuron_target = nullptr;
		XEPL::SIGNAL::Axon*   axon = nullptr;

		 _call_gene->cell_name->Split_at_lhs_rhs ( dotspot+1, &neuron_name, &method_name );

		 if ( _nucleus->Nucleus__Host_Neuron()->Find_Neuron ( &neuron_name, &neuron_target ) )
			 return neuron_target->Performed_Method ( &method_name, _call_gene );

		 if ( _cortex->Has_Organ ( &neuron_name, &neuron_target ) )
			 return neuron_target->Performed_Method ( &method_name, _call_gene );

		 if ( _cortex->Has_Axon ( &neuron_name, &axon ) )
		 {
			 axon->Trigger ( _call_gene );
			 return true;
		 }
		 return false;
	}

	bool Process_Dash_Tag ( XEPL::Brain* , XEPL::CELL::Nucleus* , XEPL::DNA::Gene* , size_t   )
	{
		return false;
	}
	bool Process_Under_Tag ( XEPL::Brain* , XEPL::CELL::Nucleus* , XEPL::DNA::Gene* , size_t   )
	{
		return false;
	}
}

bool XEPL::Brain::Cortex__Can_Process_Tag ( XEPL::TEXT::Cord*, XEPL::CELL::Nucleus* _nucleus, XEPL::DNA::Gene* _call_gene )
{
	if ( !_call_gene )
		return false;

	TEXT::String* tag = _call_gene->cell_name;

	size_t dotspot  = tag->find_first_of ( ".-_" );

	if ( dotspot == _call_gene->cell_name->npos )
		return false;

	switch( tag->at(dotspot) )
	{
		case '.' :
			return Process_Dot_Tag( this, _nucleus, _call_gene, dotspot );
		case '-' :
			return Process_Dash_Tag(this, _nucleus, _call_gene, dotspot );
		case '_' :
			return Process_Under_Tag(this, _nucleus, _call_gene, dotspot );
	}

	XEPL::TEXT::String neuron_name;
	XEPL::TEXT::String method_name;

	_call_gene->cell_name->Split_at_lhs_rhs ( dotspot+1, &neuron_name, &method_name );

	XEPL::CORTEX::Neuron* neuron_target = nullptr;

	if ( _nucleus->Nucleus__Host_Neuron()->Find_Neuron ( &neuron_name, &neuron_target ) )
		return neuron_target->Performed_Method ( &method_name, _call_gene );

	if ( this->Has_Organ ( &neuron_name, &neuron_target ) )
		return neuron_target->Performed_Method ( &method_name, _call_gene );

	XEPL::SIGNAL::Axon* axon = nullptr;

	if ( this->Has_Axon ( &neuron_name, &axon ) )
	{
		axon->Trigger ( _call_gene );
		return true;
	}

//	XEPL::SOMA::xeplCantFind ( "Cmd", this, _call_gene->cell_name->c_str() );
	return false;
}


bool XEPL::Brain::Drop_Neuron ( const char* _name )
{
	XEPL::TEXT::String neuronName ( _name );
	XEPL::CORTEX::Neuron* target = nullptr;

	if ( host_lobe->Find_Neuron ( &neuronName, &target ) )
	{
		if ( target == XEPL::tlsLobe )
			return false;

		target->Nucleus__Dropped();
	}

	return !!target;
}

void XEPL::Brain::Drop_Neuron ( XEPL::TEXT::String* _name )
{
	XEPL::CORTEX::Neuron* target = nullptr;

	if ( host_lobe->Find_Neuron ( _name, &target ) )
	{
		++target->retain_count;

		XEPL::ATOM::Scope_Release detach ( target );

		target->Nucleus__Host_Neuron()->Nucleus__Dropped();
	}
}


bool XEPL::Brain::Include_File ( const char* _filename )
{
	XEPL::DNA::Gene* load = nullptr;

	if ( !KITS::FILE::File_Into_Code ( _filename, &load ) )
	{
		XEPL::SOMA::xeplError ( _filename, nullptr, "can not include file " );
		return false;
	}

	load->Make_Gene_Safe ( true );
	XEPL::ATOM::Scope_Release release ( load );

	Execute_Dna ( load );

	return true;
}


bool XEPL::Brain::Executes_Command ( const char* _command )
{
	if ( !_command || !*_command )
		return false;

	XEPL::TEXT::String input ( _command );
	{
		XEPL::TEXT::String lhs;
		XEPL::TEXT::String rhs;

		input.Split_ch_lhs_rhs ( ' ', &lhs, &rhs );

		auto it  = commands_map->find ( lhs );
		if ( it != commands_map->end() )
		{
			XEPL::Command command = it->second;
			( *command ) ( this, host_lobe, &lhs, &rhs );
			return true;
		}
	}

	if ( host_lobe->Performed_Method ( &input, nullptr ) )
		return true;

	XEPL::SOMA::ErrorReport ( "Command not understood " )->append ( _command );
	return false;
}


bool XEPL::Brain::Has_Axon ( XEPL::TEXT::Cord* _name, XEPL::SIGNAL::Axon** __axon )
{
	const auto it = spine_map->find ( *_name );

	if ( it == spine_map->end() )
		return false;

	*__axon = it->second;
	return true;
}


bool XEPL::Brain::Has_Organ ( XEPL::TEXT::Cord* _name, XEPL::CORTEX::Neuron** __organ )
{
	const auto it = organ_map->find ( *_name );

	if ( it == organ_map->end() )
		return false;

	*__organ = it->second;
	return true;
}


void XEPL::Brain::Register_Axon ( const char* _name, XEPL::SIGNAL::Axon* _axon )
{
	if ( !_name )
		return;

	XEPL::TEXT::String name ( _name );

	spine_map->emplace ( name, _axon );

	++_axon->retain_count;
}


void XEPL::Brain::Register_Command ( const char* _name, XEPL::Command _command )
{
	if ( !_name )
		return;

	XEPL::TEXT::String name ( _name );

	commands_map->emplace ( name, _command );
}


void XEPL::Brain::Register_Organ ( const char* _name, XEPL::CORTEX::Neuron* _organ )
{
	if ( !_name )
		return;

	XEPL::TEXT::String name ( _name );

	organ_map->emplace ( name, _organ );

	++_organ->retain_count;
}


bool XEPL::Brain::Upload_Neuron ( const char* _filename )
{
	std::filesystem::path filename ( _filename );

	XEPL::TEXT::String name(_filename);
	XEPL::TEXT::String extra;

	name.Split_ch_rhs ( ' ', &extra );

	std::filesystem::path pathname(name.c_str());

	XEPL::TEXT::String stem( filename.filename().stem().c_str() );
	Drop_Neuron ( stem.c_str() );
	
	return Include_File ( name.c_str() );
}


KITS::FILE::CodeFile::CodeFile ( XEPL::TEXT::Cord* _filename )
	: XEPL::TEXT::String ( _filename )
{
	if ( find ( '.' ) == npos )
		*this += ".xml";
}


KITS::TERMINAL::CommandNode::CommandNode ( osStdIn* _std_in, const char* _string )
	: command_string ( _string )
	, prev_command ( nullptr )
	, next_command ( nullptr )
{
	if ( !_std_in->command_tail )
	{
		_std_in->command_head = this;
		_std_in->command_tail = this;
		return;
	}

	prev_command = _std_in->command_tail;
	_std_in->command_tail->next_command = this;
	_std_in->command_tail = this;

	_std_in->command = nullptr;
}


bool KITS::FILE::File_Into_Code ( const char* _filename, XEPL::DNA::Gene** __gene )
{
	XEPL::TEXT::String filename ( _filename );

	if ( filename.find ( '.' ) == filename.npos )
		filename.append ( 1, '.' ).append ( "xml" );

	XEPL::DNA::Gene* file_gene = nullptr;
	XEPL::tlsLobe->report_gene->Make_New_Gene ( "Include", &file_gene );
	file_gene->Trait_Set ( "filename", filename.c_str() );

	return KITS::FILE::File_Load_Gene ( &filename, __gene );
}


bool KITS::FILE::File_Load_Gene ( XEPL::TEXT::Cord* _filename, XEPL::DNA::Gene** __gene )
{
	if ( !_filename || _filename->empty() )
		return false;

	XEPL::TEXT::String content_string;

	if ( File_Load_String ( _filename, &content_string ) )
	{
		XEPL::XML::XeplXml parser ( &content_string );

		XEPL::DNA::Gene* gene = parser.root_gene;

		if ( gene )
		{
			++gene->retain_count;
			gene->Trait_Set ( "path", _filename );
		}

		*__gene = gene;
		return true;
	}

	return false;
}


bool KITS::FILE::File_Load_String ( XEPL::TEXT::Cord* _filename, std::string& _str )
{
	if ( !_filename || _filename->empty() )
		return false;

	{
		std::ifstream file ( _filename->c_str() );

		if ( file.is_open() )
		{
			std::stringstream buffer;
			buffer << file.rdbuf();
			file.close();

			_str = buffer.str();
			return true;
		}
	}

	XEPL::TEXT::String rename( _filename );
	if( rename.find( "/" ) == XEPL::TEXT::String::npos )
		rename.insert( 0, "resources/");

	{
		std::ifstream file ( rename.c_str() );

		if ( file.is_open() )
		{
			std::stringstream buffer;
			buffer << file.rdbuf();
			file.close();

			_str = buffer.str();
			return true;
		}
	}

	XEPL::TEXT::String bindir;
    XEPL::tlsLobe->cortex->system_gene->Trait_Get("bin", &bindir);
	bindir.push_back('/');

	rename.insert( 0, bindir );

	std::ifstream file ( rename.c_str() );

	if ( file.is_open() )
	{
		std::stringstream buffer;
		buffer << file.rdbuf();
		file.close();

		_str = buffer.str();
		return true;
	}


	std::cout << "cannot load " << rename << XEPL::EOL;

	return false;
}

bool KITS::FILE::File_Load_String ( XEPL::TEXT::Cord* _filename, XEPL::TEXT::String* _str )
{
	if ( !_str )
		return false;

	if ( !_filename || _filename->empty() )
		return false;

	std::string contents;

	if ( File_Load_String ( _filename, contents ) )
		_str->assign ( contents );

	if ( _str->empty() )
		return false;

	return true;
}


#if defined(_WIN32) || defined(_WIN64)
static const char path_separator = '\\';
#else
static const char path_separator = '/';
#endif

bool KITS::FILE::File_Locate ( XEPL::TEXT::Cord* croot, XEPL::TEXT::String* _filename )
{
	XEPL::TEXT::String pathEnd ( "." );

	if ( croot )
		pathEnd.assign ( *croot );

	XEPL::TEXT::String path;

	while ( pathEnd.Split_ch_lhs ( ':', &path ) )
	{
		path.push_back ( path_separator );
		path.append ( *_filename );

		std::filesystem::path filepath ( path.c_str() );

		if ( std::filesystem::exists ( filepath ) )
		{
			_filename->assign ( filepath.c_str() );
			return true;
		}
	}

	std::cout << "Cannot find " << *_filename << XEPL::EOL << std::flush;
	return false;
}


bool KITS::FILE::Path_To ( const char* _name, XEPL::TEXT::Cord** __cord )
{
	XEPL::TEXT::String name ( _name );
	return Path_To ( &name, __cord );
}

bool KITS::FILE::Path_To ( XEPL::TEXT::Cord* _path, XEPL::TEXT::Cord** __cord )
{
	XEPL::DNA::Gene* pathgene = nullptr;

	if ( XEPL::tlsLobe->cortex->system_gene->Get_First_Gene ( "paths", &pathgene ) )
	{
		pathgene->Trait_Get ( _path, __cord );
		return true;
	}

	return false;
}


void KITS::FILE::File_Print_To_Console ( XEPL::TEXT::Cord* _filename )
{
	if ( !_filename || _filename->empty() )
		return;

	XEPL::TEXT::String s;
	File_Load_String ( _filename, &s );
	XEPL::XML::PrintXml parser ( &s );
}


void KITS::FILE::Set_File_Path ( const char* _pathname, const char* _path )
{
	XEPL::DNA::Gene* paths = nullptr;
	XEPL::tlsLobe->cortex->system_gene->Make_One_Gene ( "paths", &paths );
	paths->Trait_Set ( _pathname, _path );
}


bool KITS::FILE::String_Into_File ( XEPL::TEXT::Cord* _txt, XEPL::TEXT::Cord* _filename )
{
	if ( !_filename || _filename->empty() )
		return false;

	if ( !_txt || _txt->empty() )
		return false;

	std::ofstream output_stream ( _filename->c_str(), std::ios::out );

	if ( !output_stream.is_open() )
		return false;

	output_stream << _txt->c_str();

	if ( !output_stream.fail() )
		output_stream.close();

	return !output_stream.fail();
}


void XEPL::SOMA::Gene_Absorb_Query ( DNA::Gene* _gene, TEXT::String* _url_options )
{
	TEXT::String lhs, rhs;
	TEXT::String llhs, rrhs;

	while ( _url_options->Split_ch_lhs_rhs ( '&', &lhs, &rhs ) )
	{
		if ( lhs.Split_ch_lhs_rhs ( '=', &llhs, &rrhs ) )
			_gene->Trait_Set ( &llhs, &rrhs );

		_url_options = &rhs;
	}

	if ( rhs.Split_ch_lhs_rhs ( '=', &lhs, &rhs ) )
		_gene->Trait_Set ( &lhs, &rhs );
}


bool XEPL::SOMA::Gene_From_Url ( const char* _text, XEPL::DNA::Gene* _gene )
{
	XEPL::TEXT::String url_string ( _text );
	XEPL::TEXT::String scratch;

	const char* pointer = url_string.c_str();

	auto it = url_string.find ( "://" );

	if ( it != std::string::npos )
	{
		scratch.assign ( pointer, it );
		_gene->Trait_Set ( "protocol", &scratch );

		pointer = url_string.erase ( 0, it+3 ).c_str();
	}

	it = url_string.find_first_of ( "/?&#" );

	if ( it == std::string::npos )
	{
		_gene->Trait_Set ( "domain", pointer );
		return true;
	}

	if ( it > 0 )
	{
		scratch.assign ( pointer, it );
		_gene->Trait_Set ( "domain", &scratch );
		pointer = url_string.erase ( 0, it+1 ).c_str();
	}

	it = url_string.find_first_of ( "?&#!" );

	if ( it == std::string::npos )
	{
		_gene->Trait_Set ( "path", pointer );
		return true;
	}

	scratch.assign ( pointer, it );
	_gene->Trait_Set ( "path", &scratch );

	pointer = url_string.erase ( 0, it ).c_str();

	bool success = _gene->traits;

	if ( success )
		_gene->Trait_Set ( "url", _text );

	_gene->Trait_Set ( "rest", pointer );

	return success;
}


KITS::HTTP::HttpHeadersMap* KITS::HTTP::Http::httpHeaderMap;

KITS::HTTP::Http::~Http()
{
	if ( message_gene )
		message_gene->Release();

	delete version_string;
}

KITS::HTTP::Http::Http ( XEPL::CORTEX::Neuron*  _parent, XEPL::DNA::Gene* _config )
	: Sensor             ( _parent, _config )
	, parse_action       ( &Http::Sensor__Scan )
	, message_gene       ( nullptr )
	, version_string     ( new XEPL::TEXT::String ( _config->Trait_Default ( "version", "HTTP/1.1" ) ) )
	, request_in_axon   ( new XEPL::SIGNAL::Axon ( this, "HttpRequestAxon",   XEPL::SIGNAL::Axon::REPEATS  ) )
	, request_out_axon  ( new XEPL::SIGNAL::Axon ( this, "eRequestOut",  XEPL::SIGNAL::Axon::REPEATS  ) )
	, response_in_axon  ( new XEPL::SIGNAL::Axon ( this, "eResponseIn",  XEPL::SIGNAL::Axon::REPEATS  ) )
	, response_out_axon ( new XEPL::SIGNAL::Axon ( this, "HttpResponseAxon", XEPL::SIGNAL::Axon::REPEATS  ) )
	, http_closed_axon  ( new XEPL::SIGNAL::Axon ( this, "closed",       XEPL::SIGNAL::Axon::PULSE    ) )
	, payload_length     ( -1 )
	, finished_bool      ( _config->Trait_Get_bool ( "close", false ) )
	, dropped_bool       ( false )
{
	Register_Method ( "Deliver", ( XEPL::CELL::Function )&Http::Method_Deliver, nullptr );
}


KITS::HTTP::HttpClient::~HttpClient()
{}

KITS::HTTP::HttpClient::HttpClient ( XEPL::CORTEX::Neuron*  _parent, XEPL::DNA::Gene* _config )
	: Http ( _parent, _config )
{
	Register_Method ( "Request", ( XEPL::CELL::Function )&HttpClient::Method_Request, 0 );
	Do_Next ( ( HttpScanner )&HttpClient::Scan_Status_Line );
}


void KITS::HTTP::HttpClient::Build_Request ( XEPL::DNA::Gene* _config, XEPL::TEXT::String* _bag )
{
	{
		XEPL::TEXT::Cord request ( _config->Trait_Default ( "request", "GET" ) );
		XEPL::TEXT::Cord uri     ( _config->Trait_Default ( "uri", "/" ) );
		XEPL::TEXT::Cord version ( _config->Trait_Default ( "version", "HTTP/1.1" ) );
		XEPL::TEXT::String ver ( version );

		if ( ver.compare ( "HTTP/1.1" ) == 0 )
			finished_bool = true;

		XEPL::TEXT::String payload ( request );
		payload.push_back ( ' ' );
		payload.append ( uri );
		payload.push_back ( ' ' );
		payload.append ( version );
		payload.append ( XEPL::EOL );
		{
			XEPL::TEXT::Cord*   name = nullptr;
			XEPL::TEXT::String* term = nullptr;

			XEPL::DNA::StableRecall header ( _config, false );

			while ( header.Next_Trait ( &name, &term ) )
			{
				payload.append ( *name );
				payload.append ( ": " );
				payload.append ( *term );
				payload.append ( XEPL::EOL );

				Property_Set ( name, term );
			}
		}

		if ( finished_bool )
			payload.append ( "connection: close" ).append ( XEPL::EOL );
		else
			payload.append ( "connection: keep-alive" ).append ( XEPL::EOL );

		payload.append ( XEPL::EOL );
		_bag->append ( payload );
	}
}


void KITS::HTTP::HttpClient::Http__Deliver() 
{
	if ( message_gene )
	{
		XEPL::TEXT::String* content = message_gene->Content();
		XEPL::TEXT::String length;
		length.assign ( std::to_string ( content?content->size():0 ) );

		if ( std::strtol ( length.c_str(), nullptr, 0 ) )
			Property_Set ( "content-length", &length );

		response_in_axon->Trigger ( message_gene );
		message_gene->Release();
		message_gene = nullptr;
	}

	Do_Next ( ( HttpScanner )&HttpClient::Scan_Status_Line );
}


void KITS::HTTP::HttpClient::Method_Request NUCLEUS_METHOD
{
	XEPL::RNA::RnaScript ( this, _call );

	XEPL::DNA::Gene* request = new XEPL::DNA::Gene( nullptr, "request", nullptr );
	XEPL::ATOM::Scope_Release detach( request );

	Build_Request ( _call, request->Make_Content_Wire() );

	request_out_axon->Trigger ( request );
}


void KITS::HTTP::HttpClient::Scan_Status_Line()
{
	XEPL::TEXT::String line;
	if ( !sensor_wire->Extract_Line ( &line ) )
		return;
	
	message_gene = new XEPL::DNA::Gene ( 0, "HttpResponse", nullptr );

	XEPL::TEXT::String version ( &line );
	XEPL::TEXT::String statusCode;
	XEPL::TEXT::String reason;

	version.Split_ch_rhs ( ' ', &statusCode );
	statusCode.Split_ch_rhs ( ' ', &reason );

	message_gene->Trait_Set ( "version",    &version );
	message_gene->Trait_Set ( "statusCode", &statusCode );
	message_gene->Trait_Set ( "reason",     &reason );

	XEPL::TEXT::String scratch;
	Property_Get ( "uri",  &scratch );
	message_gene->Trait_Set ( "uri", &scratch );

	Property_Get ( "lobe", &scratch );
	message_gene->Trait_Set ( "lobe", &scratch );
	const long term = message_gene->Trait_Get_long ( "statusCode", 100 );
	Property_Set ( "content-type", "-" );

	if ( term < 200 || term == 204 || term == 304 )
		payload_length = 0;

	if ( version.compare ( "HTTP/1.0" ) == 0 )
		finished_bool = true;

	Property_Set ( "connection", finished_bool ? "close" : "keep_alive" );


	Scan ( ( HttpScanner )&Http::Scan_Header );
}


KITS::HTTP::HttpServer::~HttpServer()
{}

KITS::HTTP::HttpServer::HttpServer ( XEPL::CORTEX::Neuron*  _parent, XEPL::DNA::Gene* _config )
	: Http ( _parent, _config )
{
	Register_Method ( "Respond", ( XEPL::CELL::Function )&HttpServer::Method_Respond, 0 );

	Do_Next ( ( HttpScanner )&HttpServer::Scan_Request_Line );
}


void KITS::HTTP::HttpServer::Build_Response ( XEPL::DNA::Gene* _config, XEPL::TEXT::String* _bag )
{
	_bag->append ( "HTTP/1.1" );
	_bag->push_back (  ' ' );
	_bag->append ( _config->Trait_Default ( "statusCode", "200" ) );
	_bag->push_back (  ' ' );
	_bag->append ( _config->Trait_Default ( "reason", "OK" ) );
	_bag->append ( XEPL::EOL );

	XEPL::TEXT::String* output=new XEPL::TEXT::String();
	output->reserve ( 4072 );

	if ( _config->content_wire )
		output->append ( *_config->Content() );

	auto was = Neuron::output;
	Neuron::output = output;

	if ( const char* contents = _config->Trait_Default ( "contents", nullptr ) )
		_bag->append ( "content-type" )
		.append ( ": " )
		.append ( contents )
		.append ( XEPL::EOL );

	if ( const char* pragma = _config->Trait_Default ( "pragma", nullptr ) )
		_bag->append ( "pragma" )
		.append ( ": " )
		.append ( pragma )
		.append ( XEPL::EOL );

	XEPL::DNA::Gene* message = nullptr;
	_config->Gene_Duplicate ( &message );
	XEPL::ATOM::Scope_Release detach ( message );
	XEPL::DNA::Gene* payloadData = nullptr;
	message->Get_First_Gene ( "Payload", &payloadData );

	if ( payloadData )
		Process_Inner_Genes ( payloadData );

	XEPL::TEXT::String val;
	val.assign ( std::to_string ( output->size() ) );
	_bag->append ( "content-length: " ).append ( val ).append ( XEPL::EOL );
	_bag->append ( XEPL::EOL );
	_bag->append ( *output );

	delete output;
	Neuron::output = was;
}


void KITS::HTTP::HttpServer::Http__Deliver()
{
	if ( message_gene )
	{
		XEPL::TEXT::String* content = message_gene->Content();

		if ( content )
		{
			XEPL::TEXT::String length;
			length.assign ( std::to_string ( content->size() ) );
			Property_Set ( "content-length", &length );
		}

		request_in_axon->Trigger ( message_gene );
		message_gene->Release();
		message_gene = nullptr;
	}

	Do_Next ( ( HttpScanner )&HttpServer::Scan_Request_Line );
}


void KITS::HTTP::HttpServer::Method_Respond ( XEPL::DNA::Gene* _config, XEPL::DNA::Gene* )
{
	XEPL::DNA::Gene* response = new XEPL::DNA::Gene ( nullptr, "Response", nullptr );
	response->Make_Content_Wire();

	Build_Response ( _config, response->content_wire->wire_string );

	response_out_axon->Trigger ( response );

	response->Release();
}


void KITS::HTTP::HttpServer::Scan_Request_Line()
{
	{
		XEPL::TEXT::String line;
		sensor_wire->Extract_Line ( &line );

		if ( line.empty() )
			return;

		line.append ( XEPL::EOL );

		report_gene->Make_Content_Wire();
		report_gene->content_wire->Append ( &line );

		message_gene = new XEPL::DNA::Gene ( 0, "HttpRequst", nullptr );
		payload_length = 0;

		XEPL::TEXT::String request ( line ), uri, version;
		request.Split_ch_rhs ( ' ', &uri );
		uri.Split_ch_rhs ( ' ', &version );

		message_gene->Trait_Set ( "request", request.c_str() );
		message_gene->Trait_Set ( "uri",     uri.c_str() );
		message_gene->Trait_Set ( "version", version.c_str() );
	}
	Scan_Header();
}


void KITS::HTTP::Http::Sensor__Closed()
{
	Http__Deliver();
	http_closed_axon->Trigger ( nullptr );
}


void KITS::HTTP::Http::Sensor__Scan()
{
	( this->*parse_action )();
}


bool KITS::HTTP::Http::Connection ( XEPL::TEXT::Cord* _definition )
{
	XEPL::TEXT::String definition ( _definition );
	std::transform ( definition.begin(), definition.end(), definition.begin(), ::tolower );

	Property_Set ( "connection", &definition );
	return true;
}


bool KITS::HTTP::Http::Content_Length ( XEPL::TEXT::Cord* _definition )
{
	XEPL::TEXT::String req;
	Property_Get ( "request", &req );

	if ( req.compare ( "HEAD" ) == 0 )
		payload_length = 0;
	else
		payload_length = std::strtol ( _definition->c_str(), nullptr, 0 );

	return true;
}


bool KITS::HTTP::Http::Content_Type ( XEPL::TEXT::Cord* _definition )
{
	XEPL::TEXT::String definition ( _definition );
	std::transform ( definition.begin(), definition.end(), definition.begin(), ::tolower );

	Property_Set ( "content-type", &definition );
	return true;
}


void KITS::HTTP::Http::Define_Header ( XEPL::TEXT::Cord* _line )
{
	XEPL::TEXT::String label;
	XEPL::TEXT::String definition;
	_line->Split_ch_lhs_rhs ( ':', &label, &definition );
	std::transform ( label.begin(), label.end(), label.begin(), ::tolower );

	Header ( &label, &definition );
}


void KITS::HTTP::Http::Do_Next ( KITS::HTTP::HttpScanner _routine )
{
	parse_action = _routine;
}


void KITS::HTTP::Http::Header ( XEPL::TEXT::Cord* _label, XEPL::TEXT::String* _definition )
{
	const auto it = KITS::HTTP::Http::httpHeaderMap->find ( *_label );

	if ( it != KITS::HTTP::Http::httpHeaderMap->end() )
	{
		HttpHeader header = it->second;

		if ( ! ( this->*header ) ( _definition ) )
			return;
	}

	XEPL::DNA::Gene* header = nullptr;
	message_gene->Make_One_Gene ( "Header", &header );
	XEPL::TEXT::String definitionNow;

	if ( header->Trait_Get ( _label, &definitionNow ) )
	{
		XEPL::TEXT::String defintionNew ( definitionNow );
		defintionNew.append ( ";" ).append ( *_definition );
		header->Trait_Set ( _label,  &defintionNew );
		header->Release();
		return;
	}

	const char* def = _definition->c_str();

	while ( isspace ( *def ) )
		++def;

	header->Trait_Set ( _label,  def );
}


void KITS::HTTP::Http::Initialize()
{
	KITS::HTTP::Http::httpHeaderMap = new KITS::HTTP::HttpHeadersMap();
	Register_Header_Field ( "content-length", ( HttpHeader )&Http::Content_Length );
	Register_Header_Field ( "content-type",   ( HttpHeader )&Http::Content_Type );
	Register_Header_Field ( "connection",     ( HttpHeader )&Http::Connection );
}

void KITS::HTTP::Http::Shutdown()
{
	if ( Http::httpHeaderMap )
	{
		Http::httpHeaderMap->clear();

		delete Http::httpHeaderMap;
		Http::httpHeaderMap = nullptr;
	}
}


void KITS::HTTP::Http::Method_Deliver NUCLEUS_METHOD
{
	Http__Deliver();
}


void KITS::HTTP::Http::Nucleus__Dropped()
{
	if ( dropped_bool )
		return;

	dropped_bool = true;
	Sensor::Nucleus__Dropped();
}


void KITS::HTTP::Http::Register_Header_Field ( const char* _label, HttpHeader _header )
{
	XEPL::TEXT::String name ( _label );
	( *Http::httpHeaderMap )[ name ] = _header;
}


void KITS::HTTP::Http::Scan ( KITS::HTTP::HttpScanner _routine )
{
	parse_action = _routine;
	( this->*parse_action )();
}


void KITS::HTTP::Http::Scan_Header()
{
	while ( 1 )
	{
		XEPL::TEXT::String line;
		sensor_wire->Extract_Line ( &line );

		if ( line.empty() )
			break;

		Define_Header ( &line );
	}

	Scan_Payload();
}


void KITS::HTTP::Http::Scan_Payload()
{
	if ( payload_length == 0 )
	{
		Http__Deliver();
		Sensor__Scan();
		return;
	}

	if ( payload_length > 0 && sensor_wire->Avail() < payload_length )
		return;

	message_gene->Content_Append ( sensor_wire );
	sensor_wire->Erase();
	Http__Deliver();
	Sensor__Scan();
}


KITS::TERMINAL::osStdIn::~osStdIn()
{
	while ( command_head )
	{
		command = command_head->next_command;
		delete command_head;
		command_head = command;
	}

	delete terminal;

	if ( receiving_gene )
		receiving_gene->Release();

	free ( input_buffer );
}

KITS::TERMINAL::osStdIn::osStdIn ( XEPL::CORTEX::Sensor* _sensor )
	: osFd ( )
	, sensor         ( _sensor )
	, terminal       ( new KITS::TERMINAL::Terminal() )
	, input_buffer   ( static_cast<char*>( malloc ( 80 ) ) )
	, cli_input_axon ( nullptr )
	, receiving_gene ( nullptr )
	, command_head   ( nullptr )
	, command_tail   ( nullptr )
	, command        ( nullptr )
	, offset         ( 0 )
	, length         ( 0 )
{
	XEPL::TEXT::Cord cli_input ( "cli" );

	cli_input_axon = new XEPL::SIGNAL::Axon ( this->sensor, &cli_input, XEPL::SIGNAL::Axon::PULSE );
	sensor->Subscribe ( cli_input_axon, ( XEPL::SIGNAL::Receiver )&osStdIn::Line_Received, sensor );
}

int KITS::TERMINAL::osStdIn::osFd__Descriptor ( void ) const
{
	return STDOUT_FILENO;
}

void KITS::TERMINAL::osStdIn::Line_Received SIGNAL_RECEIVER
{
	XEPL::CORTEX::Sensor* sensor = static_cast<XEPL::CORTEX::Sensor*> ( _memento );

	if ( _signal )
		sensor->Sensor__Scan();
	else
		XEPL::tlsLobe->cortex->Cortex__Shutdown();
}

bool  KITS::TERMINAL::osStdIn::Process_Command_Key ( int key )
{
	switch ( key )
	{
		case CTRL_KEY ( 'q' ) :
			cli_input_axon->Trigger ( nullptr );
			break;

		case CTRL_KEY ( 'c' ) :
			terminal->Disable_Raw();
			exit ( 99 );
			break;

		case KITS::TERMINAL::Terminal::ARROW_LEFT :
			if ( offset == 0 )
				break;

			--offset;
			terminal->Move_Cursor_Left();
			break;

		case KITS::TERMINAL::Terminal::ARROW_RIGHT :
			if ( offset >= length )
				break;

			++offset;
			terminal->Move_Cursor_Right();
			break;

		case KITS::TERMINAL::Terminal::DEL_KEY :
			if ( offset > 0 )
			{
				char* spot = &input_buffer[offset];
				memcpy ( spot-1, spot, length-offset );
				terminal->Move_Cursor_Left();
				terminal->Clear_To_Eol();
				write ( STDOUT_FILENO, spot-1, length-offset );
				--length;
				--offset;
			}

			break;

		case KITS::TERMINAL::Terminal::ARROW_UP :
		case KITS::TERMINAL::Terminal::ARROW_DOWN :
			{
				if ( !command )
					command = ( key == KITS::TERMINAL::Terminal::ARROW_UP ) ? command_tail : command_head;
				else
					command = ( key == KITS::TERMINAL::Terminal::ARROW_UP ) ? command->prev_command : command->next_command;

				if ( command )
				{
					size_t len = command->command_string.size();
					memcpy ( input_buffer, command->command_string.c_str(), len );
					offset = len;
					length = len;
					terminal->Move_Bol();
					terminal->Clear_To_Eol();
					write ( STDOUT_FILENO, input_buffer, length );
				}

				break;
			}

		case -1 :
			return false;

		default:
			char* rebuffer = static_cast<char*>( realloc ( input_buffer, length+1 ) );

			if ( rebuffer && rebuffer != input_buffer )
				input_buffer = rebuffer;

			if ( offset < length )
			{
				char* spot = &input_buffer[offset];
				memmove ( spot+1, spot, length+1-offset );
			}

			input_buffer[offset]=key;
			++offset;
			++length;
			break;
	}

	return true;
}


bool KITS::TERMINAL::osStdIn::Fd_Receive ( void )
{
	int key = terminal->Read_Key();

	if ( key == '\n' )
		return true;

	if ( key == '\r' )
	{
		input_buffer[length] = '\0';

		if ( length )
		{
			if ( !command_tail || command_tail->command_string.compare ( input_buffer ) != 0 )
				new CommandNode ( this, input_buffer );
		}

		if ( receiving_gene )
		{
			XEPL::ATOM::Scope_Release detach ( receiving_gene );
			receiving_gene->Trait_Set ( "command", input_buffer );
			cli_input_axon->Trigger ( receiving_gene );
		}

		receiving_gene = nullptr;

		command = nullptr;
		write ( STDOUT_FILENO, XEPL::EOL, 2 );
		length = 0;
		offset = 0;
		return true;
	}

	if ( !receiving_gene )
		receiving_gene = new XEPL::DNA::Gene ( 0, "Receive", nullptr );

	if ( Process_Command_Key ( key ) )
	{
		XEPL::TEXT::String msg ( "." );
		msg.append ( std::to_string ( key ) );

		receiving_gene->Content_Append ( &msg );

		return true;
	}

	return true;
}

XEPL::OS::fdHandler KITS::TERMINAL::osStdIn::osFd__OnFdRead() const
{
	return ( XEPL::OS::fdHandler )&osStdIn::Fd_Receive;
}


namespace XEPL::KEYWORDS
{
	void Keyword_Axon              CORTEX_REGISTRATION;
	void Keyword_Axons             CORTEX_REGISTRATION;
	void Keyword_Form              CORTEX_REGISTRATION;
	void Keyword_Forms             CORTEX_REGISTRATION;
	void Keyword_Synapse           CORTEX_REGISTRATION;
	void Keyword_Synapses          CORTEX_REGISTRATION;
	void Keyword_Lobe              CORTEX_REGISTRATION;
	void Keyword_Macro             CORTEX_REGISTRATION;
	void Keyword_Macros            CORTEX_REGISTRATION;
	void Keyword_Method            CORTEX_REGISTRATION;
	void Keyword_Methods           CORTEX_REGISTRATION;
	void Keyword_Neuron            CORTEX_REGISTRATION;
	void Keyword_Neurons           CORTEX_REGISTRATION;
	void Keyword_Properties        CORTEX_REGISTRATION;
	void Keyword_Property          CORTEX_REGISTRATION;
	void Keyword_Trigger           CORTEX_REGISTRATION;
	void Keyword_Gene              CORTEX_REGISTRATION;
	void Keyword_Genes             CORTEX_REGISTRATION;
}

void XEPL::Register_Std_Brain_Words ( XEPL::CORTEX::Cortex* _cortex )
{
	XEPL::KEYWORDS::Keyword_Axon         ( _cortex, "Axon" );
	XEPL::KEYWORDS::Keyword_Axons        ( _cortex, "Axons" );
	XEPL::KEYWORDS::Keyword_Form         ( _cortex, "Form" );
	XEPL::KEYWORDS::Keyword_Forms        ( _cortex, "Forms" );
	XEPL::KEYWORDS::Keyword_Synapse      ( _cortex, "Synapse" );
	XEPL::KEYWORDS::Keyword_Synapses     ( _cortex, "Synapses" );
	XEPL::KEYWORDS::Keyword_Lobe         ( _cortex, "Lobe" );
	XEPL::KEYWORDS::Keyword_Macro        ( _cortex, "Macro" );
	XEPL::KEYWORDS::Keyword_Macros       ( _cortex, "Macros" );
	XEPL::KEYWORDS::Keyword_Method       ( _cortex, "Method" );
	XEPL::KEYWORDS::Keyword_Methods      ( _cortex, "Methods" );
	XEPL::KEYWORDS::Keyword_Neuron       ( _cortex, "Neuron" );
	XEPL::KEYWORDS::Keyword_Neurons      ( _cortex, "Neurons" );
	XEPL::KEYWORDS::Keyword_Properties   ( _cortex, "Properties" );
	XEPL::KEYWORDS::Keyword_Property     ( _cortex, "Property" );
	XEPL::KEYWORDS::Keyword_Trigger      ( _cortex, "Trigger" );
	XEPL::KEYWORDS::Keyword_Gene         ( _cortex, "Gene" );
	XEPL::KEYWORDS::Keyword_Genes        ( _cortex, "Genes" );
}


void XEPL::Register_Std_Genes ( XEPL::CORTEX::Cortex* _cortex )
{
	_cortex->Register_Gene ( "config",  [] ( XEPL::CELL::Nucleus* _nucleus )
	{
		return _nucleus->config_gene;
	} );
	_cortex->Register_Gene ( "report",  [] ( XEPL::CELL::Nucleus* _nucleus )
	{
		return _nucleus->report_gene;
	} );
	_cortex->Register_Gene ( "forms",  [] ( XEPL::CELL::Nucleus* _nucleus )
	{
		return _nucleus->forms_gene;
	} );
	_cortex->Register_Gene ( "properties",  [] ( XEPL::CELL::Nucleus* _nucleus )
	{
		return _nucleus->properties;
	} );
	_cortex->Register_Gene ( "macros",  [] ( XEPL::CELL::Nucleus* _nucleus )
	{
		return _nucleus->macros_gene;
	} );
	_cortex->Register_Gene ( "genes",  [] ( XEPL::CELL::Nucleus* _nucleus )
	{
		return _nucleus->chromo_gene;
	} );
	_cortex->Register_Gene ( "methods",  [] ( XEPL::CELL::Nucleus* _nucleus )
	{
		return _nucleus->methods_gene;
	} );
	_cortex->Register_Gene ( "lobe",    [] ( XEPL::CELL::Nucleus* )
	{
		return XEPL::tlsLobe->fileData;
	} );
	_cortex->Register_Gene ( "index",   [] ( XEPL::CELL::Nucleus* )
	{
		return XEPL::tlsLobe->index;
	} );
	_cortex->Register_Gene ( "locals",  [] ( XEPL::CELL::Nucleus* )
	{
		return XEPL::tlsLobe->locals;
	} );
	_cortex->Register_Gene ( "trigger", [] ( XEPL::CELL::Nucleus* )
	{
		return static_cast<XEPL::DNA::Gene*> ( XEPL::tlsLobe->current_stimulus );
	} );
	_cortex->Register_Gene ( "outdex",  [] ( XEPL::CELL::Nucleus* )
	{
		return XEPL::tlsLobe->outdex;
	} );
	_cortex->Register_Gene ( "system",  [] ( XEPL::CELL::Nucleus* )
	{
		return XEPL::tlsLobe->cortex->system_gene;
	} );

}


namespace XEPL::KEYWORDS
{
	void Keyword_Build             CORTEX_REGISTRATION;
	void Keyword_Add               CORTEX_REGISTRATION;
	void Keyword_Attach            CORTEX_REGISTRATION;
	void Keyword_Assemble          CORTEX_REGISTRATION;
	void Keyword_Clear             CORTEX_REGISTRATION;
	void Keyword_Combine           CORTEX_REGISTRATION;
	void Keyword_Del               CORTEX_REGISTRATION;
	void Keyword_Dismiss           CORTEX_REGISTRATION;
	void Keyword_Enter             CORTEX_REGISTRATION;
	void Keyword_ForEach           CORTEX_REGISTRATION;
	void Keyword_Find              CORTEX_REGISTRATION;
	void Keyword_First             CORTEX_REGISTRATION;
	void Keyword_For               CORTEX_REGISTRATION;
	void Keyword_If                CORTEX_REGISTRATION;
	void Keyword_Iff               CORTEX_REGISTRATION;
	void Keyword_Index             CORTEX_REGISTRATION;
	void Keyword_Keep              CORTEX_REGISTRATION;
	void Keyword_Modify            CORTEX_REGISTRATION;
	void Keyword_New               CORTEX_REGISTRATION;
	void Keyword_Print             CORTEX_REGISTRATION;
	void Keyword_Repeat            CORTEX_REGISTRATION;
	void Keyword_Report            CORTEX_REGISTRATION;
	void Keyword_Run               CORTEX_REGISTRATION;
	void Keyword_Set               CORTEX_REGISTRATION;
	void Keyword_When              CORTEX_REGISTRATION;
	void Keyword_While             CORTEX_REGISTRATION;
	void Keyword_Parse             CORTEX_REGISTRATION;
}

void XEPL::Register_Std_Keywords ( XEPL::CORTEX::Cortex* _cortex )
{
	XEPL::KEYWORDS::Keyword_Add     ( _cortex, "AddTo"    );
	XEPL::KEYWORDS::Keyword_Assemble( _cortex, "Assemble" );
	XEPL::KEYWORDS::Keyword_Attach  ( _cortex, "Attach"   );
	XEPL::KEYWORDS::Keyword_Clear   ( _cortex, "Clear"    );
	XEPL::KEYWORDS::Keyword_Combine ( _cortex, "Combine"  );
	XEPL::KEYWORDS::Keyword_Del     ( _cortex, "Del"      );
	XEPL::KEYWORDS::Keyword_Dismiss ( _cortex, "Dismiss"  );
	XEPL::KEYWORDS::Keyword_Enter   ( _cortex, "Enter"    );
	XEPL::KEYWORDS::Keyword_ForEach ( _cortex, "ForEach"  );
	XEPL::KEYWORDS::Keyword_Find    ( _cortex, "Find"     );
	XEPL::KEYWORDS::Keyword_First   ( _cortex, "First"    );
	XEPL::KEYWORDS::Keyword_For     ( _cortex, "For"      );
	XEPL::KEYWORDS::Keyword_If      ( _cortex, "If"       );
	XEPL::KEYWORDS::Keyword_Iff     ( _cortex, "Iff"      );
	XEPL::KEYWORDS::Keyword_Index   ( _cortex, "Index"    );
	XEPL::KEYWORDS::Keyword_Keep    ( _cortex, "Keep"     );
	XEPL::KEYWORDS::Keyword_Modify  ( _cortex, "Mod"      );
	XEPL::KEYWORDS::Keyword_New     ( _cortex, "New"      );
	XEPL::KEYWORDS::Keyword_Parse   ( _cortex, "Parse"    );
	XEPL::KEYWORDS::Keyword_Print   ( _cortex, "Print"    );
	XEPL::KEYWORDS::Keyword_Repeat  ( _cortex, "Repeat"   );
	XEPL::KEYWORDS::Keyword_Report  ( _cortex, "Report"   );
	XEPL::KEYWORDS::Keyword_Run     ( _cortex, "Run"      );
	XEPL::KEYWORDS::Keyword_Set     ( _cortex, "Set"      );
	XEPL::KEYWORDS::Keyword_When    ( _cortex, "When"     );
	XEPL::KEYWORDS::Keyword_While   ( _cortex, "While"    );
}


namespace XEPL::KEYWORDS
{
	void Keyword_Html       CORTEX_REGISTRATION;
	void Keyword_HttpClient CORTEX_REGISTRATION;
	void Keyword_HttpServer CORTEX_REGISTRATION;
	void Keyword_SocketMan  CORTEX_REGISTRATION;
	void Keyword_Tcpclient  CORTEX_REGISTRATION;
	void Keyword_Tcpserver  CORTEX_REGISTRATION;
	void Keyword_Sleep      CORTEX_REGISTRATION;
	void Keyword_Timer      CORTEX_REGISTRATION;
	void Keyword_Xml        CORTEX_REGISTRATION;
}

void XEPL::Register_Std_Neurons ( XEPL::CORTEX::Cortex* _cortex )
{
	XEPL::KEYWORDS::Keyword_Html       ( _cortex, "Html"       );
	XEPL::KEYWORDS::Keyword_HttpClient ( _cortex, "HttpClient" );
	XEPL::KEYWORDS::Keyword_HttpServer ( _cortex, "HttpServer" );
	XEPL::KEYWORDS::Keyword_SocketMan  ( _cortex, "SocketMan"  );
	XEPL::KEYWORDS::Keyword_Tcpclient  ( _cortex, "TcpClient"  );
	XEPL::KEYWORDS::Keyword_Tcpserver  ( _cortex, "TcpServer"  );

	XEPL::KEYWORDS::Keyword_Sleep      ( _cortex, "Sleep"      );
	XEPL::KEYWORDS::Keyword_Timer      ( _cortex, "Timer"      );
	XEPL::KEYWORDS::Keyword_Xml        ( _cortex, "Text"       );
}


namespace XEPL::OPERATORS
{
	void Operator_Add              CORTEX_REGISTRATION;
//	void Operator_Divide           CORTEX_REGISTRATION;
	void Operator_After            CORTEX_REGISTRATION;
	void Operator_AfterAny         CORTEX_REGISTRATION;
	void Operator_AfterLast        CORTEX_REGISTRATION;
	void Operator_Append           CORTEX_REGISTRATION;
	void Operator_Before           CORTEX_REGISTRATION;
	void Operator_BeforeAny        CORTEX_REGISTRATION;
	void Operator_Cr               CORTEX_REGISTRATION;
	void Operator_DeAmp            CORTEX_REGISTRATION;
	void Operator_Empty            CORTEX_REGISTRATION;
	void Operator_Equal            CORTEX_REGISTRATION;
	void Operator_Gt               CORTEX_REGISTRATION;
	void Operator_Lt               CORTEX_REGISTRATION;
	void Operator_Exclaim          CORTEX_REGISTRATION;
	void Operator_Has              CORTEX_REGISTRATION;
	void Operator_Is               CORTEX_REGISTRATION;
	void Operator_Lf               CORTEX_REGISTRATION;
	void Operator_Lowercase        CORTEX_REGISTRATION;
	void Operator_Multiply         CORTEX_REGISTRATION;
	void Operator_NextAny          CORTEX_REGISTRATION;
	void Operator_Prepend          CORTEX_REGISTRATION;
	void Operator_Space            CORTEX_REGISTRATION;
	void Operator_Subtract         CORTEX_REGISTRATION;
	void Operator_Tab              CORTEX_REGISTRATION;
	void Operator_Depercent        CORTEX_REGISTRATION;
	void Operator_Percentify       CORTEX_REGISTRATION;

}
void XEPL::Register_Std_Operators ( XEPL::CORTEX::Cortex* _cortex )
{
	XEPL::OPERATORS::Operator_Add         ( _cortex,  "add" );
//	XEPL::OPERATORS::Operator_Divide      ( _cortex,  "div" );
	XEPL::OPERATORS::Operator_After       ( _cortex,  "after" );
	XEPL::OPERATORS::Operator_AfterAny    ( _cortex,  "afterAny" );
	XEPL::OPERATORS::Operator_AfterLast   ( _cortex,  "afterLast" );
	XEPL::OPERATORS::Operator_Append      ( _cortex,  "append" );
	XEPL::OPERATORS::Operator_Before      ( _cortex,  "before" );
	XEPL::OPERATORS::Operator_BeforeAny   ( _cortex,  "beforeAny" );
	XEPL::OPERATORS::Operator_Cr          ( _cortex,  "cr" );
	XEPL::OPERATORS::Operator_DeAmp       ( _cortex,  "deamp" );
//	XEPL::OPERATORS::Operator_Depercent   ( _cortex,  "depercent" );
	XEPL::OPERATORS::Operator_Empty       ( _cortex,  "empty" );
	XEPL::OPERATORS::Operator_Equal       ( _cortex,  "eq" );
	XEPL::OPERATORS::Operator_Gt          ( _cortex,  "gt" );
	XEPL::OPERATORS::Operator_Lt          ( _cortex,  "lt" );
	XEPL::OPERATORS::Operator_Exclaim     ( _cortex, "exclaim" );
	XEPL::OPERATORS::Operator_Has         ( _cortex,  "has" );
	XEPL::OPERATORS::Operator_Is          ( _cortex,  "is" );
	XEPL::OPERATORS::Operator_Lf          ( _cortex, "lf" );
	XEPL::OPERATORS::Operator_Lowercase   ( _cortex,  "lower" );
	XEPL::OPERATORS::Operator_Multiply    ( _cortex,  "mul" );
	XEPL::OPERATORS::Operator_NextAny     ( _cortex,  "nextAny" );
	XEPL::OPERATORS::Operator_Prepend     ( _cortex,  "prepend" );
	XEPL::OPERATORS::Operator_Space       ( _cortex, "space" );
	XEPL::OPERATORS::Operator_Subtract    ( _cortex,  "sub" );
	XEPL::OPERATORS::Operator_Tab         ( _cortex, "tab" );

	XEPL::OPERATORS::Operator_Depercent   ( _cortex, "depercent" );
	XEPL::OPERATORS::Operator_Percentify  ( _cortex, "percentify" );
}


namespace KITS::SPLICER
{
	void Splicer_Set               CORTEX_REGISTRATION;
	void Splicer_Get               CORTEX_REGISTRATION;
}

void XEPL::Register_Std_Splicers ( XEPL::CORTEX::Cortex* _cortex )
{
	KITS::SPLICER::Splicer_Set       ( _cortex,  "set" );
	KITS::SPLICER::Splicer_Get       ( _cortex,  "get" );
}


void XEPL::RENDER::Jsonl CORTEX_RENDER
{
	if ( ! tlsLobe->index->traits )
		return;

	DNA::StableRecall recall( XEPL::tlsLobe->index, false );

	const char* trait;
	const char* value;
	bool first = true;

	_render->rendition->append( "{ " );
	while ( recall.Next_Trait( &trait, &value ) )
	{
		if (!first)
			_render->rendition->append(", ");
		_render->rendition->append( "\"").append( trait ).append( "\" : \"");
		_render->rendition->append(value).append( "\"" );
		first = false;
	}
	_render->rendition->append( " }\r\n");
}


void XEPL::RENDER::Xml CORTEX_RENDER
{
	auto gene = XEPL::tlsLobe->index;

	gene->Gene_Print_rope( _render->rendition );

	_render->rendition->append( XEPL::EOL );
}


void XEPL::RENDER::Normal CORTEX_RENDER
{
	_render->rendition->append ( *_element->Content() );
}


void XEPL::RENDER::Scribble CORTEX_RENDER
{
	XEPL::TEXT::String script_result;

	if ( _element->content_wire )
		XEPL::RNA::Script_Assign ( _nucleus, _element, nullptr, &script_result );

	XEPL::TEXT::String var  ( _element->Trait_Default ( "gene", "this" ) );
	XEPL::TEXT::String form ( _element->Trait_Default ( "form", script_result.c_str() ) );

	CORTEX::Neuron* neuron = XEPL::CORTEX::Neuron::Locator ( _nucleus, &var, '/' );

	XEPL::DNA::Scope_Terms nesting ( _element );

	if ( neuron )
	{
		XEPL::TRACE( "Scribble", neuron, &form );

		if ( !form.empty() )
		{
			XEPL::TEXT::String options;
			form.Split_ch_rhs ( '?', &options );

			XEPL::DNA::Scope_Terms nest ( &options );

			if ( neuron->Nucleus__Rendered_Form ( _render, &form ) )
				return;

			SOMA::GeneLocator source ( neuron, &form );

			if ( source.gene )
				_render->Rendron__Render_Gene ( _nucleus, source.gene );
			else
				XEPL::SOMA::xeplCantFind ( "Form", neuron, form.c_str() );

			return;
		}
	}

	XEPL::SOMA::ErrorReport ( "Scribble: Gene not found: " )+ ( &var );
}


void XEPL::RENDER::Script CORTEX_RENDER
{
	XEPL::TEXT::String script_result;
	bool truth = false;

	XEPL::RNA::RnaScript ( _nucleus, _element, nullptr, &script_result, &truth, nullptr );

	_render->rendition->append ( script_result );

	if ( truth )
		_render->Process_Inner_Genes ( _element );
}


void XEPL::Register_Std_Renkeys ( XEPL::CORTEX::Cortex* _cortex )
{
	_cortex->Register_Render ( "Scribble", &XEPL::RENDER::Scribble );
	_cortex->Register_Render ( "n",        &XEPL::RENDER::Normal );
	_cortex->Register_Render ( "xeal",     &XEPL::RENDER::Script );
	_cortex->Register_Render ( "Jsonl",    &XEPL::RENDER::Jsonl );
	_cortex->Register_Render ( "Xml",      &XEPL::RENDER::Xml );
	_cortex->Enable_Renkey  ( "For" );
	_cortex->Enable_Renkey  ( "ForEach" );
	_cortex->Enable_Renkey  ( "Print" );
	_cortex->Enable_Renkey  ( "Enter" );
	_cortex->Enable_Renkey  ( "Set" );
	_cortex->Enable_Renkey  ( "Index" );
}


XEPL::RENDER::RendronHtml::RendronHtml ( XEPL::CORTEX::Neuron* _host, XEPL::DNA::Gene* _config, XEPL::TEXT::String* _bag )
	: Rendron ( _host, _config, _bag )
{}


void XEPL::RENDER::RendronHtml::Rendron__Method_Markup ( XEPL::DNA::Gene* _call_gene, XEPL::DNA::Gene* )
{
	ATOM::Bond* bond = _call_gene->inner_genes->head;

	if ( _call_gene->Trait_Get_bool ( "tag", true ) )
	{
		XEPL::XML::XmlBuilder x1 ( "html", rendition );
		x1.Close_Traits();

		while ( bond )
		{
			DNA::Gene* gene   = static_cast<DNA::Gene*> ( bond->atom );
			Markup ( this, gene, rendition );
			bond = bond->next;
		}
	}
	else
	{
		while ( bond )
		{
			DNA::Gene* gene   = static_cast<DNA::Gene*> ( bond->atom );
			Markup ( this, gene, rendition );
			bond = bond->next;
		}
	}
}


void XEPL::RENDER::RendronHtml::Rendron__Render_Gene ( XEPL::CELL::Nucleus* _nucleus, XEPL::DNA::Gene* _gene )
{
	XEPL::XML::XmlBuilder x1 ( "span", rendition );
	x1.Close_Traits();
	Scribble_Gene ( _nucleus, _gene );
}


void XEPL::RENDER::RendronHtml::Scribble_Traits ( XEPL::CELL::Nucleus*, XEPL::DNA::Gene* _element )
{
	if ( _element->traits )
	{
//		XEPL::XML::BuildXml x1 ( "span", "class='memoAttributes'", rendition );

		const char* name = nullptr;
		const char* term = nullptr;
		XEPL::DNA::StableRecall recall ( _element, false );

		while ( recall.Next_Trait ( &name, &term ) )
		{
			XEPL::XML::XmlBuilder x2 ( "span", "class='trait'", rendition );
			{
				XEPL::XML::XmlBuilder x3 ( "span", "class='traitName'", rendition );
				rendition->append ( name );
			}
			rendition->append ( " = " );
			{
				XEPL::XML::XmlBuilder x3 ( "span", "class='traitValue'", rendition );
				rendition->append ( term );
			}
		}
	}
}


void XEPL::RENDER::RendronHtml::Scribble_Content ( XEPL::CELL::Nucleus*, XEPL::DNA::Gene* _element )
{
	if ( _element && _element->Content() )
	{
		XEPL::XML::XmlBuilder x1 ( "span", "class='geneWire' onclick='PreThis(this);'", rendition );
		rendition->append ( *_element->Content() );
	}
}


void XEPL::RENDER::RendronHtml::Scribble_Gene ( XEPL::CELL::Nucleus* _nucleus, XEPL::DNA::Gene* _element )
{
	if ( !_element )
		return;

	XEPL::DNA::StableRecall recall ( _element, true );

	bool kids = recall.Has_Genes();
	{
		XEPL::XML::XmlBuilder x1 ( "div", "class='gene'", rendition );
		{
			XEPL::XML::XmlBuilder x2 ( "span", rendition );

			if ( kids )
			{
				x2.Attribute_Set ( "class", "geneName" );
				x2.Attribute_Set ( "onclick", "Debug(this,event);" );
			}
			else
				x2.Attribute_Set ( "class", "geneName" );

			x2.Close_Traits();

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

	if ( kids )
	{
		XEPL::XML::XmlBuilder x1 ( "div", "class='subgenes' ", rendition );

		XEPL::DNA::Gene* gene;

		while ( recall.Next_Gene ( &gene ) )
			Scribble_Gene ( _nucleus, gene );
	}
}


XEPL::RENDER::RendronText::RendronText ( XEPL::CORTEX::Neuron* _parent, XEPL::DNA::Gene* _config, XEPL::TEXT::String* _bag )
	: Rendron ( _parent, _config, _bag )
{}


void XEPL::RENDER::RendronText::Rendron__Method_Markup ( XEPL::DNA::Gene* _gene, XEPL::DNA::Gene* )
{
	ATOM::Bond* bond = _gene->inner_genes->head;

	while ( bond )
	{
		DNA::Gene* gene =  static_cast<DNA::Gene*> ( bond->atom );
		Markup ( this, gene, rendition );
		bond = bond->next ;
	}
}


KITS::SOCKET::Socket::Socket ( XEPL::CORTEX::Neuron*  _parent, XEPL::DNA::Gene* _config )
	: Neuron                     ( _parent, _config )
	, os_socket                  ( nullptr )
{}

KITS::SOCKET::Socket::~Socket()
{
	if ( os_socket )
	{
		os_socket->Close_Socket();
		os_socket->Release();
	}
}


KITS::SOCKET::SocketControl::SocketControl ( KITS::SOCKET::SocketMan* _socket_man )
	: Socket                     ( _socket_man, _socket_man->config_gene )
	, socket_control             ( new XEPL::OS::osSocketControl ( this ) )
{
	Socket::os_socket = socket_control;

	XEPL::SIGNAL::Signal* signal = new XEPL::SIGNAL::Signal ( socket_control );
	XEPL::ATOM::Scope_Release release ( signal );

	_socket_man->Do_Read ( signal, nullptr );
}


void KITS::SOCKET::SocketControl::Send()
{
	socket_control->Send();
}


XEPL::OS::osSocketAddress* KITS::SOCKET::SocketControl::SocketAddress()
{
	return socket_control->socket_address;
}


KITS::SOCKET::SocketMan::~SocketMan()
{
	delete fdSet;
}

KITS::SOCKET::SocketMan::SocketMan ( XEPL::CORTEX::Neuron*  _parent, XEPL::DNA::Gene* _config )
	: XEPL::CORTEX::Lobe         ( _parent, _config )
	, controlSocket              ( nullptr )
	, fdSet                      ( new XEPL::OS::osFdSet() )
	, eRead                      ( new XEPL::SIGNAL::Axon ( this, "Read",   XEPL::SIGNAL::Axon::REPEATS ) )
	, eWrite                     ( new XEPL::SIGNAL::Axon ( this, "Write",  XEPL::SIGNAL::Axon::REPEATS  ) )
	, eExcept                    ( new XEPL::SIGNAL::Axon ( this, "Except", XEPL::SIGNAL::Axon::REPEATS  ) )
	, eCancel                    ( new XEPL::SIGNAL::Axon ( this, "Cancel", XEPL::SIGNAL::Axon::REPEATS  ) )
{}


void KITS::SOCKET::SocketMan::Do_Cancel SIGNAL_RECEIVER
{
	XEPL::SIGNAL::Signal* signal = static_cast<XEPL::SIGNAL::Signal*> ( _signal );

	XEPL::OS::osFd* fd = static_cast<XEPL::OS::osFd*> ( signal->stimulus );

	Unhook ( fd );
}


void KITS::SOCKET::SocketMan::Do_Except SIGNAL_RECEIVER
{
	XEPL::SIGNAL::Signal* signal = static_cast<XEPL::SIGNAL::Signal*> ( _signal );

	XEPL::OS::osFd* fd = static_cast<XEPL::OS::osFd*> ( signal->stimulus );

	fdSet->except_pair->Set_Fd_Handler ( fd, fd->osFd__OnFdExcept() );
}


void KITS::SOCKET::SocketMan::Do_Read SIGNAL_RECEIVER
{
	XEPL::SIGNAL::Signal* signal = static_cast<XEPL::SIGNAL::Signal*> ( _signal );

	XEPL::OS::osFd* fd = static_cast<XEPL::OS::osFd*> ( signal->stimulus );

	fdSet->read_pair->Set_Fd_Handler ( fd, fd->osFd__OnFdRead() );
}


void KITS::SOCKET::SocketMan::Do_Write SIGNAL_RECEIVER
{
	XEPL::SIGNAL::Signal* signal = static_cast<XEPL::SIGNAL::Signal*> ( _signal );

	XEPL::OS::osFd* fd = static_cast<XEPL::OS::osFd*> ( signal->stimulus );

	fdSet->write_pair->Set_Fd_Handler ( fd, fd->osFd__OnFdWrite() );
}


void KITS::SOCKET::SocketMan::Lobe__Born()
{
	config_gene->Trait_Set ( "node", "127.0.0.1:0" );
	Subscribe ( eRead,   ( XEPL::SIGNAL::Receiver )&SocketMan::Do_Read,   nullptr );
	Subscribe ( eWrite,  ( XEPL::SIGNAL::Receiver )&SocketMan::Do_Write,  nullptr );
	Subscribe ( eExcept, ( XEPL::SIGNAL::Receiver )&SocketMan::Do_Except, nullptr );
	Subscribe ( eCancel, ( XEPL::SIGNAL::Receiver )&SocketMan::Do_Cancel, nullptr );
	XEPL::CORTEX::Lobe::Lobe__Born();
	controlSocket  = new KITS::SOCKET::SocketControl ( this );
}

void KITS::SOCKET::SocketMan::Lobe__Dying()
{
	Unhook ( controlSocket->Get_osFd() );
	XEPL::CORTEX::Lobe::Lobe__Dying();
}


void KITS::SOCKET::SocketMan::Lobe__Rest_Now()
{
	fdSet->Wait_On_Selected();
}

void KITS::SOCKET::SocketMan::Lobe__Wake_Up()
{
	controlSocket->Send();
}


void KITS::SOCKET::SocketMan::Unhook ( XEPL::OS::osFd* _fd )
{
	fdSet->Ignore ( _fd, fdSet->read_pair );
	fdSet->Ignore ( _fd, fdSet->write_pair );
	fdSet->Ignore ( _fd, fdSet->except_pair );
}


KITS::SOCKET::SocketTcp::~SocketTcp()
{}

KITS::SOCKET::SocketTcp::SocketTcp ( XEPL::CORTEX::Neuron* _parent, XEPL::DNA::Gene* _config )
	: Socket         ( _parent, _config )
	, os_tcp_socket  ( nullptr )
	, aborted        ( false )
	, closed_axon    ( nullptr )
{
	closed_axon = new XEPL::SIGNAL::Axon ( this, "closed", XEPL::SIGNAL::Axon::PULSE );
	Register_Method ( "SendContent", ( XEPL::CELL::Function )&SocketTcp::Method_SendContent, nullptr );
	Register_Method ( "Start",       ( XEPL::CELL::Function )&SocketTcp::Method_Start,       nullptr );
	Register_Method ( "Closed",      ( XEPL::CELL::Function )&SocketTcp::Method_Closed,      nullptr );
}

KITS::SOCKET::SocketTcp::SocketTcp ( XEPL::OS::osTcpSocket* _client, XEPL::CORTEX::Neuron*  _parent, XEPL::DNA::Gene* _config )
	: Socket         ( _parent, _config )
	, os_tcp_socket  ( _client )
	, aborted        ( false )
	, closed_axon    ( nullptr )
{
	os_socket = os_tcp_socket;
	os_tcp_socket->soc_tcp = this;

	closed_axon = new XEPL::SIGNAL::Axon ( this, "closed", XEPL::SIGNAL::Axon::PULSE );

	Property_Set    ( "port",        _config->Trait_Default ( "from", nullptr ) );
	Register_Method ( "SendContent", ( XEPL::CELL::Function )&SocketTcp::Method_SendContent, nullptr );
	Register_Method ( "Start",       ( XEPL::CELL::Function )&SocketTcp::Method_Start,       nullptr );
	Register_Method ( "Closed",      ( XEPL::CELL::Function )&SocketTcp::Method_Closed,      nullptr );
}


void KITS::SOCKET::SocketTcp::Closed_Aborted()
{
	aborted = true;
	closed_axon->Trigger ( nullptr );
}


void KITS::SOCKET::SocketTcp::Closed_Normal()
{
	closed_axon->Trigger ( nullptr );
}


void KITS::SOCKET::SocketTcp::Method_Closed NUCLEUS_METHOD
{
	Nucleus__Dropped();
}


void KITS::SOCKET::SocketTcp::Method_SendContent NUCLEUS_METHOD
{
	XEPL::SOMA::LocateIndex source ( this, _call->space_string );

	if ( !source.gene || !source.gene->content_wire )
		return;

	Send ( source.gene->content_wire );
	Process_Inner_Genes ( _call );
}


void KITS::SOCKET::SocketTcp::Method_Start NUCLEUS_METHOD
{
	Start();
}


void KITS::SOCKET::SocketTcp::Nucleus__Dropped()
{
	if ( dropped )
		return;

	KITS::SOCKET::SocketMan* socket_manager = Get_osFd()->socket_manager;

	if ( socket_manager )
		socket_manager->eCancel->Trigger_Wait ( os_tcp_socket );

	Socket::Nucleus__Dropped();
}


void KITS::SOCKET::SocketTcp::Send ( XEPL::TEXT::Wire* _bag )
{
	if ( !_bag )
		return;

	XEPL::THREAD::MutexScope lock_string ( _bag->wire_lock );

	while ( _bag->Avail() && !aborted )
	{
		if ( os_tcp_socket->Has_Backpressure() )
		{
			os_tcp_socket->Hold_Data_Back ( _bag );
			return;
		}

		os_tcp_socket->Send_Data ( _bag );
	}
}


void KITS::SOCKET::SocketTcp::Set_Tcp_Socket ( XEPL::OS::osTcpSocket* _tcp )
{
	os_socket = _tcp;
	os_tcp_socket = _tcp;
}


void KITS::SOCKET::SocketTcp::Start()
{
	KITS::SOCKET::SocketMan* socket_manager = Get_osFd()->socket_manager;

	if ( socket_manager )
	{
		XEPL::SIGNAL::Signal* signal = new XEPL::SIGNAL::Signal ( os_tcp_socket );
		XEPL::ATOM::Scope_Release release ( signal );

		socket_manager->eRead->Trigger   ( signal );
		socket_manager->eExcept->Trigger ( signal );
	}
}


XEPL::OS::osFd* KITS::SOCKET::Socket::Get_osFd()
{
	return os_socket;
}


void KITS::SOCKET::Socket::Nucleus__Dropped()
{
	os_socket->Close_Socket();
	Neuron::Nucleus__Dropped();
}


XEPL::OS::osSocketAddress*& KITS::SOCKET::Socket::SocketAddress()
{
	return os_socket->socket_address;
}


KITS::TERMINAL::StdIn::StdIn ( XEPL::CORTEX::Neuron*  _parent, XEPL::DNA::Gene* _config )
	: XEPL::CORTEX::Sensor ( _parent, _config )
	, os_stdin   ( nullptr )
	, dispatcher ( nullptr )
{
	XEPL::CORTEX::Neuron* found = nullptr;

	if ( Find_Neuron ( "SocketMan", &found ) )
	{
		dispatcher = static_cast<KITS::SOCKET::SocketMan*> ( found );

		XEPL::SIGNAL::Axon* axon = new XEPL::SIGNAL::Axon ( this->Nucleus__Host_Neuron(), "Lobe",  XEPL::SIGNAL::Axon::REPEATS );

		os_stdin = new KITS::TERMINAL::osStdIn ( this );
		os_stdin->Set_Data_Axon ( axon );

		Subscribe ( axon, ( XEPL::SIGNAL::Receiver )&KITS::TERMINAL::osStdIn::Fd_Receive, nullptr );

		XEPL::SIGNAL::Signal* signal = new XEPL::SIGNAL::Signal ( os_stdin );
		XEPL::ATOM::Scope_Release release ( signal );

		dispatcher->eRead->Trigger ( signal );
	}
}

KITS::TERMINAL::StdIn::~StdIn()
{
	if ( os_stdin )
		os_stdin->Release();
}

void KITS::TERMINAL::StdIn::Nucleus__Dropped ( void )
{
	dispatcher->eCancel->Trigger_Wait ( os_stdin );
	XEPL::CORTEX::Sensor::Nucleus__Dropped();
}

void KITS::TERMINAL::StdIn::Sensor__Scan ( void )
{
	auto lobe = XEPL::tlsLobe;
	XEPL::DNA::Gene* std_in = static_cast<XEPL::DNA::Gene*> ( lobe->current_stimulus );
	const char* command = std_in->Trait_Default ( "command", nullptr );

	if ( !command )
		return;

	lobe->cortex->Cortex__Can_Execute_Cmd ( command );
//	std::cout << "Received: " << *command << XEPL::EOL;
//	std::cout << "Keystrokes: " << *string << XEPL::EOL << std::flush;
}

void KITS::TERMINAL::StdIn::Sensor__Closed ( void )
{
	Nucleus__Dropped();
}


KITS::SOCKET::TcpClient::~TcpClient()
{}

KITS::SOCKET::TcpClient::TcpClient ( XEPL::CORTEX::Neuron*  _parent, XEPL::DNA::Gene* _config )
	: SocketTcp ( _parent, _config )
	, os_tcp_client              ( nullptr )
	, eConnect                   ( new XEPL::SIGNAL::Axon ( this, "eConnection",  XEPL::SIGNAL::Axon::PULSE ) )
	, eExcept                    ( new XEPL::SIGNAL::Axon ( this, "eException",   XEPL::SIGNAL::Axon::PULSE ) )
	, eAbort                     ( new XEPL::SIGNAL::Axon ( this, "eAborted",     XEPL::SIGNAL::Axon::PULSE ) )
	, retries                    ( 0 )
	, attempt                    ( 0 )
	, connected                  ( false )
{
	Set_Client_Socket ( new XEPL::OS::osTcpClient ( this ) );

	if ( !os_tcp_client->Set_Manager ( this ) )
		return;
	
	os_tcp_client->Create_Tcp_Socket();

	XEPL::SIGNAL::Signal* signal = new XEPL::SIGNAL::Signal ( os_tcp_client );
	XEPL::ATOM::Scope_Release release ( signal );

	os_socket->socket_manager->eRead->Trigger   ( signal );
	os_socket->socket_manager->eWrite->Trigger  ( signal );
	os_socket->socket_manager->eExcept->Trigger ( signal );

	os_tcp_client->Set_Data_Axon ( new XEPL::SIGNAL::Axon ( this, "eReceived",  XEPL::SIGNAL::Axon::REPEATS ) );

	SocketAddress() = new XEPL::OS::osSocketAddress ( _config );

	Property_Set ( "uri", _config->Trait_Default ( "uri", nullptr ) );

	retries = _config->Trait_Get_long ( "retries", retries );
	XEPL::TEXT::String ipaddr ( os_tcp_client->server_address->IpString() );
	Property_Set ( "ip", &ipaddr );

	Subscribe ( eConnect,  ( XEPL::SIGNAL::Receiver )&TcpClient::Server_Connected, nullptr );
}


void KITS::SOCKET::TcpClient::Connect()
{
	os_tcp_client->osFd_Connect();
}


void KITS::SOCKET::TcpClient::Server_Connected SIGNAL_RECEIVER
{
	Performed_Method("Connected", nullptr );
}


bool KITS::SOCKET::TcpClient::Fd_Connecting()
{
	if ( connected )
		return os_tcp_client->Send_Available();

	connected = true;
//	os_socket->socket_manager->eCancel->Trigger( ( Get_osFd() ));

	eConnect->Trigger ( this );
	return false;
}


bool KITS::SOCKET::TcpClient::Fd_Excepting()
{
	if ( !connected )
	{
		if ( attempt < retries )
		{
			++attempt;
			os_tcp_client->osFd_Connect();
			return true;
		}

		eExcept->Trigger ( this );
	}

	eAbort->Trigger ( this );
	return false;
}


void KITS::SOCKET::TcpClient::Set_Client_Socket ( XEPL::OS::osTcpClient* _tcp )
{
	os_tcp_client = _tcp;
	Set_Tcp_Socket ( _tcp );
//	closed_axon = new XEPL::SIGNAL::Axon ( this, "closed", XEPL::SIGNAL::Axon::PULSE  );
	Start();
}


KITS::SOCKET::TcpServer::~TcpServer()
{
	delete nodeName;
}

KITS::SOCKET::TcpServer::TcpServer ( XEPL::CORTEX::Neuron*  _parent, XEPL::DNA::Gene* _config )
	: SocketTcp      ( _parent, _config )
	, eConnecting                ( new XEPL::SIGNAL::Axon ( this, "connecting", XEPL::SIGNAL::Axon::TAIL ) )
	, nodeName                   ( nullptr )
	, server_socket              ( nullptr )
{
	Set_Server_Socket ( new XEPL::OS::osTcpServer ( this ) );
	Server_Create();
	Subscribe ( eConnecting, ( XEPL::SIGNAL::Receiver )&TcpServer::Client_Connected, nullptr );
	Start();
	Listen();
}


void KITS::SOCKET::TcpServer::Client_Connected SIGNAL_RECEIVER
{
	ConnectedClient* connectedTo = static_cast<ConnectedClient*> ( _signal );

	XEPL::OS::osTcpSocket* client = new XEPL::OS::osTcpSocket ( this, connectedTo->descriptor, connectedTo->address );

	client->Set_Manager ( this );

	XEPL::DNA::Scope_Duplicate clone ( config_gene );
	clone.gene->Make_Gene_Safe ( true );

	clone.gene->Trait_Tap ( "name", "" )->append ( std::to_string ( client->socket_address->Port() ) );
	clone.gene->Trait_Set ( "from", client->socket_address->IpString() );

	SocketTcp* newSocket = new SocketTcp ( client, this, clone.gene );

	newSocket->alias = new XEPL::TEXT::String ( "TcpServer" );

	client->Set_Data_Axon ( new XEPL::SIGNAL::Axon ( newSocket, "eReceived", XEPL::SIGNAL::Axon::REPEATS ) );

	if ( process_now )
		newSocket->Process_Inner_Genes ( clone.gene );
}


bool KITS::SOCKET::TcpServer::SocketMan_Connecting ( int _socket, XEPL::OS::osSocketAddress* _address )
{
	ConnectedClient* client  = new ConnectedClient ( _socket, _address );
	XEPL::ATOM::Scope_Release release ( client );
	eConnecting->Trigger ( client );
	return true;
}


void KITS::SOCKET::TcpServer::Listen ( void )
{
	KITS::SOCKET::SocketMan* socket_manager = server_socket->socket_manager;

	server_socket->Listen_For_Connections ( 0 );

	if ( socket_manager )
	{
		XEPL::SIGNAL::Signal* signal = new XEPL::SIGNAL::Signal ( server_socket );
		XEPL::ATOM::Scope_Release release ( signal );

		socket_manager->eRead->Trigger ( signal );
	}
	else
		XEPL::SOMA::ErrorReport ( "No Socketman for " )+ ( nodeName );
}


void KITS::SOCKET::TcpServer::Server_Create ( void )
{
	nodeName = new XEPL::TEXT::String ( config_gene->Trait_Tap ( "node", "127.0.0.1" ) );
	SocketAddress() = new XEPL::OS::osSocketAddress ( config_gene );
	server_socket->Create_Tcp_Socket();
}


void KITS::SOCKET::TcpServer::Set_Server_Socket ( XEPL::OS::osTcpServer* _tcp )
{
	server_socket = _tcp;
	server_socket->Set_Manager ( this );
	Set_Tcp_Socket ( _tcp );
}


struct termios orig_termios;

KITS::TERMINAL::Terminal::Terminal()
{
}

KITS::TERMINAL::Terminal::~Terminal()
{}

int KITS::TERMINAL::Terminal::Read_Key()
{
	char c;
	read ( STDIN_FILENO, &c, 1 );

	if ( c == 0x7f )
		return Terminal::DEL_KEY;

	if ( c == '\x1b' )
	{
		char seq[2];
		read ( STDIN_FILENO, &seq[0], 1 );
		read ( STDIN_FILENO, &seq[1], 1 );

		if ( seq[0] == '[' )
		{
			switch ( seq[1] )
			{
				case 'A':
					return Terminal::ARROW_UP;

				case 'B':
					return Terminal::ARROW_DOWN;

				case 'C':
					return Terminal::ARROW_RIGHT;

				case 'D':
					return Terminal::ARROW_LEFT;

				case 'H':
					return Terminal::HOME_KEY;

				case 'F':
					return Terminal::END_KEY;
			}

			return '\x1b';
		}
	}

	write ( STDOUT_FILENO, &c, 1 );
	return c;
}

#define ESC "\x1b"

void KITS::TERMINAL::Terminal::Write_Backspace()
{
	write ( STDOUT_FILENO, "\x10", 1 ) ;
}
void KITS::TERMINAL::Terminal::Move_Cursor_Left()
{
	write ( STDOUT_FILENO, ESC "[1D", 4 ) ;
}
void KITS::TERMINAL::Terminal::Move_Cursor_Right()
{
	write ( STDOUT_FILENO, ESC "[1C", 4 ) ;
}

void KITS::TERMINAL::Terminal::Clear_To_Eol()
{
	write ( STDOUT_FILENO, ESC "[0K", 4 );
}

void KITS::TERMINAL::Terminal::Move_Bol()
{
	write ( STDOUT_FILENO, "\r", 1 );
}
void KITS::TERMINAL::Terminal::Disable_Raw()
{
	tcsetattr ( STDIN_FILENO, TCSAFLUSH, &orig_termios );
}


KITS::TERMINAL::ScopeRaw::ScopeRaw()
{
	tcgetattr ( STDIN_FILENO, &orig_termios );

	struct termios raw = orig_termios;
	raw.c_lflag &= ~ ( ECHO | ICANON | IEXTEN | ISIG );
	raw.c_iflag &= ~ ( BRKINT | ICRNL | INPCK | ISTRIP | IXON );
	raw.c_oflag &= ~ ( OPOST );
	raw.c_cflag |= ( CS8 );

	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 0;

	tcsetattr ( STDIN_FILENO, TCSAFLUSH, &raw );
}

KITS::TERMINAL::ScopeRaw::~ScopeRaw()
{
	KITS::TERMINAL::Terminal::Disable_Raw();
}


KITS::TIMER::PerformanceTimer* KITS::TIMER::runtime = nullptr;

KITS::TIMER::Ticker::~Ticker()
{
	Stop();
}

KITS::TIMER::Ticker::Ticker ( std::chrono::milliseconds _interval )
	: interval   ( _interval )
	, is_running ( false )
	, tick_count ( 0 )
{}


void KITS::TIMER::Ticker::Start()
{
	if ( is_running )
		return;

	is_running = true;
	tick_count = 0;

	std__thread = std::thread ( [this]()
	{

		XEPL::SOMA::Lobe_Set_Thread_Name ( nullptr, "InterruptibleTimer" );

		XEPL::MEMORY::MemoryBackpack backpack;

		auto next_tick = std::chrono::steady_clock::now() + interval;

		while ( is_running )
		{
			std::unique_lock<std::mutex> lock ( std__mutex );
			std__condition_variable.wait_until ( lock, next_tick, [this]()
			{
				return !is_running;
			} );

			if ( is_running )
			{
				auto start_time = std::chrono::steady_clock::now();
				bool continue_timer = Deliver_Tick ( ++tick_count );

				if ( !continue_timer )
				{
					is_running = false;
					std__thread.detach();
				}

				auto elapsed_time = std::chrono::steady_clock::now() - start_time;
				next_tick += interval - elapsed_time;
			}
		}
	} );

}


void KITS::TIMER::Ticker::Stop()
{
	is_running = false;
	std__condition_variable.notify_all();

	if ( std__thread.joinable() )
		std__thread.join();
}


KITS::TIMER::Timer::~Timer()
{}

KITS::TIMER::Timer::Timer ( XEPL::CORTEX::Neuron*  _parent, XEPL::DNA::Gene* _config )
	: Neuron                     ( _parent, _config )
	, timer_list                 ( nullptr )
	, eFired                     ( new XEPL::SIGNAL::Axon ( this, "eFired", XEPL::SIGNAL::Axon::PULSE ) )
	, offset                     ( -1 )
	, rate                       ( -1 )
	, duration                   ( _config->Trait_Get_long ( "duration", 100 ) )
	, time_is_running            ( false )
{
	process_now = true;

	Subscribe ( eFired,  ( XEPL::SIGNAL::Receiver )&Timer::Handle_eFired, nullptr );

	Register_Method ( "Start",     ( XEPL::CELL::Function )&Timer::Method_Start,      nullptr ); 
	Register_Method ( "Stop",      ( XEPL::CELL::Function )&Timer::Method_Stop,       nullptr );
	Register_Method ( "Terminate", ( XEPL::CELL::Function )&Neuron::Method_Terminate, nullptr );
}


KITS::TIMER::TimerList* KITS::TIMER::Timer::masterTimerList = nullptr;

KITS::TIMER::TimerList::~TimerList()
{
	if ( !timer_list->empty() )
		Stop();

	delete mutex;
	delete timer_list;
}

KITS::TIMER::TimerList::TimerList ( long _duration )
	: Ticker ( ( std::chrono::milliseconds ) ( _duration ) )
	, timer_list                 ( new ListTimers() )
	, mutex                      ( new XEPL::THREAD::Mutex() )
	, pre_scaler                 ( _duration )
{}


bool KITS::TIMER::TimerList::Deliver_Tick ( int64_t )
{
	XEPL::THREAD::MutexScope lock_timerlist ( mutex );

	if ( timer_list->empty() )
		return false;

	Timer* timer = timer_list->front();

	if ( -- ( timer->offset ) == 0 )
	{
		XEPL::DNA::Gene* notification = new XEPL::DNA::Gene ( nullptr, "Tick", nullptr );
		XEPL::ATOM::Scope_Release detach ( notification );

		auto it = timer_list->begin();
		while (it != timer_list->end() && (*it)->offset == 0)
		{
			(*it)->Expired(notification);
			it = timer_list->begin();
		}
	}
	return true;
}


void KITS::TIMER::TimerList::Remove ( Timer* _timer )
{
	XEPL::THREAD::MutexScope lock_timerlist ( mutex );

	long ticks = 0;

	for ( auto it = timer_list->begin(); it != timer_list->end(); ++it )
	{
		ticks = ticks + ( *it )->offset;

		if ( *it == _timer )
		{
			auto nextIt = it;

			if ( ++nextIt != timer_list->end() )
				( *nextIt )->offset += _timer->offset;

			_timer->time_is_running = false;
			_timer->Release();

			break;
		}
	}

	timer_list->remove ( _timer );
}


void KITS::TIMER::TimerList::Run ( Timer* _timer, long _rate )
{
	_rate /= pre_scaler;

	if ( !_rate )
		_rate = 1;

	_timer->rate = _rate;
	_timer->timer_list = this;

	XEPL::THREAD::MutexScope lock_timerlist ( mutex );

	long rate = _timer->rate;
	bool start = timer_list->empty();

	auto   it  = timer_list->begin();

	while ( it != timer_list->end() )
	{
		Timer* its = *it;

		if ( rate < its->offset )
		{
			its->offset = its->offset - rate;
			break;
		}

		rate -= its->offset;
		++it;
	}

	_timer->offset = rate;
	_timer->time_is_running = true;
	++_timer->retain_count;
	timer_list->insert ( it, _timer );

	if ( start )
		Start();
}


void KITS::TIMER::Timer::Expired ( XEPL::DNA::Gene* _report )
{
	timer_list->Remove ( this );
	eFired->Trigger ( _report );
}


void KITS::TIMER::Timer::Handle_eFired SIGNAL_RECEIVER
{
	if ( process_now )
		Process_Inner_Genes ( config_gene );
}


void KITS::TIMER::Timer::Initialize ( long _rate )
{
	Timer::masterTimerList = new TimerList ( _rate );
}

void KITS::TIMER::Timer::Shutdown()
{
	delete Timer::masterTimerList;
	Timer::masterTimerList = nullptr;
}


void KITS::TIMER::Timer::Method_Start NUCLEUS_METHOD
{
	XEPL::TEXT::String start_rate ( _call->Trait_Default ( "rate", nullptr ) );
	Start_Timer ( std::strtol ( start_rate.c_str(), nullptr, 0 ) );
	Process_Inner_Genes ( _call );
}

void KITS::TIMER::Timer::Method_Stop NUCLEUS_METHOD
{
	Stop_Timer();
	Process_Inner_Genes ( _call );
}


void KITS::TIMER::Timer::Nucleus__Dropped()
{
	if ( time_is_running )
		Stop_Timer();

	Neuron::Nucleus__Dropped();
}


void KITS::TIMER::Timer::Stop_Timer()
{
	if ( !time_is_running )
		return;

	timer_list->Remove ( this );
}

void KITS::TIMER::Timer::Start_Timer ( long _rate )
{
	if ( time_is_running )
		Stop_Timer();

	time_is_running = true;
	long pace = _rate;

	if ( !_rate )
		pace = duration;

	Timer::masterTimerList->Run ( this, pace );
}


namespace XEPL::KEYWORDS
{
	void Keyword_Add CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			XEPL::TEXT::String script_result;
			XEPL::RNA::RnaScript ( _nucleus, _call_gene, nullptr, &script_result, nullptr, nullptr );

			XEPL::SOMA::LocateIndex  source ( _nucleus, _call_gene->space_string );
			XEPL::SOMA::GeneLocator resource ( _nucleus, &script_result );

			XEPL::DNA::Gene*& src = source.gene;
			XEPL::DNA::Gene*& res = resource.gene;

			auto lobe = tlsLobe;

			if ( !src || !res )
			{
				lobe->Set_Outdex ( res );
				return;
			}

			res->Make_One_Gene ( _call_gene->Trait_Default ( "into", nullptr ), &res );

			if ( src != res )
				res->Add_Gene ( src );

			if ( _call_gene->inner_genes )
			{
				XEPL::DNA::Scope_Index restore ( src );
				_nucleus->Process_Inner_Genes ( _call_gene );
			}

			lobe->Set_Outdex ( res );
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Assemble CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			XEPL::SOMA::GeneLocator source ( _nucleus, _call_gene->space_string );

			if ( !source.gene )
			{
				XEPL::SOMA::ErrorReport ( "Assemble can't find: " )
				->append ( *_call_gene->space_string );
				return;
			}

			_nucleus->Process_Inner_Genes ( source.gene );

			if ( _call_gene->inner_genes )
				_nucleus->Process_Inner_Genes ( _call_gene );
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Attach  CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			XEPL::TEXT::String script_result;

			XEPL::SOMA::LocateIndex   source ( _nucleus, _call_gene->space_string );
			XEPL::SOMA::GeneLocator  resource ( _nucleus, &script_result );

			XEPL::RNA::RnaScript ( _nucleus, _call_gene, nullptr, &script_result, nullptr, nullptr );

			if ( !source.gene || !resource.gene )
				return tlsLobe->Set_Outdex ( resource.gene );

			resource.gene->Content_Attach ( source.gene->content_wire );

			tlsLobe->Set_Outdex ( resource.gene );
		} );
	}
}
namespace XEPL::KEYWORDS
{
	void Keyword_Set CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
								   {
			if ( !_call_gene )
				return;

			CORTEX::Lobe* lobe = XEPL::tlsLobe;

			if ( !lobe->ephemerals )
				lobe->ephemerals = new XEPL::DNA::Ephemerals();

			XEPL::TEXT::String script_result;

			XEPL::SOMA::LocateIndex   source ( _nucleus, _call_gene->space_string );

			XEPL::RNA::RnaScript ( _nucleus, _call_gene, nullptr, &script_result, nullptr, nullptr );

			lobe->ephemerals->Set ( &script_result, source.gene );
		} );
	};
}


namespace XEPL::KEYWORDS
{
	void Keyword_Axon CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			DNA::Scope_Duplicate duplicate ( _call_gene );

			if ( _call_gene->content_wire )
				RNA::RnaScript ( _nucleus->Nucleus__Host_Neuron(), duplicate.gene );

			TEXT::String axon_name;
			duplicate.gene->Trait_Get ( "name", &axon_name );

			SIGNAL::Axon* axon;
			if ( !_nucleus->Nucleus__Host_Neuron()->Axon_Hunt ( &axon_name, &axon ) )
				axon = new SIGNAL::Axon ( _nucleus->Nucleus__Host_Neuron(), duplicate.gene );

			duplicate.gene->Traits_Release();

			if ( _call_gene->inner_genes )
				axon->Subscribe ( _nucleus, duplicate.gene );
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Axons CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			ATOM::Bond* bond = _call_gene->inner_genes->head;

			while ( bond )
			{
				DNA::Gene* gene   = static_cast<DNA::Gene*> ( bond->atom );
				DNA::Gene* rename = new DNA::Gene ( nullptr, "Axon", nullptr );

				ATOM::Scope_Release detach ( rename );
				rename->Traits_Absorb ( gene );
				rename->Gene_Absorb ( gene );
				rename->Trait_Set ( "name", gene->cell_name );

				SIGNAL::Axon* axon = nullptr;

				TEXT::String axon_name;
				_call_gene->Trait_Get ( "name", &axon_name );

				if ( !_nucleus->Nucleus__Host_Neuron()->Axon_Hunt ( &axon_name, &axon ) )
					axon = new SIGNAL::Axon ( _nucleus->Nucleus__Host_Neuron(), gene );

				if ( gene->inner_genes )
					axon->Subscribe ( _nucleus, gene );

				bond = bond->next ;
			}
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Clear CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			XEPL::SOMA::LocateIndex source ( _nucleus, _call_gene->space_string );

			source.gene->Gene_Deflate();

			XEPL::DNA::Scope_Index restore ( source.gene );

			_nucleus->Process_Inner_Genes ( _call_gene );
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Combine CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			XEPL::TEXT::String script_result;
			XEPL::RNA::RnaScript ( _nucleus, _call_gene, nullptr, &script_result, nullptr, nullptr );

			XEPL::SOMA::LocateIndex   source ( _nucleus, _call_gene->space_string );
			XEPL::SOMA::GeneLocator  resource ( _nucleus, &script_result );

			source.gene->Genes_Absorb ( resource.gene );

			XEPL::tlsLobe->Set_Outdex ( source.gene );
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Del CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			const char* whereIs = _call_gene->Trait_Default ( "where", nullptr );

			XEPL::TEXT::String script_result;
			bool script_truth = true;

			XEPL::RNA::RnaScript ( _nucleus, _call_gene, nullptr, &script_result, &script_truth, nullptr );

			XEPL::TEXT::String tag;

			if ( script_truth )
				tag.assign ( _call_gene->Trait_Default ( "tag", script_result.c_str() ) );

			XEPL::SOMA::LocateIndex source ( _nucleus, _call_gene->space_string );

			XEPL::DNA::StableRecall stable_gene ( source.gene, &tag );

			if ( whereIs )
			{
				XEPL::TEXT::String whereis ( whereIs );
				XEPL::DNA::Gene* gene;

				while (  stable_gene.Next_Gene ( &gene ) )
				{
					bool truth = true;

					XEPL::DNA::Scope_Index restore ( gene );
					XEPL::RNA::RnaScript ( _nucleus, gene, &whereis, nullptr, &truth, nullptr );

					if ( truth )
					{
						_nucleus->Process_Inner_Genes ( _call_gene );
						source.gene->Remove_Gene ( gene );
					}
				}
			}
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Dismiss CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			if ( !_call_gene->content_wire )
			{
				XEPL::SOMA::LocateIndex source ( _nucleus, _call_gene->space_string );

				if ( !_call_gene->space_string )
					source.gene->gene_owner->Remove_Gene ( source.gene );

				return;
			}

			XEPL::SOMA::LocateIndex source ( _nucleus, _call_gene->space_string );

			XEPL::TEXT::String script_result;
			XEPL::RNA::RnaScript ( _nucleus, _call_gene, nullptr, &script_result, nullptr, nullptr );

			XEPL::DNA::Gene* res = nullptr;
			source.gene->Get_First_Gene ( &script_result, &res );
			source.gene->Remove_Gene ( res );

			XEPL::tlsLobe->Set_Outdex ( source.gene );
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Enter CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			XEPL::TEXT::String script_result;

			if ( _call_gene->content_wire )
				XEPL::RNA::RnaScript ( _nucleus, _call_gene, nullptr, &script_result, nullptr, nullptr );

			if ( !_call_gene->inner_genes )
				return;

			XEPL::SOMA::LocateIndex source ( _nucleus, _call_gene->space_string );

			XEPL::DNA::StableRecall stable_gene ( source.gene, &script_result );
			XEPL::DNA::Gene* gene;
			while ( stable_gene.Next_Gene ( &gene ) )
			{
				XEPL::DNA::Scope_Index restore ( gene );
				_nucleus->Process_Inner_Genes ( _call_gene );
			}

		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Find CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			bool truth = true;
			XEPL::TEXT::String script_result;
			XEPL::RNA::RnaScript ( _nucleus, _call_gene, nullptr, &script_result, &truth, nullptr );

			SOMA::LocateIndex source ( _nucleus, _call_gene->space_string );

			if ( !source.gene )
				return;

			XEPL::TEXT::String content;

			if ( !truth )
				_call_gene->Trait_Get ( "tag", &content );
			else
				content.assign ( script_result );

			XEPL::DNA::Gene* loaded = nullptr;

			if ( source.gene->Get_First_Gene ( &content, &loaded ) )
			{
				XEPL::DNA::Scope_Index restore ( loaded );
				( void )_nucleus->Process_Inner_Gene ( "Yes", _call_gene );
			}
			else
			{
				( void )_nucleus->Process_Inner_Gene ( "No", _call_gene );
				source.gene->Get_First_Gene ( &content, &loaded );
			}

			XEPL::DNA::Scope_Index restore ( loaded );
			( void )_nucleus->Process_Inner_Gene ( "Then", _call_gene );

			if ( loaded )
				tlsLobe->Set_Outdex ( loaded );
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_First CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			XEPL::TEXT::String script_result;
			XEPL::RNA::RnaScript ( _nucleus, _call_gene, nullptr, &script_result, nullptr, nullptr );

			if ( !_call_gene->inner_genes )
				return;

			XEPL::SOMA::LocateIndex source ( _nucleus, _call_gene->space_string );

			if ( source.gene && source.gene->inner_genes )
			{
				auto head = source.gene->inner_genes->head;

				if ( head )
				{
					XEPL::DNA::Scope_Index restore ( (XEPL::DNA::Gene*)head->atom );
					_nucleus->Process_Inner_Genes ( _call_gene );
				}
			}

		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_For CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			XEPL::TEXT::String script_result;

			if ( _call_gene->content_wire )
				XEPL::RNA::Script_Assign ( _nucleus, _call_gene, nullptr, &script_result );

			XEPL::SOMA::LocateIndex source ( _nucleus, _call_gene->space_string );

			XEPL::DNA::StableRecall recall ( source.gene, &script_result );

			if ( !recall.Has_Genes() )
			{
				_nucleus->Process_Inner_Gene ( "None", _call_gene );
				return;
			}

			XEPL::DNA::Gene* element;
			recall.Next_Gene ( &element );

			{
				XEPL::DNA::Scope_Index restore ( element );

				( void )_nucleus->Process_Inner_Gene ( "First", _call_gene );
			}

			bool doEach = true;
			bool doMore = true;

			do
			{
				if ( doEach )
				{
					XEPL::DNA::Scope_Index restore ( element );
					doEach = _nucleus->Process_Inner_Gene ( "Each", _call_gene );
				}

				{
					XEPL::DNA::Scope_Index restore ( element );

					if ( !recall.Has_Genes() )
					{
						if ( doMore )
							doMore = _nucleus->Process_Inner_Gene ( "More", _call_gene );
					}
					else
						_nucleus->Process_Inner_Gene ( "Last",  _call_gene );
				}
			}
			while ( recall.Next_Gene ( &element ) );

			_nucleus->Process_Inner_Gene ( "Done", _call_gene );
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_ForEach CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			if ( !_call_gene->inner_genes )
				return;

			XEPL::SOMA::LocateIndex source ( _nucleus, _call_gene->space_string );

			XEPL::DNA::Scope_Index replace_with( source.gene );

			TEXT::String lhs;
			TEXT::String rhs;

			if ( _call_gene->content_wire )
				XEPL::RNA::RnaScript ( _nucleus, _call_gene, nullptr, &lhs, nullptr, nullptr );

			while ( lhs.Split_ch_lhs(':',&rhs ) )
			{
				XEPL::DNA::Scope_Terms press( "for", &rhs );
				
				_nucleus->Process_Inner_Genes ( _call_gene );
			}
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Form CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			_nucleus->Register_Form ( _call_gene );
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Forms CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			ATOM::Bond* bond = _call_gene->inner_genes->head;

			while ( bond )
			{
				DNA::Gene* gene =  static_cast<DNA::Gene*> ( bond->atom );
				_nucleus->Register_Form ( gene );
				bond=bond->next;
			}
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Gene CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			const char* name = _call_gene->Trait_Default ( "name", nullptr );

			if ( !name )
				return;

			_nucleus->Register_Gene ( name, _call_gene );
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Genes CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			ATOM::Bond* bond = _call_gene->inner_genes->head;

			while ( bond )
			{
				DNA::Gene* gene =  static_cast<DNA::Gene*> ( bond->atom );
				_nucleus->Register_Gene ( gene->cell_name, gene );
				bond = bond->next;
			}
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Html CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			if ( !_nucleus || !_call_gene )
				return;

			if ( _call_gene->content_wire )
				XEPL::RNA::RnaScript ( _nucleus->Nucleus__Host_Neuron(), _call_gene );

			XEPL::TEXT::String* output = _nucleus->Nucleus__Host_Neuron()->output;

			XEPL::RENDER::RendronHtml renderer ( _nucleus->Nucleus__Host_Neuron(), _call_gene, output );

			renderer.Process_Inner_Genes ( _call_gene );
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_HttpClient CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			if ( !_nucleus || !_call_gene )
				return;

			if ( _call_gene->content_wire )
				XEPL::RNA::RnaScript ( _nucleus->Nucleus__Host_Neuron(), _call_gene );

			( new KITS::HTTP::HttpClient ( _nucleus->Nucleus__Host_Neuron(), _call_gene ) )->Process_Inner_Genes ( _call_gene );
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_HttpServer CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			if ( !_nucleus || !_call_gene )
				return;

			if ( _call_gene->content_wire )
				XEPL::RNA::RnaScript ( _nucleus->Nucleus__Host_Neuron(), _call_gene );

			( new KITS::HTTP::HttpServer ( _nucleus->Nucleus__Host_Neuron(), _call_gene ) )->Process_Inner_Genes ( _call_gene );
		} );
	}
}


namespace XEPL::KEYWORDS
{
	static bool If_Yes CORTEX_KEYWORD
	{
		return _nucleus->Process_Inner_Gene ( "Yes", _call_gene );
	}

	static bool If_No CORTEX_KEYWORD
	{
		return _nucleus->Process_Inner_Gene ( "No", _call_gene );
	}

	void Keyword_If CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			XEPL::SOMA::LocateIndex source ( _nucleus, _call_gene->space_string );

			XEPL::DNA::Scope_Index replace ( source.gene );

			bool truth=false;
			XEPL::RNA::Script_Truth ( _nucleus, source.gene, _call_gene->Content(), &truth );

			if ( truth )
				If_Yes ( _nucleus, _call_gene );
			else
				If_No ( _nucleus, _call_gene );
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Iff CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			XEPL::SOMA::LocateIndex source ( _nucleus, _call_gene->space_string );

			XEPL::DNA::Scope_Index replace ( source.gene );

			bool truth = true;
			XEPL::RNA::Script_Truth ( _nucleus, source.gene, _call_gene->Content(), &truth );

			if ( truth )
				_nucleus->Process_Inner_Genes ( _call_gene );
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Index CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			XEPL::TEXT::String script_result;
			XEPL::RNA::RnaScript ( _nucleus, _call_gene, nullptr, &script_result, nullptr, nullptr );

			XEPL::SOMA::GeneLocator resource ( _nucleus, &script_result );

			XEPL::DNA::Gene*& res = resource.gene;

			if ( !res )
				return;

			XEPL::DNA::Scope_Index restore ( res );
			_nucleus->Process_Inner_Genes ( _call_gene );
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Keep CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			XEPL::TEXT::String script_result;
			XEPL::RNA::RnaScript ( _nucleus, _call_gene, nullptr, &script_result, nullptr, nullptr );

			XEPL::SOMA::GeneLocator src ( _nucleus, _call_gene->space_string ) ;
			XEPL::SOMA::GeneLocator res ( _nucleus, &script_result );

			if ( !src.gene || !res.gene )
				return;

			src.gene->Keep_Common_Genes ( res.gene );

			XEPL::tlsLobe->Set_Outdex ( src.gene );
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Macro CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			const char* name = _call_gene->Trait_Default ( "name", nullptr );

			if ( !name )
				return;

			_nucleus->Register_Macro ( name, _call_gene->Content() );
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Macros CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			if ( _call_gene->content_wire )
				RNA::RnaScript ( _nucleus, _call_gene );

			ATOM::Bond* bond = _call_gene->inner_genes->head;

			while ( bond )
			{
				DNA::Gene* gene =  static_cast<DNA::Gene*> ( bond->atom );
				_nucleus->Register_Macro ( gene->cell_name, gene->Content() );
				bond = bond->next;
			}
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Method CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			const char* name = _call_gene->Trait_Default ( "name", nullptr );
			_nucleus->Register_Method ( name, &CELL::Nucleus::Execute_Method_Method, _call_gene );
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Methods CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			ATOM::Bond* bond = _call_gene->inner_genes->head;

			while ( bond )
			{
				DNA::Gene* gene =  static_cast<DNA::Gene*> ( bond->atom );
				_nucleus->Register_Method ( gene->cell_name, &XEPL::CELL::Nucleus::Execute_Method_Method, gene );
				bond = bond->next;
			}
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Modify CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			XEPL::SOMA::LocateIndex source ( _nucleus, _call_gene->space_string );

			XEPL::DNA::Scope_Index restore ( source.gene );

			XEPL::RNA::RnaScript ( _nucleus, source.gene, _call_gene->Content(), nullptr, nullptr, nullptr );

			if ( _call_gene->inner_genes )
				_nucleus->Process_Inner_Genes ( _call_gene );

			XEPL::tlsLobe->Set_Outdex ( source.gene );
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Neuron CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			if ( _call_gene->content_wire )
				XEPL::RNA::RnaScript ( _nucleus->Nucleus__Host_Neuron(), _call_gene );

			XEPL::CORTEX::Neuron* neuron = new XEPL::CORTEX::Neuron ( _nucleus->Nucleus__Host_Neuron(), _call_gene );

			neuron->Register_Method ( "Terminate", ( CELL::Function )&CORTEX::Neuron::Method_Terminate, nullptr );

			_call_gene->Make_Gene_Safe ( true );

			neuron->Process_Inner_Genes ( _call_gene );
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Neurons CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			ATOM::Bond* bond = _call_gene->inner_genes->head;

			while ( bond )
			{
				DNA::Gene* gene   = static_cast<DNA::Gene*> ( bond->atom );
				DNA::Gene* rename = new DNA::Gene ( nullptr, "Neuron", nullptr );

				rename->Gene_Absorb ( gene );
				rename->Trait_Set ( "name", gene->cell_name );

				ATOM::Scope_Release detach ( rename );

				RNA::RnaScript ( _nucleus->Nucleus__Host_Neuron(), _call_gene );

				( new CORTEX::Neuron ( _nucleus->Nucleus__Host_Neuron(), _call_gene ) )->Process_Inner_Genes ( _call_gene );

				bond = bond->next;
			}
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_New CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			XEPL::SOMA::LocateIndex source ( _nucleus, _call_gene->space_string );

			XEPL::TEXT::String tag;

			if ( !_call_gene->Trait_Get ( "tag", &tag ) )
			{
				if ( _call_gene->content_wire )
				{
					XEPL::DNA::Scope_Index restore ( _call_gene );

					XEPL::TEXT::String script_result;
					XEPL::RNA::RnaScript ( nullptr, _call_gene, nullptr, &tag, nullptr, nullptr );
				}
			}

			if ( tag.empty() )
				return;

			const char* into = _call_gene->Trait_Default ( "into", nullptr );

			XEPL::DNA::Gene* target = nullptr;
			source.gene->Make_One_Gene ( into, &target );

			XEPL::DNA::Gene* gene = new XEPL::DNA::Gene ( target, &tag );

			XEPL::ATOM::Scope_Release detach ( gene );

			XEPL::DNA::Scope_Index restore ( gene );

			XEPL::RNA::RnaScript ( _nucleus, gene, _call_gene->Content(), nullptr, nullptr, nullptr );

			_nucleus->Process_Inner_Genes ( _call_gene );

			XEPL::tlsLobe->Set_Outdex ( gene );
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Parse CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			XEPL::SOMA::LocateIndex source ( _nucleus, _call_gene->space_string );

			XEPL::TEXT::String* content = source.gene->Content();

			if ( !content || content->empty() )
			{
				XEPL::SOMA::xeplError ( "Parse", nullptr, "No Content" );
				return;
			}

			if ( content->front() == '<' )
			{
				XEPL::XML::XmlParser parsed ( content );

				XEPL::DNA::Scope_Index restoreI ( parsed.root_gene );

				XEPL::RNA::RnaScript ( _nucleus, _call_gene );

				_nucleus->Process_Inner_Genes ( _call_gene );
				XEPL::tlsLobe->Set_Outdex ( parsed.root_gene );
			}
			else
			{
				XEPL::SOMA::xeplError ( "Parse", nullptr, "Bad doc format" );
				return;
			}
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Print CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			auto lobe = tlsLobe;

			XEPL::SOMA::LocateIndex source ( _nucleus, _call_gene->space_string );

			XEPL::ATOM::Duct output ( std::cout, 2 );
			output << lobe->cell_name->c_str();

			output <<':'<< '\t';

			XEPL::DNA::Scope_Index restore ( source.gene );

			_call_gene->Make_Content_Wire();
			XEPL::TEXT::String* script_command = _call_gene->content_wire->wire_string;

			if ( script_command->empty() )
				script_command->assign ( "$>" );

			XEPL::TEXT::String script_result;
			XEPL::RNA::Script_Assign ( _nucleus, _call_gene, script_command, &script_result );

			output << script_result << XEPL::EOL;
			output << std::flush;

			if ( _call_gene->inner_genes )
				_nucleus->Process_Inner_Genes ( _call_gene );
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Properties CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			ATOM::Bond* bond = _call_gene->inner_genes->head;

			while ( bond )
			{
				DNA::Gene* gene =  static_cast<DNA::Gene*> ( bond->atom );
				gene->Make_Gene_UnSafe ( true );
				_nucleus->Property_Set ( gene->cell_name, gene->Content() );
				bond = bond->next;
			}
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Property CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			_nucleus->Property_Set ( _call_gene->Trait_Default ( "name", nullptr ), _call_gene->Content() );
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Repeat CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			XEPL::SOMA::LocateIndex locate ( _nucleus, _call_gene->space_string );
			XEPL::DNA::Gene*  src = locate.gene;

			XEPL::DNA::Scope_Index restore ( src );

			long repeat = 0;
			{
				XEPL::TEXT::String count;
				XEPL::RNA::Script_Assign ( _nucleus, src, _call_gene->Content(), &count );

				repeat = std::strtol ( count.c_str(), nullptr, 0 );
			}

			if ( repeat>0 && _call_gene->inner_genes )
			{
				while ( repeat-- )
					_nucleus->Process_Inner_Genes ( _call_gene );
			}
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Report CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			XEPL::TEXT::String script_result;
			XEPL::RNA::RnaScript ( _nucleus, _call_gene, nullptr, &script_result, nullptr, nullptr );

			XEPL::SOMA::LocateIndex source ( _nucleus, _call_gene->space_string );

			XEPL::DNA::Scope_Index restore ( source.gene );

			if ( _call_gene->traits )
				_call_gene->Traits_Evaluate ( _nucleus );

			XEPL::DNA::Gene* report = _nucleus->report_gene;

			if ( report )
			{
				XEPL::DNA::Gene* new_gene;
				report->Make_New_Gene ( "Report", &new_gene );
				new_gene->Content_Append ( &script_result );
			}

			_nucleus->Process_Inner_Genes ( _call_gene );
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Run CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			XEPL::SOMA::LocateIndex source ( _nucleus, _call_gene->space_string );

			XEPL::DNA::Scope_Index restore ( source.gene );

			if ( _call_gene->content_wire )
				XEPL::RNA::RnaScript ( _nucleus, source.gene, _call_gene->Content(), nullptr, nullptr, nullptr );

			_nucleus->Process_Inner_Genes ( _call_gene );
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Sleep CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			if ( !_nucleus || !_call_gene )
				return;

			SOMA::LocateIndex source ( _nucleus, _call_gene->space_string );

			XEPL::DNA::Scope_Index     restore ( source.gene );
			XEPL::DNA::Scope_Duplicate dup     ( _call_gene );

			XEPL::RNA::RnaScript ( _nucleus->Nucleus__Host_Neuron(), dup.gene );

			dup.gene->Trait_Set ( "simple", "true" );

			XEPL::DNA::Gene* term;
			dup.gene->Make_New_Gene ( "Terminate", &term );

			KITS::TIMER::Timer* timer = new KITS::TIMER::Timer ( _nucleus->Nucleus__Host_Neuron(), dup.gene );
			timer->Start_Timer ( 0 );

		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_SocketMan CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			if ( !_nucleus || !_call_gene )
				return;

			XEPL::DNA::Scope_Duplicate duplicate ( _call_gene );

			duplicate.gene->Trait_Tap ( "name", "SocketMan" );

			if ( _call_gene->content_wire )
				XEPL::RNA::RnaScript ( _nucleus->Nucleus__Host_Neuron(), _call_gene );

			( new KITS::SOCKET::SocketMan ( _nucleus->Nucleus__Host_Neuron(), duplicate.gene ) )->Start_Lobe ( true );
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Synapse CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			_call_gene->Make_Gene_Safe ( true );

			XEPL::SOMA::LocateIndex source ( _nucleus, _call_gene->space_string );

			XEPL::DNA::Scope_Index restore ( source.gene );

			XEPL::TEXT::String axonName;
			_call_gene->Trait_Get ( "axon", &axonName );

			XEPL::DNA::Scope_Duplicate duplicate ( _call_gene );
			duplicate.gene->Traits_Release();

			XEPL::CORTEX::AxonLocator statement ( _nucleus->Nucleus__Host_Neuron(), &axonName, '.', '-' );

			if ( statement.axon )
			{
				_nucleus->Nucleus__Host_Neuron()
				->Subscribe ( statement.axon, ( XEPL::SIGNAL::Receiver )&XEPL::CORTEX::Neuron::Axon_Synapse, duplicate.gene );
				return;
			}

			XEPL::SOMA::xeplCantFind ( "Axon", _nucleus, axonName.c_str() );
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Synapses CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			_call_gene->Make_Gene_Safe ( true );

			ATOM::Bond* bond = _call_gene->inner_genes->head;

			while ( bond )
			{
				DNA::Gene* gene =  static_cast<DNA::Gene*> ( bond->atom );
				if ( gene->inner_genes )
				{
					CORTEX::AxonLocator target ( _nucleus->Nucleus__Host_Neuron(), gene->cell_name, '.', '-' );

					if ( target.axon )
					{
						_nucleus->Nucleus__Host_Neuron()->Subscribe ( target.axon, ( SIGNAL::Receiver )&CORTEX::Neuron::Axon_Synapse, gene );
						bond = bond->next;
					}
					else
						SOMA::xeplCantFind ( "Axon", _nucleus, gene->cell_name->c_str() );
					
					bond = bond->next;
				}
			}
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Tcpclient CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			if ( !_nucleus || !_call_gene )
				return;

			XEPL::RNA::RnaScript ( _nucleus->Nucleus__Host_Neuron(), _call_gene );

			KITS::SOCKET::TcpClient* client = new KITS::SOCKET::TcpClient ( _nucleus->Nucleus__Host_Neuron(), _call_gene );

			client->Process_Inner_Genes ( _call_gene );

			client->Connect();
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Tcpserver CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			XEPL::DNA::Scope_Duplicate duplicate ( _call_gene );

			if ( _call_gene->content_wire )
				XEPL::RNA::RnaScript ( _nucleus->Nucleus__Host_Neuron(), duplicate.gene );

			if ( ! duplicate.gene->Trait_Default ( "name", nullptr ) )
				duplicate.gene->Trait_Set ( "name", "TcpServer" );

			new KITS::SOCKET::TcpServer ( _nucleus->Nucleus__Host_Neuron(),  duplicate.gene );
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Timer CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			if ( !_nucleus || !_call_gene )
				return;

			XEPL::RNA::RnaScript ( _nucleus->Nucleus__Host_Neuron(), _call_gene );

			KITS::TIMER::Timer* timer = new KITS::TIMER::Timer ( _nucleus->Nucleus__Host_Neuron(), _call_gene );

			if ( !_call_gene->Trait_Get_bool ( "simple", true ) )
				timer->Process_Inner_Genes ( _call_gene );

			if ( _call_gene->Trait_Get_bool ( "start", true ) )
				timer->Start_Timer ( 0 );
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Trigger CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			if ( _call_gene->content_wire )
				XEPL::RNA::RnaScript ( _nucleus, _call_gene );

			const char* name = _call_gene->Trait_Default ( "axon", nullptr );
			if ( !name )
			{
				XEPL::SOMA::xeplCantFind ( "Include", _nucleus, "Missing #axon name" );
				return;
			}

			XEPL::SIGNAL::Axon* axon = nullptr;

			if ( _nucleus->Nucleus__Host_Neuron()->Axon_Hunt ( name, &axon ) )
			{
				if ( _call_gene->space_string )
				{
					XEPL::SOMA::GeneLocator locate ( _nucleus, _call_gene->space_string );

					XEPL::DNA::Scope_Index restore ( locate.gene );
					axon->Trigger ( locate.gene );
				}
				else
					axon->Trigger ( tlsLobe->index );
			}
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_When CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			XEPL::TEXT::String script_result;

			if ( _call_gene->content_wire )
				XEPL::RNA::Script_Assign ( _nucleus, _call_gene, nullptr, &script_result );

			XEPL::TEXT::String choice ( _call_gene->Trait_Default ( "tag", nullptr ) );

			if ( choice.empty() )
			{
				if ( script_result.empty() )
					return;

				choice.assign ( script_result );
			}

			if ( _nucleus->Process_Inner_Gene ( &choice, _call_gene ) )
			{
				XEPL::TEXT::String tag ( _call_gene->Trait_Default ( "yes", "Found" ) );
				_nucleus->Process_Inner_Gene ( &tag, _call_gene );
				return;
			}

			XEPL::TEXT::String tag ( _call_gene->Trait_Default ( "no", "NotFound" ) );

			if ( !_nucleus->Process_Inner_Gene ( &tag, _call_gene ) )
				XEPL::SOMA::xeplError ( "When", _nucleus, "tag not found" );
		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_While CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			XEPL::SOMA::GeneLocator source ( _nucleus, _call_gene->space_string );

			XEPL::DNA::Gene* src = source.gene;

			if ( !src )
				src = tlsLobe->index;

			if ( !src || !src->content_wire )
				return;

			XEPL::TEXT::String line;
			auto nest = tlsLobe->short_term_stack;
			while ( src->content_wire->Extract_Line ( &line ) )
			{
				nest->Set ( "line", &line );
				XEPL::TEXT::String script_result;

				if ( _call_gene->content_wire )
					XEPL::RNA::RnaScript ( _nucleus, _call_gene, &line, &script_result, nullptr, nullptr );

				_nucleus->Process_Inner_Genes ( _call_gene );

				line.clear();
			}

		} );
	}
}


namespace XEPL::KEYWORDS
{
	void Keyword_Xml CORTEX_REGISTRATION
	{
		_cortex->Register_Keyword ( _as, [] CORTEX_KEYWORD
		{
			if ( !_nucleus )
				return;

			XEPL::DNA::Scope_Duplicate duplicate ( _call_gene );

			XEPL::RNA::RnaScript ( _nucleus->Nucleus__Host_Neuron(), duplicate.gene );

			if ( !duplicate.gene->Trait_Default ( "name", nullptr ) )
				duplicate.gene->Trait_Set (  "name", "Text" );

			XEPL::TEXT::String* output = _nucleus->Nucleus__Host_Neuron()->output;

			XEPL::RENDER::RendronText renderer ( _nucleus->Nucleus__Host_Neuron(), duplicate.gene, output );

			renderer.Process_Inner_Genes ( _call_gene );
		} );
	}
}


namespace XEPL::OPERATORS
{
	void Operator_After CORTEX_REGISTRATION
	{
		_cortex->Register_Operator ( _as, [] CORTEX_OPERATOR
		{
			if ( _rhs )
			{
				size_t ndx = _lhs->find ( _rhs->c_str() );

				if ( ndx != _rhs->npos )
				{
					_lhs->erase ( 0, ndx+_rhs->size() );
					_truth = true;
					return;
				}
			}
			_truth = false;
		} );
	}
}


namespace XEPL::OPERATORS
{
	void Operator_AfterAny CORTEX_REGISTRATION
	{
		_cortex->Register_Operator ( _as, [] CORTEX_OPERATOR
		{
			if ( _rhs )
			{
				XEPL::TEXT::String::size_type idx = _lhs->find_first_of ( *_rhs );

				if ( idx != XEPL::TEXT::String::npos )
				{
					_lhs->erase ( 0, idx+1 );
					_truth = true;
					return;
				}
			}

			_lhs->erase();
			_truth = false;
		} );
	}
}


namespace XEPL::OPERATORS
{
	void Operator_AfterLast CORTEX_REGISTRATION
	{
		_cortex->Register_Operator ( _as, [] CORTEX_OPERATOR
		{
			if ( _rhs )
			{
				XEPL::TEXT::String::size_type idx = _lhs->rfind ( *_rhs );

				if ( idx != XEPL::TEXT::String::npos )
				{
					_lhs->erase ( 0, idx+_rhs->size() );
					_truth = true;
					return;
				}
			}

			_truth = false;
		} );
	}
}


namespace XEPL::OPERATORS
{
	void Operator_Append CORTEX_REGISTRATION
	{
		_cortex->Register_Operator ( _as, [] CORTEX_OPERATOR
		{
			if ( _rhs )
				_lhs->append( *_rhs );

			_truth = !_lhs->empty();
		} );
	}
}


namespace XEPL::OPERATORS
{
	void Operator_Before CORTEX_REGISTRATION
	{
		_cortex->Register_Operator ( _as, [] CORTEX_OPERATOR
		{
			if ( _rhs )
			{
				XEPL::TEXT::String::size_type idx = _lhs->find ( *_rhs );

				if ( idx != XEPL::TEXT::String::npos )
				{
					_lhs->erase ( idx );
					_truth = true;
					return;
				}
			}

			_truth = false;
		} );
	}
}


namespace XEPL::OPERATORS
{
	void Operator_BeforeAny CORTEX_REGISTRATION
	{
		_cortex->Register_Operator ( _as, [] CORTEX_OPERATOR
		{
			if ( _rhs )
			{
				XEPL::TEXT::String::size_type idx = _lhs->find_first_of ( *_rhs );

				if ( idx != XEPL::TEXT::String::npos )
				{
					_lhs->erase ( idx );
					_truth = true;
					return;
				}
			}

			_truth = false;
		} );
	}
}


namespace XEPL::OPERATORS
{
	void Operator_Cr CORTEX_REGISTRATION
	{
		_cortex->Register_Operator ( _as, [] CORTEX_OPERATOR
		{
			_lhs->push_back ( '\r' );

			if ( _rhs )
				_lhs->append ( *_rhs );

		} );
	}
}


namespace XEPL::OPERATORS
{
	void Operator_DeAmp CORTEX_REGISTRATION
	{
		_cortex->Register_Operator ( _as, [] ( [[maybe_unused]] XEPL::TEXT::String * _lhs,                                \
		                                       [[maybe_unused]] XEPL::TEXT::String * _rhs, [[maybe_unused]] bool &_truth )
		{
			const char amper='&';
			const char* start = _lhs->c_str();
			const char* pos1 = std::strchr ( start, amper );

			if ( !pos1 )
			{
				if ( _rhs )
					_lhs->append ( *_rhs );

				return;
			}

			const char* pos2 = nullptr;
			XEPL::TEXT::String scratch;
			scratch.reserve ( _lhs->size() );

			while ( pos1 )
			{
				long length = ( pos1-start );

				if ( length >=1 )
					scratch.append ( start, length );

				pos1++;
				pos2 = std::strpbrk ( pos1, "&;" );
				const char* conv = nullptr;

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

			_lhs->assign ( scratch );

			_truth = !_lhs->empty();

			if ( _rhs )
				_lhs->append ( *_rhs );
		} );
	}
}


namespace XEPL::OPERATORS
{
	static // chatGPT
	int hexCharToInt ( char ch )
	{
		if ( ch >= '0' && ch <= '9' )
			return ch - '0';

		if ( ch >= 'A' && ch <= 'F' )
			return ch - 'A' + 10;

		if ( ch >= 'a' && ch <= 'f' )
			return ch - 'a' + 10;

		return 0;
	}

	static // chatGPT
	std::string charToHex ( unsigned char ch )
	{
		std::stringstream ss;
		ss << std::hex << std::uppercase << std::setw ( 2 ) << std::setfill ( '0' ) << static_cast<int> ( ch );
		return ss.str();
	}

	static // chatGPT
	bool decodeUrl ( TEXT::String* _lhs, TEXT::String* _into )
	{
		auto input = *_lhs;
		auto output = *_into;

		std::stringstream decoded;
		bool conversion_made = false;

		for ( size_t i = 0; i < input.size(); ++i )
		{
			if ( input[i] == '%' && i + 2 < input.size() && std::isxdigit ( input[i + 1] ) && std::isxdigit ( input[i + 2] ) )
			{
				char ch = static_cast<char> ( ( hexCharToInt ( input[i + 1] ) << 4 ) + hexCharToInt ( input[i + 2] ) );
				decoded << ch;
				i += 2;
				conversion_made = true;
			}
			else if ( input[i] == '+' )
			{
				decoded << ' ';
				conversion_made = true;
			}
			else
				decoded << input[i];
		}

		_into->assign ( decoded.str() );
		return conversion_made;
	}

	static // chatGPT
	bool encodeUrl ( TEXT::String* _lhs, TEXT::String* _into )
	{
		auto input = *_lhs;
		auto output = *_into;
		bool conversion_made = false;
		std::stringstream encoded;

		for ( const auto& ch : input )
		{
			if ( std::isalnum ( ch ) || ch == '-' || ch == '_' || ch == '.' || ch == '~' )
				encoded << ch;

			else if ( ch == ' ' )
			{
				encoded << '+';
				conversion_made = true;
			}
			else
			{
				encoded << '%' << charToHex ( static_cast<unsigned char> ( ch ) );
				conversion_made = true;
			}
		}

		_into->assign ( encoded.str() );
		return conversion_made;
	}

	void Operator_Percentify CORTEX_REGISTRATION
	{
		_cortex->Register_Operator ( _as, [] CORTEX_OPERATOR
		{
			TEXT::String output;

			if ( encodeUrl ( _lhs, &output ) )
				_lhs->assign ( output );

			_truth = !_lhs->empty();

			if ( _rhs )
				_lhs->append ( *_rhs );
		} );
	}

	void Operator_Depercent CORTEX_REGISTRATION
	{
		_cortex->Register_Operator ( _as, [] CORTEX_OPERATOR
		{
			TEXT::String output;

			if ( decodeUrl ( _lhs, &output ) )
				_lhs->assign ( output );

			_truth = !_lhs->empty();

			if ( _rhs )
				_lhs->append ( *_rhs );
		} );
	}

}


namespace XEPL::OPERATORS
{
	void Operator_Empty CORTEX_REGISTRATION
	{
		_cortex->Register_Operator ( _as, [] CORTEX_OPERATOR
		{
			_truth = _lhs->empty();
		} );
	}
}


namespace XEPL::OPERATORS
{
	void Operator_Exclaim CORTEX_REGISTRATION
	{
		_cortex->Register_Operator ( _as, [] CORTEX_OPERATOR
		{
			_lhs->push_back ( '!' );

			if ( _rhs )
				_lhs->append ( *_rhs );
		} );
	}
}


namespace XEPL::OPERATORS
{
	void Operator_Has CORTEX_REGISTRATION
	{
		_cortex->Register_Operator ( _as, [] CORTEX_OPERATOR
		{
			if ( _rhs )
				_truth = _lhs->find ( *_rhs ) != _lhs->npos;
			else
				_truth = false;
		} );
	}
}


namespace XEPL::OPERATORS
{
	void Operator_Is CORTEX_REGISTRATION
	{
		_cortex->Register_Operator ( _as, [] CORTEX_OPERATOR
		{
			if ( _rhs )
				_truth = _lhs->compare ( *_rhs ) == 0;
			else
				_truth = false;
		} );
	}
}


namespace XEPL::OPERATORS
{
	void Operator_Lf CORTEX_REGISTRATION
	{
		_cortex->Register_Operator ( _as, [] CORTEX_OPERATOR
		{
			_lhs->push_back ( '\n' );

			if ( _rhs )
				_lhs->append ( *_rhs );

		} );
	}
}


namespace XEPL::OPERATORS
{
	void Operator_Lowercase CORTEX_REGISTRATION
	{
		_cortex->Register_Operator ( _as, [] CORTEX_OPERATOR
		{
			std::transform ( _lhs->begin(), _lhs->end(), _lhs->begin(), ::tolower );

			if ( _rhs )
				_lhs->append ( *_rhs );

			_truth = !_lhs->empty();
		} );
	}
}


namespace XEPL::OPERATORS
{
	void Operator_Lt CORTEX_REGISTRATION
	{
		_cortex->Register_Operator ( _as, [] CORTEX_OPERATOR
		{
			if ( _rhs )
				_truth = std::strtol ( _lhs->c_str(), nullptr, 0 ) < std::strtol ( _rhs->c_str(), nullptr, 0 );
		} );
	}
	void Operator_Gt CORTEX_REGISTRATION
	{
		_cortex->Register_Operator ( _as, [] CORTEX_OPERATOR
		{
			if ( _rhs )
				_truth = std::strtol ( _lhs->c_str(), nullptr, 0 ) > std::strtol ( _rhs->c_str(), nullptr, 0 );
		} );
	}
	void Operator_Equal CORTEX_REGISTRATION
	{
		_cortex->Register_Operator ( _as, [] CORTEX_OPERATOR
		{
			if ( _rhs )
				_truth = std::strtol ( _lhs->c_str(), nullptr, 0 ) == std::strtol ( _rhs->c_str(), nullptr, 0 );
		} );
	}
	void Operator_Add CORTEX_REGISTRATION
	{
		_cortex->Register_Operator ( _as, [] CORTEX_OPERATOR
		{
			if ( _rhs )
				_lhs->assign ( std::to_string ( std::strtol ( _lhs->c_str(), nullptr, 0 )
				                                + std::strtol ( _rhs->c_str(), nullptr, 0 ) ) );
		} );
	}
	void Operator_Subtract CORTEX_REGISTRATION
	{
		_cortex->Register_Operator ( _as, [] CORTEX_OPERATOR
		{
			if ( _rhs )
				_lhs->assign ( std::to_string ( std::strtol ( _lhs->c_str(), nullptr, 0 )
				                                - std::strtol ( _rhs->c_str(), nullptr, 0 ) ) );
		} );
	}
	void Operator_Multiply CORTEX_REGISTRATION
	{
		_cortex->Register_Operator ( _as, [] CORTEX_OPERATOR
		{
			if ( _rhs )
				_lhs->assign ( std::to_string ( std::strtol ( _lhs->c_str(), nullptr, 0 )
				                                * std::strtol ( _rhs->c_str(), nullptr, 0 ) ) );
		} );
	}
}


namespace XEPL::OPERATORS
{
	void Operator_NextAny CORTEX_REGISTRATION
	{
		_cortex->Register_Operator ( _as, [] CORTEX_OPERATOR
		{
			if ( _rhs )
			{
				XEPL::TEXT::String::size_type idx = _lhs->find_first_of ( *_rhs );

				if ( idx != XEPL::TEXT::String::npos )
				{
					_lhs->erase ( idx+1 );
					_lhs->erase ( 0, idx );
					_truth = true;
					return;
				}
			}

			_lhs->erase();
			_truth = false;
		} );
	}
}


namespace XEPL::OPERATORS
{
	void Operator_Prepend CORTEX_REGISTRATION
	{
		_cortex->Register_Operator ( _as, [] CORTEX_OPERATOR
		{
			if ( _rhs )
				_lhs->insert ( 0, _rhs->c_str() );

			_truth = !_lhs->empty();
		} );
	}
}


namespace XEPL::OPERATORS
{
	void Operator_Space CORTEX_REGISTRATION
	{
		_cortex->Register_Operator ( _as, [] CORTEX_OPERATOR
		{
			_lhs->push_back ( ' ' );

			if ( _rhs )
				_lhs->append ( *_rhs );

		} );
	}
}


namespace XEPL::OPERATORS
{
	void Operator_Tab CORTEX_REGISTRATION
	{
		_cortex->Register_Operator ( _as, [] CORTEX_OPERATOR
		{
			_lhs->push_back ( '\t' );

			if ( _rhs )
				_lhs->append ( *_rhs );

		} );
	}
}


namespace KITS::SPLICER
{
	void Splicer_Get CORTEX_REGISTRATION
	{
		_cortex->Register_Splicer ( _as, [] DNA_SPLICER
		{
			if ( !_param )
				return;

			if ( !XEPL::tlsLobe->ephemerals )
				return;

			auto it =  XEPL::tlsLobe->ephemerals->find ( *_param );
			if ( it == XEPL::tlsLobe->ephemerals->end() )
				return;

			__gene = it->second;
		} );
	};
}


namespace KITS::SPLICER
{
	void Splicer_Set CORTEX_REGISTRATION
	{
		_cortex->Register_Splicer ( _as, [] DNA_SPLICER
		{
			if ( !__gene )
				return;

			if ( !_param )
				return;

			if ( !XEPL::tlsLobe->ephemerals )
				XEPL::tlsLobe->ephemerals = new XEPL::DNA::Ephemerals();

			XEPL::tlsLobe->ephemerals->Set ( _param, __gene );
		} );
	};
}
////////////////////////////////////////////////////////////////////////////////
// Licensed under the DAZL License (DAZL-1.0) (the License)
//+----------------------------Attribution Notice:-------------------------+
//|  o   o     o--o     o--o      o     ONE          Original Work:        |
//|   \ /      |        |   |     |               Keith Edwin Robbins      |
//|    O       O-o      O--o      |     1.0.0     Project Repository:      |
//|   / \      |        |         |             https://github.com/xepl    |
//|  o   o     o--o     o         O---o           Notification Chain:      |
//|                                                   Pull Request         |
//+- visit: XEPL.Shop --- get: XEPL.Services --- join: XEPL.Tech ----------+
////////////////////////////////////////////////////////////////////////////////
// g++ -std=c++17 -c -O2 -DNDEBUG  -fno-rtti -fno-exceptions -o libxepl.a xepl.cc
// g++ -std=c++17 -c -O2 -DNDEBUG -o libxepl.a xepl.cc

// g++ -std=c++17 -c -Wall -DCOUNT_NEWS=1 -DNO_RECYCLE_STRING=1 -o libxepl.a xepl.cc
