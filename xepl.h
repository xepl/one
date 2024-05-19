// SPDX: AGPL-3.0-only
/* XEPL Operating Environment - Copyright (c) 2024 Keith Edwin Robbins
	Project Name: XEPL Operating Environment
	File Name:    xepl.h
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
/* Inheritance Tree
Recycler / NoCopy
  Action - DropAction
  Action - SignalAction
  ActionList
  Atom - Cell - Gene
  Atom - Nucleus
  Atom - Cell/Nucleus - Neuron
  Atom - Cell/Nucleus - Neuron - Lobe   - Kits:SocketMan
  Atom - Cell/Nucleus - Neuron - Lobe   - Kits:Timer
  Atom - Cell/Nucleus - Neuron - Kits::Socket - Kits::SocketTcp - Kits::TcpServer
  Atom - Cell/Nucleus - Neuron - Kits::Timer
  Atom - Cell/Nucleus - Neuron - Kits::Html
  Atom - Cell/Nucleus - Rendon - Kits::RendonHtml
  Atom - Cell/Nucleus - Rendon - Kits::RendonText
  Atom -      Nucleus - Senson - Kits:Http - Kits::HttpServer
  Atom - Receptor - Relay - Synapse
  Atom - Spike - Rendezvous
  Atom - Axon
  Atom - Kits::osFd - Kits::osSocket - osTcpSocket - osTcpServer
  Atom - Kits::ConnectedClient
  Chain - GeneChain - Genes
  Chain - NeuronChain : ReceptorChain
  Parser - Script
  Parser - XmlParser - PrintXml : XeplXml
  ParserBag - RnaBag : XmlBag
  String - ErrorReport
*/

#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <string>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>
#include <cstring>

#include <algorithm>

// The XEPL Cortex ... is in the XEPL namespace

namespace XEPL
{
	/// memory recycling
	union BlockHeader;
	class PoolOfBlocks;
	class HeapOfPools;
	class Recycler;
	class Backpack;

	/// text handling
	class String;
	class Wire;
	typedef const String Cord;
	typedef const char   Text;

	/// thread managment
	class Mutex;
	class MutexScope;
	class MutexResource;
	class Semaphore;
	class Thread;

	/// atoms = the reference counted everything
	class Atom;
	class Bond;
	class Chain;

	/// cell - atoms with a name
	class Cell;
	class Nucleus;
	class Method;
	class MethodMap;

	/// gene - cellular database
	class Gene;
	class BondMap;
	class ChainMap;
	class Genes;
	class GeneChain;
	class GeneScope;
	class Ephemerals;
	class DuplicateTraits;
	class ShortTerms;
	class StableGenes;
	class StableTraits;
	class ShortTermMap;
	class Trait;
	class TraitMap;
	class Traits;

	/// neuron - a compputer in a computer
	class Neuron;
	class AxonMap;
	class NeuronChain;
	class NeuronMap;
	class RelayMap;

	/// senson - catch and release
	class Senson;

	/// rendon - present in form
	class Rendon;

	/// lobe - common neuron pool
	class Lobe;
	class Action;
	class ActionList;
	class DropAction;
	class SignalAction;
	class ScopeIndex;
	class Indicies;

	/// signals - messages between neurons
	class Axon;
	class AxonChain;
	class Synapse;
	class Receptor;
	class ReceptorChain;
	class Relay;
	class Spike;
	class Rendezvous;

	/// cortex - the custom brain container
	class Cortex;
	class ErrorReport;
	class Counters;

	/// parser - root text parser
	class Parser;
	class ParserBag;
	class ParserSelect;

	/// rna scripts
	class Script;
	class RnaBag;

	/// xml support
	class XmlBuilder;
	class XmlParser;
	class XmlBag;
	class XmlNode;
	class XeplXml;
	class PrintXml;

	/// global typedefs
	typedef void (    Nucleus::*Function )  ( Gene*,    Gene* );
	typedef void (     Neuron::*Receiver )  ( Atom*,    Atom* );
	typedef void (             *Keyword  )  ( Neuron*,  Gene*, String* );
	typedef void (             *Render   )  ( Nucleus*, Gene*, Rendon* );
	typedef void (             *Operator )  ( Script*,  Cord* );
	typedef void (             *Command  )  ( String*  );
	typedef Gene*(             *Mutual   )  ( Nucleus* );


	/// global variables
	extern thread_local Lobe*    tlsLobe;
	extern class Cortex*         cortex;
	extern std::mutex            output_lock;

	/// option flags
	extern bool Show_Trace;
	extern bool Show_Memory_Counts;
	extern bool Show_Counters;
	
		/// global routines
	void    Set_Thread_Name  ( Thread*, Text*   );
	String* Escape_Quotes    ( Cord*,   String* );
	String* Long_Commafy     ( long,    String* );
	String* Long_In_Bytes    ( long,    String* );
	std::string* Long_In_Time ( long long, std::string* );
	
	long    Into_Long        ( Cord* );
	bool    Split_ch_lhs_rhs ( Cord*,   char,     String*, String* );
	bool    Split_ch_lhs     ( String*, char,     String* );
	bool    Split_ch_rhs     ( String*, char,     String* );
	int     hexCharToInt     ( char ch );
	void    xeplCantFind     ( Text*,   Nucleus*, Cord*   );
	void    xeplCantFind     ( Text*,   Nucleus*, Text*   );
	void    TRACE            ( Text*,   Neuron*,  Cord*,   Cord* = nullptr );
	void    Evaluate_Inner_Scripts ( Nucleus*, Gene*, Cord*, String* );

	/// per thread counters, aligned with descriptions
	class Counters
	{
		static const char* Counter_Descriptions;
	public:
		typedef long Counter;
		Counter   count_genes;
		Counter   count_traits;
		Counter   count_lobes;
		Counter   count_neurons;
		Counter   count_dispatched;
		Counter   count_rests;
		Counter   count_actions;
		Counter   count_wakes;
		void Add          ( Counters* to_counter  );
		void Report       ( String*   into_string );
		void Final_Report ( void );
	};



	/// memory recyler variables
	namespace Memory
	{
		static constexpr long poolWidth    = 16;
		static constexpr long maxPoolIndex =  5;
		static constexpr long overHead     = sizeof( void* );
	}

	/// memory counters
	extern std::atomic_size_t   pool_mallocs[Memory::maxPoolIndex+1];
	extern std::atomic_size_t    pool_cached[Memory::maxPoolIndex+1];
	extern std::atomic_size_t     pool_freed[Memory::maxPoolIndex+1];
	extern std::atomic_size_t      pool_held[Memory::maxPoolIndex+1];
	extern std::atomic_size_t   total_mallocs;
	extern std::atomic_size_t    total_cached;
	extern std::atomic_size_t     total_freed;
	extern std::atomic_size_t      total_held;
	extern std::atomic_size_t   total_biggies_out;
	extern std::atomic_size_t   total_biggies_in;
	extern std::atomic_size_t   largest_biggie;
	extern std::atomic_size_t   num_total_news;
	extern std::atomic_size_t   num_total_dels;

	/// limted list of memory pools
	extern thread_local class HeapOfPools
	{
		PoolOfBlocks*    pool_of_blocks[Memory::maxPoolIndex+1];
		size_t           count_biggies_out;
		size_t           count_biggies_in;
		size_t           largest_biggie_out;
	public:
		~HeapOfPools( void );
		HeapOfPools ( void );
		void* Get_Block     ( size_t );
		void  Recycle_Block ( void*  );
		void  Report        ( String* );
	} *tlsHeap;

	/// header in front of every memory block
	union BlockHeader
	{
		BlockHeader*    next_block;
		long            pool_index;
	};

	/// single list of same sized memory blocks
	class PoolOfBlocks
	{
		friend class    HeapOfPools;
		BlockHeader*    head_block;
		const long      pool_index;
		const size_t    block_size;
		size_t          blocks_malloced;
		size_t          blocks_cached;
		size_t          blocks_freed;
		size_t          blocks_holding;
		~PoolOfBlocks( void );
		explicit PoolOfBlocks ( long );
		void* Get_Or_Malloc   ( void );
		void  Catch_Or_Free   ( BlockHeader* );
		void  Report          ( String* );
	};

	/// base class recycler
	class Recycler
	{
	public:
		static void* operator new    ( size_t );
		static void  operator delete ( void*  );
		static void Report_Heap( String* into_string );
	};

	/// some things shouldn't be allocated
	template <class T> class TAllocatorT;
	class NoAllocator
	{
		static void* operator new    ( size_t ) = delete;
		static void  operator delete ( void*  ) = delete;
	};
	
	/// these shouldn't be copied or moved
	class NoCopy : public Recycler
	{
	public:
		NoCopy() {};
		NoCopy( const NoCopy&  )           = delete;
		NoCopy(       NoCopy&& )           = delete;
		NoCopy& operator=( const NoCopy& ) = delete;
		NoCopy& operator=(       NoCopy&&) = delete;
	};

	/// master backpack 1 per thread
	class Backpack : NoAllocator, NoCopy
	{
		HeapOfPools*  heap;
	public:
		~Backpack();
		Backpack();
	};

	/// C++ leak detector
	class MemoryCounts
	{
		std::ostream* std_ostream;
	public:
		~MemoryCounts ( void );
		explicit MemoryCounts ( std::ostream* );
	};

	/// non-blocking memory re-allocator
	class RecycleCounts
	{
		std::ostream* std_ostream;
	public:
		~RecycleCounts ( void );
		explicit RecycleCounts ( std::ostream* );
		void    Print_Pool_Counts( std::ostream*, Text*, const std::atomic_size_t&, std::atomic_size_t*, int );
	};

	/// template allocator support
	template <class T>
	class  TAllocatorT
	{
	public:
		typedef T           value_type;
		typedef T*          pointer;
		typedef const T*    const_pointer;
		typedef T&          reference;
		typedef const T&    const_reference;
		typedef size_t      size_type;
		typedef ptrdiff_t   difference_type;
		template <class U>
		explicit TAllocatorT ( const TAllocatorT<U>& ) {}
		TAllocatorT ( void ) {}
		pointer allocate ( size_type _num ) {
			return ( T* ) ( Recycler::operator new ( _num * sizeof ( T ) ) );
		}
		void deallocate ( pointer _ptr, size_type ) {
			Recycler::operator delete ( _ptr );
		}
	};

	/// convienence allocators
	using StdStringAllocator = TAllocatorT<std::string>;
	using CharAllocator      = TAllocatorT<char>;
	using IntAllocator       = TAllocatorT<int>;

//    _  _ ____ ___  ____
//    |\/| |__| |__] [__
//    |  | |  | |    ___]
//

	/// recycled ordered map of string keys
	template <class Tkey, class Tvalue>
	class MapT
	: public Recycler
	, public std::map< Tkey, Tvalue, std::less<Tkey>, 
		TAllocatorT<std::pair<const Tkey, Tvalue> > >
	{};

	/// recycled unordered map for pointer keys
	template <class Tkey, class Tvalue>
	class UMapT
	: public Recycler
	, public std::unordered_map<Tkey, Tvalue, std::hash<Tkey>, std::equal_to<Tkey>, 
		TAllocatorT<std::pair<const Tkey, Tvalue>>>
	{};

	/// ordered map, delete the value on extract
	template<class TKey, typename TPtr=void*>
	class MapDeleteT : public MapT<TKey, TPtr>
	{
	public:
		~MapDeleteT ( void ) {
			for (auto it = this->begin(); it != this->end();) {
				delete it->second;
				it = this->erase(it);
			}
		}
	};



	/// std Mutex wrapper
	class Mutex : public Recycler
	{
	public:
		std::recursive_mutex  mutex;
		Mutex();
	};

	/// Stack for hoilding locked mutex
	class MutexScope : NoAllocator
	{
		Mutex*  mutex;
	public:
		~MutexScope ( void );
		explicit MutexScope ( Mutex* locking_mutex );
	};

	/// std::thread wrapper
	class Thread
	{
		friend class Lobe;
	public:
		std::thread*  std_thread;
	private:
		Lobe*         lobe;
		Semaphore*    semaphore_rest;
		explicit Thread     ( Lobe*, Semaphore* );
		void Concieve_Child ( Semaphore* );
		void Bury_Child     ( void );
	};

	/// Bnary Semapuore
	class Semaphore : public std::condition_variable, public std::mutex
	{
	public:
		Semaphore ( void );
		void Give ( void );
	};



	/// recycled strings don't render in my debugger
#ifndef NDEBUG
	typedef	std::string CppString;
#else
	typedef std::basic_string<char, std::char_traits<char>, CharAllocator> CppString;
#endif




	/// my recycled string class
	class String : public Recycler, public CppString
	{
	public:
		~String ( void );
		String  ( void );
		String          ( Text*             from_cstring    );
		String          ( const CppString*  from_std_string );
		explicit String ( Cord*             from_xepl_string );
		explicit String ( Text*             from_chars, size_t number_of_bytes );
	};



	/// all atoms share common flags
	using AtomFlags = long;
	static constexpr AtomFlags lysing_flag  = 1 << 0;
	static constexpr AtomFlags dropped_flag = 1 << 1;
	static constexpr AtomFlags closed_flag  = 1 << 2;
	static constexpr AtomFlags dupe_flag    = 1 << 3;

	/// the atom is reference counted and self-destructs (lyces iteslf)
	class Atom : public NoCopy
	{
		std::atomic_long  retain_count;
		std::atomic_long  atom_flags;
	protected:
		virtual ~Atom ( void );
		Atom ( void ) : retain_count ( 1 ), atom_flags ( 0 ) {}
	public:
		void  Attach      ( void )             { ++retain_count; }
		void  Release     ( void );
		void  Set_Flags   ( AtomFlags _flags)  {         atom_flags |=  _flags;       }
		void  Clear_Flags ( AtomFlags _flags)  {         atom_flags &= ~_flags;       }
		bool  Test_Flags  ( AtomFlags _flags)  { return (atom_flags &   _flags) != 0; }
	};


	/// ordered map, release the atom on extract
	template<class Tkey, typename TAtom = Atom*>
	class MapReleaseT : public MapT<Tkey, TAtom>
	{
	public:
		~MapReleaseT() {
			for (auto it = this->begin(); it != this->end();) {
				it->second->Release();
				it = this->erase(it);
			}
		}
	};


	/// atoms bond together
	class Bond : public NoCopy
	{
	public:
		Bond*  next_bond;
		Bond*  prev_bond;
		Atom*  atom;
		explicit Bond ( Atom* _atom, Bond* _previous )
		: next_bond  ( nullptr   )
		, prev_bond  ( _previous )
		, atom ( _atom )           { if ( _atom ) _atom->Attach(); 
		                             if ( _previous ) _previous->next_bond = this;  }
		~Bond ( void )             { if (  atom )  atom->Release(); }
	};
	class BondMap : public UMapT<Atom*, Bond*> {};


	/// the head bond is a chain of atoms
	class Chain : public NoCopy
	{
	public:
		Mutex*    chain_lock;
		Bond*     head_bond;
		Bond*     tail_bond;
		bool      is_my_lock;
		~Chain();
		explicit Chain      ( bool   use_protection     );
		explicit Chain      ( Atom*  starting_with_atom );
		Bond*  Add_Atom     ( Atom*  this_atom  );
		bool   Pull_Atom    ( Atom** found_atom );
		bool   Remove_Atom  ( Atom*  this_atom  );
		void   Remove_Bond  ( Bond*  this_bond  );
	};
	class ChainMap : public MapDeleteT<Cord, Chain*> {};


	/// Wrap the Gene that is a xml Method
	class Method : public NoCopy
	{
		Function   cell_function;
		Gene*      method_gene;
	public:
		~Method ( void );
		explicit Method ( Function cell_function, Cord* method_name, Gene* param_gene );
		void Perform    ( Nucleus*using_cell,     Gene* xepl_gene );
	};


	/// The Cell is a named Atom
	class Cell : public Atom
	{
	protected:
		virtual ~Cell ( void ) override;
		explicit Cell ( Cord* new_cell_name );
		explicit Cell ( Text* new_cell_name );
	public:
		Cord* cell_name;
	};


	/// The Gene Cell is the Xepl XML database
	class Gene : public Cell
	{
	protected:
		virtual ~Gene ( void ) override;
	public:
		Wire*     content_wire;
		Traits*   traits;
		Genes*    inner_genes;
		String*   space_string;
		Mutex*    content_mutex;
		Gene*     owner_link;
		explicit Gene ( Text*  xml_text );
		explicit Gene ( Gene*  parent_gene, Cord* gene_name, Cord* space_name );
		explicit Gene ( Gene*  parent_gene, Text* gene_name, Cord* space_name);
		explicit Gene ( Gene*  parent_gene, Cord* gene_name);
		Cord*   Trait_Raw        ( Cord*  trait_name );
		Cord*   Trait_Raw        ( Text*  trait_name );
		Text*   Trait_Default    ( Text*  trait_name,  Text*   defaut_to  );
		Cord*   Trait_Get        ( Cord*  trait_name,  String* term_value );
		Cord*   Trait_Get        ( Text*  trait_name,  String* term_value );
		Cord*   Trait_Tap        ( Text*  trait_name,  Text*   term_value );
		void    Trait_Set        ( Cord*  trait_name,  Cord*   term_value );
		void    Trait_Set        ( Cord*  trait_name,  Text*   term_value );
		void    Trait_Set        ( Text*  trait_name,  Cord*   term_value );
		void    Trait_Set        ( Text*  trait_name,  Text*   term_value );
		String* Make_Content     ( void );
		bool    Has_Content      ( void );
		Gene*   First            ( void );
		void    Absorb_Traits    ( Gene*       from_gene   );
		bool    Duplicate_Traits ( Traits**    new_traits  );
		void    Evaluate_Traits  ( Nucleus*    host_cell   );
		void    Append_Content   ( Wire*       with_wire   );
		void    Append_Content   ( Cord*       with_cord   );
		void    Assign_Content   ( Cord*       to_cord     );
		bool    Copy_Content     ( String*     into_string );
		void    Append_Content   ( Text*       content_source, long number_of_bytes);
		Gene*   Make_One         ( Text*  gene_name );
		Gene*   Get_First        ( Text*  gene_name );
		bool    Make_One_Gene    ( Cord*  gene_name,  Gene** only_gene_named );
		bool    Make_One_Gene    ( Text*  gene_mname, Gene** only_gene_named );
		bool    Replace_Gene     ( Cord*  gene_name, Gene* replacemnt_gene   );
		bool    Get_First_Gene   ( Cord*  gene_name, Gene** first_gene_named );
		bool    Get_First_Gene   ( Text*  gene_name, Gene** first_gene_named );
		void    Add_Gene         ( Gene*  this_gene );
		void    Remove_Gene      ( Gene*  this_gene );
		void    Copy_Genes_Into  ( GeneChain** copy_of_chain );
		void    Deflate_Gene     ( void  );
		void    Absorb_Gene      ( Gene*       absorb_this );
		void    Duplicate_Gene   ( Gene**      duplicate_gene );
		void    Print_Into       ( String*     into_string, int traversal_depth = 0 );
	};


	class GeneChain : public Chain
	{
		Gene*  current_gene;
	public:
		~GeneChain ( void );
		explicit GeneChain ( const Chain* from_chain );
		bool     Next      ( Gene**       next_gene);
	};


	/// A Gene has Inner Genes
	class Genes : public GeneChain
	{
		BondMap*   bond_map;
		ChainMap*  chain_map;
	public:
		~Genes( void );
		Genes ( void );
		explicit Genes ( Gene* );
		void   Flush         ( void );
		void   Add_Gene      ( Gene* adding_gene );
		bool   Find_Gene     ( Cord* gene_name, Gene** found_gene );
		void   Remove_Bond   ( Bond* gene_bond );
		void   Print_Into    ( String* into_string, int depth_to_traverse );
		friend class Gene;
	};


	/// Hold onto the Gene in the stack and ~ replace
	class GeneScope : NoAllocator
	{
		Gene*&  gene_reference;
		Gene*   the_previous_gene;
	public:
		GeneScope  ( Gene*& _gene, Gene* _replacement_gene );
		~GeneScope ( void );
	};


	/// For walking the Gene
	class StableGenes : NoAllocator
	{
	public:
		GeneChain*  stable_gene_chain;
		~StableGenes ( void );
		explicit StableGenes ( Gene* copy_from );
		bool  Next_Gene ( Gene** into_gene );
	};


	/// For walking the Gene Traits
	class StableTraits : NoAllocator
	{
		Traits*  stable_traits;
		Trait*   current_trait;
	public:
		~StableTraits ( void );
		explicit StableTraits ( Gene* using_gene );
		bool Next_Trait ( Cord** tag_name, String** value_string );
	};


	/// Same Gene with unique Traits
	class DuplicateTraits : NoAllocator
	{
	public:
		Gene* gene;
		explicit DuplicateTraits ( Gene* _gene )
		: gene ( nullptr ) { _gene->Duplicate_Gene ( &gene ); }
		~DuplicateTraits() { gene->Release(); }
	};


	/// Named String pair
	class Trait : public NoCopy
	{
	public:
		Trait*  next_trait;
		Cord*   trait_name;
		String* trait_term;
		explicit Trait ( Cord*  trait_name,  Cord* term_value, Trait* next_trait );
		explicit Trait ( Trait* using_trait,                   Trait* next_trait );
		~Trait ( void );
		void Print_Into ( String* into_string );
	};
	class TraitMap : public MapT<Cord, Trait*> {};

	/// A map/list of key/value pairs
	class Traits   : public NoCopy
	{
	public:
		Trait*     first_trait;
		TraitMap*  map_of_traits;
		~Traits ( void );
		Traits  ( void );
		void  Set_Trait      ( Cord*    trait_name,    Cord*    term_value );
		void  Evaluate       ( Gene*    attributes,    Nucleus* using_gene );
		void  Duplicate_Into ( Traits** copy_of_traits );
		void  Print_Into     ( String*  into_string    );
	};


	/// Add Properties and Gene to a Cell implementing Methods and Forms
	class Nucleus : public NoCopy
	{
	protected:
		MethodMap*  method_map;
		virtual ~Nucleus ( void );
		explicit Nucleus ( Neuron*, Cord* );
	public:
		Neuron*     parent_neuron;
		Gene*       observer;
		Gene*       shadows;
		virtual Neuron*  Host ( void );
		virtual void Nucleus_Path     ( String*  into_string,  const char using_separator );
		virtual bool Nucleus_Processed( Nucleus* using_cell,   Gene*      xepl_gene );
		virtual bool Nucleus_Viewer   ( Cord*      view_name,  Gene** );
		virtual void Nucleus_Dropped  ( void );
	    bool Nucleus_Rendered ( Rendon*  using_rendon, Cord*      form_name );
		bool Took_Action          ( Gene*  xepl_gene );
		void Register_Method      ( Cord*  method_name, Function cell_function, Gene* xepl_gene);
		void Register_Method      ( Text*  method_name, Function cell_function, Gene* xepl_gene);
		bool Performed_Method     ( Cord*  method_name, Gene*    on_gene );
		bool Performed_Method     ( Text*  method_name, Gene*    on_gene );
		void Method_Execute       ( Gene*  call_gene,   Gene* after_code );
		void Register_Gene        ( Cord*  gene_name,   Gene* to_be );
		void Register_Gene        ( Text*  gene_name,   Gene* to_be );
		bool Process_Gene         ( Gene*  code_gene );
		void Process_Inner_Genes  ( Gene*  code_gene );
		bool Process_Exact_Gene   ( Cord*  gene_name,   Gene* inside_gene );
		bool Process_Exact_Gene   ( Text*  gene_name,   Gene* inside_gene );
		bool Property_Get         ( Cord*  trait_name,  String* append_to );
		bool Property_Hunt        ( Cord*  trait_name,  String* append_to );
		void Property_Set         ( Cord*  trait_name,  Cord*   using_value );
		void Property_Set         ( Text*  trait_name,  Cord*   using_value );
		void Register_Form        ( Gene*  form_gene );
		bool Form_Get             ( Cord*  form_name,   Gene** found_gene );
	};


	/// Axon and Receptor to a Cell Nucleus to send/receive Action
	class Neuron : public Cell, public Nucleus
	{
		friend class Synapse;
		friend class Receptor;
		friend class DropAction;
		ReceptorChain*  receptor_chain;
		AxonChain*      axon_chain;
		AxonMap*        axon_map;
		RelayMap*       relay_map;
		NeuronMap*      neuron_map;
		NeuronChain*    neuron_chain;
		void  Drop_My_Axons       ( void );
		void  Drop_My_Neurons     ( void );
		void  Drop_My_Receptors   ( void );
		void  Register_Neuron     ( Neuron* child_neuron );
		void  Unregister_Neuron   ( Neuron* child_neuron );
		bool  Hunt_Neuron         ( Cord*   neuron_name, Neuron** found_neuron );
		bool  Get_Neuron          ( Cord*   neuron_name, Neuron** found_neuron );
		void  Connect_Receptor    ( Axon*   axon_name,   Receptor* to_receptor );
		void  Disconnect_Receptor ( Receptor* from_receptor );
		void  Disconnect_Relay    ( Relay*    from_receptor );
		void  Relays_Dont_Deliver ( Cell*,  Relay* );
	protected:
		virtual ~Neuron ( void ) override;
		virtual Neuron* Host           ( void ) override;
		virtual void Nucleus_Dropped   ( void ) override;
		virtual void Nucleus_Path      ( String* into_string, const char separator ) override;
		virtual void Neuron_Axon_Relay ( Axon*   the_axon,    Receptor*  through_receptor, Relay** the_relay );
		virtual void Neuron_Drop_Relay ( Relay* );
	public:
		Cord*          alias;
		explicit Neuron  ( Text*       neuron_name );
		explicit Neuron  ( Neuron*     parent_neuron, Gene* config_gene );
		void  Register_Axon     ( Axon* );
		void  Unregister_Axon   ( Axon* );
		bool  Get_Axon          ( Cord*   axon_name,    Axon**   found_axon  );
		bool  Hunt_Axon         ( Cord*   axon_name,    Axon**   found_axon  );
		void  Receive_Axon      ( Atom*   target_axon,  Atom*    signal_atom );
		void  Synapse_Axon      ( Axon*   target_axon,  Receiver to_receiver, Cell* memento_cell );
		bool  Find_Neuron       ( Cord*   neuron_name,  Neuron** found_neuron );
		bool  Find_Neuron       ( Text*   neuron_name,  Neuron** found_neuron );
		bool  Drop_Neuron       ( Cord*   neuron_name );
		bool  Drop_Neuron       ( Text*   neuron_name );
		void  Show_Neurons      ( String* into_string );
		void  Register_Macro    ( Cord*   macro_name,   String* macro_string );
		bool  Performed_Macro   ( Cord*   macro_name,   Cord*   macro_string, String* seed_string, bool& truth, String* );
		bool  Macro_Hunt        ( Cord*   macro_name,   String* macro_string );
		bool  Feature_Get       ( Cord*   feature_name, String* feature_value );
		bool  Feature_Get       ( Text*   feature_name, String* feature_value );
		void  Method_Terminate  ( Gene*   unused1,      Gene* unused2 );
	};
	class NeuronMap : public MapReleaseT<Cord, Neuron*> {};

	class NeuronChain : public Chain
	{
		NeuronChain( );
		Neuron* Last ( void );
		friend class Neuron;
	};


	/// hold the Index on the stack and replace
	class ScopeIndex : NoAllocator
	{
	public:
		ScopeIndex( Gene* );
		~ScopeIndex();
	};


	/// indicies reach back into the stack of indexes
	class Indicies : NoAllocator
	{
		Bond*   tail_bond;
	public:
		Indicies();
		void  Stack  ( Lobe* host_lobe, Gene* replacement_gene );
		void  Unstack( Lobe* host_lobe) ;
		Gene* Index  ( int  );
	};


	/// rendons render Nucleus Forms into the Output
	class Rendon : public Cell, public Nucleus
	{
		friend class Nucleus;
		virtual bool Nucleus_Processed ( Nucleus*, Gene* ) override;
		virtual void Rendon_Markup     ( Gene*,    Gene* ) = 0;
		void         Generate_Payload  ( Nucleus*, Gene* );
	protected:
		void     Markup ( Nucleus* using_cell,   Gene* using_form, String* add_to_string );
		explicit Rendon ( Neuron*  render_host,  Gene* using_form, String* add_to_string );
		~Rendon () override;
	public:
		String*  rendition;
		Rendon*  was_rendon;
		virtual void Rendon_Render ( Nucleus* render_cell, Gene* using_form );
	};


	/// Senson is a Neuron collecting sensor
	class Senson : public Neuron
	{
		virtual void Senson_Closed  ( void ) = 0;
		virtual void Senson_Scan    ( void ) = 0;
		void Method_Senson_Received ( Gene*, Gene* );
		void Method_Senson_Closed   ( Gene*, Gene* );
	protected:
		Wire*  senson_wire;
		virtual ~Senson ( void ) override;
	public:
		explicit Senson ( Neuron* parent_neuron, Gene* config_gene );
	};


	/// List of Action to be processed by the Lobe
	class ActionList : public NoCopy
	{
		friend class Lobe;
		Mutex*    actions_lock;
		Action*   head_action;
		Action*   tail_action;
		Lobe*     lobe;
		bool      list_is_closed;
	public:
		~ActionList ( void );
		explicit ActionList    ( Lobe*  host_lobe );
		void Close_Action_List ( void );
		void Flush_Action_list ( void );
		bool Pull_Action       ( Action**   next_action );
		void Post_Action       ( Action*  latest_action );
	};


	/// Aliased Genes that are available during the current Action only
	class Ephemerals : public MapReleaseT< Cord, Gene* >
	{
	public:
		void Set ( Cord* gene_name, Gene* now_linked_to );
	};

	/// ShortTerms are Traits restored by stack/scope
	class ShortTerms : NoAllocator
	{
		ShortTerms*&   restore_terms_ref;
		ShortTerms*    previous_terms;
		ShortTerms*    hot_terms;
		ShortTermMap*  term_map;
	public:
		~ShortTerms ( void );
		ShortTerms  ( void );
		explicit ShortTerms ( Gene* using_traits_from );
		explicit ShortTerms ( Text*       trait_name, Cord* term_value );
		void  Nest_Traits   ( Gene*       using_traits_from );
		bool  Get_Into      ( Cord*       trait_name, String* into_string );
		void  Set           ( Cord*       trait_name, Cord*   term_value  );
		void  Set           ( Text*       trait_name, Cord*   term_value  );
		static void Replace_Traits ( Gene* using_terms );
	};


	/// The Lobe is the Thread Neuorn foor all its Neurons
	class Lobe : public Neuron
	{
		virtual void Nucleus_Dropped   ( void   ) override;
		virtual void Neuron_Drop_Relay ( Relay* ) override;
		virtual void Neuron_Axon_Relay ( Axon*, Receptor*, Relay** ) override;
		void Relay_Nop                 ( Cell*, Relay* );
		void Method_Terminate          ( Gene*, Gene* );
	protected:
		virtual ~Lobe                  ( void ) override;
		virtual void Lobe_Rest_Now     ( void );
	public:
		ActionList*       pending_actions;
		Semaphore*        rest_semaphore;
		Thread*           cpp_thread;
		Gene*             index_link;
		Gene*             outdex_link;
		Gene*             locals;
		Ephemerals*       ephemerals;
		ShortTerms*       short_terms;
		Lobe*             parent_lobe;
		Rendon*           active_rendon;
		Atom*             trigger_atom;
		String*           output_string;
		Indicies          indicies;
		Counters          counters;
		explicit Lobe ( Text*    lobe_name );
		explicit Lobe ( Neuron*  parent_neuron, Gene* config_gene );
		virtual void Lobe_Dying    ( void );
		virtual void Lobe_Wake_Up  ( void );
		virtual void Lobe_Born     ( void );
		void Start_Lobe            ( void );
		void Stop_Lobe             ( void );
		void Main_Loop             ( Semaphore*  loaded_semaphore );
		bool Dispatch_Action       ( void );
		void Close_Dispatch        ( void );
		void Set_Outdex            ( Gene* linked_gene );
		Gene* Index                ( int );
	};



	/// A shared Queue between Genes or Neurons
	class Wire : public Atom
	{
	protected:
		virtual ~Wire ( void ) override;
	public:
		Mutex*    wire_mutex;
		String*   wire_string;
		Wire ( void );
		size_t Avail        ( void );
		void   Erase        ( void );
		void   Append       ( Text*    from_chars, size_t number_of_bytes );
		void   Append       ( Wire*    from_wire   );
		void   Append       ( Cord*    from_cord   );
		void   Assign       ( Cord*    from_cord   );
		bool   Extract_Line ( String*  into_string );
		bool   Expire       ( size_t   number_of_bytes );
		void   Print_Into   ( String*  into_string );
	};

	/// Error String catch all
	class ErrorReport : public String
	{
	public:
		~ErrorReport         ( void );
		explicit ErrorReport ( Text* );
		explicit ErrorReport ( Text*, Text* );
		explicit ErrorReport ( Text*, Cord* );
	};



	/// A signal conduit for delivering an Action to Receptor Neuron
	class Axon : public Atom
	{
	protected:
		virtual ~Axon ( void ) override;
	public:
		Cord*            axon_name;
		Neuron*          host_neuron;
		ReceptorChain*   receptor_chain;
		explicit Axon ( Neuron*  host_neuron, Cord*  axon_name );
		explicit Axon ( Neuron*  host_neuron, Text*  axon_name );
		void Synapse          ( Neuron* host_neuron,  Gene* action_gene );
		void Trigger          ( Atom*   signal_gene );
		void Trigger_Wait     ( Atom*   signal_gene );
		void Cancel_Receptors ( void );
	};
	class AxonMap : public MapReleaseT<Cord, Axon*> {};
	class AxonChain : public Chain
	{
	public:
		AxonChain ( void );
		void Cancel_All_Receptors ( void );
	};


	/// Receptors attach to an Axon and catch/deliver the Action
	class Receptor : public Atom
	{
	protected:
		virtual  ~Receptor ( void ) override;
	public:
		Neuron*   target_neuron;
		Axon*     signal_axon;
		Receiver  signal_receiver;
		Atom*     memento_atom;

		explicit Receptor ( Neuron* target_neuron, Axon* axon_name, Receiver, Atom* deliver_memento ) ;
		virtual void Receptor_Activate ( Atom* ) const;
		virtual void Receptor_Cancel   ( void );
	};
	class ReceptorChain : public Chain
	{
	public:
		explicit ReceptorChain     ( void );
		void Deliver_Signal        ( Atom* );
		void Disconnect_Receptors  ( void );
	};

	/// The Relay is a Receptor for the Neuron to pass a Signal thru to the target Neuron
	class Relay : public Receptor
	{
	protected:
		virtual ~Relay() override;
	public:
		ReceptorChain*  receptor_chain;
		explicit Relay ( Neuron*, Axon*, Receiver, Receptor* );
		virtual void Receptor_Activate( Atom* ) const override;
	};
	class RelayMap : public UMapT<Axon*, Relay*> {};

	/// The Synapse is the queing Relay attachment from the Lobe to the Axon
	class Synapse : public Relay
	{
		ActionList*     action_list;
	public:
		explicit Synapse ( Lobe*, Axon*, Receiver, Receptor* );
		virtual void Receptor_Activate( Atom* ) const override;
	};


	/// An Action is generated by one Neuron for processing by another Synapse Neuron
	class Action : public NoCopy
	{
	public:
		Receptor*   receptor;
		Atom*       trigger_atom;
		Action*     next_action;
		explicit Action ( Receptor* deliver_to_receptor, Atom* signal_atom );
		virtual ~Action ( void );
		virtual void Action_Execute  ( void ) = 0;
	};

	/// The Signal Action carries an Atom to be processed
	class SignalAction : public Action
	{
	public:
		explicit     SignalAction   ( Receptor* deliver_to_receptor, Atom* signal_atom );
		virtual void Action_Execute ( void ) override;
	};

	/// The DropAction indicates the Neuron is to be dropped
	class DropAction : public Action
	{
		Neuron*  neuron;
	public:
		explicit     DropAction     ( Neuron* drop_neuron );
		virtual void Action_Execute ( void ) override;
	};



	/// A Spike is an Action Atom carried along the Axon
	class Spike : public Atom
	{
	protected:
		virtual ~Spike ( void ) override;
	public:
		Atom*  stimulus;
		explicit Spike ( Atom* );
	};

	/// Rendevous is a Spike that must be processed and completed before continuing
	class Rendezvous : public Spike
	{
		friend class Axon;
		Lobe*         lobe;
		Semaphore**   semaphore;
		bool*         u_must_wait;
	protected:
		virtual ~Rendezvous ( void ) override;
	public:
		explicit Rendezvous ( Atom*, Semaphore**, bool* _must_wait );
	};


	class ParserChoice;
	class ParserChoices;
	typedef bool ( *ParserOption ) (Parser* );
	typedef long ParserFlags;

	/// Integrated Parser framework uses ParserBag for String walking
	class Parser : public NoCopy
	{
	protected:
		~Parser( void );
	public:
		ParserBag*    parser_bag;
		ErrorReport*  error_string;
		Parser ( void );
		void Record_Error ( Text*, Text*   );
		void Record_Error ( Text*, String* );
	};

	/// For walking the data String for the Parser
	class ParserBag : public NoCopy
	{
	protected:
		explicit ParserBag ( Cord* bag_to_parse, Parser* parent_parser );
	public:
		Parser*  parser_host;
		Text*    start_position;
		Text*    current_position;
		Text*    last_position;
		Text*    start_of_line;
		long     line_number;
		long Remaining        ( void );
		long Offset           ( void );
		long Column           ( void );
		void Skip_Whitespace  ( void );
		bool Discard_Char     ( char );
		bool Consume          ( char );
		bool Consume          ( char, char );
		bool Consume          ( char, char, char );
		bool Consume          ( char, char, char, char );
		void Parse_Error      ( Text*  error_message );
	};



	/// One of the ParserOption available at this processing point
	class ParserChoice
	{
	public:
		ParserOption  parser_option;
		ParserFlags   parser_flags;
	};

	/// All the possible ParserChoice at this processing point
	class ParserChoices : public Recycler
	{
	public:
		static constexpr long     MAX_CHOICES=7;
		ParserChoice  choice_array[MAX_CHOICES];
	};

	/// Parser runs through ParserChoices during the ~ ParserSelect
	class ParserSelect : NoAllocator, NoCopy
	{
		Parser*          parser;
		ParserChoices*   parser_choices;
		int              number_of_choices;
	public:
		~ParserSelect ( void );
		explicit ParserSelect ( Parser* );
		void Add_Option ( ParserFlags , ParserOption );
		const static ParserFlags No_Flags   = 0;
		const static ParserFlags Completes  = 1<<0;
		const static ParserFlags Can_Repeat = 1<<1;
	};


	/// the RnaBag scripting language Parser
	class Script : public Parser, NoAllocator
	{
		friend class RnaBag;
		Neuron*  neuron;
		RnaBag*  rna;
		Lobe*    lobe;
	protected:
		void  Translate           ( Cord* expression );
		void  Process_Gene        ( Gene* new_active_gene );
		bool  Extract_Value       ( void  );
		void  Enter_Child_Gene    ( void  );
		void  Select_Attribute    ( void  );
		void  Serialize           ( void  );
		void  Declare_Content     ( void  );
		void  Mutate_Value        ( void  );
		void  Enter_Inner_Block   ( void  );
		void  Extract_Parameter   ( String** into_string );
		void  Process_Neuron      ( void  );
		void  Change_Gene         ( void  );
		void  Extract_Property    ( void  );
		void  Extract_Feature     ( void  );
		bool  Get_Property        ( void );
		bool  Hunt_Property       ( String* tag_name  );
		void  Tap_Term            ( void  );
		void  Tap_Index           ( String* trait_name, String* term_value );
		void  Tap_Local           ( String* trait_name, String* term_value );
		void  Tap_Property        ( String* trait_name, String* term_value );
		void  Tap_Short_Term      ( String* trait_name, String* term_value );
		void  Ternary_Choice      ( void  );
		void  Report_Any_Errors   ( void  );
	public:
		String*    value;
		Gene*      gene;
		bool       truth;
		~Script();
		explicit Script ( Neuron* host_neuron, Gene* active_gene, String* = nullptr );
		explicit Script ( Neuron* host_neuron, Gene* active_gene, Cord* expr, String* result);
		explicit Script ( Neuron* host_neuron, Gene* active_gene, Cord* expr, String* result, bool* truth_in_out, Cord* seed, bool append_result );
		bool Get_Next_Value ( void  );
	};



	/// ParserBag String support fo the Script
	class RnaBag : public ParserBag
	{
	public:
		Script*  script;
		explicit RnaBag  ( Cord* script_cord, Script* active_script );
		void Pull_Tag    ( String* into_string );
		bool Starts_Tag  ( void );
		bool In_Tag      ( void );
		bool Pull_Number ( void );
		void Pull_String ( void );
	};



	/// The current XmlNode for the XmlParser
	class XmlNode : public Recycler
	{
	public:
		XmlParser*   xml_parser;
		XmlNode*     parent_node;
		Gene*        element_gene;
		bool         parser_wants_it;
		~XmlNode();
		explicit XmlNode ( XmlParser* );
	};

	/// The ParserBag for the XmlParser
	class XmlBag : public ParserBag
	{
	public:
		XmlParser* xml_parser;
		explicit XmlBag ( Cord*, XmlParser* );
		void  Discard_Shell_Directive ( void );
		char  Extract_Quoted_Value    ( String* _extracted_value );
		void  Extract_Attribute_Name  ( String* _extracted_value );
		void  Extract_CData           ( String* _extracted_value );
		void  Extract_Comment         ( String* _extracted_value );
		void  Extract_DocType         ( String* _extracted_value );
		void  Extract_Instruction     ( String* _extracted_value );
		void  Extract_PCData          ( String* _extracted_value );
		Cord* Extract_Space_Tag       ( String* _extracted_tag );
		bool  at_xmlPCData ( void )       { return *current_position && *current_position != '<'; }
		bool  in_xmlPCData ( char _char ) { return _char && _char != '<'; }
		bool  at_xmlTag    ( void )       { return iswalnum ( *current_position ) || // XEPL+- allow numbers to start tags.
			*current_position == '_'      ||
			*current_position == ':';
		}
		bool  in_xmlTag ( void )          {	return iswalnum ( *current_position ) ||
			*current_position == ':'      ||
			*current_position == '_'      ||
			*current_position == '.'      ||
			*current_position == '+'      ||    // allow tags with '+' too.
			*current_position == '-';
		}
	};



	/// Converts from String to a Gene using the Parser and the XmlBag
	class XmlParser : public Parser
	{
	protected:
		bool do_BeginNode    ( void );
		bool do_CData        ( void );
		bool do_CloseNode    ( void );
		bool do_CloseSplit   ( void );
		bool do_Comment      ( void );
		bool do_Content      ( void );
		bool do_DocType      ( void );
		bool do_Instruction  ( void );
		bool do_NodeSplit    ( void );
	public:
		XmlBag*        xml_bag;
		XmlNode*       active_node;
		Gene*          root_gene;
		virtual ~XmlParser ( void );
		explicit XmlParser ( Gene*, Cord* );
		virtual void Xml_DocType       ( String* );
		virtual bool Xml_New_Comment   ( String* );
		virtual bool Xml_New_Element   ( Gene*   );
		virtual bool Xml_End_Element   ( Gene*   );
		virtual bool Xml_New_Content   ( String* );
		virtual bool Xml_New_Attribute ( String*, String*, char );
		virtual void Xml_Make_Element  ( Gene*,   Cord*, Cord*, Gene** );
		virtual void Xml_Undo_Element  ( Gene*   );
		bool ParseIt                   ( void );
	};

	/// XeplXml is Xml with _comments
	class XeplXml : public XmlParser
	{
	public:
		explicit XeplXml ( Gene*, Cord* );
		virtual bool Xml_New_Element( Gene* ) override;
	};



	/// Support to build an XML String
	class XmlBuilder : public NoCopy
	{
		String*   tag_n_space;
		String*   build_string;
		bool      attributes_closed;
		bool      build_closed;
	public:
		~XmlBuilder ( void );
		explicit XmlBuilder ( Text* tag_name, Text*   attributes,  String* into_string);
		explicit XmlBuilder ( Cord* tag_name, String* into_string, Cord*   space_name );
		explicit XmlBuilder ( Text* tag_name, String* into_string );
		void  Close_Attributes ( void );
		void  Close            ( void );
		void  Absorb_Traits    ( Nucleus*, Gene* );
		void  Attribute_Set    ( Cord*,    Cord* );
		void  Attribute_Set    ( Text*,    Text* );
	};



	class KeywordsMap  : public MapT<Cord, Keyword>  {};
	class OperatorsMap : public MapT<Cord, Operator> {};
	class CommandsMap  : public MapT<Cord, Command > {};
	class MutualsMap   : public MapT<Cord, Mutual>   {};
	class RenderMap    : public MapT<Cord, Render >  {};



	/// The Cortex is your single point of organization and uses a host Lobe as a Neuron proxy
	class Cortex : NoAllocator, NoCopy
	{
		MemoryCounts     memory_counts;
		RecycleCounts    recycle_counts;
		Backpack         memory_backpack;
		KeywordsMap*     keywords_map;
		OperatorsMap*    operators_map;
		CommandsMap*     commands_map;
		MutualsMap*      mutual_map;
		RenderMap*       render_map;
	public:
		Lobe*            host_lobe;
		ShortTerms       short_term_memories;
		Counters         final_counters;
		~Cortex ( void );
		explicit Cortex ( Text*  cortex_name, std::ostream& output_stream );
		void  Close_Cortex        ( void );
		void  Register_Keyword    ( Text*    keyword_name,  Keyword  );
		void  Register_Operator   ( Text*    operator_name, Operator );
		void  Register_Command    ( Text*    command_name,  Command  );
		void  Register_Mutual     ( Text*    gene_name,     Mutual   );
		void  Register_Render     ( Text*    render_name,   Render   );
		bool  Did_Command         ( Text*    command_string );
		bool  Did_Keyword         ( Nucleus* host_cell, Gene* call_tag );
		bool  Did_Dot_Tag         ( Nucleus* host_cell, Gene* call_tag );
		bool  Did_Render          ( Nucleus* host_cell, Gene* call_tag,  Rendon* );
		bool  Did_Operator        ( Cord* operator_name, Script* active_script, Cord* parameter );
		static Gene*   Locate_Gene   ( Nucleus* starting_at,  Cord* gene_name  );
		static Neuron* Locate_Neuron ( Nucleus* starting_at,  Cord* neuron_name, char separator );
		static Axon*   Locate_Axon   ( Nucleus* starting_at,  Cord* axon_name,   char separator );
	};

}

std::ostream& operator<< (std::ostream&, XEPL::Cord* );
std::ostream& operator<< (std::ostream&, XEPL::Cord& );
