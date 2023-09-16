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
#include <algorithm>
#include <arpa/inet.h>
#include <atomic>
#include <cctype>
#include <cerrno>
#include <chrono>
#include <climits>
#include <condition_variable>
#include <csignal>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <iosfwd>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <mutex>
#include <netdb.h>
#include <netinet/in.h>
#include <numeric>
#include <ostream>
#include <set>
#include <sstream>
#include <string>
#include <sys/select.h>
#include <sys/socket.h>
#include <thread>
#include <tuple>
#include <unistd.h>
#include <unordered_map>
#include <utility>
#include <vector>


namespace XEPL
{
	namespace ATOM      {};
	namespace CELL      {};
	namespace CORTEX    {};
	namespace DNA       {};
	namespace RNA       {};
	namespace SIGNAL    {};
	namespace MEMORY    {};
	namespace OS        {};
	namespace THREAD    {};
	namespace KEYWORDS  {};
	namespace OPERATORS {};
	namespace RENDER    {};
	namespace TEXT      {};
	namespace MAPS      {};
	namespace SOMA      {};
	namespace XML       {};
}

#define CORTEX_REGISTRATION                                     \
( XEPL::CORTEX::Cortex* _cortex, const char* _as )

#define CORTEX_KEYWORD                                          \
( [[maybe_unused]] XEPL::CELL::Nucleus*  _nucleus,              \
 [[maybe_unused]] XEPL::DNA::Gene*       _call_gene)

#define CORTEX_OPERATOR                                         \
( [[maybe_unused]] XEPL::TEXT::String*  _lhs,                   \
 [[maybe_unused]] XEPL::TEXT::String*   _rhs,                   \
 [[maybe_unused]] bool&                 _truth)

#define SIGNAL_RECEIVER                                         \
( [[maybe_unused]] XEPL::ATOM::Atom*    _signal,                \
 [[maybe_unused]]  XEPL::ATOM::Atom*    _memento )

#define NUCLEUS_METHOD                                          \
( [[maybe_unused]] XEPL::DNA::Gene*     _call,                  \
 [[maybe_unused]] XEPL::DNA::Gene*      _definition )

#define DNA_SPLICER                                             \
( [[maybe_unused]] XEPL::TEXT::Cord*    _seed,                  \
 [[maybe_unused]] XEPL::CELL::Nucleus*  _nucleus,               \
 [[maybe_unused]] XEPL::DNA::Gene*&     __gene,                 \
 [[maybe_unused]] XEPL::TEXT::Cord*     _param,                 \
 [[maybe_unused]] bool*                 _truth )

#define CORTEX_RENDER                                           \
( [[maybe_unused]] XEPL::CELL::Nucleus* _nucleus,               \
[[maybe_unused]] XEPL::DNA::Gene*       _element,               \
[[maybe_unused]] XEPL::CORTEX::Rendron* _render )

namespace XEPL::ATOM
{
	class Atom;
	class Bond;
	class Chain;
	class Scope_Release;

	class Duct;
	typedef void ( *Denter ) ( Duct& );
	Duct& operator<< ( Duct&, Denter );
}


namespace XEPL::CELL
{
	class Cell;
	class Method;
	class MethodMap;
	class Nucleus;
}

namespace XEPL::CORTEX
{
	class AxonLocator;
	class AxonMap;
	class ChromosMap;
	class Cortex;
	class KeywordsMap;
	class Lobe;
	class Neuron;
	class NeuronChain;
	class NeuronLocator;
	class NeuronMap;
	class NucleusLocator;
	class OperatorsMap;
	class RelayMap;
	class RenderMap;
	class Rendron;
	class RenkeysMap;
	class Scope_Render;
	class Sensor;
	class SplicerMap;
}

namespace XEPL::DNA
{
	class BondMap;
	class ChainMap;
	class Gene;
	class GeneChain;
	class Genes;
	class Ephemerals;
	class Scope_Duplicate;
	class Scope_Gene;
	class Scope_Index;
	class Scope_Terms;
	class StableRecall;
	class TermMap;
	class Trait;
	class TraitMap;
	class Traits;
}

namespace XEPL::MAPS
{
	template<class T, class P> class MapT;
	template<class T, class P> class MapDeleteT;
	template<class T, class P> class MapReleaseT;
}

namespace XEPL::MEMORY
{
	class HeapOfPools;
	class MemoryBackpack;
	class PoolOfBlocks;
	class Recycler;
	union BlockHeader;
	template <class T> class TAllocatorT;
	template <class T> class Scope_Delete;
}

namespace XEPL::RNA
{
	class RnaScript;
	class RnaBag;
	class Script_Append;
	class Script_Assign;
	class Script_Truth;
}

namespace XEPL::SIGNAL
{
	class Action;
	class ActionList;
	class Action__Drop;
	class Action__Signal;
	class Axon;
	class AxonChain;
	class Synapse;
	class Receptor;
	class ReceptorChain;
	class Relay;
	class Signal;
	class Signal__Rendezvous;
}

namespace XEPL::TEXT
{
	class Names;
	class Parser;
	class ParserBag;
	class ParserChoices;
	class ParserSelect;
	class String;
	class Wire;
	struct ParserChoice;
	typedef const String Cord;
}

namespace XEPL::THREAD
{
	class Mutex;
	class MutexScope;
	class MutexResource;
	class Semaphore;
	class Thread;
}

namespace XEPL::XML
{
	class XmlBuilder;
	class XmlParser;
	class XmlBag;
	class XmlNode;
	class PrintXml;
	class XeplXml;
	class XmlIntsructionMap;
}


namespace XEPL
{
	extern thread_local CORTEX::Lobe* tlsLobe;

	namespace CELL
	{
		typedef void ( Nucleus::*Function ) NUCLEUS_METHOD;
	}

	namespace SIGNAL
	{
		typedef void ( ActionList::*DeliverAction )  ( SIGNAL::Action* );
		typedef void ( CORTEX::Neuron::*Receiver ) SIGNAL_RECEIVER;
	}

	namespace TEXT
	{
		typedef bool ( *ParserOption ) ( TEXT::Parser* );
	}

	namespace DNA
	{
		typedef DNA::Gene* ( *Chromosome )  ( XEPL::CELL::Nucleus* _nucleus );
		typedef void       ( *Splicer    )  DNA_SPLICER;
	}

	namespace CORTEX
	{
		typedef void ( *Keyword  ) CORTEX_KEYWORD;
		typedef void ( *Render   ) CORTEX_RENDER;
		typedef void ( *Operator ) CORTEX_OPERATOR;
	}

	namespace XML
	{
		typedef void ( *xmlInstruction )  ( TEXT::Cord*, TEXT::Cord* );
	}
}

namespace XEPL::SOMA
{
	class ErrorReport;

	TEXT::String* Escape_Quotes  ( TEXT::Cord*,     TEXT::String* );
	void  Escape_Quotes_Remove   ( TEXT::Cord*,     TEXT::String* );
	void  Gene_Absorb_Query      ( DNA::Gene*,      TEXT::String* );
	bool  Gene_From_Url          ( const char*,     DNA::Gene* );
	void  Lobe_Set_Thread_Name   ( THREAD::Thread*, const char*   );

	std::string* Long_Commafy    ( long,            std::string* );
	std::string* Long_In_Bytes   ( long,            std::string* );

	class GeneLocator
	{
	public:
		DNA::Gene*                 gene = nullptr;

		GeneLocator ( CELL::Nucleus* _nucleus, TEXT::Cord* _name )
		{
			if ( _name )
				Hunt ( _nucleus, _name );
		}
		bool Hunt ( CELL::Nucleus* _nucleus, TEXT::Cord* _name );
	};

	class LocateIndex : public GeneLocator
	{
	public:
		LocateIndex ( CELL::Nucleus* _nucleus, TEXT::Cord* _name );
	};
}


namespace XEPL
{
	extern bool Show_Collisions;
	extern bool Show_Trace;
	extern bool Show_Memory_Counts;
	extern bool Show_Counters;
	extern bool Show_Final_Report;
	extern bool alive;
	extern const char* EOL;
}

namespace XEPL::MEMORY
{
	constexpr long Block4k = 4*1024-32;
	constexpr long Block8K = 8*1024-32;
}

namespace XEPL
{
	void TRACE(const char* action, XEPL::CORTEX::Neuron* neuron, XEPL::TEXT::Cord* name);
}


class XEPL::MEMORY::Recycler
{
public:
	static void* operator new    ( size_t );
	static void  operator delete ( void* );
};


template <class T>
class  XEPL::MEMORY::TAllocatorT
{
public:

	typedef T                      value_type;
	typedef T*                     pointer;
	typedef const T*               const_pointer;
	typedef T&                     reference;
	typedef const T&               const_reference;
	typedef size_t                 size_type;
	typedef ptrdiff_t              difference_type;

	~TAllocatorT ( void )
	{}

	template <class U> explicit TAllocatorT ( const TAllocatorT<U>& )
	{}

	TAllocatorT ( void )
	{}

	pointer allocate             ( size_type _num )
	{
		return ( T* ) ( Recycler::operator new ( _num * sizeof ( T ) ) );
	}
	void deallocate              ( pointer _ptr, size_type )
	{
		Recycler::operator delete ( _ptr );
	}
};


template<typename T1, typename T2>
using PairTAllocator     = XEPL::MEMORY::TAllocatorT<std::pair<T1, T2>>;

using StdStringAllocator = XEPL::MEMORY::TAllocatorT<std::string>;
using CharAllocator      = XEPL::MEMORY::TAllocatorT<char>;
using IntAllocator       = XEPL::MEMORY::TAllocatorT<int>;


#ifdef NO_RECYCLE_STRING

namespace XEPL::TEXT
{
	typedef	std::string CppString;
}

#else

namespace XEPL::TEXT
{
	typedef std::basic_string<char, std::char_traits<char>, CharAllocator> CppString;
}

#endif


class XEPL::TEXT::String : public MEMORY::Recycler, public TEXT::CppString
{
public:
	        ~String ( void ) {}
	         String ( void );
			 String ( const TEXT::CppString* );
	explicit String ( const TEXT::Cord* );
	         String ( const char* );
	explicit String ( const char*, size_t );

	String& operator= (const String& rhs)
	{
		if (this != &rhs)
		{
			this->assign(rhs);
		}
		return *this;
	}

	String operator+ (const char* str) const
	{
		String result = *this;
		result += str;
		return result;
	}

	String operator+ (const String& str) const
	{
		String result = *this;
		result += str;
		return result;
	}

	String& operator+ (String* str)
	{
		this->append(*str);
		return *this;
	}

	String& operator+ (Cord* str)
	{
		this->append(*str);
		return *this;
	}

	String& operator+= (const char* rhs)
	{
		this->append(rhs);
		return *this;
	}

	String* operator+= (Cord* rhs)
	{
		this->append(rhs->c_str());
		return this;
	}

	String& operator+= (const String& rhs)
	{
		this->append(rhs);
		return *this;  
	}
	bool operator== (const String& rhs) const { return this->compare(rhs) == 0; }
	bool operator!= (const String& rhs) const { return !(*this == rhs); }
	bool operator<  (const String& rhs) const { return this->compare(rhs) < 0; }
	bool operator>  (const String& rhs) const { return rhs < *this; }
	bool operator<= (const String& rhs) const { return !(rhs < *this); }
	bool operator>= (const String& rhs) const { return !(*this < rhs); }


	bool  Split_at_lhs_rhs ( size_t, TEXT::String*, TEXT::String* ) const;
	bool  Split_ch_lhs_rhs ( char,   TEXT::String*, TEXT::String* ) const;
	bool  Split_ch_lhs     ( char,   TEXT::String* );
	bool  Split_ch_rhs     ( char,   TEXT::String* );

	friend std::ostream& operator<< (std::ostream& os, const String& str);
	friend std::istream& operator>> (std::istream& is, String& str);
};

namespace std
{
	template <>
	struct hash<XEPL::TEXT::Cord>
	{
		std::size_t operator() (const XEPL::TEXT::Cord& str) const
		{
			// djb2
			return std::accumulate(str.begin(), str.end(), static_cast<std::size_t>(5381),
				[](std::size_t hash, char c) {
					return ((hash << 5) + hash) + static_cast<unsigned char>(c); // hash * 33 + c
				}
			);
		}
	};
}


class XEPL::ATOM::Atom : public XEPL::MEMORY::Recycler
{
public:
	std::atomic_long               retain_count;
protected:
	virtual ~Atom( void ) {}
	         Atom( void );

public:

	void  Release                ( void );
	void  Destroy                ( void );
};


class XEPL::CELL::Cell : public ATOM::Atom
{
public:
	TEXT::String*                  cell_name;

protected:
	~Cell         ( void ) override;
	explicit Cell ( TEXT::Cord* );
	explicit Cell ( const char* );

};


namespace XEPL::MAPS
{
	template <class Tkey, class Tvalue>
	class MapT
		: public MEMORY::Recycler
		, public std::map< Tkey, Tvalue, std::less<Tkey>, MEMORY::TAllocatorT<std::pair<const Tkey, Tvalue> > >
	{};


	template <class Tkey, class Tvalue>
	class UMapT
		: public MEMORY::Recycler
		, public std::unordered_map<Tkey, Tvalue, std::hash<Tkey>, std::equal_to<Tkey>, MEMORY::TAllocatorT<std::pair<const Tkey, Tvalue>>>
	{
	public:
		int Collisions()
		{
			size_t num_collisions = 0;

			for ( size_t i = 0; i < this->bucket_count(); ++i )
			{
				if ( this->bucket_size ( i ) > 1 )
					num_collisions += ( this->bucket_size ( i ) - 1 );
			}

			return ( int )num_collisions;
		}
	};
}


namespace XEPL::MAPS
{
	template<class TKey, typename TPtr=void*>
	class MapDeleteT : public MapT<TKey, TPtr>
	{
	public:
		~MapDeleteT ( void )
		{
			auto    it  = this->begin();

			while ( it != this->end() )
			{
				delete it->second;
				it = this->erase ( it );
			}
		}
	};
}


class XEPL::CELL::MethodMap : public MAPS::MapDeleteT<TEXT::Cord, CELL::Method*>
{};


class XEPL::DNA::Traits : public MEMORY::Recycler
{
public:
	DNA::Trait*                    first_trait;
	DNA::TraitMap*                 map_of_traits;

	~Traits ( void );
	 Traits ( void );

	void   Set_Trait             ( TEXT::Cord*, TEXT::Cord* );
	void   Evaluate              ( DNA::Gene*,  CELL::Nucleus* );
	void   Duplicate_Into        ( DNA::Traits** );

	void   Print_Into_Bag        ( TEXT::String*  );

	friend ATOM::Duct& operator<< ( ATOM::Duct&, DNA::Traits* );
};


namespace XEPL::MAPS
{
	template<class Tkey, typename TAtom=ATOM::Atom*>
	class MapReleaseT : public MapT<Tkey, TAtom>
	{
	public:
		~MapReleaseT ( void )
		{
			auto    it  = this->begin();

			while ( it != this->end() )
			{
				it->second->Release();
				this->erase ( it );

				it = this->begin();
			}
		}
	};
}


class XEPL::THREAD::Mutex : public MEMORY::Recycler
{
public:
	std::recursive_mutex           mutex;

	Mutex() : mutex()
	{}
};


class XEPL::TEXT::Wire : public ATOM::Atom
{
public:
	THREAD::Mutex*                 wire_lock;
	TEXT::String*                  wire_string;

protected:
	        ~Wire ( void ) override;
public:
	         Wire ( void );
	explicit Wire ( TEXT::Cord* );

	bool   Append                ( TEXT::Wire* );
	bool   Append                ( TEXT::Cord* );
	bool   Append                ( const char*, size_t );

	void   Assign                ( TEXT::Cord* );
	void   Assign                ( const char* );
	size_t Avail                 ( void  ) const;
	void   Erase                 ( void  );
	bool   Expire                ( size_t );
	void   Print                 ( TEXT::String* ) const;

	bool   Extract_Line          ( XEPL::TEXT::String* );

	friend ATOM::Duct& operator<< ( ATOM::Duct&, TEXT::Wire* );
};


class XEPL::THREAD::MutexScope : public MEMORY::Recycler
{
	THREAD::Mutex*                 mutex;
public:
	        ~MutexScope ( void )                                     { if ( mutex ) mutex->mutex.unlock(); }
	explicit MutexScope ( THREAD::Mutex* _mutex ) : mutex ( _mutex ) { if ( mutex ) mutex->mutex.lock(); }
};


#define LOCK_TRAITS    THREAD::MutexScope lock_Traits   ( traits_mutex );
#define LOCK_CONTENTS  THREAD::MutexScope lock_Contents ( content_mutex );


class XEPL::DNA::Gene : public CELL::Cell
{
public:
	TEXT::Wire*                    content_wire;
	DNA::Traits*                   traits;
	DNA::Genes*                    inner_genes;
	TEXT::String*                  space_string;
	THREAD::Mutex*                 content_mutex;
	THREAD::Mutex*                 traits_mutex;
	DNA::Gene*                     gene_owner;
	SIGNAL::Axon*                  watch_axon;
	TEXT::String*                  watch_expr;
	bool                           is_just_a_copy;
	bool                           thread_safe;

protected:
	        ~Gene ( void ) override;
public:
	explicit Gene ( const char* );
	explicit Gene ( DNA::Gene*, TEXT::Cord*, TEXT::Cord* );
	explicit Gene ( DNA::Gene*, const char*, TEXT::Cord* );
	explicit Gene ( DNA::Gene* _parent,      TEXT::Cord* _name ) : Gene ( _parent, _name, nullptr ) {}

	void Watch                     ( SIGNAL::Axon*, TEXT::String* );

	// Wired Content

	void     Content_Attach        ( TEXT::Wire* );
	bool     Content_Append        ( TEXT::Wire* );
	bool     Content_Append        ( TEXT::Cord* );
	void     Content_Assign        ( TEXT::Cord* );
	void     Content_Drop          ( void  );
	TEXT::String*    Make_Content_Wire     ( void  );
	TEXT::String*    Content       ( void  ) const;

	// Inner Gene operations

	void     Genes_Absorb          ( DNA::Gene* );
	void     Keep_Common_Genes     ( DNA::Gene* );
	void     Remove_Common_Genes   ( DNA::Gene* );

	void     Add_Gene              ( DNA::Gene* );
	void     Remove_Gene           ( DNA::Gene* );
	bool     Replace_Gene          ( DNA::Gene* );

	void     Copy_Genes_Into       ( DNA::GeneChain** ) const;
	void     Get_Gene_Chain        ( TEXT::Cord*, DNA::GeneChain** ) const;

	bool     Get_First_Gene        ( TEXT::Cord*, DNA::Gene** ) const;
	bool     Get_First_Gene        ( const char*, DNA::Gene** ) const;

	void     Make_New_Gene         ( TEXT::Cord*, DNA::Gene** );
	void     Make_New_Gene         ( const char*, DNA::Gene** );
	bool     Make_One_Gene         ( TEXT::Cord*, DNA::Gene** );
	bool     Make_One_Gene         ( const char*, DNA::Gene** );

	// My Traits

	bool           Trait_Get       ( TEXT::Cord*, TEXT::Cord**  ) const;
	TEXT::Cord*    Trait_Get       ( TEXT::Cord*, TEXT::String* ) const;
	const char*    Trait_Get       ( TEXT::Cord*, const char*   ) const;
	TEXT::Cord*    Trait_Get       ( const char*, TEXT::String* ) const;
	const char*    Trait_Default   ( const char*, const char*   ) const;
	const char*    Trait_Default   ( TEXT::Cord*, const char*   ) const;

	bool     Trait_Get_bool        ( TEXT::Cord*, bool  ) const;
	bool     Trait_Get_bool        ( const char*, bool  ) const;
	long     Trait_Get_long        ( TEXT::Cord*, long  ) const;
	long     Trait_Get_long        ( const char*, long  ) const;

	void     Trait_Set             ( TEXT::Cord*, TEXT::Cord* );
	void     Trait_Set             ( TEXT::Cord*, const char* );
	void     Trait_Set             ( const char*, TEXT::Cord* );
	void     Trait_Set             ( const char*, const char* );

	TEXT::String*  Trait_Raw       ( TEXT::Cord* );
	TEXT::String*  Trait_Raw       ( const char* );
	TEXT::String*  Trait_Tap       ( const char*, const char* );

	void     Traits_Absorb         ( DNA::Gene* );
	bool     Traits_Duplicate      ( DNA::Traits**  )    const;
	void     Traits_Evaluate       ( CELL::Nucleus* );
	void     Traits_Release        ( void  );

	// Whole Gene Operations

	void     Gene_Absorb           ( DNA::Gene* );
	void     Gene_Deflate          ( void  );
	void     Gene_Duplicate        ( DNA::Gene**  );
	bool     Gene_Print_duct       ( bool,  ATOM::Duct& );
	void     Gene_Print_rope       ( TEXT::String*, int = 0 ) const;
	void     Gene_Seal_Up          ( void  );

	void     Make_Gene_Safe        ( bool );
	void     Make_Gene_UnSafe      ( bool );

	friend ATOM::Duct& operator<< ( ATOM::Duct&, DNA::Gene* );
};


class XEPL::CELL::Nucleus : public MEMORY::Recycler
{
public:
	CORTEX::Neuron*                parent_neuron;
	CELL::Cell*                    host_cell;
	DNA::Gene*                     config_gene;
	DNA::Gene*                     report_gene;
	DNA::Gene*                     properties;
	DNA::Gene*                     chromo_gene;
	DNA::Gene*                     forms_gene;
	DNA::Gene*                     macros_gene;
	DNA::Gene*                     methods_gene;
	CELL::MethodMap*               method_map;

protected:
	virtual ~Nucleus ( void );

	explicit Nucleus ( DNA::Gene*, CELL::Cell* );

public:
	virtual CORTEX::Neuron*  Nucleus__Host_Neuron ( void ) = 0;
	virtual void Nucleus__Path           ( TEXT::String*,    TEXT::Cord* ) const = 0;
	virtual bool Nucleus__Processed_Gene ( CELL::Nucleus*,   DNA::Gene*  );
	virtual bool Nucleus__Rendered_Form  ( CORTEX::Rendron*, TEXT::Cord* );
	virtual void Nucleus__Dropped        ( void );

	void Path                    ( TEXT::String*, const char* ) const;

	// Forms
	bool Form_Get                ( TEXT::Cord*, DNA::Gene** );
	void Register_Form           ( DNA::Gene* );

	// Macros
	bool Macro_Hunt              ( TEXT::Cord*, TEXT::String* );
	bool Performed_Macro         ( TEXT::Cord*, TEXT::String*, bool&, TEXT::String* );
	void Register_Macro          ( TEXT::Cord*, TEXT::String* );
	void Register_Macro          ( const char*, TEXT::String* );

	// Methods
	void Execute_Method_Method   ( DNA::Gene*,  DNA::Gene* );
	bool Performed_Method        ( TEXT::Cord*, DNA::Gene* );
	bool Performed_Method        ( const char*, DNA::Gene* );
	void Register_Method         ( TEXT::Cord*, CELL::Function, DNA::Gene* );
	void Register_Method         ( const char*, CELL::Function, DNA::Gene* );

	// Gene Processing
	bool Process_Gene            ( DNA::Gene* );
	void Process_Inner_Genes     ( DNA::Gene* );
	bool Process_Inner_Gene      ( TEXT::Cord*, DNA::Gene* );
	bool Process_Inner_Gene      ( const char*, DNA::Gene* );

	// Properties
	bool Property_Get            ( TEXT::Cord*, TEXT::String* );
	bool Property_Get            ( const char*, TEXT::String* );
	bool Property_Hunt           ( TEXT::Cord*, TEXT::String* );
	void Property_Set            ( TEXT::Cord*, TEXT::Cord* );
	void Property_Set            ( const char*, TEXT::Cord* );
	void Property_Set            ( const char*, const char* );

	// Chromosomes
	void Register_Gene           ( TEXT::Cord*, DNA::Gene* );
	void Register_Gene           ( const char*, DNA::Gene* );
	void Make_Genes              ( void );

	// Features
	TEXT::String* Feature_Get    ( TEXT::Cord*, TEXT::String* );
	TEXT::String* Feature_Get    ( const char*, TEXT::String* );
};


class XEPL::CORTEX::Neuron : public CELL::Cell, public CELL::Nucleus
{
public:
	CORTEX::Lobe*                  host_lobe;
	SIGNAL::ReceptorChain*         receptor_chain;
	SIGNAL::AxonChain*             axon_chain;
	CORTEX::AxonMap*               axon_map;
	CORTEX::RelayMap*              relay_map;
	CORTEX::NeuronMap*             neuron_map;
	CORTEX::NeuronChain*           neuron_chain;
	TEXT::String*                  alias;
	TEXT::String*                  output;
	bool                           dropped;
	bool                           process_now;

protected:
	        ~Neuron ( void ) override;
public:
	explicit Neuron ( const char* );
	explicit Neuron ( CORTEX::Neuron*, DNA::Gene* );

	void            Nucleus__Dropped     ( void ) override;
	CORTEX::Neuron* Nucleus__Host_Neuron ( void ) override;
	void            Nucleus__Path        ( TEXT::String*, TEXT::Cord* ) const override;

	virtual void    Neuron__Dismiss      ( void );
	virtual void    Neuron__Build_Relay  ( SIGNAL::Axon*, SIGNAL::Receptor*, SIGNAL::Relay** );
	virtual void    Neuron__Drop_Relay   ( SIGNAL::Relay* );

	bool  Axon_Get               ( TEXT::Cord*,  SIGNAL::Axon**   );
	bool  Axon_Hunt              ( TEXT::Cord*,  SIGNAL::Axon**   );
	bool  Axon_Hunt              ( const char*,  SIGNAL::Axon**   );

	void  Axon_Synapse           SIGNAL_RECEIVER;
	void  Axon_Hold              ( SIGNAL::Axon* );
	void  Axon_Register          ( SIGNAL::Axon* );
	void  Axon_Release           ( SIGNAL::Axon* );
	void  Axon_Unregister        ( SIGNAL::Axon* );

	void  Drop_My_Axons          ( void );
	void  Drop_My_Neurons        ( void );
	void  Drop_My_Receptors      ( void );

	void  Drop_Neuron            ( TEXT::Cord* );

	void  Method_Terminate       NUCLEUS_METHOD;

	bool  Find_Neuron            ( TEXT::Cord*,  CORTEX::Neuron** );
	bool  Find_Neuron            ( const char*,  CORTEX::Neuron** );
	bool  Get_Neuron             ( TEXT::Cord*,  CORTEX::Neuron** );
	bool  Hunt_Neuron            ( TEXT::Cord*,  CORTEX::Neuron** );
	void  Register_Neuron        ( CORTEX::Neuron* );
	void  Unregister_Neuron      ( CORTEX::Neuron* );

	int  Show_Neurons           ( TEXT::String* );

	void  Subscribe              ( SIGNAL::Axon*, SIGNAL::Receiver, CELL::Cell* );
	void  Receptor_Build         ( SIGNAL::Axon*, SIGNAL::Receptor* );
	void  Receptor_Detach        ( SIGNAL::Receptor* );
	void  Relay_Detach           ( SIGNAL::Relay* );
	void  Undeliverable          ( CELL::Cell*,   SIGNAL::Relay* );

	static Neuron* Locator       ( CELL::Nucleus*, TEXT::String*, char );
};


namespace XEPL::COUNTERS
{
	class TlsCounters;
}


class XEPL::COUNTERS::TlsCounters
{
public:
	typedef long long Counter;

	Counter                        count_rec_news;
	Counter                        count_rec_dels;
	Counter                        count_news;
	Counter                        count_dels;
	Counter                        count_strings;
	Counter                        count_atoms;
	Counter                        count_cells;
	Counter                        count_genes;
	Counter                        count_traits;
	Counter                        count_lobes;
	Counter                        count_neurons;
	Counter                        count_rests;
	Counter                        count_actions;
	Counter                        count_wakes;
	Counter                        count_keywords_executed;
	Counter                        count_scripts;
	Counter                        count_operations;
	Counter                        count_splices;
	Counter                        count_macros;
	Counter                        count_methods_performed;
	Counter                        count_exceptions;

	void Add                     ( TlsCounters*     );
	void Report                  ( TEXT::CppString* );
	void Final_Report            ( void );
};

#define THREAD_COUNTER(counter)if (XEPL::tlsLobe) ++XEPL::tlsLobe->counters.counter ;
//#define THREAD_COUNTER(discard)


class XEPL::SIGNAL::Axon : public ATOM::Atom
{
public:
	enum AxonType { PRESS, PULSE, REPEATS, TAIL };

	TEXT::String*                  axon_name;
	CORTEX::Neuron*                host_neuron;
	SIGNAL::ReceptorChain*         receptor_chain;
	AxonType                       axon_type;

protected:
	        ~Axon ( void ) override;

public:
	explicit Axon ( CORTEX::Neuron*, DNA::Gene* );
	explicit Axon ( CORTEX::Neuron*, TEXT::Cord*, AxonType );
	explicit Axon ( CORTEX::Neuron*, const char*, AxonType );

	void Subscribe               ( CELL::Nucleus*,  DNA::Gene* );

	void Trigger                 ( ATOM::Atom* ) const;
	void Trigger_Wait            ( ATOM::Atom* ) const;

	void Cancel_Receptors        ( void );

};


namespace XEPL::CORTEX
{
	class GeneAction : public MEMORY::Recycler
	{
	public:
		XEPL::SIGNAL::Axon*        axon;
		XEPL::DNA::Gene*           gene;

		~GeneAction();
		GeneAction( SIGNAL::Axon* _axon, DNA::Gene* _gene);
	};

	typedef std::vector<GeneAction*> GeneVector;
	typedef std::map<SIGNAL::Axon*, GeneAction*> GeneActionMap;

	class GeneActions : public MEMORY::Recycler
	{
	public:
		GeneActionMap*             action_map;
		GeneVector*                gene_vector;

		~GeneActions();
		 GeneActions();

		void  Add                ( SIGNAL::Axon*, DNA::Gene* );
		void  Notify             ( void );
	};
}

class XEPL::CORTEX::Lobe : public CORTEX::Neuron
{
public:
	DNA::Gene*                     index;
	DNA::Gene*                     outdex;
	DNA::Gene*                     locals;
	DNA::Ephemerals*               ephemerals;
	ATOM::Atom*                    current_stimulus;
	CORTEX::Lobe*                  parent_lobe;
	CORTEX::Cortex*                cortex;
	SIGNAL::ActionList*            pending_actions;
	THREAD::Semaphore*             semaphore_rest;
	THREAD::Semaphore*             semaphore_birth;
	THREAD::Semaphore*             semaphore_loaded;
	THREAD::Thread*                cpp_thread;
	DNA::Scope_Terms*              short_term_stack;
	CORTEX::Rendron*               renderer;
	bool                           healthy;
	bool                           started;
	COUNTERS::TlsCounters          counters;
	XEPL::DNA::Gene*               fileData;
	GeneActions*                   gene_actions;

	        ~Lobe ( void ) override;
	explicit Lobe ( const char* );
	explicit Lobe ( CORTEX::Neuron*, DNA::Gene* );

	virtual void Lobe__Born      ( void );
	virtual void Lobe__Dying     ( void );
	virtual void Lobe__Rest_Now  ( void );
	virtual void Lobe__Wake_Up   ( void );

	void Neuron__Dismiss         ( void ) override;
	void Neuron__Build_Relay     ( SIGNAL::Axon*, SIGNAL::Receptor*, SIGNAL::Relay** ) override;
	void Neuron__Drop_Relay      ( SIGNAL::Relay* ) override;
	void Nucleus__Dropped        ( void ) override;

	void Method_Terminate        NUCLEUS_METHOD;

	bool Dispatch_Action         ( void );
	void Main_Processing_Loop    ( void );
	void Relay_Nop               ( CELL::Cell*, SIGNAL::Relay* );

	void Start_Lobe              ( bool );
	void Stop_Lobe               ( void );

	void Make_Index              ( void );
	void Make_Locals             ( void );
	void Set_Outdex              ( DNA::Gene* );

};


class XEPL::ATOM::Scope_Release
{
	ATOM::Atom*                    atom;
public:
	        ~Scope_Release ( void ) { if ( atom && --atom->retain_count < 1 ) atom->Destroy(); }
	explicit Scope_Release ( ATOM::Atom* _atom ) : atom ( _atom ) {}
};


class XEPL::SIGNAL::ActionList : public MEMORY::Recycler
{
	friend class CORTEX::Lobe;

	THREAD::Mutex*                 actions_lock;
	SIGNAL::Action*                head;
	SIGNAL::Action*                tail;
	SIGNAL::Action*                tail_actions;
	SIGNAL::Action*                tail_action_last;
	CORTEX::Lobe*                  target_lobe;
	bool                           closed;

public:
	~ActionList ( void ); 

	explicit ActionList ( CORTEX::Lobe* );

	void Press_Before_all        ( SIGNAL::Action*  );
	void Pulse_Only_Once         ( SIGNAL::Action*  );
	void Repeat_Multiple         ( SIGNAL::Action*  );
	void Tail_After              ( SIGNAL::Action*  );

	bool Get_Next_Action         ( SIGNAL::Action** );

	void Close                   ( void );
	void Flush                   ( void );
};


class XEPL::SIGNAL::Action : public MEMORY::Recycler
{
public:
	SIGNAL::Receptor*              receptor;
	ATOM::Atom*                    trigger;
	SIGNAL::Action*                action_next;

	virtual ~Action ( void );
	explicit Action ( SIGNAL::Receptor*, ATOM::Atom* );

	virtual void Action__Execute ( void ) = 0;

	bool Already_Pending         ( void );
	void Clear_Pending           ( void );
};


class XEPL::SIGNAL::Action__Drop : public SIGNAL::Action
{
	CORTEX::Neuron*                neuron;

public:
	explicit Action__Drop ( CORTEX::Neuron* );

	void Action__Execute         ( void ) override;
};


class XEPL::SIGNAL::Action__Signal : public SIGNAL::Action
{
public:
	explicit Action__Signal ( SIGNAL::Receptor*, ATOM::Atom* );

	void Action__Execute         ( void ) override;
};


class XEPL::SIGNAL::Receptor : public ATOM::Atom
{
public:
	SIGNAL::Axon*                  host_axon;
	CORTEX::Neuron*                target_neuron;
	SIGNAL::Action*                active_action;
	SIGNAL::Receiver               signal_receiver;
	ATOM::Atom*                    memento;

	        ~Receptor ( void ) override;
	explicit Receptor ( CORTEX::Neuron*, SIGNAL::Axon*, SIGNAL::Receiver, ATOM::Atom* ) ;

	virtual void Receptor__Activate ( ATOM::Atom* ) const;
	virtual void Receptor__Cancel   ( void );
};


class XEPL::SIGNAL::Signal : public ATOM::Atom
{
public:
	ATOM::Atom*                    stimulus;

	~Signal         ( void );
	explicit Signal ( ATOM::Atom* );
};


class XEPL::ATOM::Bond : public XEPL::MEMORY::Recycler
{
public:
	ATOM::Bond*                    next;
	ATOM::Bond*                    prev;
	ATOM::Atom*                    atom;

	        ~Bond ( void );
	explicit Bond ( ATOM::Atom*, ATOM::Bond* );
};


class XEPL::ATOM::Chain : public MEMORY::Recycler
{
public:
	bool                           is_my_lock;
	THREAD::Mutex*                 chain_lock;
	ATOM::Bond*                    head;
	ATOM::Bond*                    tail;

	virtual ~Chain();
	explicit Chain               ( bool );
	explicit Chain               ( THREAD::Mutex* );

	ATOM::Bond*  Add_Atom        ( ATOM::Atom*  );

	void   Remove_Bond           ( ATOM::Bond*  );
	bool   Remove_Atom           ( ATOM::Atom*  );
	bool   Pull_Atom             ( ATOM::Atom** );

	bool   Replicate_Atoms       ( bool, ATOM::Chain** ) const;
};


namespace XEPL::SOMA
{
	void xeplCantFind ( const char*, CELL::Nucleus*, const char* );
	void xeplError    ( const char*, CELL::Nucleus*, const char* );

	void OffBalance   ( void );
	void ezError      ( const char* );
}


#define LOCK_CHAIN THREAD::MutexScope lock_chain ( chain_lock );


class XEPL::SIGNAL::ReceptorChain : public ATOM::Chain
{
public:
	        ~ReceptorChain ( void ) override;
	explicit ReceptorChain ( void );

	void Deliver_Signal          ( ATOM::Atom* ) const;
	void Detach_Receptors        ( void );
};


class XEPL::SIGNAL::Relay : public SIGNAL::Receptor
{
public:
	SIGNAL::ReceptorChain*         receptor_chain;

	~Relay() override;

	explicit Relay ( CORTEX::Neuron*, SIGNAL::Axon*, SIGNAL::Receiver, SIGNAL::Receptor* );

	void Receptor__Activate      ( ATOM::Atom* ) const override;
};


class XEPL::SIGNAL::Signal__Rendezvous : public SIGNAL::Signal
{
	friend class SIGNAL::Axon;

	const CORTEX::Lobe*            lobe;
	THREAD::Semaphore*             semaphore;
	bool*                          is_dissolved;
public:
			~Signal__Rendezvous ( void );
	explicit Signal__Rendezvous ( ATOM::Atom*, bool* );
};


class XEPL::THREAD::Semaphore : public std::condition_variable, public std::mutex
{
public:
	Semaphore ( void );

	void Give                    ( void );
	void Give_As_Owner           ( void );
};


class XEPL::SIGNAL::AxonChain : public ATOM::Chain
{
public:
	~AxonChain( void ) override;
	AxonChain ( void );

	void Cancel_All_Receptors    ( void );
};


class XEPL::CORTEX::AxonLocator
{
public:
	CORTEX::Neuron*                owner_neuron;
	TEXT::String*                  axon_name;
	SIGNAL::Axon*                  axon;

	        ~AxonLocator ( void );
	explicit AxonLocator ( CORTEX::Neuron*, TEXT::Cord*, char, char );
};


class XEPL::MEMORY::MemoryBackpack
{
	HeapOfPools*                   heap;
public:
	~MemoryBackpack();
	MemoryBackpack();
};


namespace XEPL::MEMORY
{
	class MemoryCounts
	{
		std::ostream*              std__ostream;
	public:
		        ~MemoryCounts ( void );
		explicit MemoryCounts ( std::ostream* );
	};

	class RecycleCounts
	{
		std::ostream*              std__ostream;
	public:
		        ~RecycleCounts ( void );
		explicit RecycleCounts ( std::ostream* );
	};
}


namespace KITS::TIMER
{
	static
	std::string* formatMilliseconds ( long long milliseconds, std::string* result )
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

	class PerformanceTimer;
	extern PerformanceTimer* runtime;

	class PerformanceTimer
	{
		std::chrono::time_point<std::chrono::high_resolution_clock> start_;
		std::time_t                start_time;
	public:
		PerformanceTimer()
		: start_                 ( std::chrono::high_resolution_clock::now() )
		, start_time             ( std::chrono::system_clock::to_time_t ( std::chrono::system_clock::now() ) )
		{
			runtime = this;
		}

		std::string* Time( std::string* _into )
		{
			auto      end = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds> ( end - start_ );
			formatMilliseconds ( duration.count(), _into );
			return _into;
		}

		~PerformanceTimer()
		{
			auto      end = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds> ( end - start_ );

			std::string ms_report;
			std::cout << "Started at: "   << std::ctime ( &start_time );
			std::cout << "Elapsed time: " << *formatMilliseconds ( duration.count(), &ms_report ) << std::endl;
		}

	private:
	};
	
}


class XEPL::CORTEX::Cortex
{
	KITS::TIMER::PerformanceTimer  runtime;
	XEPL::MEMORY::MemoryCounts     report_memory_usage;
	XEPL::MEMORY::RecycleCounts    report_backpack_usage;
	MEMORY::MemoryBackpack         memory_backpack;
	DNA::Scope_Terms*              short_terms;

protected:
	        ~Cortex ( void );
	explicit Cortex ( const char*, std::ostream& );

public:
	CORTEX::ChromosMap*            chromos_map;
	CORTEX::KeywordsMap*           keywords_map;
	CORTEX::OperatorsMap*          operators_map;
	CORTEX::RenderMap*             render_map;
	CORTEX::RenkeysMap*            renkeys_map;
	CORTEX::SplicerMap*            splicer_map;

	CORTEX::Lobe*                  host_lobe;
	DNA::Gene*                     system_gene;
	ATOM::Duct*                    output_duct;
	COUNTERS::TlsCounters          final_counters;


	virtual void Cortex__Initialize ( void );
	virtual bool Cortex__Can_Execute_Cmd    ( const char* );
	virtual void Cortex__Shutdown   ( void );

	virtual bool Cortex__Can_Process_Tag ( TEXT::Cord*, CELL::Nucleus*, DNA::Gene* ) { return false; }

	void  Register_Keyword       ( const char*, CORTEX::Keyword );
	void  Register_Operator      ( const char*, CORTEX::Operator );
	void  Register_Splicer       ( const char*, DNA::Splicer );
	void  Register_Render        ( TEXT::Cord*, Render );
	void  Register_Render        ( const char*, Render );
	void  Enable_Renkey          ( const char* );

	bool  Executes_Keyword       ( TEXT::Cord*, CELL::Nucleus*, DNA::Gene* );
	bool  Did_Operation          ( TEXT::Cord*, TEXT::String*,  TEXT::String*,  bool& );
	bool  Did_Splice             ( TEXT::Cord*, TEXT::String*,  CELL::Nucleus*, DNA::Gene*&, TEXT::String*, bool* );
	bool  Did_Render             ( TEXT::Cord*, CELL::Nucleus*, DNA::Gene*,     CORTEX::Rendron* );

	void  Register_Gene          ( TEXT::Cord*, DNA::Chromosome );
	void  Register_Gene          ( const char*, DNA::Chromosome );

	void  Execute_Dna            ( DNA::Gene* );
	bool  Execute_Rna            ( TEXT::Cord* );
	bool  Execute_Rna            ( const char* );
	bool  Execute_Xml            ( const char* );
};


namespace KITS::FILE
{
	bool  File_Into_Code         ( const char*,        XEPL::DNA::Gene** );
	bool  File_Load_Gene         ( XEPL::TEXT::Cord*,  XEPL::DNA::Gene** );
	bool  File_Load_String       ( XEPL::TEXT::Cord*,  std::string&  );
	bool  File_Load_String       ( XEPL::TEXT::Cord*,  XEPL::TEXT::String* );
	bool  File_Locate            ( XEPL::TEXT::Cord*,  XEPL::TEXT::String* );
	void  File_Print_To_Console  ( XEPL::TEXT::Cord* );
	bool  String_Into_File       ( XEPL::TEXT::Cord*,  XEPL::TEXT::Cord* );
	bool  Path_To                ( XEPL::TEXT::Cord*,  XEPL::TEXT::Cord** );
	bool  Path_To                ( const char*,        XEPL::TEXT::Cord** );
	void  Set_File_Path          ( const char*, const char* );
}


class XEPL::ATOM::Duct : public XEPL::MEMORY::Recycler, public std::ostream
{
	const int                      indent;
	int                            num_spaces;

public:
	explicit Duct ( std::ostream&, int );

	static void Indent           ( Duct& );
	static void Undent           ( Duct& );
	static void Dent             ( Duct& );
};


namespace XEPL::DNA
{
	class Ephemerals : public MAPS::MapReleaseT<TEXT::Cord, DNA::Gene* >
	{
	public:
		void Set ( TEXT::Cord* _name, DNA::Gene* _gene );
	};
}


class XEPL::DNA::Scope_Duplicate
{
public:
	DNA::Gene*                     gene;

	~Scope_Duplicate()
	{
		if ( gene && --gene->retain_count < 1 )
			gene->Destroy();
	}

	explicit Scope_Duplicate ( DNA::Gene* _gene )
		: gene ( _gene )
	{
		if ( _gene )
		{
			if ( _gene->traits )
				_gene->Gene_Duplicate ( &gene );
			else
				++_gene->retain_count;
		}
	}
};


class XEPL::DNA::Scope_Gene
{
	DNA::Gene*&                    gene_reference;
	DNA::Gene*                     previous_gene;

public:
	Scope_Gene ( DNA::Gene*& _gene, DNA::Gene* _replacement )
		: gene_reference ( _gene )
		, previous_gene  ( gene_reference )
	{
		gene_reference = _replacement;
	}

	~Scope_Gene ( void )
	{
		gene_reference = previous_gene;
	}
};


class XEPL::DNA::Scope_Terms
{
	Scope_Terms*&                  restore_nest;
	Scope_Terms*                   previous_nest;
	Scope_Terms*                   hot_nest;
	TermMap*                       terms;
	bool                           pushed;

public:
	~Scope_Terms ( void );
	Scope_Terms  ( void );

	explicit Scope_Terms         ( DNA::Gene* );
	explicit Scope_Terms         ( TEXT::Cord* );
	explicit Scope_Terms         ( const char*, TEXT::String* );

	void Nest_From_Str           ( TEXT::Cord* );
	void Nest_Gene_Terms         ( DNA::Gene* );

	bool  Get                    ( TEXT::Cord*, TEXT::String* ) const;
	void  Set                    ( TEXT::Cord*, TEXT::Cord* );
	void  Set                    ( const char*, TEXT::Cord* );

	void  Report                 ( DNA::Gene* );
};


class XEPL::TEXT::Parser : public MEMORY::Recycler
{
public:
	TEXT::ParserBag*               parser_bag;
	TEXT::String*                  error_string;

protected:
	~Parser( void );

public:
	Parser ( void );

	bool  Empty                  ( void ) const;

	void Record_Error            ( const char*, const char*   );
	void Record_Error            ( const char*, TEXT::String* );

};


class XEPL::RNA::RnaScript : public TEXT::Parser
{
	friend class RnaBag;
	friend class ScriptTagValue;

	CORTEX::Neuron*                neuron;
	RNA::RnaBag*                   rna_bag;
	TEXT::String*                  value;
	DNA::Gene*                     gene;
	bool                           truth;

	void Do_Operation            ( void );
	void Do_Splice               ( void );

public:

	explicit RnaScript ( CELL::Nucleus*, DNA::Gene* );
	explicit RnaScript ( CELL::Nucleus*, DNA::Gene*, TEXT::Cord*, TEXT::String*, bool*, TEXT::Cord*, bool = true );

};

namespace XEPL::RNA
{
	void  Evaluate_Script ( CELL::Nucleus*,  DNA::Gene*, TEXT::Cord*, TEXT::String* );
}


class XEPL::RNA::Script_Assign : public RNA::RnaScript
{
public:
	Script_Assign ( CELL::Nucleus*  _nucleus, DNA::Gene* _gene, TEXT::Cord* _expr, TEXT::String* _result )
		: RNA::RnaScript ( _nucleus, _gene, _expr, _result, nullptr, nullptr, false )
	{}
};

class XEPL::RNA::Script_Append : public RNA::RnaScript
{
public:
	Script_Append ( CELL::Nucleus* _nucleus, DNA::Gene* _gene, TEXT::Cord* _expr, TEXT::String* _result )
		: RNA::RnaScript ( _nucleus, _gene, _expr, _result, nullptr, nullptr, true )
	{}
};

class XEPL::RNA::Script_Truth : public RNA::RnaScript
{
public:
	Script_Truth ( CELL::Nucleus* _nucleus, DNA::Gene* _gene, TEXT::Cord* _expr, bool* _truth )
		: RNA::RnaScript ( _nucleus, _gene, _expr, nullptr, _truth, nullptr, false )
	{}
};


class XEPL::SOMA::ErrorReport : public TEXT::String
{
public:
	static bool                    capture;
	bool                           report;

	~ErrorReport         ( void );
	explicit ErrorReport ( const char* _text );
	explicit ErrorReport ( TEXT::Cord* _text );

	TEXT::String* operator ->();

	ErrorReport& operator+       ( const char* _txt );
	ErrorReport& operator+       ( TEXT::String* _str );

	static void finalReport      ( TEXT::String* into );
	static void Flush            ( void );
};


class XEPL::XML::XmlParser : public TEXT::Parser
{
public:
	XML::XmlBag*                   xml_bag;
	XML::XmlNode*                  active_node;
	XML::XmlIntsructionMap*        instructions;
	DNA::Gene*                     root_gene;

	virtual ~XmlParser ( void );
	explicit XmlParser ( TEXT::Cord*, bool = false );

	bool do_BeginNode            ( void );
	bool do_CData                ( void );
	bool do_CloseNode            ( void );
	bool do_CloseSplit           ( void );
	bool do_Comment              ( void );
	bool do_Content              ( void );
	bool do_DocType              ( void );
	bool do_Instruction          ( void );
	bool do_NodeSplit            ( void );
	void ParseIt                 ( void );
	void Perform_Instruction     ( TEXT::Cord* );
	void Register_Instruction    ( TEXT::Cord*, XML::xmlInstruction );

	virtual void Xml__DocType       ( TEXT::String* ) {}
	virtual bool Xml__New_Comment   ( TEXT::String* ) { return true; }
	virtual bool Xml__New_Element   ( DNA::Gene*    ) { return true; }
	virtual bool Xml__End_Element   ( DNA::Gene*    ) { return true; }
	virtual bool Xml__New_Content   ( TEXT::String* ) { return true; }
	virtual bool Xml__New_Attribute ( TEXT::String*, TEXT::String*, char ) { return true; }

	virtual void Xml__Make_Element  ( DNA::Gene*, TEXT::Cord*, TEXT::Cord*, DNA::Gene** );
	virtual void Xml__Undo_Element  ( DNA::Gene*   );
};


class XEPL::CORTEX::RenderMap    : public MAPS::MapT <TEXT::Cord, Render >          {};
class XEPL::CORTEX::RenkeysMap   : public MAPS::MapT <TEXT::Cord, bool >            {};
class XEPL::CORTEX::ChromosMap   : public MAPS::UMapT<TEXT::Cord, DNA::Chromosome>  {};
class XEPL::CORTEX::SplicerMap   : public MAPS::UMapT<TEXT::Cord, DNA::Splicer>     {};

class XEPL::CORTEX::KeywordsMap  : public MAPS::UMapT<TEXT::Cord, XEPL::CORTEX::Keyword>
{
public:
	KeywordsMap()
	{
		max_load_factor ( .25 );
	}
	~KeywordsMap()
	{
		if ( XEPL::Show_Collisions && Collisions() )
		{
			std::cout << "Keyword Collisions " << Collisions() << " of " << this->size();
			std::cout << " Load " << this->load_factor() << " of " << this->max_load_factor() << std::endl;
		}
	}
};

class XEPL::CORTEX::OperatorsMap : public MAPS::UMapT<TEXT::Cord, XEPL::CORTEX::Operator>
{
public:
	OperatorsMap()
	{
		max_load_factor ( .8 );
	}
	~OperatorsMap()
	{
		if ( XEPL::Show_Collisions && Collisions() )
		{
			std::cout << "Operator Collisions " << Collisions() << " of " << this->size();
			std::cout << " Load " << this->load_factor() << " of " << this->max_load_factor() << std::endl;
		}
	}
};


class XEPL::CORTEX::Scope_Render
{
	CORTEX::Rendron*&              r_renderer;
	CORTEX::Rendron*               was_renderer;

public:
	        ~Scope_Render ( void );
	explicit Scope_Render ( CORTEX::Rendron* );
};


class XEPL::CORTEX::Rendron : public CELL::Cell, public CELL::Nucleus
{
	CORTEX::Scope_Render           push_renderer;
public:
	TEXT::String*                  rendition;

protected:
	explicit Rendron ( CORTEX::Neuron*, DNA::Gene*, TEXT::String* );

public:
	CORTEX::Neuron* Nucleus__Host_Neuron    ( void ) override;
	bool            Nucleus__Processed_Gene ( CELL::Nucleus*, DNA::Gene*  ) override;
	void            Nucleus__Path           ( TEXT::String*,  TEXT::Cord* ) const override;

	virtual void Rendron__Method_Markup     ( DNA::Gene*,     DNA::Gene* ) = 0;
	virtual void Rendron__Render_Gene       ( CELL::Nucleus*, DNA::Gene* );

	void  Markup                 ( CELL::Nucleus*, DNA::Gene*, TEXT::String* );
	void  Payload                ( CELL::Nucleus*, DNA::Gene* );
};

namespace XEPL::RENDER
{
	void Normal                    CORTEX_RENDER;
	void Script                    CORTEX_RENDER;
	void Scribble                  CORTEX_RENDER;
	void Jsonl                     CORTEX_RENDER;
	void Xml                       CORTEX_RENDER;
}


class XEPL::DNA::ChainMap : public MAPS::MapDeleteT<TEXT::Cord, ATOM::Chain*>
{};


class XEPL::DNA::GeneChain : public ATOM::Chain
{
	DNA::Gene*                     current_gene;

public:
	        ~GeneChain ( void ) override;
	explicit GeneChain ( const ATOM::Chain* );

	bool Next                    ( DNA::Gene** );
};


class XEPL::DNA::Genes : public DNA::GeneChain
{
	friend class DNA::Gene;

private:
	DNA::BondMap*                  bond_map;
	DNA::ChainMap*                 chain_map;

public:
	~Genes( void ) override;
	Genes ( void );

	bool   Find_Gene             ( TEXT::Cord*, DNA::Gene**   ) const;
	bool   Clone_Tree            ( TEXT::Cord*, ATOM::Chain** ) const;

	void   Add_Gene              ( DNA::Gene* );
	bool   Has_Gene              ( DNA::Gene* ) const;

	void   Remove_Bond           ( ATOM::Bond* );

	void   Flush                 ( void );

	void   Morph_Safe            ( void );
	void   Morph_Unsafe          ( void );

	void   Print_Into_Bag        ( TEXT::String*, int ) const;

	friend ATOM::Duct& operator<<( ATOM::Duct& lhs, DNA::Genes* rhs );
};


class XEPL::DNA::StableRecall
{
	friend class DNA::Gene;

	DNA::GeneChain*                stable_genes;
	DNA::Traits*                   stable_traits;
	DNA::Trait*                    current_trait;

public:
	        ~StableRecall ( void );
	explicit StableRecall ( DNA::Gene*,   bool );
	explicit StableRecall ( DNA::Gene*,   TEXT::Cord* );

	bool  Next_Trait             ( const char**, const char**   );
	bool  Next_Trait             ( TEXT::Cord**, TEXT::String** );
	bool  Next_Gene              ( DNA::Gene** );
	bool  Has_Genes              ( void ) const;
};


class XEPL::DNA::Trait : public MEMORY::Recycler
{
public:
	TEXT::Cord*                    trait_name;
	TEXT::String*                  trait_term;
	DNA::Trait*                    next_trait;

	        ~Trait ( void );
	explicit Trait ( TEXT::Cord*, TEXT::Cord*, DNA::Trait* );
	explicit Trait ( DNA::Trait*,  DNA::Trait* );

	void Print_Into_Bag          ( TEXT::String* );

	friend ATOM::Duct& operator<< ( ATOM::Duct&, DNA::Trait* );
};


class XEPL::DNA::TraitMap : public MAPS::UMapT<TEXT::Cord, DNA::Trait*>
{};


class XEPL::THREAD::MutexResource
{
	THREAD::Mutex**                mutex_at;

public:
	explicit MutexResource ( THREAD::Mutex** );

private:
	void Consolidate             ( THREAD::Mutex* );
};


class XEPL::XML::XmlBuilder : public MEMORY::Recycler
{
	TEXT::String*                  tag_n_space;
	TEXT::String*                  build_string;
	bool                           attributes_closed;

public:
	~XmlBuilder ( void );

	explicit XmlBuilder ( const char*, const char*, TEXT::String* );
	explicit XmlBuilder ( const char*, TEXT::String* );
	explicit XmlBuilder ( TEXT::Cord*, TEXT::String*, TEXT::Cord* );

	TEXT::String* Close_Traits   ( void );

	void  Absorb_Attributes      ( CELL::Nucleus*, DNA::Gene* );
	void  Attribute_Set          ( TEXT::Cord*,    TEXT::Cord* );
	void  Attribute_Set          ( const char*,    const char* );
};


namespace XEPL::DNA
{
	class BondMap : public MAPS::UMapT<ATOM::Atom*, ATOM::Bond*>
	{};
}


template<class T>
class XEPL::MEMORY::Scope_Delete
{
public:
	T*                             ptr;

	~Scope_Delete()
	{
		delete ptr;
	}

	explicit Scope_Delete ( T* _ptr )
		: ptr ( _ptr )
	{}

	Scope_Delete ()
		: ptr ( nullptr )
	{}
};


class XEPL::SIGNAL::Synapse : public SIGNAL::Relay
{
	SIGNAL::ActionList*            action_list;
	SIGNAL::DeliverAction          delivery_function;

public:
	        ~Synapse (void) override;
	explicit Synapse ( CORTEX::Lobe*, SIGNAL::Axon*, SIGNAL::Receiver, SIGNAL::Receptor* );

	void Receptor__Activate      ( ATOM::Atom* ) const override;
};


class XEPL::THREAD::Thread
{
	friend class CORTEX::Lobe;

	std::thread*                   std__thread;
	CORTEX::Lobe*                  lobe;
	THREAD::Semaphore*             semaphore_rest;

	explicit Thread ( CORTEX::Lobe*, THREAD::Semaphore* );

	void Conception              ( void );
	void Burial                  ( void );
};


namespace KITS::FILE
{
	class CodeFile : public XEPL::TEXT::String
	{
	public:
		explicit CodeFile ( XEPL::TEXT::Cord* );
	};
}


namespace XEPL::MEMORY::CONSTANTS
{
	const long poolWidth    = 16;
	const long maxPoolIndex =  5;
}


namespace XEPL::MEMORY::COUNTERS
{
	extern std::atomic_size_t        needed[CONSTANTS::maxPoolIndex+1];
	extern std::atomic_size_t        cached[CONSTANTS::maxPoolIndex+1];
	extern std::atomic_size_t      discards[CONSTANTS::maxPoolIndex+1];
	extern std::atomic_size_t       holding[CONSTANTS::maxPoolIndex+1];
	
	extern std::atomic_size_t      finalCount_needed;
	extern std::atomic_size_t      finalCount_discard;
	extern std::atomic_size_t      finalCount_holding;
	extern std::atomic_size_t      finalCount_cached;
	extern std::atomic_size_t      finalCount_biggies;
	extern std::atomic_size_t      finalCount_largest;

	extern std::atomic_size_t      num_total_news;
	extern std::atomic_size_t      num_total_dels;
}


union XEPL::MEMORY::BlockHeader
{
	BlockHeader*                   next_block;
	long                           pool_index;
};


class XEPL::MEMORY::PoolOfBlocks
{
	friend class HeapOfPools;

	BlockHeader*                   head_block;
	const long                     pool_index;
	const size_t                   block_size;
	size_t                         mallocs_needed;
	size_t                         total_cached;
	size_t                         discarded_returns;
	size_t                         blocks_now_holding;

	~PoolOfBlocks( void )
	{
		COUNTERS::finalCount_needed      += mallocs_needed;
		COUNTERS::finalCount_cached      += total_cached;
		COUNTERS::finalCount_discard     += discarded_returns;
		COUNTERS::finalCount_holding     += blocks_now_holding;

		COUNTERS::needed  [ pool_index ] += mallocs_needed;
		COUNTERS::cached  [ pool_index ] += total_cached;
		COUNTERS::discards[ pool_index ] += discarded_returns;
		COUNTERS::holding [ pool_index ] += blocks_now_holding;

		while ( head_block )
		{
			BlockHeader* block=head_block;
			head_block=block->next_block;
			free ( block );
		}
	}

	explicit PoolOfBlocks ( long _index )
		: head_block         ( nullptr )
		, pool_index         ( _index )
		, block_size         ( ( _index+1 )*CONSTANTS::poolWidth )
		, mallocs_needed     ( 0 )
		, total_cached       ( 0 )
		, discarded_returns  ( 0 )
		, blocks_now_holding ( 0 )
	{}

	void Return_Block ( BlockHeader* _block )
	{
		if ( blocks_now_holding > total_cached || blocks_now_holding > mallocs_needed )
		{
			++discarded_returns;
			free ( _block );
		}
		else
		{
			++blocks_now_holding;
			_block->next_block = head_block;
			head_block = _block;
		}
	}

	void* Get_User_Ptr( void )
	{
		BlockHeader* block = head_block;

		if ( block )
		{
			--blocks_now_holding;
			++total_cached;
			head_block = block->next_block;
		}
		else
		{
			++mallocs_needed;
			block = static_cast<BlockHeader*> ( malloc ( block_size ) );
		}

		block->pool_index = pool_index;

		return block+1;
	}
};


class XEPL::MEMORY::HeapOfPools
{
	PoolOfBlocks*                  pools[CONSTANTS::maxPoolIndex+1];
	unsigned long                  count_to_big_to_pool;
	size_t                         largest_block_requested;

public:
	~HeapOfPools( void );
	 HeapOfPools( void );

	void* Get_Size ( size_t _size )
	{
		THREAD_COUNTER ( count_rec_news )

		auto block_id = _size/CONSTANTS::poolWidth;

		if ( block_id <= CONSTANTS::maxPoolIndex )
		{
			if ( !block_id )
				block_id = 1;

			return pools[block_id]->Get_User_Ptr();
		}

		++count_to_big_to_pool;

		if ( largest_block_requested < _size )
			largest_block_requested = _size;

		const size_t block_size = _size + sizeof ( BlockHeader );

		BlockHeader* block = static_cast<BlockHeader*> ( malloc ( block_size ) );
		block->pool_index = 0;
		return block + 1;
	}

	void  Captures_ptr ( void* _ptr )
	{
		if ( !_ptr )
			return;

		THREAD_COUNTER ( count_rec_dels )

		BlockHeader* block = static_cast<BlockHeader*> ( _ptr )-1;

		if ( block->pool_index )
			pools[block->pool_index]->Return_Block ( block );
		else
			free ( block );
	}
};

namespace XEPL::MEMORY
{
	extern thread_local class HeapOfPools* tlsHeap;
}

namespace XEPL::MEMORY
{
	void RecycleCounts_Show_Entry( std::ostream*, const char*, long, std::atomic_size_t*, int );
	void RecycleCounts_Reset     ( void );
	int  RecycleCounts_Report    ( std::ostream* );
}


class XEPL::CELL::Method : public MEMORY::Recycler
{
public:
	CELL::Function                 cell_function;
	DNA::Gene*                     soft_method;

	        ~Method ( void );
	explicit Method ( CELL::Function, TEXT::Cord*, DNA::Gene* );
	
	void Perform                 ( CELL::Nucleus*, DNA::Gene* );
};


class XEPL::DNA::Scope_Index
{
	DNA::Gene*&                    previous_reference;
	DNA::Gene*                     previous_index;

public:
	explicit Scope_Index ( DNA::Gene* _replacement )
		: previous_reference ( tlsLobe->index )
		, previous_index     ( previous_reference )
	{
		previous_reference = _replacement;
	}

	~Scope_Index ( void )
	{
		previous_reference = previous_index;
	}
};


namespace XEPL::CORTEX
{
	class NeuronMap : public MAPS::MapReleaseT<TEXT::Cord, CORTEX::Neuron*> {};
	class AxonMap   : public MAPS::MapReleaseT<TEXT::Cord, SIGNAL::Axon*>  {};
	class RelayMap  : public MAPS::UMapT<SIGNAL::Axon*,    SIGNAL::Relay*>  {};

	class NeuronChain : public ATOM::Chain
	{
	public:
		NeuronChain( )
			: ATOM::Chain ( false )
		{}

		XEPL::CORTEX::Neuron* Last ( void ) const
		{
			if ( tail )
				return static_cast<CORTEX::Neuron*> ( tail->atom );

			return nullptr;
		}
	};

}


class XEPL::TEXT::ParserBag : public MEMORY::Recycler
{
protected:
	const char*                    start_of_data;
	const char*                    end_of_data;
	TEXT::Parser*                  parser_host;
public:
	const char*                    current_position;
	char                           tip_char;

protected:
	virtual ~ParserBag() {}
	explicit ParserBag ( TEXT::Cord*, TEXT::Parser* );

public:
	long Remaining               ( void ) const;
	void Rewrite                 ( char _as );

	void Next                    ( void  );
	bool Nextt                   ( void  );

	void Nudge                   ( int _offset );
	bool Nudget                  ( int _offset );

	void Skip_Whitespace         ( void );

	bool Consume                 ( char _c1 );
	bool Consume                 ( char, char );
	bool Consume                 ( char, char, char );
	bool Consume                 ( char, char, char, char );
	bool Discard_Char            ( char );
	void Parse_Error             ( const char* );
};


struct XEPL::TEXT::ParserChoice
{
	TEXT::ParserOption             parser_option;
	int                            parser_flags;
};


class XEPL::TEXT::ParserChoices : public MEMORY::Recycler
{
	friend class TEXT::ParserSelect;

	static const long              MAX_CHOICES=7;
	ParserChoice                   choices[MAX_CHOICES];

	ParserChoices();
};


class XEPL::TEXT::ParserSelect
{
	TEXT::Parser*                  parser;
	TEXT::ParserChoices*           parser_choices;
	int                            number_of_options;

public:
	const static int               No_Options = 0;
	const static int               Completes  = 1<<0;
	const static int               Can_Repeat = 1<<1;

	        ~ParserSelect ( void );
	explicit ParserSelect ( TEXT::Parser* );

	void Add_Option              ( int, TEXT::ParserOption );
};


class XEPL::XML::XmlNode : public MEMORY::Recycler
{
public:
	XML::XmlParser*                 xml_parser;
	XML::XmlNode*            parent_node;
	DNA::Gene*                     element_gene;
	bool                           parser_wants_it;

	~XmlNode();
	explicit XmlNode ( XML::XmlParser* );
};

class XEPL::XML::XmlBag : public TEXT::ParserBag
{
public:
	XML::XmlParser* xml_parser;

	explicit XmlBag ( TEXT::Cord*, XML::XmlParser* );

	void  Discard_Shell_Directive ( void );
	char  Extract_Quoted_Value    ( TEXT::String* );
	void  Extract_Attribute_Name  ( TEXT::String* );
	void  Extract_CData           ( TEXT::String* );
	void  Extract_Comment         ( TEXT::String* );
	void  Extract_DocType         ( TEXT::String* );
	void  Extract_Instruction     ( TEXT::String* );
	void  Extract_PCData          ( TEXT::String* );

	TEXT::Cord* Extract_Space_Tag ( TEXT::String* );

	bool  in_xmlTag ( const char _char ) // allow tags with '+' too.
	{
		return iswalnum ( _char ) || // allow numbers to start tags.
		       _char == ':'      ||
		       _char == '_'      ||
		       _char == '.'      ||
		       _char == '+'      ||
		       _char == '-';
	}
	bool at_xmlPCData ( void )
	{
		return tip_char && tip_char != '<';
	}

	bool in_xmlPCData ( const char _char )
	{
		return _char && _char != '<';
	}

	bool at_xmlTag ( void )
	{
		return iswalnum ( tip_char ) || // XEPL+- allow numbers to start tags.
		       tip_char == '_'      ||
		       tip_char == ':';
	}
};

namespace XEPL::XML
{
	class XmlIntsructionMap : public MAPS::MapT<TEXT::Cord, XML::xmlInstruction> {};
}


class XEPL::XML::PrintXml : public XML::XmlParser
{
	int                            depth;
	int                            skip;
	bool                           returned;
	bool                           had_content;

public:
	~PrintXml( void );
	explicit PrintXml ( TEXT::String* );

	bool Xml__New_Attribute      ( TEXT::String*, TEXT::String*, char ) override;
	bool Xml__New_Comment        ( TEXT::String*  ) override;
	bool Xml__New_Content        ( TEXT::String*  ) override;
	bool Xml__New_Element        ( DNA::Gene*     ) override;
	bool Xml__End_Element        ( DNA::Gene*     ) override;
};


class XEPL::RNA::RnaBag : public TEXT::ParserBag
{
public:
	RNA::RnaScript*                   script;

	explicit RnaBag ( TEXT::Cord*, RNA::RnaScript* );

	bool  Extract_Value           ( void  );
	bool  Get_Next_Value          ( void  );
	bool  Hunt_Property           ( void  );
	bool  Process_Parameter       ( TEXT::String** );
	void  Pull_Index              ( void  );
	void  Pull_Or_Set_Local       ( void  );
	bool  Pull_Number             ( void  );
	void  Pull_Or_Set_Property    ( void  );
	void  Pull_Short_Term         ( void  );
	bool  Revalue_Tag             ( TEXT::String* );
	void  Change_Variable         ( void  );
	void  Declare_Content         ( void  );
	void  Enter_Child_Gene        ( void  );
	void  Enter_Inner_Block       ( void  );
	void  Enter_Neuron            ( void  );
	void  Extract_Neuron_Attribute( void  );
	void  Extract_Next_Value      ( void  );
	void  Extract_Neuron_Property ( void  );
	void  Extract_Neuron_Feature  ( void  );
	void  Extract_Tag             ( TEXT::String* );
	void  Process_Gene            ( DNA::Gene*& );
	void  Process_Gene2           ( DNA::Gene*& );
	void  Pull_String             ( void  );
	void  Select_Attribute        ( void  );
	void  Serialize               ( void  );
	void  Ternary_Choice          ( void  );
	void  Mutate_Value            ( void  );
};


namespace XEPL::RNA
{
	class ScriptTagValue : public TEXT::String
	{
	public:
		TEXT::String* term = nullptr;

		explicit ScriptTagValue ( RNA::RnaBag* _bag )
		{
			_bag->Extract_Tag ( this );

			if ( _bag->tip_char == '=' )
			{
				_bag->tip_char = *++_bag->current_position;
				_bag->Get_Next_Value();
				term = _bag->script->value;
			}
		}
	};
}


class XEPL::DNA::TermMap : public XEPL::MAPS::UMapT<XEPL::TEXT::Cord, XEPL::TEXT::String> {};


class XEPL::CORTEX::Sensor : public CORTEX::Neuron
{
protected:
	TEXT::Wire*                    sensor_wire;
	bool                           sensor_is_closed;

	~Sensor ( void ) override;

public:
	explicit Sensor              ( CORTEX::Neuron*, DNA::Gene* );

	void Method_Sensor_Received              NUCLEUS_METHOD;
	void Method_Sensor_Closed           NUCLEUS_METHOD;

	virtual void Sensor__Closed  ( void ) = 0;
	virtual void Sensor__Scan    ( void ) = 0;
};


class XEPL::XML::XeplXml : public XML::XmlParser
{
public:
	explicit XeplXml ( TEXT::String* );

	bool Xml__New_Element        ( DNA::Gene* ) override;
};


namespace XEPL::OS
{
	class HandlerMap;
	class osFd;
	class osFdPair;
	class osFdSelect;
	class osFdSet;
	class osSocket;
	class osSocketAddress;
	class osSocketControl;
	class osTcpClient;
	class osTcpServer;
	class osTcpSocket;
	class osTimer_list;
	
	typedef bool ( osFd::*fdHandler ) ( void );

	void  OS_Initialize  ( void );
	int   OS_Shutdown    ( void );
}


namespace XEPL
{
	class CommandsMap;
	class OrganMap;
	class SpineMap;
	class Brain;

	typedef void ( *Command  ) ( XEPL::Brain*, XEPL::CORTEX::Lobe*, XEPL::TEXT::String*, XEPL::TEXT::String* );

	void  Register_Std_Brain_Words ( XEPL::CORTEX::Cortex* );
	void  Register_Std_Genes       ( XEPL::CORTEX::Cortex* );
	void  Register_Std_Keywords    ( XEPL::CORTEX::Cortex* );
	void  Register_Std_Renkeys     ( XEPL::CORTEX::Cortex* );
	void  Register_Std_Operators   ( XEPL::CORTEX::Cortex* );
	void  Register_Std_Splicers    ( XEPL::CORTEX::Cortex* );
	void  Register_Std_Neurons     ( XEPL::CORTEX::Cortex* );
}


namespace KITS::HTTP
{
	class Http;
	class HttpServer;
	class HttpClient;
	class HttpHeadersMap;

	typedef bool ( Http::*HttpHeader )  ( XEPL::TEXT::String* );
	typedef void ( Http::*HttpScanner ) ( void  );
}


namespace KITS::SOCKET
{
	class Socket;
	class SocketMan;
	class SocketControl;
	class SocketTcp;
	class TcpClient;
	class TcpServer;
}


#define BRAIN_REGISTRATION                           \
( XEPL::Brain* _brain, const char* _as )

#define BRAIN_COMMAND                                \
  ([[maybe_unused]] XEPL::Brain * _cortex,           \
   [[maybe_unused]] XEPL::CORTEX::Lobe * _lobe,      \
   [[maybe_unused]] XEPL::TEXT::String * _cmd,       \
   [[maybe_unused]] XEPL::TEXT::String * _opt)

class XEPL::Brain : public XEPL::CORTEX::Cortex
{
	XEPL::OrganMap*               organ_map;
	XEPL::SpineMap*               spine_map;
	XEPL::CommandsMap*            commands_map;
public:
	~Brain();
	Brain ( const char*, bool standard = true );

	void  Register_Command       ( const char*,        XEPL::Command );
	void  Register_Axon          ( const char*,        XEPL::SIGNAL::Axon* );
	void  Register_Organ         ( const char*,        XEPL::CORTEX::Neuron* );

	bool  Executes_Command       ( const char* );

	bool  Has_Axon               ( XEPL::TEXT::Cord*,  XEPL::SIGNAL::Axon** );
	bool  Has_Organ              ( XEPL::TEXT::Cord*,  XEPL::CORTEX::Neuron** );

	bool  Upload_Neuron          ( const char* );
	bool  Include_File           ( const char* );

	bool  Cortex__Can_Execute_Cmd( const char* ) override;
	bool  Cortex__Can_Process_Tag( TEXT::Cord*, XEPL::CELL::Nucleus*, XEPL::DNA::Gene* ) override;

	void  Come_Alive             ( void  );
	
	int   CliLoop                ( void  );
	int   CliLoop3               ( void  );

	void  Drop_Neuron            ( XEPL::TEXT::String* );
	bool  Drop_Neuron            ( const char* );
};


namespace KITS::HTTP
{
	class Http : public XEPL::CORTEX::Sensor
	{
	private:
		static HttpHeadersMap*     httpHeaderMap;

	protected:
		HttpScanner                parse_action;
		XEPL::DNA::Gene*           message_gene;
		XEPL::TEXT::String*        version_string;
		XEPL::SIGNAL::Axon*        request_in_axon;
		XEPL::SIGNAL::Axon*        request_out_axon;
		XEPL::SIGNAL::Axon*        response_in_axon;
		XEPL::SIGNAL::Axon*        response_out_axon;
		XEPL::SIGNAL::Axon*        http_closed_axon;
		size_t                     payload_length;
		bool                       finished_bool;
		bool                       dropped_bool;

		~Http ( void ) override;
	public:
		explicit Http ( XEPL::CORTEX::Neuron*, XEPL::DNA::Gene* );

		bool Connection            ( XEPL::TEXT::Cord* );
		bool Content_Length        ( XEPL::TEXT::Cord* );
		bool Content_Type          ( XEPL::TEXT::Cord* );
		void Define_Header         ( XEPL::TEXT::Cord* );
		void Do_Next               ( KITS::HTTP::HttpScanner );
		void Header                ( XEPL::TEXT::Cord*, XEPL::TEXT::String* );
		void Method_Deliver        ( XEPL::DNA::Gene*,  XEPL::DNA::Gene* );
		void Scan                  ( KITS::HTTP::HttpScanner );
		void Scan_Header           ( void );
		void Scan_Payload          ( void );

		void Nucleus__Dropped      ( void ) override;
		void Sensor__Closed        ( void ) override;
		void Sensor__Scan          ( void ) override;

		virtual void Http__Deliver ( void ) = 0;

		static void Initialize     ( void );
		static void Shutdown       ( void );

	protected:
		static void Register_Header_Field ( const char*, HttpHeader );
	};

}


namespace KITS::HTTP
{
	class HttpClient : public KITS::HTTP::Http
	{
	protected:
		~HttpClient() override;

	public:
		explicit HttpClient ( XEPL::CORTEX::Neuron*, XEPL::DNA::Gene* );

		void Http__Deliver       ( void  ) override;

		void Build_Request       ( XEPL::DNA::Gene*, XEPL::TEXT::String* );
		void Method_Request     NUCLEUS_METHOD;
		void Scan_Status_Line    ( void  );
	};
}


namespace KITS::HTTP
{
	class HttpServer : public KITS::HTTP::Http
	{
	protected:
		~HttpServer() override;

	public:
		explicit HttpServer      ( XEPL::CORTEX::Neuron*, XEPL::DNA::Gene* );

		void Http__Deliver       ( void ) override;

		void Build_Response      ( XEPL::DNA::Gene*, XEPL::TEXT::String* );
		void Method_Respond      NUCLEUS_METHOD;
		void Scan_Request_Line   ( void );
	};
}


class KITS::SOCKET::Socket : public XEPL::CORTEX::Neuron
{
protected:
	XEPL::OS::osSocket*            os_socket;

	~Socket ( void ) override;

public:
	explicit Socket ( XEPL::CORTEX::Neuron*, XEPL::DNA::Gene* );

	void Nucleus__Dropped        ( void ) override;

	XEPL::OS::osFd*  Get_osFd    ( void );

	XEPL::OS::osSocketAddress*& SocketAddress ( void );
};


class KITS::SOCKET::SocketTcp : public KITS::SOCKET::Socket
{
	XEPL::OS::osTcpSocket*         os_tcp_socket;
	bool                           aborted;

protected:
	XEPL::SIGNAL::Axon*            closed_axon;

	~SocketTcp ( void ) override;

public:
	explicit SocketTcp ( XEPL::CORTEX::Neuron*,  XEPL::DNA::Gene* );
	explicit SocketTcp ( XEPL::OS::osTcpSocket*, XEPL::CORTEX::Neuron*, XEPL::DNA::Gene* );

	void Nucleus__Dropped        ( void ) override;

	void Closed_Aborted          ( void );
	void Closed_Normal           ( void );
	void Send                    ( XEPL::TEXT::Wire*   );
	void Set_Tcp_Socket          ( XEPL::OS::osTcpSocket* );
	void Start                   ( void );

	void Method_Closed            NUCLEUS_METHOD;
	void Method_SendContent       NUCLEUS_METHOD;
	void Method_Start             NUCLEUS_METHOD;
};


class KITS::SOCKET::SocketMan : public XEPL::CORTEX::Lobe
{
	friend class KITS::SOCKET::SocketControl;

public:
	KITS::SOCKET::SocketControl*   controlSocket;
	XEPL::OS::osFdSet*             fdSet;
	XEPL::SIGNAL::Axon*            eRead;
	XEPL::SIGNAL::Axon*            eWrite;
	XEPL::SIGNAL::Axon*            eExcept;
	XEPL::SIGNAL::Axon*            eCancel;

protected:
	~SocketMan ( void ) override;

	void Do_Read                   SIGNAL_RECEIVER;
	void Do_Write                  SIGNAL_RECEIVER;
	void Do_Except                 SIGNAL_RECEIVER;
	void Do_Cancel                 SIGNAL_RECEIVER;

public:
	explicit SocketMan ( XEPL::CORTEX::Neuron*, XEPL::DNA::Gene* );

	void Lobe__Born              ( void ) override;
	void Lobe__Dying             ( void ) override;
	void Lobe__Rest_Now          ( void ) override;
	void Lobe__Wake_Up           ( void ) override;

	void Unhook                  ( XEPL::OS::osFd* );
};


class KITS::SOCKET::TcpClient : public KITS::SOCKET::SocketTcp
{
	XEPL::OS::osTcpClient*         os_tcp_client;
	XEPL::SIGNAL::Axon*            eConnect;
	XEPL::SIGNAL::Axon*            eExcept;
	XEPL::SIGNAL::Axon*            eAbort;
	long                           retries;
	int                            attempt;
	bool                           connected;

protected:
	~TcpClient ( void ) override;

public:
	explicit TcpClient ( XEPL::CORTEX::Neuron*, XEPL::DNA::Gene* );

	bool Fd_Connecting           ( void       );
	bool Fd_Excepting            ( void       );
	void Set_Client_Socket       ( XEPL::OS::osTcpClient* );

	void Connect                 ( void );
	void Server_Connected          SIGNAL_RECEIVER;
};


class KITS::SOCKET::TcpServer : public KITS::SOCKET::SocketTcp
{
	XEPL::SIGNAL::Axon*            eConnecting;
	XEPL::TEXT::String*            nodeName;
	XEPL::OS::osTcpServer*         server_socket;

protected:
	~TcpServer ( void ) override;

public:
	explicit TcpServer ( XEPL::CORTEX::Neuron*, XEPL::DNA::Gene* );

	bool SocketMan_Connecting    ( int,   XEPL::OS::osSocketAddress* );
	void Client_Connected        SIGNAL_RECEIVER;
	void Listen                  ( void );
	void Server_Create           ( void );
	void Set_Server_Socket       ( XEPL::OS::osTcpServer* );
};


namespace KITS::TIMER
{
	class Timer;
	class TimerList;
	class Ticker;
	class PerformanceTimer;
}


class KITS::TIMER::Timer : public XEPL::CORTEX::Neuron
{
	friend class KITS::TIMER::TimerList;

	static	KITS::TIMER::TimerList* masterTimerList;

	KITS::TIMER::TimerList*        timer_list;
	XEPL::SIGNAL::Axon*            eFired;
	long                           offset;
	long                           rate;
	long                           duration;
	bool                           time_is_running;

protected:
	~Timer ( void ) override;

	void Handle_eFired             SIGNAL_RECEIVER;
	void Method_Start              NUCLEUS_METHOD;
	void Method_Stop               NUCLEUS_METHOD;

public:
	explicit Timer ( XEPL::CORTEX::Neuron*, XEPL::DNA::Gene* );

	void Nucleus__Dropped        ( void  ) override;
	void Start_Timer             ( long  );
	void Expired                 ( XEPL::DNA::Gene* );
	void Stop_Timer              ( void  );

	static void Initialize       ( long  );
	static void Shutdown         ( void  );

};


namespace KITS::TERMINAL
{
	class osStdIn;
	class StdIn;
	class Terminal;
	class CommandNode;
	class ScopeRaw;
}


class KITS::TERMINAL::StdIn : public XEPL::CORTEX::Sensor
{
	KITS::TERMINAL::osStdIn*   os_stdin;
	KITS::SOCKET::SocketMan*   dispatcher;

protected:
	~StdIn();

public:
	explicit StdIn ( XEPL::CORTEX::Neuron*, XEPL::DNA::Gene* );

	void Nucleus__Dropped ( void ) override;
	void Sensor__Scan     ( void ) override;
	void Sensor__Closed   ( void ) override;
};


#define CTRL_KEY(k) ((k) & 0x1f)


class KITS::TERMINAL::Terminal
{
public:
	enum editorKey
	{
		ARROW_LEFT = 1000,
		ARROW_RIGHT,
		ARROW_UP,
		ARROW_DOWN,
		DEL_KEY,
		HOME_KEY,
		END_KEY,
	};

	Terminal();
	~Terminal();

	static void Disable_Raw ( void );

	int  Read_Key           ( void );
	void Write_Backspace    ( void );
	void Move_Cursor_Left   ( void );
	void Move_Cursor_Right  ( void );
	void Move_Bol           ( void );
	void Clear_To_Eol       ( void );
};

class KITS::TERMINAL::ScopeRaw
{
public:
	~ScopeRaw();
	ScopeRaw();
};


class XEPL::OS::osFd : public ATOM::Atom
{
	XEPL::TEXT::String*            socket_name;
	XEPL::DNA::Gene*               receiving;
	XEPL::SIGNAL::Axon*            eReceived;

public:
	KITS::SOCKET::SocketMan*       socket_manager;
	XEPL::TEXT::Wire*              backpressure;
	XEPL::THREAD::Mutex*           mutex;

	bool Set_Manager             ( XEPL::CORTEX::Neuron* );
	void Set_Data_Axon           ( XEPL::SIGNAL::Axon*   );

	void Hold_Data_Back          ( XEPL::TEXT::Wire*  );
	bool Has_Backpressure        ( void      ) const;

	virtual int osFd__Descriptor ( void ) const = 0;

protected:
	virtual ~osFd ( void );
	explicit osFd ( void );

	void Receive_From_Fd         ( const char*, long, bool );

private:
	friend class KITS::SOCKET::SocketMan;

	bool Fd_Does_Nothing()
	{
		return false;
	}
	virtual fdHandler osFd__OnFdRead ( void ) const
	{
		return ( fdHandler )&osFd::Fd_Does_Nothing;
	}
	virtual fdHandler osFd__OnFdWrite ( void ) const
	{
		return ( fdHandler )&osFd::Fd_Does_Nothing;
	}
	virtual fdHandler osFd__OnFdExcept ( void ) const
	{
		return ( fdHandler )&osFd::Fd_Does_Nothing;
	}
};


class XEPL::OS::osFdPair : public XEPL::MEMORY::Recycler
{
	using SetInts = std::set<int, std::less<int>, IntAllocator>;

	friend class XEPL::OS::osFdSelect;

	SetInts*                       descriptor_set;
	XEPL::OS::HandlerMap*          handler_map;
	XEPL::OS::osFdSelect*          watch_fds;
	XEPL::OS::osFdSelect*          active_fds;
	int                            num_fds;
	int                            max_fd;

public:
	osFdPair ( void );
	~osFdPair ( void );

	int  Deliver_Fd              ( int );
	int  Max_Fd                  ( int ) const;
	void Ignore_Fd               ( XEPL::OS::osFd* );
	void Set_Fd_Handler          ( XEPL::OS::osFd*, XEPL::OS::fdHandler );

	struct fd_set* Active_Fds    ( void ) const;
};


class XEPL::OS::osFdSelect : public XEPL::MEMORY::Recycler
{
public:
	fd_set*                        os_fd_set;

	~osFdSelect();
	osFdSelect ( void );

	void Duplicate_Into          ( XEPL::OS::osFdSelect* ) const;

	void Set_Fd                  ( int );
	bool Is_Fd_Ready             ( int ) const;
	int  Deliver_Fds             ( XEPL::OS::osFdPair*, int );
	void Clear_Fd                ( int );
};


class XEPL::OS::osFdSet
{
	friend class KITS::SOCKET::SocketMan;

	XEPL::OS::osFdPair*            read_pair;
	XEPL::OS::osFdPair*            write_pair;
	XEPL::OS::osFdPair*            except_pair;

public:
	~osFdSet ( void );
	osFdSet  ( void );

protected:
	void Ignore                  ( XEPL::OS::osFd*, XEPL::OS::osFdPair* );
	void Wait_On_Selected        ( void );
};


class XEPL::OS::HandlerMap : public std::map< int, std::pair<XEPL::OS::osFd*, XEPL::OS::fdHandler> > {};


class XEPL::OS::osSocket : public XEPL::OS::osFd
{
public:
	int                            socket_fd;
	XEPL::OS::osSocketAddress*     socket_address;
	KITS::SOCKET::Socket*          socket_neuron;

public:
	~osSocket ( void ) override;
	explicit osSocket ( KITS::SOCKET::Socket* );
	explicit osSocket ( KITS::SOCKET::Socket*, int, XEPL::OS::osSocketAddress* );

	int  osFd__Descriptor        ( void ) const override;

	void Bind_Socket             ( bool );
	void Build_Socket            ( int  );
	void Close_Socket            ( void );
};


struct sockaddr;
struct sockaddr_in;

class XEPL::OS::osSocketAddress
{
	::sockaddr_in*                 sockAddr;
	XEPL::TEXT::String*            ip_string;

public:
	~osSocketAddress             ( void );
	osSocketAddress              ( void );
	explicit osSocketAddress     ( XEPL::DNA::Gene* );

	long Length                  ( void ) const;
	long Port                    ( void ) const;
	XEPL::TEXT::Cord* IpString   ( void );
	::sockaddr*       Get        ( void ) const;

	static void Hostname         ( XEPL::TEXT::String* );

};


class KITS::SOCKET::SocketControl : public KITS::SOCKET::Socket
{
	XEPL::OS::osSocketControl*     socket_control;

public:
	explicit SocketControl ( KITS::SOCKET::SocketMan* );

	XEPL::OS::osSocketAddress* SocketAddress ( void );

	void Send                    ( void );
};


class XEPL::OS::osSocketControl : public XEPL::OS::osSocket
{
	KITS::SOCKET::SocketControl*   controlSocket;

public:
	explicit osSocketControl ( KITS::SOCKET::SocketControl* );

	XEPL::OS::fdHandler osFd__OnFdRead ( void ) const override;
	
	bool    Fd_Receive           ( void );
	void    Send                 ( void );
};


class XEPL::OS::osTcpSocket : public XEPL::OS::osSocket
{
public:
	KITS::SOCKET::SocketTcp*       soc_tcp;
	long                           messages_sent;
	long                           messages_received;
	long                           bytes_sent;
	long                           bytes_received;

	explicit osTcpSocket         ( KITS::SOCKET::SocketTcp* );
	explicit osTcpSocket         ( KITS::SOCKET::SocketTcp*, int, XEPL::OS::osSocketAddress* );

	void Create_Tcp_Socket       ( void );
	void Send_Data               ( XEPL::TEXT::Wire* );
	bool Read_Available          ( void );
	bool Send_Available          ( void );

private:
	XEPL::OS::fdHandler osFd__OnFdRead  ( void ) const override
	{
		return ( XEPL::OS::fdHandler )&osTcpSocket::Read_Available;
	}
	XEPL::OS::fdHandler osFd__OnFdWrite ( void ) const override
	{
		return ( XEPL::OS::fdHandler )&osTcpSocket::Send_Available;
	}

};


class XEPL::OS::osTcpClient : public XEPL::OS::osTcpSocket
{
	KITS::SOCKET::TcpClient*       tcp_client;

protected:
	~osTcpClient ( void );

public:
	XEPL::OS::osSocketAddress*     server_address;

	explicit osTcpClient         ( KITS::SOCKET::TcpClient* );

	XEPL::OS::fdHandler osFd__OnFdExcept  ( void ) const override;
	XEPL::OS::fdHandler osFd__OnFdWrite   ( void ) const override;
	
	bool    osFd_Connected       ( void );
	bool    osFd_Excepted        ( void );
	void    osFd_Connect         ( void );
};


class XEPL::OS::osTcpServer : public XEPL::OS::osTcpSocket
{
	KITS::SOCKET::TcpServer*       tcp_server;

protected:
	~osTcpServer();

public:
	explicit osTcpServer         ( KITS::SOCKET::TcpServer* );

	void Listen_For_Connections  ( long );
	bool Fd_Is_Connecting        ( void );

private:
	XEPL::OS::fdHandler osFd__OnFdRead ( void ) const override
	{
		return ( XEPL::OS::fdHandler )&osTcpServer::Fd_Is_Connecting;
	}
};


class XEPL::CommandsMap : public XEPL::MAPS::MapT <XEPL::TEXT::Cord, XEPL::Command >         {};
class XEPL::SpineMap    : public XEPL::MAPS::MapReleaseT<XEPL::TEXT::Cord, XEPL::SIGNAL::Axon*>     {};
class XEPL::OrganMap    : public XEPL::MAPS::MapReleaseT<XEPL::TEXT::Cord, XEPL::CORTEX::Neuron* >  {};


class KITS::TERMINAL::CommandNode
{
public:
	XEPL::TEXT::String   command_string;
	CommandNode*        prev_command;
	CommandNode*        next_command;

	CommandNode ( osStdIn*, const char* );
};


class KITS::TERMINAL::osStdIn : public XEPL::OS::osFd
{
	friend class KITS::TERMINAL::CommandNode;

	XEPL::CORTEX::Sensor*          sensor;
	KITS::TERMINAL::Terminal*      terminal;
	char*                          input_buffer;
	XEPL::SIGNAL::Axon*            cli_input_axon;
	XEPL::DNA::Gene*               receiving_gene;
	KITS::TERMINAL::CommandNode*   command_head;
	KITS::TERMINAL::CommandNode*   command_tail;
	KITS::TERMINAL::CommandNode*   command;
	long                           offset;
	long                           length;

public:
	~osStdIn() override;

	explicit osStdIn ( XEPL::CORTEX::Sensor* );

	XEPL::OS::fdHandler osFd__OnFdRead   ( void ) const override;
	int                 osFd__Descriptor ( void ) const override;

	bool Process_Command_Key     ( int  );
	bool Fd_Receive              ( void );
	void Line_Received             SIGNAL_RECEIVER;
};


namespace KITS::HTTP
{
	class HttpHeadersMap : public XEPL::MAPS::MapT<XEPL::TEXT::Cord,  KITS::HTTP::HttpHeader > {};
}


namespace XEPL::RENDER
{
	class RendronHtml : public XEPL::CORTEX::Rendron
	{
	public:
		explicit RendronHtml ( XEPL::CORTEX::Neuron*, XEPL::DNA::Gene*, XEPL::TEXT::String* );

		void Rendron__Method_Markup ( XEPL::DNA::Gene*,      XEPL::DNA::Gene* ) override;
		void Rendron__Render_Gene   ( XEPL::CELL::Nucleus*,  XEPL::DNA::Gene* ) override;

		void Scribble_Traits     ( XEPL::CELL::Nucleus*, XEPL::DNA::Gene* );
		void Scribble_Content    ( XEPL::CELL::Nucleus*, XEPL::DNA::Gene* );
		void Scribble_Gene       ( XEPL::CELL::Nucleus*, XEPL::DNA::Gene* );
	};
}


namespace XEPL::RENDER
{
	class RendronText : public XEPL::CORTEX::Rendron
	{
	public:
		explicit RendronText ( XEPL::CORTEX::Neuron*, XEPL::DNA::Gene*, XEPL::TEXT::String* );

		void Rendron__Method_Markup ( XEPL::DNA::Gene*, XEPL::DNA::Gene* ) override;
	};
}


namespace KITS::SOCKET
{
	class ConnectedClient : public XEPL::ATOM::Atom
	{
	public:
		int                         descriptor;
		XEPL::OS::osSocketAddress*  address;

		ConnectedClient ( int _descriptor, XEPL::OS::osSocketAddress* _address )
			: Atom()
			, descriptor ( _descriptor )
			, address    ( _address )
		{}

		~ConnectedClient() override
		{}
	};
}


class KITS::TIMER::Ticker
{
public:

	void Start                   ( void );
	void Stop                    ( void );

protected:
	virtual ~Ticker();
	explicit Ticker ( std::chrono::milliseconds interval );
	
	virtual bool Deliver_Tick    ( int64_t tick_count ) = 0;

private:
	std::chrono::milliseconds      interval;
	std::atomic<bool>              is_running;
	int64_t                        tick_count;
	std::thread                    std__thread;
	std::mutex                     std__mutex;
	std::condition_variable        std__condition_variable;
};


namespace KITS::TIMER
{
	class TimerList : public Ticker
	{
		using TimerAllocator = XEPL::MEMORY::TAllocatorT<KITS::TIMER::Timer*>;
		using ListTimers     = std::list<KITS::TIMER::Timer*, TimerAllocator>;

	public:
		ListTimers*                timer_list;
		XEPL::THREAD::Mutex*       mutex;
		long                       pre_scaler;

	public:
		~TimerList();
		explicit TimerList ( long );

		bool Deliver_Tick        ( int64_t tick_count ) override;

		void Run                 ( KITS::TIMER::Timer*, long );
		void Remove              ( KITS::TIMER::Timer* );
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
