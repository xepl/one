// SPDX: AGPL-3.0-only
/* XEPL Operating Environment - Copyright (c) 2024 Keith Edwin Robbins
	Project Name: XEPL Operating Environment
	File Name:    xepl.cc
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

#include "xepl.h"

/// This detects C++ memory leaks

std::atomic_size_t XEPL::num_total_news{0};
std::atomic_size_t XEPL::num_total_dels{0};

void* operator new ( size_t _size )
{
	++XEPL::num_total_news;
	return malloc ( _size );
}

void operator delete ( void* _ptr ) throw()
{
	if ( !_ptr )
		return;
	++XEPL::num_total_dels;
	free ( _ptr );
}

XEPL::MemoryCounts::~MemoryCounts()
{
	size_t leaking_allocations = num_total_news-num_total_dels;

	if ( std_ostream && num_total_news && ( Show_Memory_Counts || leaking_allocations ) )
	{
		String news_string;
		Long_Commafy(num_total_news, &news_string);

		*std_ostream << "Others:  " << std::setw( 12 ) << news_string << " :" << std::endl;
	}

	if ( leaking_allocations )
		std::cerr << " ---LEAKING new/delete: " << leaking_allocations << '\n' << std::flush;
}

XEPL::MemoryCounts::MemoryCounts ( std::ostream* _ostream )
	: std_ostream ( _ostream )
{
	num_total_dels = 0;
	num_total_news = 0;
}

//   8888ba.88ba             dP
//   88  `8b  `8b            88
//   88   88   88 dP    dP d8888P .d8888b. dP.  .dP
//   88   88   88 88    88   88   88ooood8  `8bd8'
//   88   88   88 88.  .88   88   88.  ...  .d88b.
//   dP   dP   dP `88888P'   dP   `88888P' dP'  `dP
//
XEPL::Mutex::Mutex()
	: mutex() {}

XEPL::MutexScope::~MutexScope ( void )
{
	if ( mutex )
		mutex->mutex.unlock();
}

XEPL::MutexScope::MutexScope ( Mutex* _mutex )
: mutex ( _mutex )
{
	if ( mutex )
		mutex->mutex.lock();
}
 //   d888888P dP                                        dP
//      88    88                                        88
//      88    88d888b. 88d888b. .d8888b. .d8888b. .d888b88
//      88    88'  `88 88'  `88 88ooood8 88'  `88 88'  `88
//      88    88    88 88       88.  ... 88.  .88 88.  .88
//      dP    dP    dP dP       `88888P' `88888P8 `88888P8
//

XEPL::Thread::Thread ( Lobe* _lobe, Semaphore* _semaphore )
	: std_thread      ( nullptr )
	, lobe            ( _lobe )
	, semaphore_rest  ( _semaphore )
{}

void XEPL::Thread::Bury_Child()
{
	if ( std_thread )
	{
		std_thread->join();
		std_thread->~thread();

		delete std_thread;
		std_thread=nullptr;

		lobe = nullptr;
	}
}

void XEPL::Thread::Concieve_Child( Semaphore* _semaphore )
{
	std_thread = ::new std::thread ( [_semaphore] ( Thread* _thread )
	{
		tlsLobe = _thread->lobe;

		Backpack     memory_backpack;
		ShortTerms   short_term_memories;

		tlsLobe->Main_Loop( _semaphore );
	}, this );
}

//   .d88888b                                        dP
//   88.    "'                                       88
//   `Y88888b. .d8888b. 88d8b.d8b. .d8888b. 88d888b. 88d888b. .d8888b. 88d888b. .d8888b.
//         `8b 88ooood8 88'`88'`88 88'  `88 88'  `88 88'  `88 88'  `88 88'  `88 88ooood8
//   d8'   .8P 88.  ... 88  88  88 88.  .88 88.  .88 88    88 88.  .88 88       88.  ...
//    Y88888P  `88888P' dP  dP  dP `88888P8 88Y888P' dP    dP `88888P' dP       `88888P'
//                                          88
//                                          dP

XEPL::Semaphore::Semaphore()
	: std::condition_variable()
	, std::mutex()
{}

void XEPL::Semaphore::Give ( void )
{
	std::unique_lock<std::mutex> lock ( *this );
	notify_one();
}


std::ostream& operator<< (std::ostream&, XEPL::Cord* );
std::ostream& operator<< (std::ostream&, XEPL::Cord& );

XEPL::String::~String ( void ) {}

XEPL::String::String ( void )
	: CppString()
{}

XEPL::String::String ( const CppString* _string )
	: CppString()
{
	if ( _string )
		assign ( *_string );
}

XEPL::String::String ( const String* _string )
	: CppString()
{
	if ( _string )
		assign ( *_string );
}

XEPL::String::String ( Text* _string )
	: CppString()
{
	if ( _string )
		assign ( _string );
}

XEPL::String::String ( Text* _string, size_t _length )
	: CppString()
{
	if ( _string && _length )
		assign ( _string, _length );
}

std::ostream& operator<< (std::ostream& os, XEPL::Cord* str)
{
	return os << str->c_str();
}

std::ostream& operator<< (std::ostream& os, XEPL::Cord& str)
{
	return os << str.c_str();
}

//    888888ba                    dP                                  dP
//    88    `8b                   88                                  88
//   a88aaaa8P' .d8888b. .d8888b. 88  .dP  88d888b. .d8888b. .d8888b. 88  .dP
//    88   `8b. 88'  `88 88'  `"" 88888"   88'  `88 88'  `88 88'  `"" 88888"
//    88    .88 88.  .88 88.  ... 88  `8b. 88.  .88 88.  .88 88.  ... 88  `8b.
//    88888888P `88888P8 `88888P' dP   `YP 88Y888P' `88888P8 `88888P' dP   `YP
//                                         88
//                                         dP
XEPL::Backpack::~Backpack()
{
	heap->~HeapOfPools();
	free ( heap );

	tlsHeap = nullptr;
}

XEPL::Backpack::Backpack()
	: heap (  new ( malloc ( sizeof ( HeapOfPools ) ) ) HeapOfPools() )
{
	tlsHeap = heap;
}

//    888888ba                                      dP
//    88    `8b                                     88
//   a88aaaa8P' .d8888b. .d8888b. dP    dP .d8888b. 88 .d8888b. 88d888b.
//    88   `8b. 88ooood8 88'  `"" 88    88 88'  `"" 88 88ooood8 88'  `88
//    88     88 88.  ... 88.  ... 88.  .88 88.  ... 88 88.  ... 88
//    dP     dP `88888P' `88888P' `8888P88 `88888P' dP `88888P' dP
//                                     .88
//                                 d8888P

void* XEPL::Recycler::operator new ( size_t size )
{
	if ( HeapOfPools* heap = tlsHeap )
		return heap->Get_Block ( size );

	return ::operator new ( size );
}

void XEPL::Recycler::operator delete ( void* p )
{
	if ( HeapOfPools* heap = tlsHeap )
		heap->Recycle_Block ( p );
	else
		::operator delete ( p );
}
//   dP     dP                              .88888.  .8888b  888888ba                    dP
//   88     88                             d8'   `8b 88   "  88    `8b                   88
//   88aaaaa88a .d8888b. .d8888b. 88d888b. 88     88 88aaa  a88aaaa8P' .d8888b. .d8888b. 88 .d8888b.
//   88     88  88ooood8 88'  `88 88'  `88 88     88 88      88        88'  `88 88'  `88 88 Y8ooooo.
//   88     88  88.  ... 88.  .88 88.  .88 Y8.   .8P 88      88        88.  .88 88.  .88 88       88
//   dP     dP  `88888P' `88888P8 88Y888P'  `8888P'  dP      dP        `88888P' `88888P' dP `88888P'
//                                88
//                                dP

thread_local class XEPL::HeapOfPools* XEPL::tlsHeap = nullptr;

XEPL::HeapOfPools::~HeapOfPools()
{
	total_biggies_out += count_biggies_out;
	total_biggies_in  += count_biggies_in;

	if ( largest_biggie < largest_biggie_out )
		largest_biggie = largest_biggie_out; // may overwrite on race

	for ( long index = 1; index <= Memory::maxPoolIndex; ++index )
	{
		pool_of_blocks[index]->~PoolOfBlocks();
		free ( pool_of_blocks[index] );
	}
}

XEPL::HeapOfPools::HeapOfPools()
	: pool_of_blocks  ()
	, count_biggies_out    ( 0 )
	, count_biggies_in     ( 0 )
	, largest_biggie_out   ( 0 )
{
	for ( long index = 1; index <= Memory::maxPoolIndex; ++index )
	{
		void* ptr = malloc ( sizeof ( PoolOfBlocks ) );
		pool_of_blocks[index] = ( new ( ptr ) PoolOfBlocks ( index ) );
	}

	pool_of_blocks[0] = nullptr;
}

void* XEPL::HeapOfPools::Get_Block ( size_t _size )
{
	auto block_id = _size/Memory::poolWidth;
	if ( block_id <= Memory::maxPoolIndex )
	{
		if ( !block_id )
			block_id = 1;

		return pool_of_blocks[block_id]->Get_Or_Malloc();
	}

	++count_biggies_out;

	if ( largest_biggie_out < _size )
		largest_biggie_out = _size;

	const size_t block_size = _size + sizeof ( BlockHeader );

	BlockHeader* block = static_cast<BlockHeader*> ( malloc ( block_size ) );
	block->pool_index = 0;
	return block + 1;
}

void  XEPL::HeapOfPools::Recycle_Block ( void* _ptr )
{
	if ( !_ptr )
		return;

	BlockHeader* returned_block = static_cast<BlockHeader*> ( _ptr )-1;

	if ( returned_block->pool_index )
		pool_of_blocks[returned_block->pool_index]->Catch_Or_Free ( returned_block );
	else
	{
		++count_biggies_in;
		free ( returned_block );
	}
}


//    888888ba                    dP  .88888.  .8888b  888888ba  dP                   dP
//    88    `8b                   88 d8'   `8b 88   "  88    `8b 88                   88
//   a88aaaa8P' .d8888b. .d8888b. 88 88     88 88aaa  a88aaaa8P' 88 .d8888b. .d8888b. 88  .dP  .d8888b.
//    88        88'  `88 88'  `88 88 88     88 88      88   `8b. 88 88'  `88 88'  `"" 88888"   Y8ooooo.
//    88        88.  .88 88.  .88 88 Y8.   .8P 88      88    .88 88 88.  .88 88.  ... 88  `8b.       88
//    dP        `88888P' `88888P' dP  `8888P'  dP      88888888P dP `88888P' `88888P' dP   `YP `88888P'
//

XEPL::PoolOfBlocks::~PoolOfBlocks( void )
{
	total_mallocs  += blocks_malloced;
	total_cached   += blocks_cached;
	total_freed    += blocks_freed;
	total_held     += blocks_holding;

	pool_mallocs  [ pool_index ] += blocks_malloced;
	pool_cached   [ pool_index ] += blocks_cached;
	pool_freed    [ pool_index ] += blocks_freed;
	pool_held     [ pool_index ] += blocks_holding;

	while ( BlockHeader* block = head_block )
	{
		head_block=block->next_block;
		free ( block );
	}
}

XEPL::PoolOfBlocks::PoolOfBlocks ( long _index )
	: head_block         ( nullptr )
	, pool_index         ( _index )
	, block_size         ( ( _index+1 )*Memory::poolWidth )
	, blocks_malloced    ( 0 )
	, blocks_cached      ( 0 )
	, blocks_freed       ( 0 )
	, blocks_holding     ( 0 )
{}

void XEPL::PoolOfBlocks::Catch_Or_Free ( BlockHeader* _block )
{
	if ( blocks_holding > blocks_cached || blocks_holding > blocks_malloced )
	{
		++blocks_freed;
		free ( _block );
	}
	else
	{
		++blocks_holding;
		_block->next_block = head_block;
		head_block = _block;
	}
}

void* XEPL::PoolOfBlocks::Get_Or_Malloc( void )
{
	BlockHeader* block = head_block;
	if ( block )
	{
		--blocks_holding;
		++blocks_cached;
		head_block = block->next_block;
	}
	else
	{
		++blocks_malloced;
		block = static_cast<BlockHeader*> ( malloc ( block_size ) );
	}
	block->pool_index = pool_index;

	return block+1;
}

// required for operation
std::atomic_size_t XEPL::pool_mallocs[] = {0};
std::atomic_size_t XEPL::pool_cached[]  = {0};
std::atomic_size_t XEPL::pool_freed[]   = {0};
std::atomic_size_t XEPL::pool_held[]    = {0};

// optional for observation
std::atomic_size_t XEPL::total_mallocs     ( 0 );
std::atomic_size_t XEPL::total_cached      ( 0 );
std::atomic_size_t XEPL::total_freed       ( 0 );
std::atomic_size_t XEPL::total_held        ( 0 );
std::atomic_size_t XEPL::total_biggies_out ( 0 );
std::atomic_size_t XEPL::total_biggies_in  ( 0 );
std::atomic_size_t XEPL::largest_biggie    ( 0 );


//    888888ba                                      dP                    a88888b.                              dP
//    88    `8b                                     88                   d8'   `88                              88
//   a88aaaa8P' .d8888b. .d8888b. dP    dP .d8888b. 88 .d8888b. 88d888b. 88        .d8888b. dP    dP 88d888b. d8888P .d8888b.
//    88   `8b. 88ooood8 88'  `"" 88    88 88'  `"" 88 88ooood8 88'  `88 88        88'  `88 88    88 88'  `88   88   Y8ooooo.
//    88     88 88.  ... 88.  ... 88.  .88 88.  ... 88 88.  ... 88       Y8.   .88 88.  .88 88.  .88 88    88   88         88
//    dP     dP `88888P' `88888P' `8888P88 `88888P' dP `88888P' dP        Y88888P' `88888P' `88888P' dP    dP   dP   `88888P'
//                                     .88
//                                 d8888P

XEPL::RecycleCounts::~RecycleCounts()
{
	long biggies_out = total_biggies_out;
	long biggies_in = total_biggies_in;

	long leaking = total_mallocs - total_freed - total_held;

	if ( std_ostream && ( Show_Memory_Counts || leaking ) )
	{
		const long width = 13;

		*std_ostream << "\nCounter:         Total : ";
		for ( long index = 1; index <= Memory::maxPoolIndex; ++index )
			*std_ostream << std::setw ( width ) << ( index+1 )*Memory::poolWidth << " ";

		Print_Pool_Counts ( std_ostream, "Mallocs: ", total_mallocs, pool_mallocs, width );
		Print_Pool_Counts ( std_ostream, "Freed:   ", total_freed,   pool_freed,   width );
		Print_Pool_Counts ( std_ostream, "Cached:  ", total_cached,  pool_cached,  width );
		Print_Pool_Counts ( std_ostream, "Held:    ", total_held,    pool_held,    width );

		String counts;
		Long_Commafy ( biggies_out, &counts );
		*std_ostream << "\nBiggies: " << std::setw( width ) << counts;

		counts.assign(" :  Largest: " );
		Long_In_Bytes ( largest_biggie, &counts );

		*std_ostream << counts << std::endl;
	}

	if ( leaking )
		std::cerr << " ***LEAKING " << leaking << " Recycled Allocations: " << std::endl;

	if ( biggies_in-biggies_out )
		std::cerr << " ***LEAKING " << biggies_in-biggies_out << " Biggie Allocations: " << std::endl;
}

XEPL::RecycleCounts::RecycleCounts ( std::ostream* _ostream )
	: std_ostream ( _ostream )
{
	total_mallocs     = 0;
	total_cached      = 0;
	total_freed       = 0;
	total_held        = 0;
	total_biggies_out = 0;
	total_biggies_in  = 0;
	largest_biggie    = 0;
}

void XEPL::PoolOfBlocks::Report( String* _into )
{
	_into->append( std::to_string(block_size)     ).append( ":(" );
	_into->append( std::to_string(blocks_malloced)).append(",");
	_into->append( std::to_string(blocks_freed)   ).append(",");
	_into->append( std::to_string(blocks_cached)  ).append(",");
	_into->append( std::to_string(blocks_holding) ).append(") ");;
}

void XEPL::HeapOfPools::Report(String* _into )
{
	_into->append("[");
	_into->append( std::to_string(largest_biggie_out) ).append( "," );
	_into->append( std::to_string(count_biggies_out)  ).append( "/" );
	_into->append( std::to_string(count_biggies_in)   ).append("] ");

	for ( long index = 1; index <= Memory::maxPoolIndex; ++index )
		pool_of_blocks[index]->Report( _into );
}

void XEPL::Recycler::Report_Heap( String* _into )
{
	tlsHeap->Report( _into );
}

void XEPL::RecycleCounts::Print_Pool_Counts ( std::ostream* _report, Text* _label, const std::atomic_size_t& _total, std::atomic_size_t* _array, int _width )
{
	long mega_sum = 0;

	String counts;
	*_report << "\n" << _label << std::setw ( _width ) << *Long_Commafy ( _total, &counts ) << " : ";

	for ( long index = 1; index <= Memory::maxPoolIndex; ++index )
	{
		counts.clear();
		*_report << std::setw ( _width ) << *Long_Commafy ( _array[index], &counts ) << " ";

		mega_sum += ( index+1 )*Memory::poolWidth * _array[index];
	}

	counts.clear();
	*_report << std::setw ( _width+3 ) << *Long_In_Bytes ( mega_sum, &counts );
}

//    a88888b.                              dP
//   d8'   `88                              88
//   88        .d8888b. dP    dP 88d888b. d8888P .d8888b. 88d888b. .d8888b.
//   88        88'  `88 88    88 88'  `88   88   88ooood8 88'  `88 Y8ooooo.
//   Y8.   .88 88.  .88 88.  .88 88    88   88   88.  ... 88             88
//    Y88888P' `88888P' `88888P' dP    dP   dP   `88888P' dP       `88888P'
//

	/// per thread counter descriptions
	const char* XEPL::Counters::Counter_Descriptions = "\
 genes:\
 traits:\
 lobes:\
 neurons:\
 dispatched:\
 rests:\
 actions:\
 wakes:\
 ";

void XEPL::Counters::Add ( Counters* _counters )
{
	long record_size = sizeof ( Counters );
	long number_of_counters = record_size/sizeof ( long );
	Counter* scounter = (Counter*) ( _counters );
	Counter* tcounter = (Counter*) ( this );

	while ( number_of_counters-- )
		*tcounter+++=*scounter++;
}

void XEPL::Counters::Report ( String* _string )
{
	String long_label ( Counter_Descriptions );
	String lhs;
	Counter* tcounter = (Counter*) ( this );

	_string->append( "" );
	while ( Split_ch_lhs ( &long_label, ':', &lhs ) )
	{
		_string->append ( lhs ).append ( ": " );
		_string->append ( std::to_string ( *tcounter ) ).append ( " " );
		tcounter++;
	}
}

void XEPL::Counters::Final_Report()
{
	String long_label ( Counter_Descriptions );
	String lhs;
	Counter* tcounter = (Counter*) ( this );

	String count;

	while ( Split_ch_lhs ( &long_label, ':', &lhs ) )
	{
		count.clear();
		Long_Commafy ( *tcounter++, &count );
		std::cout << std::setw ( 12 ) << lhs << std::setw ( 15 )
		          << count << '\n';
	}

	std::cout << std::flush;
}

//    .d888888    dP
//   d8'    88    88
//   88aaaaa88a d8888P .d8888b. 88d8b.d8b.
//   88     88    88   88'  `88 88'`88'`88
//   88     88    88   88.  .88 88  88  88
//   88     88    dP   `88888P' dP  dP  dP
//

XEPL::Atom::~Atom( void )
{}

void  XEPL::Atom::Release( void )
{
	if ( --retain_count == 0 )
		delete this;
}


//    a88888b. dP                oo
//   d8'   `88 88
//   88        88d888b. .d8888b. dP 88d888b.
//   88        88'  `88 88'  `88 88 88'  `88
//   Y8.   .88 88    88 88.  .88 88 88    88
//    Y88888P' dP    dP `88888P8 dP dP    dP
//

XEPL::Chain::~Chain()
{
	if ( head_bond )
	{
		MutexScope lock_chain ( chain_lock );

		Bond* bond = head_bond;
		while ( bond )
		{
			Bond* next = bond->next_bond;
			delete bond;
			bond = next;
		}
	}

	if ( is_my_lock )
		delete chain_lock;
}

XEPL::Chain::Chain ( bool _locked )
	: chain_lock  ( nullptr )
	, head_bond   ( nullptr )
	, tail_bond   ( nullptr )
	, is_my_lock  ( _locked )
{
	if ( is_my_lock )
		chain_lock = new Mutex();
}

XEPL::Chain::Chain ( Atom* _atom )
	: chain_lock ( nullptr )
	, head_bond  ( new Bond ( _atom, nullptr ) )
	, tail_bond  ( head_bond )
	, is_my_lock ( false )
{}

XEPL::Bond* XEPL::Chain::Add_Atom ( Atom* _atom )
{
	MutexScope lock_chain ( chain_lock );

	Bond* fresh_bond = new Bond ( _atom, tail_bond );

	if ( !head_bond )
		head_bond = fresh_bond;
	else
		tail_bond->next_bond = fresh_bond;

	tail_bond = fresh_bond;

	return fresh_bond;
}

bool XEPL::Chain::Pull_Atom ( Atom** _atom )
{
	MutexScope lock_chain ( chain_lock );

	if ( head_bond )
	{
		Bond* last_head = head_bond;

		*_atom = head_bond->atom;
		head_bond->atom = nullptr;

		head_bond = head_bond->next_bond;

		delete last_head;

		if ( head_bond )
			head_bond->prev_bond = nullptr;
		else
			tail_bond = nullptr;

		return true;
	}

	return false;
}

bool XEPL::Chain::Remove_Atom ( Atom* _atom )
{
	MutexScope lock_chain ( chain_lock );

	Bond* bond = head_bond;

	while ( bond && bond->atom != _atom )
		bond=bond->next_bond;

	if ( !bond )
		return !!head_bond;

	if ( bond == head_bond )
		head_bond = bond->next_bond;
	else
		bond->prev_bond->next_bond = bond->next_bond;

	if ( bond == tail_bond )
		tail_bond = bond->prev_bond;
	else
		bond->next_bond->prev_bond = bond->prev_bond;

	delete bond;

	return !head_bond;
}

void XEPL::Chain::Remove_Bond ( Bond* _bond )
{
	MutexScope lock_chain ( chain_lock );

	if ( _bond->prev_bond )
		_bond->prev_bond->next_bond = _bond->next_bond;
	else
		head_bond = _bond->next_bond;

	if ( _bond->next_bond )
		_bond->next_bond->prev_bond = _bond->prev_bond;
	else
		tail_bond = _bond->prev_bond;

	delete _bond;
}


//    a88888b.          dP dP
//   d8'   `88          88 88
//   88        .d8888b. 88 88
//   88        88ooood8 88 88
//   Y8.   .88 88.  ... 88 88
//    Y88888P' `88888P' dP dP
//

XEPL::Cell::~Cell()
{
	delete cell_name;
}

XEPL::Cell::Cell ( Cord* _cord )
	: Atom()
	, cell_name ( new Cord ( _cord ) )
{}

XEPL::Cell::Cell ( Text* _chars )
	: Atom()
	, cell_name ( new Cord ( _chars ) )
{}

//    a88888b.                     dP
//   d8'   `88                     88
//   88        .d8888b. 88d888b. d8888P .d8888b. dP.  .dP
//   88        88'  `88 88'  `88   88   88ooood8  `8bd8'
//   Y8.   .88 88.  .88 88         88   88.  ...  .d88b.
//    Y88888P' `88888P' dP         dP   `88888P' dP'  `dP
//

XEPL::Cortex* XEPL::cortex = nullptr;

void XEPL::Cortex::Close_Cortex( void )
{
	if( !host_lobe->index_link )
		return;

	host_lobe->Lobe_Dying();
	host_lobe->index_link->Release();
	host_lobe->indicies.Unstack( host_lobe );
	host_lobe->index_link = nullptr;
}

XEPL::Cortex::~Cortex()
{
	Close_Cortex();

	host_lobe->Release();
	host_lobe = nullptr;
	tlsLobe   = nullptr;
	
	delete keywords_map;
	delete operators_map;
	delete render_map;
	delete mutual_map;
	delete commands_map;

	cortex  = nullptr;

	if ( Show_Counters )
		final_counters.Final_Report();
}

XEPL::Cortex::Cortex ( Text* _name, std::ostream& _ostream )
	: memory_counts    ( &_ostream )
	, recycle_counts   ( &_ostream )
	, memory_backpack  ()
	, keywords_map     ( new KeywordsMap()  )
	, operators_map    ( new OperatorsMap() )
	, commands_map     ( new CommandsMap()  )
	, mutual_map       ( new MutualsMap()   )
	, render_map       ( new RenderMap()    )
	, host_lobe        ( tlsLobe = new Lobe ( _name ) )
	, short_term_memories()
	, final_counters   ()
{
	
	cortex  = this;

	Set_Thread_Name ( host_lobe->cpp_thread, _name );

	host_lobe->Attach();
	host_lobe->indicies.Stack(host_lobe, new Gene ( nullptr, "Index", host_lobe->cell_name ));
	host_lobe->Lobe_Born();
}

void XEPL::Cortex::Register_Command ( Text* _chars, Command _command )
{
	if ( !_chars )
		return;

	String name_cord ( _chars );

	auto [it, noob] = commands_map->insert_or_assign ( name_cord, _command );

	if (!noob)
		ErrorReport error_report("Replaced command: ", &name_cord );

	TRACE( "New_Command", nullptr, &name_cord);
}

bool XEPL::Cortex::Did_Command ( Text* _chars )
{
	if ( !_chars || !*_chars )
		return false;

	String command_name ( _chars );
	{
		String cmd;
		String opt;

		Split_ch_lhs_rhs ( &command_name, ' ', &cmd, &opt );

		auto it  = commands_map->find ( cmd );
		if ( it != commands_map->end() )
		{
			( *it->second ) ( &opt );
			return true;
		}
	}

	if ( tlsLobe->Performed_Method ( &command_name, nullptr ) )
		return true;

	ErrorReport error_report( "Command not understood: ", _chars );
	return false;
}

bool XEPL::Cortex::Did_Dot_Tag ( Nucleus* _nucleus, Gene* _call_gene )
{
	String  neuron_name;
	String  method_name;
	Neuron* target_neuron = nullptr;

	Split_ch_lhs_rhs ( _call_gene->cell_name, '.', &neuron_name, &method_name );

	if ( _nucleus->Host()->Find_Neuron ( &neuron_name, &target_neuron ) )
		return target_neuron->Performed_Method ( &method_name, _call_gene );

	return false;
}

void XEPL::Cortex::Register_Keyword ( Text* _chars, Keyword _keyword )
{
	if (!_chars)
		return;

	Cord name_cord(_chars);

	auto [it, noob] = keywords_map->insert_or_assign(name_cord, _keyword);

	if (!noob)
		ErrorReport error_report("Replaced keyword: ", &name_cord);

	TRACE( "New_Keyword", nullptr, &name_cord);
}

bool XEPL::Cortex::Did_Keyword ( Nucleus* _nucleus, Gene* _call_gene )
{
	auto it  = keywords_map->find ( *_call_gene->cell_name );
	if ( it == keywords_map->end() )
		return false;

	auto host = _nucleus->Host();

	String param;
	String* param_string = &param;

	XEPL::String content;
	if ( _call_gene->Copy_Content(&content) )
	{
		if ( *content.c_str() == '{' )
			Script ( host, _call_gene, &param );
		else
			param_string = &content;
	}

	if ( Show_Trace )
	{
		String trace_string;
		_call_gene->Print_Into(&trace_string, 1);
		TRACE( "DO_Keyword", _nucleus->Host(), &trace_string, param_string );
	}

	it->second ( _nucleus->Host(), _call_gene, param_string );

	return true;
}

XEPL::Gene* XEPL::Cortex::Locate_Gene ( Nucleus* _nucleus, Cord* _gene_name )
{
	if ( auto ephemerals = tlsLobe->ephemerals )
	{
		auto it  = ephemerals->find ( *_gene_name );
		if ( it != ephemerals->end() )
			return it->second;
	}

	Nucleus* current_nucleus = _nucleus;
	Gene*    located_gene    = nullptr;
	Gene*    vitals          = nullptr;

	while ( current_nucleus )
	{
		if ( current_nucleus->observer->Get_First_Gene("Vitals", &vitals ) )
			if ( vitals->inner_genes && ( vitals->Get_First_Gene ( _gene_name, &located_gene ) ) )
				return located_gene;

		current_nucleus = current_nucleus->parent_neuron;
	}

	auto it  = cortex->mutual_map->find ( *_gene_name );
	if ( it != cortex->mutual_map->end() )
		return it->second ( _nucleus );

	return nullptr;
}

XEPL::Neuron* XEPL::Cortex::Locate_Neuron ( Nucleus* _nucleus, Cord* _cord, char _split_char )
{
	Neuron* neuron = nullptr;

	size_t spot = _cord->find ( _split_char );
	if ( spot  == _cord->npos )
	{
		_nucleus->Host()->Find_Neuron ( _cord, &neuron );
		return neuron;
	}

	Cord* remaining_path = _cord;
	String neuron_name;
	String neuron_root;

	neuron = cortex->host_lobe;
	
	while ( neuron && Split_ch_lhs_rhs ( remaining_path, _split_char, &neuron_root, &neuron_name ) )
	{
		if ( !neuron->Find_Neuron ( &neuron_root, &neuron ) )
			break;
		remaining_path = &neuron_name;
	}

	if ( neuron )
		neuron->Find_Neuron ( &neuron_name, &neuron );

	return neuron;
}

XEPL::Axon* XEPL::Cortex::Locate_Axon ( Nucleus* _nucleus, Cord* _cord, char _split_char )
{
	if ( !_cord )
		return nullptr;

	Axon* located_axon = nullptr;

	String  axon_name;
	String  path_string;

	if ( Split_ch_lhs_rhs ( _cord, _split_char, &path_string, &axon_name ) )
	{
		Neuron* located_neuron = nullptr;
		if ( _nucleus->Host()->Find_Neuron ( &path_string, &located_neuron ) )
			located_neuron->Get_Axon ( &axon_name, &located_axon );
	}
	else
		_nucleus->Host()->Hunt_Axon ( &axon_name, &located_axon );

	return located_axon;
}

void XEPL::Cortex::Register_Mutual ( Text* _chars, Mutual _mutual )
{
	String name_cord ( _chars );

	auto [it, noob] = mutual_map->insert_or_assign ( name_cord, _mutual );

	if (!noob)
		ErrorReport error_report("Replaced mutual: ", &name_cord);
}

void XEPL::Cortex::Register_Operator ( Text* _chars, Operator _operator )
{
	if ( !_chars )
		return;

	Cord operator_name ( _chars );

	auto [it, noob] = operators_map->insert_or_assign ( operator_name, _operator );

	if (!noob)
		ErrorReport error_report("Replaced operator: ", &operator_name);

	TRACE( "New_Operator", nullptr, &operator_name);
}

bool XEPL::Cortex::Did_Operator ( Cord* _operator_name, Script* _script, Cord* _param )
{
	auto it  = operators_map->find ( *_operator_name );
	if ( it == operators_map->end() )
		return false;

	it->second ( _script, _param );

	return true;
}

void XEPL::Cortex::Register_Render ( Text* _chars, Render _render )
{
	Cord name_cord(_chars);

	auto [it, noob] = render_map->insert_or_assign( name_cord, _render );

	if (!noob)
		ErrorReport error_report("Replaced render: ", &name_cord);

	TRACE( "New_Render", nullptr, &name_cord);
}

bool XEPL::Cortex::Did_Render ( Nucleus* _nucleus, Gene* _gene, Rendon* _rendon )
{
	DuplicateTraits duplicate ( _gene );
	duplicate.gene->Evaluate_Traits ( _nucleus );

	auto it  = render_map->find ( duplicate.gene->cell_name );
	if ( it == render_map->end() )
		return _nucleus->Took_Action(duplicate.gene);

	it->second ( _nucleus, duplicate.gene, _rendon );
	return true;
}

//    .88888.
//   d8'   `88
//   88        .d8888b. 88d888b. .d8888b.
//   88   YP88 88ooood8 88'  `88 88ooood8
//   Y8.   .88 88.  ... 88    88 88.  ...
//    `88888'  `88888P' dP    dP `88888P'

XEPL::Gene::~Gene()
{
	delete space_string;
	delete traits;
	if ( !Test_Flags(dupe_flag) )
	{
		delete inner_genes;

		if ( content_wire )
			content_wire->Release();

		delete content_mutex;
	}
	if(auto lobe=tlsLobe)--lobe->counters.count_genes;
}

XEPL::Gene::Gene ( Text* _xml )
	: Cell ( "")
	, content_wire    ( nullptr )
	, traits          ( nullptr )
	, inner_genes     ( nullptr )
	, space_string    ( nullptr )
	, content_mutex   ( new Mutex() )
	, owner_link      ( nullptr )
{
	++tlsLobe->counters.count_genes;

	Cord text ( _xml );
	Gene* parsed_gene = new Gene( nullptr, "text", nullptr);

	XmlParser parser ( parsed_gene, &text );

	if ( parser.ParseIt() )
	{
		Gene* gene=parsed_gene->First();

		const_cast<String*>(cell_name)->assign( *gene->cell_name );
		
		if ( gene->space_string )
			space_string = new String( gene->space_string );

		Absorb_Gene ( gene );
	}
	parsed_gene->Release();
}

XEPL::Gene::Gene ( Gene* _parent, Cord* _name, Cord* _space )
	: Cell ( _name )
	, content_wire    ( nullptr )
	, traits          ( nullptr )
	, inner_genes     ( nullptr )
	, space_string    ( _space ? new String ( _space ) : nullptr )
	, content_mutex   ( new Mutex() )
	, owner_link      ( _parent )
{
	if(auto lobe=tlsLobe)++lobe->counters.count_genes;

	if ( _parent )
	{
		_parent->Add_Gene ( this );
		Release();
	}
}

XEPL::Gene::Gene ( Gene* _parent, Text* _name, Cord* _space )
	: Cell ( _name )
	, content_wire    ( nullptr )
	, traits          ( nullptr )
	, inner_genes     ( nullptr )
	, space_string    ( _space ? new String ( _space ) : nullptr )
	, content_mutex   ( new Mutex() )
	, owner_link      ( _parent )
{
	if(auto lobe=tlsLobe)++lobe->counters.count_genes;

	if ( _parent )
	{
		_parent->Add_Gene ( this );
		Release();
	}
}

XEPL::Gene::Gene ( Gene* _parent, Cord* _name )
	: Gene( _parent, _name, nullptr )
{}

void XEPL::Gene::Deflate_Gene ( void )
{
	MutexScope lock_Contents ( content_mutex );

	if ( content_wire )
	{
		content_wire->Release();
		content_wire=nullptr;
	}

	delete traits;
	delete inner_genes;

	inner_genes = nullptr;
	traits      = nullptr;
}

void XEPL::Gene::Print_Into ( String* _string, int _depth )
{
	if ( !_string )
		return;

	XmlBuilder x1 ( cell_name, _string, space_string );

	{
		MutexScope lock_Contents ( content_mutex );

		if ( traits )
			traits->Print_Into ( _string );

		if ( content_wire )
		{
			x1.Close_Attributes();

			content_wire->Print_Into ( _string );
		}
	}

	if ( --_depth )
	{
		if ( inner_genes )
		{
			x1.Close_Attributes();
			inner_genes->Print_Into ( _string, _depth );
		}
	}
}


bool XEPL::Gene::Has_Content( void )
{
	MutexScope lock_Contents ( content_mutex );

	return !!content_wire;
}

bool XEPL::Gene::Copy_Content( String* _into )
{
	MutexScope lock_Contents ( content_mutex );

	if ( !content_wire )
		return false;

	_into->append( *content_wire->wire_string );
	return true;
}

XEPL::String* XEPL::Gene::Make_Content ( void )
{
	MutexScope lock_Contents ( content_mutex );

	if ( !content_wire )
		content_wire = new Wire();

	return content_wire->wire_string;
}

void XEPL::Gene::Assign_Content ( Cord* _cord )
{
	MutexScope lock_Contents ( content_mutex );
	
	if ( !content_wire )
		content_wire = new Wire();

	content_wire->Assign ( _cord );
}

void XEPL::Gene::Append_Content ( Wire* _wire )
{
	if ( !_wire )
		return;

	MutexScope lock_Contents ( content_mutex );

	if ( !content_wire )
		content_wire = new Wire();

	content_wire->Append ( _wire );
}

void XEPL::Gene::Append_Content ( Text* _chars, long _length )
{
	MutexScope lock_Contents ( content_mutex );

	if ( !content_wire )
		content_wire = new Wire();

	content_wire->Append(_chars, _length);
}

void XEPL::Gene::Append_Content ( Cord* _cord )
{
	if ( !_cord )
		return;

	MutexScope lock_Contents ( content_mutex );

	if ( !content_wire )
		content_wire = new Wire();

	content_wire->Append ( _cord );
}


XEPL::Gene* XEPL::Gene::First( void )
{
	MutexScope lock_Contents ( content_mutex );

	if ( inner_genes )
		return static_cast<Gene*>( inner_genes->head_bond->atom );

	return nullptr;
}

XEPL::Gene* XEPL::Gene::Make_One( Text*       _chars )
{
	Cord tag( _chars );

	MutexScope lock_Contents ( content_mutex );

	Gene* gene = nullptr;
	if ( inner_genes && inner_genes->Find_Gene ( &tag, &gene ) )
		return gene;

	return new Gene ( this, &tag, nullptr );
}

bool XEPL::Gene::Make_One_Gene ( Cord* _cord, Gene** _gene )
{
	MutexScope lock_Contents ( content_mutex );

	if ( Get_First_Gene ( _cord, _gene ) )
		return false;

	*_gene = new Gene ( this, _cord, nullptr );

	return true;
}

bool XEPL::Gene::Make_One_Gene ( Text*       _chars, Gene** _gene )
{
	if ( !_chars )
	{
		*_gene = this;
		return false;
	}
	else
	{
		String name ( _chars );
		return Make_One_Gene ( &name, _gene );
	}
}

void XEPL::Gene::Add_Gene ( Gene* _gene )
{
	if ( !_gene )
		return;

	MutexScope lock_Contents ( content_mutex );

	if ( !inner_genes )
		inner_genes = new Genes(_gene);
	else
		inner_genes->Add_Gene ( _gene );
}

XEPL::Gene* XEPL::Gene::Get_First ( Text* _chars )
{
	if ( !inner_genes )
		return nullptr;

	Gene* gene = nullptr;
	Cord name_cord( _chars );

	MutexScope lock_Contents ( content_mutex );

	inner_genes->Find_Gene ( &name_cord, &gene );

	return gene;
}

bool XEPL::Gene::Get_First_Gene ( Cord* _cord, Gene** _gene )
{
	MutexScope lock_Contents ( content_mutex );

	if ( !inner_genes )
		return false;

	return inner_genes->Find_Gene ( _cord, _gene );
}

bool XEPL::Gene::Get_First_Gene ( Text* _chars, Gene** _gene )
{
	if ( !inner_genes )
		return false;
	
	Cord name_cord( _chars );

	MutexScope lock_Contents ( content_mutex );

	return inner_genes->Find_Gene ( &name_cord, _gene );
}

void XEPL::Gene::Remove_Gene ( Gene* _gene )
{
	if ( !inner_genes || !_gene  )
		return;

	MutexScope lock_Contents ( content_mutex );

	auto it  = inner_genes->bond_map->find ( _gene );
	if ( it == inner_genes->bond_map->end() )
		return;

	Bond* bond = it->second;

	inner_genes->Remove_Bond ( bond );

	if ( !inner_genes->bond_map )
	{
		delete inner_genes;
		inner_genes=nullptr;
	}
}

bool XEPL::Gene::Replace_Gene ( Cord* _cord, Gene* _gene )
{
	MutexScope lock_Contents ( content_mutex );

	Gene* gene = nullptr;

	if ( Get_First_Gene ( _cord, &gene ) )
		Remove_Gene ( gene );

	Add_Gene ( _gene );

	return gene != nullptr;
}

void XEPL::Gene::Duplicate_Gene ( Gene** _gene )
{
	Gene* cloned_gene = new Gene ( nullptr, cell_name, space_string );

	delete cloned_gene->content_mutex;
	cloned_gene->content_mutex  = content_mutex;

	cloned_gene->owner_link     = owner_link ;
	cloned_gene->content_wire   = content_wire;
	cloned_gene->inner_genes    = inner_genes;

	cloned_gene->Set_Flags( dupe_flag );

	*_gene = cloned_gene;

	MutexScope lock_Traits   ( content_mutex );

	if( traits )
		traits->Duplicate_Into ( &cloned_gene->traits );
}

void XEPL::Gene::Absorb_Gene ( Gene* _gene )
{
	if ( !_gene )
		return;

	if ( _gene->inner_genes )
	{
		StableGenes recall ( _gene );

		MutexScope lock_Contents ( content_mutex );

		if ( !inner_genes )
			inner_genes = new Genes ();

		Gene* gene = nullptr;
		while ( recall.Next_Gene ( &gene ) )
			inner_genes->Add_Gene ( gene );
	}

	if ( _gene->content_wire)
		Append_Content ( _gene->content_wire );

	if ( _gene->traits )
		Absorb_Traits ( _gene );
}

void XEPL::Gene::Copy_Genes_Into ( GeneChain** _chain )
{
	if ( !inner_genes )
	{
		_chain = nullptr;
		return;
	}

	MutexScope lock_Contents ( content_mutex );

	*_chain = new GeneChain ( inner_genes );
}

//  8""""8                          ""8""
//  8    " eeee eeeee eeee            8   eeeee  eeeee e eeeee eeeee
//  8e     8    8   8 8               8e  8   8  8   8 8   8   8   "
//  88  ee 8eee 8e  8 8eee   eeee     88  8eee8e 8eee8 8e  8e  8eeee
//  88   8 88   88  8 88              88  88   8 88  8 88  88     88
//  88eee8 88ee 88  8 88ee            88  88   8 88  8 88  88  8ee88

void XEPL::Gene::Trait_Set ( Cord* _cord1, Cord* _cord2 )
{
	MutexScope lock_Traits   ( content_mutex );

	if ( !traits )
		traits=new Traits();

	traits->Set_Trait ( _cord1, _cord2 );
}

XEPL::Cord* XEPL::Gene::Trait_Get ( Cord* _cord, String* _string )
{
	MutexScope lock_Traits   ( content_mutex );

	if ( !traits )
		return nullptr;

	auto it  = traits->map_of_traits->find ( *_cord );
	if ( it == traits->map_of_traits->end() )
		return nullptr;

	_string->assign ( * it->second->trait_term );
	return _string;
}

XEPL::Cord* XEPL::Gene::Trait_Raw ( Cord* _cord )
{
	MutexScope lock_Traits   ( content_mutex );

	if ( !traits )
		return nullptr;

	auto it  = traits->map_of_traits->find ( *_cord );
	if ( it != traits->map_of_traits->end() )
		return it->second->trait_term;

	return nullptr;
}

XEPL::Text* XEPL::Gene::Trait_Default ( Text* _name_chars, Text*       _default_chars )
{
	Cord* trait_value = Trait_Raw ( _name_chars );
	if ( trait_value)
		return trait_value->c_str();

	return _default_chars;
}

XEPL::Cord* XEPL::Gene::Trait_Tap ( Text* _chars, Text*       _default )
{
	Cord trait_name ( _chars );

	MutexScope lock_Traits   ( content_mutex );

	if ( !traits )
		traits=new Traits();

	auto it  = traits->map_of_traits->find ( trait_name );
	if ( it == traits->map_of_traits->end() )
	{
		Trait_Set ( &trait_name, _default );
		return Trait_Raw ( &trait_name );
	}
	return it->second->trait_term;
}

void XEPL::Gene::Absorb_Traits ( Gene* _gene )
{
	if ( !_gene || !_gene->traits )
		return;

	MutexScope lock_their_Traits ( _gene->content_mutex );
	MutexScope lock_Traits       (        content_mutex );

	if ( !traits )
		traits = new Traits();

	Trait* their_trait = _gene->traits->first_trait;

	while ( their_trait )
	{
		traits->first_trait = new Trait ( their_trait, traits->first_trait );

		traits->map_of_traits->insert_or_assign(*their_trait->trait_name, traits->first_trait);

		their_trait = their_trait->next_trait;
	}
}

void XEPL::Gene::Evaluate_Traits ( Nucleus* _nucleus )
{
	MutexScope lock_Traits   ( content_mutex );

	if ( !traits )
		return;

	traits->Evaluate ( this, _nucleus );
}

bool XEPL::Gene::Duplicate_Traits ( Traits** _traits )
{
	MutexScope lock_Traits   ( content_mutex );

	if ( !traits )
		return false;

	traits->Duplicate_Into ( _traits );

	return true;
}

XEPL::Cord* XEPL::Gene::Trait_Raw ( Text* _chars )
{
	Cord name ( _chars );
	return Trait_Raw ( &name );
}

XEPL::Cord* XEPL::Gene::Trait_Get ( Text* _chars, String* _string )
{
	if ( !traits )
		return nullptr;

	String name ( _chars );
	return Trait_Get ( &name, _string );
}

void XEPL::Gene::Trait_Set ( Text* _chars1, Text* _chars2 )
{
	String name ( _chars1 );
	String val ( _chars2 );
	Trait_Set ( &name, &val );
}

void XEPL::Gene::Trait_Set ( Text* _chars, Cord* _cord )
{
	String name ( _chars );
	Trait_Set ( &name, _cord );
}

void XEPL::Gene::Trait_Set ( Cord* _cord, Text* _chars )
{
	String val ( _chars );
	Trait_Set ( _cord, &val );
}

//    .88888.
//   d8'   `88
//   88        .d8888b. 88d888b. .d8888b. .d8888b.
//   88   YP88 88ooood8 88'  `88 88ooood8 Y8ooooo.
//   Y8.   .88 88.  ... 88    88 88.  ...       88
//    `88888'  `88888P' dP    dP `88888P' `88888P'

XEPL::Genes::~Genes()
{
	if ( !head_bond )
		return;

	delete chain_map;
	delete bond_map;
}

XEPL::Genes::Genes ()
	: GeneChain ( this )
	, bond_map  ( nullptr )
	, chain_map ( nullptr )
{}

XEPL::Genes::Genes ( Gene* _gene)
	: GeneChain ( this )
	, bond_map  ( new BondMap() )
	, chain_map ( new ChainMap() )
{
	Chain* chain = new Chain ( _gene );

	chain_map->emplace ( *_gene->cell_name, chain              );
	bond_map->emplace  (  _gene,            Add_Atom ( _gene ) );
}

void XEPL::Genes::Flush ( void )
{
	delete chain_map;
	delete bond_map;
	
	chain_map = nullptr;
	bond_map  = nullptr;
}

void XEPL::Genes::Add_Gene ( Gene* _gene )
{
	if ( !_gene )
		return;

	if ( !bond_map )
	{
		bond_map  = new BondMap();
		chain_map = new ChainMap();
	}

	auto [bond_it, bond_noob] = bond_map->try_emplace( _gene );
	if (bond_noob)
	{
		bond_it->second = Add_Atom(_gene);

		auto [chain_it, chain_noob] = chain_map->try_emplace( *_gene->cell_name );

		if (chain_noob)
			chain_it->second = new Chain(_gene);
		else
			chain_it->second->Add_Atom(_gene);
	}
}

bool XEPL::Genes::Find_Gene ( Cord* _cord, Gene** _gene )
{
	if ( !chain_map )
		return false;

	auto it  = chain_map->find ( *_cord );
	if ( it == chain_map->end() )
		return false;

	*_gene = static_cast<Gene*> ( it->second->head_bond->atom );

	return true;
}

void XEPL::Genes::Remove_Bond ( Bond* _bond )
{
	if ( !_bond )
		return;

	Gene* remove_gene = static_cast<Gene*> ( _bond->atom );

	remove_gene->owner_link = nullptr;
	bond_map->erase ( _bond->atom );

	auto it  = chain_map->find ( *remove_gene->cell_name );
	if ( it != chain_map->end() && it->second->Remove_Atom ( _bond->atom ) )
	{
		delete it->second;
		chain_map->erase ( it );
	}

	GeneChain::Remove_Bond ( _bond );

	if ( bond_map->empty() )
		Flush();
}

void XEPL::Genes::Print_Into ( String* _string, int _depth )
{
	if ( !_string )
		return;

	GeneChain memos ( this );
	Gene* gene = nullptr;

	while ( memos.Next ( &gene ) )
		gene->Print_Into ( _string, _depth );
}

//    .88888.                              a88888b. dP                oo
//   d8'   `88                            d8'   `88 88
//   88        .d8888b. 88d888b. .d8888b. 88        88d888b. .d8888b. dP 88d888b.
//   88   YP88 88ooood8 88'  `88 88ooood8 88        88'  `88 88'  `88 88 88'  `88
//   Y8.   .88 88.  ... 88    88 88.  ... Y8.   .88 88    88 88.  .88 88 88    88
//    `88888'  `88888P' dP    dP `88888P'  Y88888P' dP    dP `88888P8 dP dP    dP

XEPL::GeneChain::~GeneChain()
{
	if ( current_gene )
		current_gene->Release();

	if ( !head_bond )
		return;

	Atom* atom = nullptr;

	while ( Pull_Atom ( &atom ) )
		atom->Release();
}

XEPL::GeneChain::GeneChain ( const Chain* _chain )
	: Chain   ( false )
	, current_gene  ( nullptr )
{
	if ( !_chain )
		return;

	if ( !_chain->head_bond )
		return;

	MutexScope lock_chain ( _chain->chain_lock );

	Bond* prev = nullptr;
	Bond* bond = _chain->head_bond;

	while ( bond )
	{
		Bond* bond_clone = new Bond ( bond->atom, prev );

		if ( prev )
			prev->next_bond = bond_clone;
		else
			head_bond = bond_clone;

		prev = bond_clone;
		bond = bond->next_bond;
	}

	tail_bond = prev;
}

bool XEPL::GeneChain::Next ( Gene** _gene )
{
	if ( current_gene )
		current_gene->Release();

	Atom* atom = nullptr;

	if ( head_bond && Pull_Atom ( &atom ) )
	{
		current_gene = static_cast<Gene*> ( atom );
		*_gene = current_gene;
		return true;
	}

	current_gene = nullptr;
	return false;
}

//    .88888.                             .d88888b
//   d8'   `88                            88.    "'
//   88        .d8888b. 88d888b. .d8888b. `Y88888b. .d8888b. .d8888b. 88d888b. .d8888b.
//   88   YP88 88ooood8 88'  `88 88ooood8       `8b 88'  `"" 88'  `88 88'  `88 88ooood8
//   Y8.   .88 88.  ... 88    88 88.  ... d8'   .8P 88.  ... 88.  .88 88.  .88 88.  ...
//    `88888'  `88888P' dP    dP `88888P'  Y88888P  `88888P' `88888P' 88Y888P' `88888P'
//                                                                    88

XEPL::GeneScope::GeneScope ( Gene*& _gene, Gene* _replacement_gene )
	: gene_reference     ( _gene )
	, the_previous_gene  ( gene_reference )
{
	gene_reference = _replacement_gene;
}

XEPL::GeneScope::~GeneScope ( void )
{
	gene_reference = the_previous_gene;
}

//   d888888P                   oo   dP
//      88                           88
//      88    88d888b. .d8888b. dP d8888P
//      88    88'  `88 88'  `88 88   88
//      88    88       88.  .88 88   88
//      dP    dP       `88888P8 dP   dP

XEPL::Trait::~Trait( void )
{
	delete trait_name;
	delete trait_term;
	--tlsLobe->counters.count_traits;
}

XEPL::Trait::Trait ( Cord* _name, Cord* _term, Trait* _next_trait )
	: next_trait ( _next_trait )
	, trait_name ( new Cord   ( _name ) )
	, trait_term ( new String ( _term ) )
{
	++tlsLobe->counters.count_traits;
}

XEPL::Trait::Trait ( Trait* _trait, Trait* _next_trait )
	: next_trait  ( _next_trait )
	, trait_name  ( new Cord   ( _trait->trait_name ) )
	, trait_term  ( new String ( _trait->trait_term ) )
{
	++tlsLobe->counters.count_traits;
}

void XEPL::Trait::Print_Into ( String* _into_string )
{
	if ( !_into_string )
		return;

	_into_string->push_back ( ' ' );
	_into_string->append ( *trait_name );
	_into_string->push_back ( '=' );
	Escape_Quotes ( trait_term, _into_string );
}

//   d888888P                   oo   dP
//      88                           88
//      88    88d888b. .d8888b. dP d8888P .d8888b.
//      88    88'  `88 88'  `88 88   88   Y8ooooo.
//      88    88       88.  .88 88   88         88
//      dP    dP       `88888P8 dP   dP   `88888P'
//
//
XEPL::Traits::~Traits()
{
	delete map_of_traits;

	Trait* trait = first_trait;
	while ( trait )
	{
		Trait* next = trait->next_trait;
		delete trait;
		trait = next;
	}
}

XEPL::Traits::Traits()
	: first_trait   ( nullptr )
	, map_of_traits ( new TraitMap() )
{}

void XEPL::Traits::Set_Trait ( Cord* _name_cord, Cord* _term_cord )
{
	auto [it, noob] = map_of_traits->try_emplace( *_name_cord );
	if (noob)
	{
		first_trait = new Trait(_name_cord, _term_cord, first_trait);
		it->second = first_trait;
	}
	else
		it->second->trait_term->assign( *_term_cord );
}

void XEPL::Traits::Evaluate ( Gene* _gene, Nucleus* _nucleus )
{
	Trait* trait = first_trait;
	auto host = _nucleus->Host();
	while ( trait )
	{
		if ( trait->trait_term->front() == '{' )
			Script ( host, _gene, trait->trait_term, trait->trait_term );

		trait = trait->next_trait;
	}
}

void XEPL::Traits::Duplicate_Into ( Traits** _traits )
{
	Traits* clone_traits = new Traits();
	Trait*  trait = first_trait;

	while ( trait )
	{
		clone_traits->first_trait = new Trait ( trait, clone_traits->first_trait );
		clone_traits->map_of_traits->emplace( *trait->trait_name, clone_traits->first_trait);

		trait = trait->next_trait;
	}
	*_traits = clone_traits;
}

void XEPL::Traits::Print_Into ( String* _string )
{
	Trait* trait = first_trait;
	while ( trait )
	{
		trait->Print_Into ( _string );
		trait = trait->next_trait;
	}
}

//   8888ba.88ba             dP   dP                      dP
//   88  `8b  `8b            88   88                      88
//   88   88   88 .d8888b. d8888P 88d888b. .d8888b. .d888b88
//   88   88   88 88ooood8   88   88'  `88 88'  `88 88'  `88
//   88   88   88 88.  ...   88   88    88 88.  .88 88.  .88
//   dP   dP   dP `88888P'   dP   dP    dP `88888P' `88888P8
//
XEPL::Method::~Method()
{
	method_gene->Release();
}

XEPL::Method::Method ( Function _function, Cord* _cord, Gene* _gene )
: cell_function ( _function )
, method_gene   ( nullptr )
{
	if ( _gene )
	{
		method_gene = new Gene ( nullptr, _cord, _gene->space_string );
		method_gene->Absorb_Gene ( _gene );
	}
	else
		method_gene = new Gene ( nullptr, _cord, nullptr );
}

void XEPL::Method::Perform ( Nucleus* _nucleus, Gene* _call_gene )
{
	if ( method_gene->traits )
		ShortTerms::Replace_Traits( method_gene  );

	if ( _call_gene && _call_gene->traits )
		ShortTerms::Replace_Traits( _call_gene  );

	( _nucleus->*cell_function ) ( _call_gene, method_gene );
}

class XEPL::MethodMap : public MapDeleteT<Cord, Method*> {};

//   888888ba                    dP
//   88    `8b                   88
//   88     88 dP    dP .d8888b. 88 .d8888b. dP    dP .d8888b.
//   88     88 88    88 88'  `"" 88 88ooood8 88    88 Y8ooooo.
//   88     88 88.  .88 88.  ... 88 88.  ... 88.  .88       88
//   dP     dP `88888P' `88888P' dP `88888P' `88888P' `88888P'

XEPL::Nucleus::~Nucleus()
{
	if ( parent_neuron )
	{
		parent_neuron->observer->Remove_Gene(observer);
		parent_neuron->shadows->Remove_Gene(shadows);
		parent_neuron->Release();
	}
	else
	{
		observer->Release();
		shadows->Release();
	}

	delete method_map;
}

XEPL::Nucleus::Nucleus ( Neuron* _neuron, Cord* _name )
	: method_map    ( nullptr )
	, parent_neuron ( _neuron )
	, observer      ( new Gene( _neuron?_neuron->observer:nullptr, "Observer", _name ) )
	, shadows       ( new Gene( _neuron?_neuron->shadows:nullptr, "Shadows", _name ))
{
	if ( parent_neuron )
		parent_neuron->Attach();
}

void XEPL::Nucleus::Nucleus_Path ( String*, const char )
{}

XEPL::Neuron* XEPL::Nucleus::Host ( void )
{
	return parent_neuron;
}

void XEPL::Nucleus::Nucleus_Dropped()
{}

bool XEPL::Nucleus::Nucleus_Rendered ( Rendon* _rendon, Cord* _cord )
{
	Gene* form_gene = nullptr;
	if ( Nucleus_Viewer( _cord, &form_gene ) )
	{
		_rendon->Generate_Payload ( this, form_gene );
		form_gene->Release();
		return true;
	}

	if ( Form_Get ( _cord, &form_gene ) )
	{
		_rendon->Generate_Payload ( this, form_gene );
		return true;
	}
	
	return false;
}


bool XEPL::Nucleus::Nucleus_Viewer ( Cord*, Gene** )
{
	return false;
}



void XEPL::Nucleus::Register_Form ( Gene* _gene )
{
	if ( !_gene )
		return;

	Gene* gene = nullptr;
	if ( !shadows->Make_One("Forms")->Make_One_Gene ( _gene->cell_name, &gene ) )
		gene->Deflate_Gene();

	gene->Absorb_Gene ( _gene );
}

bool XEPL::Nucleus::Form_Get ( Cord* _cord, Gene** _gene )
{
	Gene* forms_gene = shadows->Get_First("Forms");
	if ( !forms_gene )
		return false;

	return forms_gene->Get_First_Gene ( _cord, _gene );
}

void XEPL::Nucleus::Register_Gene ( Cord* _cord, Gene* _method_gene )
{
	TRACE( "Name_Gene", Host(), _cord );

	Gene* vitals_gene = nullptr;

	observer->Make_One_Gene("Vitals", &vitals_gene);

	vitals_gene->Replace_Gene ( _cord, _method_gene );
}

void XEPL::Nucleus::Register_Gene ( Text*       _chars, Gene* _method_gene )
{
	Cord name_cord ( _chars );
	return Register_Gene ( &name_cord, _method_gene );
}



void XEPL::Nucleus::Property_Set ( Cord* _name, Cord* _val )
{
	Gene* vitals_gene = nullptr;

	observer->Make_One_Gene("Vitals", &vitals_gene);

	vitals_gene->Trait_Set ( _name, _val );
}

void XEPL::Nucleus::Property_Set ( Text* _name, Cord* _val )
{
	Cord name ( _name );
	return Property_Set ( &name, _val );
}

bool XEPL::Nucleus::Property_Get ( Cord* _as, String* _into )
{
	Gene* vitals_gene = observer->Get_First("Vitals");
	if ( vitals_gene )
		return vitals_gene->Trait_Get ( _as, _into );

	return false;
}

bool XEPL::Nucleus::Property_Hunt ( Cord* _as, String* _into )
{
	Gene* vitals_gene = observer->Get_First("Vitals");
	if( vitals_gene && vitals_gene->Trait_Get ( _as, _into ) )
		return true;

	if ( parent_neuron )
		return parent_neuron->Property_Hunt ( _as, _into );

	return false;
}



void XEPL::Nucleus::Register_Method ( Cord* _name, Function _function, Gene* _gene )
{
	TRACE( _gene?"Xml_Method":"Cpp_Method", Host(), _name );

	if ( !_name )
		return;

	if ( _gene )
		shadows->Make_One("Methods")->Add_Gene(_gene);

	if ( !method_map )
		method_map   = new MethodMap();

	Method* method = new Method(_function, _name, _gene);

	auto [it,noob] = method_map->try_emplace(*_name);
	if ( !noob )
		delete it->second;

	it->second = method;
}

void XEPL::Nucleus::Register_Method ( Text* _name, Function _function, Gene* _gene )
{
	String name ( _name );
	return Register_Method ( &name, _function, _gene );
}

void XEPL::Nucleus::Method_Execute ( Gene* _call_gene, Gene* _code_gene )
{
	if ( _call_gene && _call_gene->Has_Content() )
		Script ( Host(), _call_gene );

	if ( _code_gene->Has_Content() )
		Script ( Host(), _code_gene );

	if ( _code_gene->inner_genes )
		Process_Inner_Genes( _code_gene );
}

bool XEPL::Nucleus::Performed_Method ( Cord* _name_cord, Gene* _call_gene )
{
	if ( !method_map )
		return false;

	auto it  = method_map->find ( *_name_cord );
	if ( it == method_map->end() )
		return false;

	if ( Show_Trace )
	{
		String trace_string;
		if ( _call_gene)
			_call_gene->Print_Into(&trace_string, 1);
		else
			trace_string.assign(*_name_cord);
		TRACE( "ENTR_Method", Host(), &trace_string );
	}

	it->second->Perform ( this, _call_gene );

	return true;
}

bool XEPL::Nucleus::Performed_Method ( Text* _chars, Gene* _gene )
{
	Cord name_cord ( _chars );
	return Performed_Method ( &name_cord, _gene );
}



bool XEPL::Nucleus::Nucleus_Processed ( Nucleus* _nucleus, Gene* _duplicate_gene )
{
	if ( !_nucleus )
		return false;

	return _nucleus->Process_Gene ( _duplicate_gene );
}

bool XEPL::Nucleus::Took_Action(Gene* _call_gene)
{
	if ( cortex->Did_Keyword ( this, _call_gene ) )
		return true;

	if ( this->Performed_Method ( _call_gene->cell_name, _call_gene ) )
		return true;

	if ( cortex->Did_Dot_Tag ( this, _call_gene ) )
		return true;

	return false;
}

bool XEPL::Nucleus::Process_Gene ( Gene* _dupliclate_gene )
{
	{
		if ( _dupliclate_gene->traits )
			_dupliclate_gene->Evaluate_Traits ( this );

		ShortTerms nesting_here ((Gene*)nullptr);

		if ( this->Took_Action(_dupliclate_gene))
			return true;
	}

	if ( auto active_rendon = tlsLobe->active_rendon )
		return active_rendon->Nucleus_Processed ( this, _dupliclate_gene );

	return false;
}

void XEPL::Nucleus::Process_Inner_Genes ( Gene* _gene )
{
	if ( !_gene->inner_genes )
		return;

	Bond* gene_bond = _gene->inner_genes->head_bond;

	while ( gene_bond )
	{
		Gene* gene = static_cast<Gene*>(gene_bond->atom );
		
		bool processed = false;
		if ( gene->traits )
		{
			DuplicateTraits duplicate ( gene);
			processed = Process_Gene ( duplicate.gene );
		}
		else
			processed = Process_Gene( gene );

		if ( !processed )
			xeplCantFind ( "Statement", this, gene->cell_name );

		gene_bond = gene_bond->next_bond;
	}
}

bool XEPL::Nucleus::Process_Exact_Gene ( Cord* _cord, Gene* _host_gene )
{
	Gene* matching_gene = nullptr;

	if ( !_host_gene->Get_First_Gene ( _cord, &matching_gene ) )
		return false;

	Script ( Host(), matching_gene );

	if ( matching_gene->inner_genes )
		Process_Inner_Genes ( matching_gene );

	return true;
}

bool XEPL::Nucleus::Process_Exact_Gene ( Text* _chars, Gene* _host_gene )
{
	Cord name_cord ( _chars );
	return Process_Exact_Gene ( &name_cord, _host_gene );
}

//   888888ba
//   88    `8b
//   88     88 .d8888b. dP    dP 88d888b. .d8888b. 88d888b.
//   88     88 88ooood8 88    88 88'  `88 88'  `88 88'  `88
//   88     88 88.  ... 88.  .88 88       88.  .88 88    88
//   dP     dP `88888P' `88888P' dP       `88888P' dP    dP

XEPL::Neuron::~Neuron()
{
	TRACE( "Delete", parent_neuron, cell_name );
	if(auto lobe=tlsLobe)--lobe->counters.count_neurons;
}

XEPL::Neuron::Neuron ( Text* _name )
	: Cell      ( _name )
	, Nucleus         ( nullptr, cell_name )
	, receptor_chain  ( nullptr )
	, axon_chain      ( nullptr )
	, axon_map        ( nullptr )
	, relay_map       ( nullptr )
	, neuron_map      ( nullptr )
	, neuron_chain    ( nullptr )
	, alias           ( nullptr )
{
	if(auto lobe=tlsLobe)++lobe->counters.count_neurons;
}

XEPL::Neuron::Neuron ( Neuron*  _parent_neuron, Gene* _config_gene )
	: Cell      ( _config_gene->Trait_Default ( "name", _config_gene->cell_name->c_str() ) )
	, Nucleus         ( _parent_neuron, cell_name )
	, receptor_chain  ( nullptr )
	, axon_chain      ( nullptr )
	, axon_map        ( nullptr )
	, relay_map       ( nullptr )
	, neuron_map      ( nullptr )
	, neuron_chain    ( nullptr )
	, alias           ( nullptr )
{
	++tlsLobe->counters.count_neurons;

	XEPL::String content;
	if ( _config_gene->Copy_Content(&content) )
		Script( this, _config_gene, &content );

	if ( parent_neuron->neuron_map )
		parent_neuron->Drop_Neuron ( cell_name );

	parent_neuron->Register_Neuron ( this );
}

void XEPL::Neuron::Nucleus_Dropped()
{
	if ( Test_Flags( dropped_flag ) )
		return;

	Set_Flags( dropped_flag );

	Performed_Method ( "Finished", nullptr );

	Drop_My_Receptors();
	Drop_My_Neurons();
	Drop_My_Axons();

	if ( parent_neuron )
		parent_neuron->Unregister_Neuron ( this );

	Nucleus::Nucleus_Dropped();

	Release();
}


XEPL::Neuron* XEPL::Neuron::Host ( void )
{
	return this;
}

void XEPL::Neuron::Nucleus_Path ( String* _string, const char _char )
{
	if ( parent_neuron )
	{
		parent_neuron->Nucleus_Path ( _string, _char );

		if ( !_string->empty() )
			_string->push_back ( _char );
	}
	_string->append ( *cell_name );
}

void XEPL::Neuron::Method_Terminate ( Gene*, Gene* )
{
	Nucleus_Dropped();
}


bool XEPL::Neuron::Feature_Get ( Cord* _feature_cord, String* _build_string )
{
	if ( _feature_cord->empty() )
	{
		_build_string->append(*Host()->cell_name);
		return true;
	}
	
	if ( _feature_cord->compare ( "path" ) == 0 )
	{
		_build_string->push_back ( '/' );
		Nucleus_Path ( _build_string, '/' );
		return true;
	}

	if ( _feature_cord->compare( "neurons" ) == 0 )
	{
		XmlBuilder builder( "neurons", _build_string );
		builder.Close_Attributes();
		Show_Neurons( _build_string );
		builder.Close();
		return _build_string;
	}
	return false;
}

bool XEPL::Neuron::Feature_Get ( Text* _chars, String* _string )
{
	Cord feature_cord ( _chars );
	return Feature_Get ( &feature_cord, _string );
}



bool XEPL::Neuron::Drop_Neuron(Text* _chars )
{
	Cord name_cord(_chars);
	return Drop_Neuron( &name_cord );
}

bool XEPL::Neuron::Drop_Neuron ( Cord* _cord )
{
	auto it  = neuron_map->find ( *_cord );
	if ( it == neuron_map->end() )
		return false;

	it->second->Nucleus_Dropped();

	return true;
}

void XEPL::Neuron::Drop_My_Neurons()
{
	if ( neuron_chain )
	{
		while ( Neuron* neuron = neuron_chain->Last() )
			neuron->Nucleus_Dropped();
	}
	delete neuron_chain;
	delete neuron_map;
	neuron_chain = nullptr;
	neuron_map   = nullptr;
}

void XEPL::Neuron::Register_Neuron ( Neuron* _neuron )
{
	Cord* neuron_name = _neuron->cell_name;

	TRACE( "New_Neuron", this, neuron_name );

	if ( !neuron_chain )
	{
		neuron_chain = new NeuronChain();
		neuron_map   = new NeuronMap();
	}
	MutexScope lock_chain( neuron_chain->chain_lock );

	neuron_chain->Add_Atom ( _neuron );

	auto [it, noob] = neuron_map->try_emplace ( *neuron_name );
	if ( !noob )
	{
		it->second->Release();
		TRACE( "Replace_Neuron", this, neuron_name );
	}
	it->second = _neuron;
}

void XEPL::Neuron::Unregister_Neuron ( Neuron* _neuron )
{
	TRACE( "Rem_Neuron", this, _neuron->cell_name );

	if ( neuron_chain )
	{
		MutexScope lock_chain( neuron_chain->chain_lock );

		auto it  = neuron_map->find ( *_neuron->cell_name );
		if ( it != neuron_map->end() )
		{
			neuron_map->erase ( it );
			neuron_chain->Remove_Atom ( _neuron );
		}
	}
}

bool XEPL::Neuron::Get_Neuron ( Cord* _cord, Neuron** _neuron )
{
	MutexScope lock_chain( neuron_chain->chain_lock );

	auto it  = neuron_map->find ( *_cord );
	if ( it == neuron_map->end() )
		return false;

	*_neuron = it->second;

	return true;
}

bool XEPL::Neuron::Hunt_Neuron ( Cord* _cord, Neuron** _neuron )
{
	if ( _cord->compare( *cell_name ) == 0 )
	{
		*_neuron = this;
		return true;
	}

	if ( neuron_map && Get_Neuron ( _cord, _neuron ) )
		return true;

	if ( alias && ( _cord->compare( *alias ) == 0 ) )
	{
		*_neuron = this;
		return true;
	}

	if ( parent_neuron )
		return parent_neuron->Hunt_Neuron ( _cord, _neuron );

	return false;
}

bool XEPL::Neuron::Find_Neuron ( Cord* _cord, Neuron** _neuron  )
{
	if ( _cord->empty() )
	{
		*_neuron = cortex->host_lobe;
		return false;
	}
	if ( Hunt_Neuron ( _cord, _neuron ) )
	{
		return true;
	}
	if ( _cord->compare("this") == 0 )
	{
		*_neuron = this;
		return true;
	}
	if ( _cord->compare("parent") == 0 )
	{
		*_neuron = parent_neuron;
		return true;
	}
	*_neuron = nullptr;
	return false;
}

bool XEPL::Neuron::Find_Neuron ( Text* _chars, Neuron** _neuron  )
{
	String name ( _chars );
	return Find_Neuron ( &name, _neuron );
}

void XEPL::Neuron::Show_Neurons( String* _string )
{
	if ( !neuron_map )
		return;

	MutexScope lock_chain( neuron_chain->chain_lock );

	for ( auto& [_, inner_neuron] : *neuron_map )
	{
		XmlBuilder x1(inner_neuron->cell_name->c_str(), _string);
		x1.Close_Attributes();

		if (inner_neuron->neuron_map)
			inner_neuron->Show_Neurons( _string );
	}
}



void XEPL::Neuron::Drop_My_Axons()
{
	if ( !axon_chain )
		return;

	{
		MutexScope lock_chain ( axon_chain->chain_lock );
		axon_chain->Cancel_All_Receptors();
	}
	delete axon_chain;
	delete axon_map;
	axon_chain = nullptr;
	axon_map   = nullptr;
}

void XEPL::Neuron::Register_Axon ( Axon* _axon )
{
	if ( !axon_chain )
	{
		axon_chain = new AxonChain ();
		axon_map   = new AxonMap();
	}
	MutexScope lock_chain ( axon_chain->chain_lock );

	axon_chain->Add_Atom ( _axon );

	auto [it, noob] = axon_map->insert_or_assign( *_axon->axon_name, _axon);
	if (!noob)
		ErrorReport error_report("Replaced axon: ", _axon->axon_name );

	TRACE( "New_Axon", this, _axon->axon_name );
}

void XEPL::Neuron::Unregister_Axon ( Axon* _axon )
{
	if ( axon_chain )
	{
		MutexScope lock_chain ( axon_chain->chain_lock );

		auto it  = axon_map->find ( *_axon->axon_name );
		if ( it != axon_map->end() )
		{
			axon_map->erase ( it );
			axon_chain->Remove_Atom( _axon );
		}
	}
}

bool XEPL::Neuron::Get_Axon ( Cord* _cord, Axon** _axon )
{
	if ( !axon_chain )
		return false;

	MutexScope lock_chain ( axon_chain->chain_lock );

	auto it  = axon_map->find ( *_cord );
	if ( it == axon_map->end() )
		return false;

	*_axon = it->second;

	return true;
}

bool XEPL::Neuron::Hunt_Axon ( Cord* _cord, Axon** _axon )
{
	if ( axon_chain && Get_Axon ( _cord, _axon ) )
		return _axon;

	if ( parent_neuron )
		return parent_neuron->Hunt_Axon ( _cord, _axon );

	return false;
}

void XEPL::Neuron::Receive_Axon ( Atom* _impulse, Atom* _atom )
{
	if ( Gene* trigger_gene = static_cast<Gene*> ( _atom ) )
	{
		ScopeIndex swap_index ( static_cast<Gene*> ( _impulse ) );

		if ( trigger_gene->Has_Content() )
			Script( this, trigger_gene );

		Process_Inner_Genes ( trigger_gene );
	}
}



void XEPL::Neuron::Drop_My_Receptors()
{
	if ( receptor_chain )
	{
		MutexScope lock_chain( receptor_chain->chain_lock );

		receptor_chain->Disconnect_Receptors();
	}
	delete receptor_chain;
	receptor_chain = nullptr;
}

void XEPL::Neuron::Disconnect_Receptor ( Receptor* _receptor )
{
	receptor_chain->Remove_Atom ( _receptor );

	const auto it = relay_map->find ( _receptor->signal_axon );

	Relay* relay = it->second;

	if ( relay->receptor_chain->Remove_Atom ( _receptor ) )
	{
		relay_map->erase ( it );
		if ( relay_map->empty() )
		{
			delete relay_map;
			relay_map=nullptr;
		}

		Neuron_Drop_Relay ( relay );
	}
	_receptor->Release();
}

void XEPL::Neuron::Connect_Receptor ( Axon* _axon, Receptor* _receptor )
{
	if ( !relay_map )
		relay_map = new RelayMap();

	auto [it, noob]  = relay_map->try_emplace ( _axon );
	if ( noob )
		Neuron_Axon_Relay ( _axon, _receptor, &it->second );

	it->second->receptor_chain->Add_Atom ( _receptor );
}

void XEPL::Neuron::Synapse_Axon ( Axon* _axon, Receiver _receiver, Cell* _cell )
{
	TRACE("Synapse", this, _axon->axon_name );

	if ( !receptor_chain )
		receptor_chain = new ReceptorChain();

	Receptor* receptor = new Receptor ( this, _axon, _receiver, _cell );
	Connect_Receptor ( _axon, receptor );
	receptor_chain->Add_Atom ( receptor );
}



void XEPL::Neuron::Relays_Dont_Deliver ( Cell*, Relay* )
{}

void XEPL::Neuron::Neuron_Axon_Relay ( Axon* _axon, Receptor* _receptor, Relay** _relay )
{
	*_relay = new Relay ( this, _axon, ( Receiver )&Neuron::Relays_Dont_Deliver, _receptor );
	parent_neuron->Connect_Receptor ( _axon, *_relay );
}

void XEPL::Neuron::Disconnect_Relay ( Relay* _relay )
{
	auto it = relay_map->find ( _relay->signal_axon );

	Relay* parent_relay = it->second;

	if ( parent_relay->receptor_chain->Remove_Atom ( _relay ) )
	{
		relay_map->erase ( it );
		if ( relay_map->empty() )
		{
			delete relay_map;
			relay_map=nullptr;
		}
		Neuron_Drop_Relay ( parent_relay );
	}
	_relay->Release();
}

void XEPL::Neuron::Neuron_Drop_Relay ( Relay* _relay )
{
	parent_neuron->Disconnect_Relay ( _relay );
}



void XEPL::Neuron::Register_Macro ( Cord* _name, String* _macro )
{
	if ( !_name )
		return;

	Gene* gene = nullptr;
	if ( !shadows->Make_One("Macros")->Make_One_Gene ( _name, &gene ) )
		gene->Deflate_Gene();

	gene->Assign_Content ( _macro );
}

bool XEPL::Neuron::Macro_Hunt ( Cord* _cord, String* _string )
{
	Gene* macros_gene = shadows->Get_First("Macros");
	if ( macros_gene )
	{
		Gene* macro_gene = nullptr;

		if ( macros_gene->Get_First_Gene ( _cord, &macro_gene ) )
		{
			macro_gene->Copy_Content(_string);
			return true;
		}
	}

	if ( !parent_neuron )
		return false;

	return parent_neuron->Macro_Hunt ( _cord, _string );
}

bool XEPL::Neuron::Performed_Macro ( Cord* _opcode, Cord* _seed, String* _param, bool& _truth, String* _result )
{
	String expr;

	if ( Macro_Hunt ( _opcode, &expr ) )
	{
		ShortTerms press("_", _seed );
		if ( _param )
			press.Set( "__", _param );

		Script ( this, nullptr, &expr, _result, &_truth, _seed, false );
		return true;
	}

	return false;
}



XEPL::NeuronChain::NeuronChain( )
: Chain ( true )
{}

XEPL::Neuron* XEPL::NeuronChain::Last ( void )
{
	if ( tail_bond )
		return static_cast<Neuron*> ( tail_bond->atom );
	return nullptr;
}

//    888888ba                          dP
//    88    `8b                         88
//   a88aaaa8P' .d8888b. 88d888b. .d888b88 .d8888b. 88d888b.
//    88   `8b. 88ooood8 88'  `88 88'  `88 88'  `88 88'  `88
//    88     88 88.  ... 88    88 88.  .88 88.  .88 88    88
//    dP     dP `88888P' dP    dP `88888P8 `88888P' dP    dP

XEPL::Rendon::~Rendon ()
{
	tlsLobe->active_rendon = was_rendon;
}

XEPL::Rendon::Rendon ( Neuron* _owner, Gene* _config, String* _bag )
	: Cell      ( _config->cell_name )
	, Nucleus   ( _owner, cell_name )
	, rendition ( _bag )
	, was_rendon( tlsLobe->active_rendon )
{
	tlsLobe->active_rendon = this;

	Register_Method ( "Markup", ( Function )&Rendon::Rendon_Markup, nullptr );
}

bool XEPL::Rendon::Nucleus_Processed ( Nucleus* _nucleus, Gene* _gene )
{
	Markup ( _nucleus, _gene, rendition );
	return true;
}

void XEPL::Rendon::Rendon_Render ( Nucleus*, Gene* _gene )
{
	_gene->Print_Into ( rendition );
}

void XEPL::Rendon::Generate_Payload ( Nucleus* _nucleus, Gene* _gene )
{
	XEPL::String content;
	if ( _gene->Copy_Content(&content) )
		Evaluate_Inner_Scripts ( _nucleus, _gene, &content, rendition );

	if ( !_gene->inner_genes )
		return;

	Bond* bond = _gene->inner_genes->head_bond;
	while ( bond )
	{
		Gene* gene =  static_cast<Gene*> ( bond->atom );
		Markup ( _nucleus, gene, rendition );
		bond = bond->next_bond ;
	}
}

void XEPL::Rendon::Markup ( Nucleus* _nucleus, Gene* _gene, String* _string )
{
	if ( cortex->Did_Render ( _nucleus, _gene, this ) )
		return;

	if ( Show_Trace )
		ErrorReport error_report( "renderer missed tag: ", _gene->cell_name );

	XmlBuilder x1 ( _gene->cell_name, _string, _gene->space_string );
	if ( _gene->traits )
		x1.Absorb_Traits ( _nucleus, _gene );
	x1.Close_Attributes();

	XEPL::String content;
	if ( _gene->Copy_Content(&content) )
		Evaluate_Inner_Scripts ( _nucleus, _gene, &content, _string );

	if ( !_gene->inner_genes )
		return;

	Bond* bond = _gene->inner_genes->head_bond;
	while ( bond )
	{
		Markup ( _nucleus, static_cast<Gene*> ( bond->atom ), rendition );
		bond = bond->next_bond ;
	}
}

//   .d88888b
//   88.    "'
//   `Y88888b. .d8888b. 88d888b. .d8888b. .d8888b. 88d888b.
//         `8b 88ooood8 88'  `88 Y8ooooo. 88'  `88 88'  `88
//   d8'   .8P 88.  ... 88    88       88 88.  .88 88    88
//    Y88888P  `88888P' dP    dP `88888P' `88888P' dP    dP

XEPL::Senson::~Senson()
{
	if ( senson_wire )
		senson_wire->Release();
}

XEPL::Senson::Senson ( Neuron*  _neuron, Gene* _gene )
	: Neuron      ( _neuron, _gene )
	, senson_wire ( new Wire() )
{
	Register_Method ( "Senson_Is_Closed", ( Function )&Senson::Method_Senson_Closed,   nullptr );
	Register_Method ( "Senson_Has_Input", ( Function )&Senson::Method_Senson_Received, nullptr );
}

void XEPL::Senson::Method_Senson_Closed ( Gene*, Gene* )
{
	if ( Test_Flags(closed_flag) )
		return;

	Set_Flags( closed_flag );

	Senson_Closed();
}

void XEPL::Senson::Method_Senson_Received ( Gene*, Gene* )
{
	if ( Test_Flags( closed_flag) )
		return;

	Gene* trigger = static_cast<Gene*> ( tlsLobe->trigger_atom );

	if ( trigger  )
	{
		senson_wire->Append ( trigger->content_wire );
		Senson_Scan();
	}
}

//   dP                 dP
//   88                 88
//   88        .d8888b. 88d888b. .d8888b.
//   88        88'  `88 88'  `88 88ooood8
//   88        88.  .88 88.  .88 88.  ...
//   88888888P `88888P' 88Y8888' `88888P'

XEPL::Lobe::~Lobe()
{
	cortex->final_counters.Add ( &counters );

	if ( outdex_link )
		outdex_link->Release();

	if ( locals )
		locals->Release();

	delete ephemerals;
	delete pending_actions;
	delete rest_semaphore;
	delete cpp_thread;

	if(auto lobe=tlsLobe)--lobe->counters.count_lobes;
}

XEPL::Lobe::Lobe ( Text* _chars )
	: Neuron ( _chars )
	, pending_actions  ( new ActionList ( this ) )
	, rest_semaphore   ( new Semaphore() )
	, cpp_thread       ( new Thread ( this, rest_semaphore ) )
	, index_link       ( nullptr )
	, outdex_link      ( nullptr )
	, locals           ( nullptr )
	, ephemerals       ( nullptr )
	, short_terms      ( nullptr )
	, parent_lobe      ( nullptr )
	, active_rendon    ( nullptr )
	, trigger_atom     ( nullptr )
	, output_string    ( nullptr )
	, counters         ()
{
	if(auto lobe=tlsLobe)++lobe->counters.count_lobes;

	Register_Method ( "Terminate", ( Function )&Lobe::Method_Terminate, nullptr );
}

XEPL::Lobe::Lobe ( Neuron* _parent_neuron, Gene* _config_gene )
	: Neuron ( _parent_neuron, _config_gene )
	, pending_actions ( new ActionList ( this ) )
	, rest_semaphore  ( new Semaphore() )
	, cpp_thread      ( new Thread ( this, rest_semaphore ) )
	, index_link      ( nullptr )
	, outdex_link     ( nullptr )
	, locals          ( nullptr )
	, ephemerals      ( nullptr )
	, short_terms     ( nullptr )
	, parent_lobe     ( tlsLobe )
	, active_rendon   ( nullptr )
	, trigger_atom    ( nullptr )
	, output_string   ( nullptr )
	, counters        ()
{
	if ( _config_gene )
	{
		if ( XEPL::Show_Counters )
			observer->Make_One("Counters");
		if ( XEPL::Show_Memory_Counts )
			observer->Make_One("Heap");
		shadows->Make_One("config")->Absorb_Gene(_config_gene);
	}

	++tlsLobe->counters.count_lobes;
}



void XEPL::Lobe::Start_Lobe ( void )
{
	Semaphore started_semaphore;

	std::unique_lock<std::mutex> lock_step ( started_semaphore );

	cpp_thread->Concieve_Child( &started_semaphore );

	started_semaphore.wait ( lock_step );
}

void XEPL::Lobe::Lobe_Wake_Up ( void )
{
	++tlsLobe->counters.count_wakes;
	rest_semaphore->Give();
}

void XEPL::Lobe::Stop_Lobe ( void )
{
	pending_actions->Close_Action_List();
	Lobe_Wake_Up();
	cpp_thread->Bury_Child();
}

void XEPL::Lobe::Nucleus_Dropped ( void )
{
	if ( Test_Flags(dropped_flag) )
		return;

	Stop_Lobe();

	Release();
}

void XEPL::Lobe::Relay_Nop ( Cell*, Relay* )
{}

void XEPL::Lobe::Method_Terminate ( Gene*, Gene* )
{
	if ( !Test_Flags(dropped_flag) )
	{
		if ( parent_lobe )
			parent_lobe->pending_actions->Post_Action ( new DropAction ( this ) );
		else
			Set_Flags( lysing_flag );
	}
}



XEPL::Gene* XEPL::Lobe::Index( int _up_count )
{
	return indicies.Index( _up_count );
}

void XEPL::Lobe::Set_Outdex ( Gene* _gene )
{
	if ( outdex_link )
		outdex_link->Release();

	outdex_link = _gene;

	if ( outdex_link )
		outdex_link->Attach();
}

bool XEPL::Lobe::Dispatch_Action ( void )
{
	if ( Test_Flags( lysing_flag ) )
		return false;

	Action* action = nullptr;

	if ( !pending_actions->Pull_Action ( &action ) )
		Set_Flags( lysing_flag );

	if ( !action )
		return false;

	++tlsLobe->counters.count_dispatched;

	action->Action_Execute();

	delete action;

	return true;
}

void XEPL::Lobe::Close_Dispatch()
{
	if ( locals )
		locals->Release();

	if ( ephemerals )
		delete ephemerals;

	index_link->Deflate_Gene();

	ephemerals = nullptr;
	locals     = nullptr;
}

void XEPL::Lobe::Lobe_Born ( void )
{
	Register_Method ( "Terminate", ( Function )&Lobe::Method_Terminate, nullptr );

	Gene* config_gene = shadows->Get_First("config");
	if ( !config_gene)
		return;

	if ( config_gene->inner_genes )
		Process_Inner_Genes ( config_gene );
}

void XEPL::Lobe::Lobe_Dying ( void )
{
	Neuron::Nucleus_Dropped();
}

void XEPL::Lobe::Lobe_Rest_Now ( void )
{
	std::unique_lock<std::mutex> lock ( *cpp_thread->semaphore_rest );
	{
		MutexScope lock_actions ( pending_actions->actions_lock );

		if ( pending_actions->head_action != nullptr )
			return;

		if ( pending_actions->list_is_closed )
			return;
	}

	++counters.count_rests;

	cpp_thread->semaphore_rest->wait ( lock );
}



void XEPL::Lobe::Main_Loop ( Semaphore* _semaphore )
{
	{
		String neuron_path_string;
		Nucleus_Path ( &neuron_path_string, '/' );
		Set_Thread_Name ( cpp_thread, neuron_path_string.c_str() );
	}
	this->Attach();

	Gene* index = new Gene ( nullptr, "Index", cell_name );

	ScopeIndex stack( index );

	Lobe_Born();

	_semaphore->Give();

	String scratch;

	while ( !Test_Flags( lysing_flag ) )
	{
		while ( !Test_Flags( lysing_flag ) && Dispatch_Action() )
			Close_Dispatch();

		if ( XEPL::Show_Counters )
		{
			scratch.clear();
			counters.Report( &scratch );
			observer->Make_One("Counters")->Assign_Content(&scratch);
		}

		if ( XEPL::Show_Memory_Counts )
		{
			scratch.clear();
			Recycler::Report_Heap( &scratch );
			observer->Make_One("Heap")->Assign_Content(&scratch);
		}

		if ( !Test_Flags( lysing_flag ) )
			Lobe_Rest_Now();
	}

	Lobe_Dying();

	index->Release();
}



void XEPL::Lobe::Neuron_Axon_Relay ( Axon* _axon, Receptor* _receptor, Relay** _relay )
{
	*_relay = new class Synapse ( this, _axon, ( Receiver )&Lobe::Relay_Nop, _receptor );
	_axon->receptor_chain->Add_Atom ( *_relay );
}

void XEPL::Lobe::Neuron_Drop_Relay ( Relay* _relay )
{
	_relay->signal_axon->receptor_chain->Remove_Atom ( _relay );
	_relay->Release();
}

//    .d888888
//   d8'    88
//   88aaaaa88a dP.  .dP .d8888b. 88d888b.
//   88     88   `8bd8'  88'  `88 88'  `88
//   88     88   .d88b.  88.  .88 88    88
//   88     88  dP'  `dP `88888P' dP    dP

XEPL::Axon::~Axon()
{
	delete receptor_chain;
	delete axon_name;
}

XEPL::Axon::Axon ( Neuron* _owner, Text* _name )
	: Atom()
	, axon_name       ( new Cord( _name ) )
	, host_neuron     ( _owner )
	, receptor_chain  ( new ReceptorChain() )
{
	host_neuron->Register_Axon ( this );
}

XEPL::Axon::Axon ( Neuron* _owner, Cord* _name )
	: Atom()
	, axon_name       ( new Cord( _name ) )
	, host_neuron     ( _owner )
	, receptor_chain  ( new ReceptorChain() )
{
	host_neuron->Register_Axon ( this );
}

void XEPL::Axon::Synapse ( Neuron* _neuron, Gene* _config )
{
	_neuron->Synapse_Axon ( this, ( Receiver )&Neuron::Receive_Axon, _config );
}

void XEPL::Axon::Cancel_Receptors()
{
	receptor_chain->Disconnect_Receptors();
	
	delete receptor_chain;
	receptor_chain = nullptr;
	
	host_neuron->Unregister_Axon ( this );

	Release();
}

void XEPL::Axon::Trigger ( Atom* _atom )
{
	if ( Show_Trace )
		TRACE( "Trigger", this->host_neuron, this->axon_name );

	receptor_chain->Deliver_Signal ( _atom );
}

void XEPL::Axon::Trigger_Wait ( Atom* _atom )
{
	Semaphore* semaphore = new Semaphore();
	{
		bool i_must_wait = true;
		Rendezvous* rendezvous = new Rendezvous ( _atom, &semaphore, &i_must_wait );

		std::unique_lock<std::mutex> block ( *semaphore );
		{
			receptor_chain->Deliver_Signal ( rendezvous );

			rendezvous->Release();

			if ( i_must_wait )
				semaphore->std::condition_variable::wait ( block );
		}
	}
	delete semaphore;
}


//    .d888888                              a88888b. dP                oo
//   d8'    88                             d8'   `88 88
//   88aaaaa88a dP.  .dP .d8888b. 88d888b. 88        88d888b. .d8888b. dP 88d888b.
//   88     88   `8bd8'  88'  `88 88'  `88 88        88'  `88 88'  `88 88 88'  `88
//   88     88   .d88b.  88.  .88 88    88 Y8.   .88 88    88 88.  .88 88 88    88
//   88     88  dP'  `dP `88888P' dP    dP  Y88888P' dP    dP `88888P8 dP dP    dP

XEPL::AxonChain::AxonChain ( void )
	: Chain ( true )
{}

void XEPL::AxonChain::Cancel_All_Receptors()
{
	while ( head_bond  )
		static_cast<Axon*> ( head_bond->atom )->Cancel_Receptors();
}

//    888888ba                                        dP
//    88    `8b                                       88
//   a88aaaa8P' .d8888b. .d8888b. .d8888b. 88d888b. d8888P .d8888b. 88d888b.
//    88   `8b. 88ooood8 88'  `"" 88ooood8 88'  `88   88   88'  `88 88'  `88
//    88     88 88.  ... 88.  ... 88.  ... 88.  .88   88   88.  .88 88
//    dP     dP `88888P' `88888P' `88888P' 88Y888P'   dP   `88888P' dP
//                                         88
//                                         dP

XEPL::Receptor::~Receptor()
{
	if ( memento_atom )
		memento_atom->Release();

	if ( signal_axon )
		signal_axon->Release();

	if ( target_neuron )
		target_neuron->Release();
}

XEPL::Receptor::Receptor ( Neuron* _neuron, Axon* _axon, Receiver _receiver, Atom* _atom )
	: Atom()
	, target_neuron     ( _neuron )
	, signal_axon       ( _axon )
	, signal_receiver   ( _receiver )
	, memento_atom      ( _atom )
{
	if ( signal_axon )
		signal_axon->Attach();

	if ( target_neuron )
		target_neuron->Attach();

	if ( memento_atom )
		memento_atom->Attach();
}

void XEPL::Receptor::Receptor_Cancel()
{
	target_neuron->Disconnect_Receptor ( this );
}

void XEPL::Receptor::Receptor_Activate ( Atom* _trigger_atom ) const
{
	if ( Show_Trace )
		TRACE( "ENTR_Axon", target_neuron, signal_axon->axon_name );

	tlsLobe->trigger_atom = _trigger_atom;

	( target_neuron->*signal_receiver ) ( _trigger_atom, static_cast<Gene*> ( memento_atom ) );
}


XEPL::ReceptorChain::ReceptorChain ()
	:  Chain ( true )
{}

void XEPL::ReceptorChain::Deliver_Signal ( Atom* _atom )
{
	Bond* my_bonds;
	Bond* copy_bond;
	{
		MutexScope lock_synapse ( chain_lock );

		if ( !head_bond )
			return;

		Bond* at_bond = head_bond;
		my_bonds = copy_bond = new Bond( at_bond->atom, nullptr );
		at_bond  = at_bond->next_bond;
		
		while ( at_bond )
		{
			copy_bond = new Bond( at_bond->atom, copy_bond );
			at_bond = at_bond->next_bond;
		}
	}

	while ( my_bonds )
	{
		copy_bond = my_bonds;
		static_cast<Receptor*> ( copy_bond->atom )->Receptor_Activate ( _atom ) ;
		my_bonds = copy_bond->next_bond;
		delete copy_bond;
	}
}

void XEPL::ReceptorChain::Disconnect_Receptors()
{
	MutexScope lock_chain (chain_lock);

	while ( head_bond )
		static_cast<Receptor*> ( head_bond->atom )->Receptor_Cancel();
}

//    888888ba           dP
//    88    `8b          88
//   a88aaaa8P' .d8888b. 88 .d8888b. dP    dP
//    88   `8b. 88ooood8 88 88'  `88 88    88
//    88     88 88.  ... 88 88.  .88 88.  .88
//    dP     dP `88888P' dP `88888P8 `8888P88
//                                        .88
//                                    d8888P

XEPL::Relay::~Relay()
{
	delete receptor_chain;
}

XEPL::Relay::Relay ( Neuron* _neuron, Axon* _axon, Receiver _receiver, Receptor* _receptor )
	: Receptor ( _neuron, _axon, _receiver, _receptor )
	, receptor_chain ( new ReceptorChain() )
{}

void XEPL::Relay::Receptor_Activate ( Atom* _atom ) const
{
	receptor_chain->Deliver_Signal ( _atom );
}

//   .d88888b
//   88.    "'
//   `Y88888b. dP    dP 88d888b. .d8888b. 88d888b. .d8888b. .d8888b.
//         `8b 88    88 88'  `88 88'  `88 88'  `88 Y8ooooo. 88ooood8
//   d8'   .8P 88.  .88 88    88 88.  .88 88.  .88       88 88.  ...
//    Y88888P  `8888P88 dP    dP `88888P8 88Y888P' `88888P' `88888P'
//                  .88                   88
//              d8888P                    dP

XEPL::Synapse::Synapse ( Lobe* _lobe, Axon* _axon, Receiver _receiver, Receptor* _receptor  )
	: Relay       ( _lobe, _axon, _receiver, _receptor )
	, action_list ( _lobe->pending_actions )
{}

void XEPL::Synapse::Receptor_Activate ( Atom* _atom ) const
{
	Bond* my_bonds;
	Bond* copy_bond;
	{
		MutexScope lock_synapse (  receptor_chain->chain_lock );

		if ( ! receptor_chain->head_bond )
			return;

		Bond* at_bond = receptor_chain->head_bond;
		my_bonds = copy_bond = new Bond( at_bond->atom, nullptr );
		at_bond  = at_bond->next_bond;

		while ( at_bond )
		{
			copy_bond = new Bond( at_bond->atom, copy_bond );
			at_bond = at_bond->next_bond;
		}
	}

	while ( my_bonds )
	{
		copy_bond = my_bonds;
		Receptor* receptor = static_cast<Receptor*> ( copy_bond->atom );
		action_list->Post_Action( new SignalAction ( receptor, _atom ) );
		my_bonds = my_bonds->next_bond;
		delete copy_bond;
	}
}

//   .d88888b           oo dP
//   88.    "'             88
//   `Y88888b. 88d888b. dP 88  .dP  .d8888b.
//         `8b 88'  `88 88 88888"   88ooood8
//   d8'   .8P 88.  .88 88 88  `8b. 88.  ...
//    Y88888P  88Y888P' dP dP   `YP `88888P'
//             88
//             dP

XEPL::Spike::~Spike()
{
	if ( stimulus )
		stimulus->Release();
}

XEPL::Spike::Spike ( Atom* _stimulus )
	: Atom()
	, stimulus ( _stimulus )
{
	if ( stimulus )
		stimulus->Attach();
}

//    888888ba                          dP
//    88    `8b                         88
//   a88aaaa8P' .d8888b. 88d888b. .d888b88 .d8888b. d888888b dP   .dP .d8888b. dP    dP .d8888b.
//    88   `8b. 88ooood8 88'  `88 88'  `88 88ooood8    .d8P' 88   d8' 88'  `88 88    88 Y8ooooo.
//    88     88 88.  ... 88    88 88.  .88 88.  ...  .Y8P    88 .88'  88.  .88 88.  .88       88
//    dP     dP `88888P' dP    dP `88888P8 `88888P' d888888P 8888P'   `88888P' `88888P' `88888P'

XEPL::Rendezvous::~Rendezvous()
{
	if ( lobe != tlsLobe )
		(*semaphore)->Give();
	else
		*u_must_wait = false;
}

XEPL::Rendezvous::Rendezvous ( Atom* _atom, Semaphore** _semaphore, bool* _must_wait )
	: Spike         ( _atom )
	, lobe          ( tlsLobe )
	, semaphore     ( _semaphore )
	, u_must_wait   ( _must_wait )
{}

//    .d888888             dP   oo
//   d8'    88             88
//   88aaaaa88a .d8888b. d8888P dP .d8888b. 88d888b.
//   88     88  88'  `""   88   88 88'  `88 88'  `88
//   88     88  88.  ...   88   88 88.  .88 88    88
//   88     88  `88888P'   dP   dP `88888P' dP    dP

XEPL::Action::~Action ( void )
{
	if ( receptor )
		receptor->Release();

	if ( trigger_atom )
		trigger_atom->Release();
}

XEPL::Action::Action ( Receptor* _receptor, Atom* _atom )
	: receptor ( _receptor )
	, trigger_atom    ( _atom )
	, next_action     ( nullptr )
{
	++tlsLobe->counters.count_actions;

	if ( receptor )
		receptor->Attach();

	if ( trigger_atom )
		trigger_atom->Attach();
}

//   .d88888b  oo                            dP  .d888888             dP   oo
//   88.    "'                               88 d8'    88             88
//   `Y88888b. dP .d8888b. 88d888b. .d8888b. 88 88aaaaa88a .d8888b. d8888P dP .d8888b. 88d888b.
//         `8b 88 88'  `88 88'  `88 88'  `88 88 88     88  88'  `""   88   88 88'  `88 88'  `88
//   d8'   .8P 88 88.  .88 88    88 88.  .88 88 88     88  88.  ...   88   88 88.  .88 88    88
//    Y88888P  dP `8888P88 dP    dP `88888P8 dP 88     88  `88888P'   dP   dP `88888P' dP    dP
//                     .88
//                 d8888P

XEPL::SignalAction::SignalAction ( Receptor* _receptor, Atom* _atom )
	: Action ( _receptor, _atom )
{}

void XEPL::SignalAction::Action_Execute ( void )
{
	receptor->Receptor_Activate ( trigger_atom );
}

//   888888ba                              .d888888             dP   oo
//   88    `8b                            d8'    88             88
//   88     88 88d888b. .d8888b. 88d888b. 88aaaaa88a .d8888b. d8888P dP .d8888b. 88d888b.
//   88     88 88'  `88 88'  `88 88'  `88 88     88  88'  `""   88   88 88'  `88 88'  `88
//   88    .8P 88       88.  .88 88.  .88 88     88  88.  ...   88   88 88.  .88 88    88
//   8888888P  dP       `88888P' 88Y888P' 88     88  `88888P'   dP   dP `88888P' dP    dP
//                               88
//                               dP

XEPL::DropAction::DropAction ( Neuron* _neuron )
	: Action ( nullptr, nullptr )
	, neuron ( _neuron )
{
	neuron->Attach();
}

void XEPL::DropAction::Action_Execute ( void )
{
	neuron->Nucleus_Dropped();
	neuron->Release();
}

//    .d888888             dP   oo                   dP        oo            dP
//   d8'    88             88                        88                      88
//   88aaaaa88a .d8888b. d8888P dP .d8888b. 88d888b. 88        dP .d8888b. d8888P
//   88     88  88'  `""   88   88 88'  `88 88'  `88 88        88 Y8ooooo.   88
//   88     88  88.  ...   88   88 88.  .88 88    88 88        88       88   88
//   88     88  `88888P'   dP   dP `88888P' dP    dP 88888888P dP `88888P'   dP

XEPL::ActionList::~ActionList ( void )
{
	delete actions_lock;
}

XEPL::ActionList::ActionList ( Lobe* _lobe )
	: actions_lock     ( new Mutex() )
	, head_action      ( nullptr )
	, tail_action      ( nullptr )
	, lobe             ( _lobe )
	, list_is_closed   ( false )
{}

void XEPL::ActionList::Close_Action_List()
{
	MutexScope lock_actions ( actions_lock );

	lobe->Set_Flags(closed_flag);
	list_is_closed = true;
}

void XEPL::ActionList::Flush_Action_list()
{
	while ( head_action )
	{
		Action* action = head_action;
		head_action = head_action->next_action;

		if ( !head_action )
			tail_action = nullptr;

		delete action;
	}
}

bool XEPL::ActionList::Pull_Action ( Action** _action )
{
	MutexScope lock_actions ( actions_lock );

	if ( list_is_closed )
	{
		Flush_Action_list();
		return false;
	}
	if ( head_action )
	{
		*_action = head_action;
		head_action = head_action->next_action;

		if ( !head_action )
			tail_action = nullptr;
	}
	return true;
}

void XEPL::ActionList::Post_Action  ( Action* _action )
{
	bool wake_lobe = false;
	{
		MutexScope lock_actions ( actions_lock );

		if ( list_is_closed )
		{
			delete _action;
			return;
		}

		_action->next_action = nullptr;

		if ( tail_action )
			tail_action->next_action = _action;
		else
		{
			head_action = _action;
			wake_lobe = lobe != tlsLobe;
		}

		tail_action = _action;
	}
	if ( wake_lobe )
		lobe->Lobe_Wake_Up();
}

//    88888888b          dP                                                      dP
//    88                 88                                                      88
//   a88aaaa    88d888b. 88d888b. .d8888b. 88d8b.d8b. .d8888b. 88d888b. .d8888b. 88 .d8888b.
//    88        88'  `88 88'  `88 88ooood8 88'`88'`88 88ooood8 88'  `88 88'  `88 88 Y8ooooo.
//    88        88.  .88 88    88 88.  ... 88  88  88 88.  ... 88       88.  .88 88       88
//    88888888P 88Y888P' dP    dP `88888P' dP  dP  dP `88888P' dP       `88888P8 dP `88888P'
//              88
//              dP
void XEPL::Ephemerals::Set ( Cord* _cord, Gene* _gene )
{
	auto [it, noob] = try_emplace(*_cord);
	if ( !noob )
		it->second->Release();

	it->second = _gene;

	if ( !_gene )
		return;

	_gene->Attach();
}

//   dP                dP oo          oo
//   88                88
//   88 88d888b. .d888b88 dP .d8888b. dP .d8888b. .d8888b.
//   88 88'  `88 88'  `88 88 88'  `"" 88 88ooood8 Y8ooooo.
//   88 88    88 88.  .88 88 88.  ... 88 88.  ...       88
//   dP dP    dP `88888P8 dP `88888P' dP `88888P' `88888P'

XEPL::Indicies::Indicies()
{
	tail_bond = nullptr;
}

void XEPL::Indicies::Stack( Lobe* _host_lobe, Gene* _new_index )
{
	tail_bond = new Bond( _new_index, tail_bond );
	_host_lobe->index_link = _new_index;
}

void XEPL::Indicies::Unstack( Lobe* _host_lobe )
{
	auto bond = tail_bond->prev_bond;
	delete tail_bond;
	tail_bond = bond;
	if ( bond )
		_host_lobe->index_link = static_cast<Gene*>(bond->atom);
}

XEPL::Gene* XEPL::Indicies::Index( int _up_count )
{
	auto bond = tail_bond;
	while( _up_count-- && bond )
		bond = bond->prev_bond;

	if ( bond )
		return static_cast<Gene*>(bond->atom);

	return nullptr;
}

//   .d88888b                                      dP                dP
//   88.    "'                                     88                88
//   `Y88888b. .d8888b. .d8888b. 88d888b. .d8888b. 88 88d888b. .d888b88 .d8888b. dP.  .dP
//         `8b 88'  `"" 88'  `88 88'  `88 88ooood8 88 88'  `88 88'  `88 88ooood8  `8bd8'
//   d8'   .8P 88.  ... 88.  .88 88.  .88 88.  ... 88 88    88 88.  .88 88.  ...  .d88b.
//    Y88888P  `88888P' `88888P' 88Y888P' `88888P' dP dP    dP `88888P8 `88888P' dP'  `dP
//                               88
//                               dP

XEPL::ScopeIndex::ScopeIndex( Gene* _new_index )
{
	auto lobe=tlsLobe;
	lobe->indicies.Stack( lobe, _new_index );
}

XEPL::ScopeIndex::~ScopeIndex()
{
	auto lobe=tlsLobe;
	lobe->indicies.Unstack( lobe );
}

//   .d88888b  dP                           dP   d888888P
//   88.    "' 88                           88      88
//   `Y88888b. 88d888b. .d8888b. 88d888b. d8888P    88    .d8888b. 88d888b. 88d8b.d8b. .d8888b.
//         `8b 88'  `88 88'  `88 88'  `88   88      88    88ooood8 88'  `88 88'`88'`88 Y8ooooo.
//   d8'   .8P 88    88 88.  .88 88         88      88    88.  ... 88       88  88  88       88
//    Y88888P  dP    dP `88888P' dP         dP      dP    `88888P' dP       dP  dP  dP `88888P'

class XEPL::ShortTermMap : public MapT<Cord, String> {};

XEPL::ShortTerms::~ShortTerms()
{
	if (!tlsLobe)
		return;
	
	restore_terms_ref = previous_terms;

	delete term_map;
}

XEPL::ShortTerms::ShortTerms ( void )
	: restore_terms_ref  ( tlsLobe->short_terms )
	, previous_terms     ( restore_terms_ref )
	, hot_terms          ( previous_terms )
	, term_map           ( nullptr )
{
	restore_terms_ref  = this;
}

XEPL::ShortTerms::ShortTerms ( Gene* _gene )
	: restore_terms_ref  ( tlsLobe->short_terms )
	, previous_terms     ( restore_terms_ref )
	, hot_terms          ( previous_terms ? previous_terms->hot_terms : nullptr )
	, term_map           ( nullptr )
{
	if ( _gene && _gene->traits )
		Nest_Traits ( _gene );

	restore_terms_ref = this;
}

XEPL::ShortTerms::ShortTerms ( Text* _chars, Cord* _cord )
	: restore_terms_ref  ( tlsLobe->short_terms )
	, previous_terms     ( restore_terms_ref )
	, hot_terms          ( previous_terms ? previous_terms->hot_terms : nullptr)
	, term_map           ( nullptr )
{
	Set( _chars, _cord );
	restore_terms_ref = this;
}

void XEPL::ShortTerms::Set ( Cord* _name_cord, Cord* _term_cord )
{
	if ( !_term_cord )
		return;

	if ( !term_map )
		term_map = new ShortTermMap();

	hot_terms = this;
	term_map->insert_or_assign( *_name_cord, *_term_cord );
}

void XEPL::ShortTerms::Set ( Text* _chars, Cord* _cord )
{
	Cord trait_name( _chars );
	Set( &trait_name, _cord );
}

bool XEPL::ShortTerms::Get_Into ( Cord* _cord, String* _string )
{
	const ShortTerms* short_terms = term_map ? this : hot_terms;

	while ( short_terms )
	{
		if ( short_terms->term_map )
		{
			auto it  = short_terms->term_map->find ( *_cord );
			if ( it != short_terms->term_map->end() )
			{
				_string->assign ( (*it).second );
				return true;
			}
		}
		if ( short_terms->previous_terms )
			short_terms = short_terms->previous_terms->hot_terms;
		else
			short_terms = nullptr;
	}
	return false;
}

void XEPL::ShortTerms::Replace_Traits( Gene* _gene )
{
	tlsLobe->short_terms->Nest_Traits(_gene);
}

void XEPL::ShortTerms::Nest_Traits ( Gene* _gene )
{
	if ( !_gene || !_gene->traits )
		return;

	if ( !term_map )
		term_map = new ShortTermMap();

	hot_terms = this;

	for ( auto& [name, value] : *_gene->traits->map_of_traits )
		term_map->insert_or_assign(name, value->trait_term);
}

//   .d88888b    dP            dP       dP           .88888.
//   88.    "'   88            88       88          d8'   `88
//   `Y88888b. d8888P .d8888b. 88d888b. 88 .d8888b. 88        .d8888b. 88d888b. .d8888b. .d8888b.
//         `8b   88   88'  `88 88'  `88 88 88ooood8 88   YP88 88ooood8 88'  `88 88ooood8 Y8ooooo.
//   d8'   .8P   88   88.  .88 88.  .88 88 88.  ... Y8.   .88 88.  ... 88    88 88.  ...       88
//    Y88888P    dP   `88888P8 88Y8888' dP `88888P'  `88888'  `88888P' dP    dP `88888P' `88888P'

XEPL::StableGenes::~StableGenes ( void )
{
	delete stable_gene_chain;
}

XEPL::StableGenes::StableGenes ( Gene* _gene )
	: stable_gene_chain ( nullptr )
{
	if ( _gene )
		_gene->Copy_Genes_Into ( &stable_gene_chain );
}

bool XEPL::StableGenes::Next_Gene ( Gene** _gene )
{
	if ( !stable_gene_chain )
		return false;

	if ( stable_gene_chain->Next ( _gene ) )
		return true;

	delete stable_gene_chain;
	stable_gene_chain=nullptr;

	return false;
}

//   .d88888b    dP            dP       dP          d888888P                   oo   dP
//   88.    "'   88            88       88             88                           88
//   `Y88888b. d8888P .d8888b. 88d888b. 88 .d8888b.    88    88d888b. .d8888b. dP d8888P .d8888b.
//         `8b   88   88'  `88 88'  `88 88 88ooood8    88    88'  `88 88'  `88 88   88   Y8ooooo.
//   d8'   .8P   88   88.  .88 88.  .88 88 88.  ...    88    88       88.  .88 88   88         88
//    Y88888P    dP   `88888P8 88Y8888' dP `88888P'    dP    dP       `88888P8 dP   dP   `88888P'

XEPL::StableTraits::~StableTraits ( void )
{
	delete stable_traits;
}

XEPL::StableTraits::StableTraits ( Gene* _gene )
	: stable_traits   ( nullptr )
	, current_trait   ( nullptr )
{
	if ( _gene && _gene->Duplicate_Traits ( &stable_traits ) )
		current_trait = stable_traits ? stable_traits->first_trait : nullptr;
}

bool XEPL::StableTraits::Next_Trait ( Cord** _chars1, String** _chars2 )
{
	if ( !current_trait )
	{
		current_trait = stable_traits ? stable_traits->first_trait : nullptr;
		_chars1 = nullptr;
		_chars2 = nullptr;
		return false;
	}

	*_chars1 = current_trait->trait_name;
	*_chars2 = current_trait->trait_term;
	current_trait = current_trait->next_trait;
	return true;
}

//   dP   dP   dP oo
//   88   88   88
//   88  .8P  .8P dP 88d888b. .d8888b.
//   88  d8'  d8' 88 88'  `88 88ooood8
//   88.d8P8.d8P  88 88       88.  ...
//   8888' Y88'   dP dP       `88888P'

XEPL::Wire::~Wire( void )
{
	delete wire_mutex;
	delete wire_string;
}

XEPL::Wire::Wire()
	: Atom()
	, wire_mutex  ( new Mutex() )
	, wire_string ( new String() )
{}

void XEPL::Wire::Assign ( Cord* _cord )
{
	if ( !_cord )
		return;

	MutexScope lock_wire ( wire_mutex );
	wire_string->assign ( *_cord );
}

void XEPL::Wire::Append ( Wire* _wire )
{
	if ( !_wire )
		return ;

	MutexScope lock_wire       (  this->wire_mutex );
	MutexScope lock_their_wire ( _wire->wire_mutex );

	wire_string->append ( *_wire->wire_string );
}

void XEPL::Wire::Append ( Text* _chars, size_t _size )
{
	MutexScope lock_wire ( wire_mutex );
	wire_string->append ( _chars, _size );
}

void XEPL::Wire::Append ( Cord* _cord )
{
	MutexScope lock_wire ( wire_mutex );
	wire_string->append ( *_cord );
}

void XEPL::Wire::Print_Into ( String* _string )
{
	if ( !_string )
		return;

	MutexScope lock_wire ( wire_mutex );
	_string->append ( *wire_string );
}

void XEPL::Wire::Erase()
{
	MutexScope lock_wire ( wire_mutex );
	wire_string->erase();
}

size_t XEPL::Wire::Avail ( void )
{
	MutexScope lock_wire ( wire_mutex );
	return wire_string->size();
}

bool XEPL::Wire::Expire ( size_t _size )
{
	MutexScope lock_wire ( wire_mutex );
	wire_string->erase ( 0, _size );
	return wire_string->empty();
}

bool XEPL::Wire::Extract_Line ( String* _string )
{
	MutexScope lock_wire ( wire_mutex );

	if ( wire_string->empty() )
		return false;

	size_t offset = wire_string->find ( '\n', 0 );

	if ( offset != String::npos )
	{
		size_t eol = offset;

		if ( eol && wire_string->at ( eol-1 )=='\r' )
			eol--;

		_string->append ( wire_string->c_str(), eol );
		wire_string->erase ( 0, offset+1 );
	}
	else
	{
		_string->append ( wire_string->c_str() );
		wire_string->erase ( 0, wire_string->size() );
	}
	return true;
}

//    888888ba
//    88    `8b
//   a88aaaa8P' .d8888b. 88d888b. .d8888b. .d8888b. 88d888b.
//    88        88'  `88 88'  `88 Y8ooooo. 88ooood8 88'  `88
//    88        88.  .88 88             88 88.  ... 88
//    dP        `88888P8 dP       `88888P' `88888P' dP

XEPL::Parser::~Parser()
{
	delete error_string;
}

XEPL::Parser::Parser()
	: parser_bag   ( nullptr )
	, error_string ( nullptr )
{}

void XEPL::Parser::Record_Error ( Text* _reason, Text*       _explain )
{
	if ( error_string )
		return;
	error_string = new ErrorReport( "line: " );
	error_string->append( std::to_string(parser_bag->line_number));
	error_string->append( " column: " );
	error_string->append( std::to_string( parser_bag->Column()) );
	error_string->append(": ");

	if (_reason )
		error_string->append( _reason );

//	error_string->append(": ");

	if ( _explain )
	{
		error_string->push_back(' ');
		error_string->append ( _explain );
	}
}

void XEPL::Parser::Record_Error ( Text* _reason, String* _explain )
{
	Record_Error ( _reason, _explain->c_str() );
}

//    888888ba                                                888888ba
//    88    `8b                                               88    `8b
//   a88aaaa8P' .d8888b. 88d888b. .d8888b. .d8888b. 88d888b. a88aaaa8P' .d8888b. .d8888b.
//    88        88'  `88 88'  `88 Y8ooooo. 88ooood8 88'  `88  88   `8b. 88'  `88 88'  `88
//    88        88.  .88 88             88 88.  ... 88        88    .88 88.  .88 88.  .88
//    dP        `88888P8 dP       `88888P' `88888P' dP        88888888P `88888P8 `8888P88
//                                                                                    .88
//                                                                                d8888P

XEPL::ParserBag::ParserBag ( Cord* _bag, Parser* _parser )
	: parser_host       ( _parser )
	, start_position    ( _bag->c_str() )
	, current_position  ( start_position )
	, last_position     ( start_position+_bag->size() )
	, start_of_line     ( start_position )
	, line_number       ( 1 )
{
	if ( *last_position )
		Parse_Error ( "buffer is not terminated" );
}

long XEPL::ParserBag::Remaining ( void )
{
	return ( last_position-current_position );
}

long XEPL::ParserBag::Offset ( void )
{
	return ( current_position-start_position );
}

long XEPL::ParserBag::Column ( void )
{
	return ( current_position-start_of_line );
}

void XEPL::ParserBag::Skip_Whitespace()
{
	while ( isspace ( *current_position ) )
	{
		if ( *++current_position == '\n' )
		{
			++line_number;
			start_of_line = current_position;
		}
	}
}

bool XEPL::ParserBag::Discard_Char ( char _c1 )
{
	if ( *current_position == _c1 )
	{
		++current_position;
		return true;
	}
	String msg ( "Expected: " );
	msg.push_back ( _c1 );
	parser_host->Record_Error ( msg.c_str(), current_position );
	return false;
}

bool XEPL::ParserBag::Consume ( char _c1 )
{
	return ( * ( current_position+0 ) ==_c1 && ++current_position );
}

bool XEPL::ParserBag::Consume ( char _c1, char _c2 )
{
	return
		* ( current_position+0 ) == _c1 &&
		* ( current_position+1 ) == _c2 &&
		(current_position+=2);
}

bool XEPL::ParserBag::Consume ( char _c1, char _c2, char _c3 )
{
	return
		* ( current_position+0 ) == _c1 &&
		* ( current_position+1 ) == _c2 &&
		* ( current_position+2 ) == _c3 &&
		(current_position+=3);
}

bool XEPL::ParserBag::Consume ( char _c1, char _c2, char _c3, char _c4 )
{
	return
		* ( current_position+0 ) == _c1 &&
		* ( current_position+1 ) == _c2 &&
		* ( current_position+2 ) == _c3 &&
		* ( current_position+3 ) == _c4 &&
		(current_position+=4);
}

void XEPL::ParserBag::Parse_Error ( Text* _reason )
{
	parser_host->Record_Error ( _reason, "" );
}

//    888888ba                                               .d88888b           dP                     dP
//    88    `8b                                              88.    "'          88                     88
//   a88aaaa8P' .d8888b. 88d888b. .d8888b. .d8888b. 88d888b. `Y88888b. .d8888b. 88 .d8888b. .d8888b. d8888P
//    88        88'  `88 88'  `88 Y8ooooo. 88ooood8 88'  `88       `8b 88ooood8 88 88ooood8 88'  `""   88
//    88        88.  .88 88             88 88.  ... 88       d8'   .8P 88.  ... 88 88.  ... 88.  ...   88
//    dP        `88888P8 dP       `88888P' `88888P' dP        Y88888P  `88888P' dP `88888P' `88888P'   dP

XEPL::ParserSelect::~ParserSelect( void )
{
	while ( !parser->error_string )
	{
		if ( !parser->parser_bag->Remaining() )
			parser->Record_Error ( "unexpected EOF", static_cast<char*> ( nullptr ) );

		ParserChoice* choice = parser_choices->choice_array;
		while ( choice && !parser->error_string )
		{
			if ( choice->parser_option && ( *choice->parser_option ) ( parser ) )
			{
				if ( choice->parser_flags & Completes )
				{
					delete parser_choices;
					return;
				}
				if ( ! ( choice->parser_flags & Can_Repeat )  )
					choice->parser_option = nullptr;

				break;
			}
			choice++;
		}
		if ( !choice )
			parser->Record_Error ( "not claimed", "parser" );
	}
	delete parser_choices;
}

XEPL::ParserSelect::ParserSelect ( Parser* _parser )
	: parser             ( _parser )
	, parser_choices     ( new ParserChoices() )
	, number_of_choices  ( 0 )
{}

void XEPL::ParserSelect::Add_Option ( ParserFlags _flags, ParserOption _option )
{
	parser_choices->choice_array[number_of_choices++] = { _option, _flags };
}

//   .d88888b                    oo            dP
//   88.    "'                                 88
//   `Y88888b. .d8888b. 88d888b. dP 88d888b. d8888P
//         `8b 88'  `"" 88'  `88 88 88'  `88   88
//   d8'   .8P 88.  ... 88       88 88.  .88   88
//    Y88888P  `88888P' dP       dP 88Y888P'   dP
//                                  88
//                                  dP

XEPL::Script::~Script()
{
	delete value;
}

XEPL::Script::Script ( Neuron* _neuron, Gene* _gene, String* _result )
	: Parser  ()
	, neuron  ( _neuron )
	, rna     ( nullptr )
	, lobe    ( tlsLobe )
	, value   ( new String() )
	, gene    ( _gene )
	, truth   ( false )
{
	XEPL::String content;
	if ( gene->Copy_Content(&content) )
		Translate(&content);

	if (_result)
		_result->assign( *value );
}

XEPL::Script::Script ( Neuron* _neuron, Gene* _gene, Cord* _expr, String* _result )
	: Parser  ()
	, neuron  ( _neuron )
	, rna     ( nullptr )
	, lobe    ( tlsLobe )
	, value   ( new String() )
	, gene    ( _gene )
	, truth   ( false )
{
	if ( !_gene )
		return;

	Translate( _expr );

	if (_result)
		_result->assign( *value );
}

XEPL::Script::Script ( Neuron*  _neuron, Gene* _gene, Cord* _expr, String* _result, bool* _truth, Cord* _seed, bool _append )
	: Parser  ()
	, neuron  ( _neuron )
	, rna     ( nullptr )
	, lobe    ( tlsLobe )
	, value   ( new String( _seed ) )
	, gene    ( _gene )
	, truth   ( false )
{
	if ( !_expr )
		return;

	if ( _truth )
		truth = *_truth;

	Translate(_expr);

	if ( _truth )
		*_truth = truth;

	if ( _result )
	{
		if ( _append )
			_result->append ( *value );
		else
			_result->assign ( *value );
	}
}

void XEPL::Script::Translate (Cord* _expr )
{
	RnaBag safe_bag( _expr, this );
	parser_bag = rna = &safe_bag;

	if ( gene && gene->content_mutex )
	{
		MutexScope lock_traits ( gene->content_mutex );
		while ( !error_string && Get_Next_Value() )
		{}
	}
	else
	{
		while ( !error_string && Get_Next_Value() )
		{}
	}

	Report_Any_Errors();

	parser_bag = rna = nullptr;
}

void XEPL::Script::Report_Any_Errors()
{
	if ( error_string )
	{
		long offset = parser_bag->Offset();
		constexpr auto record_len = 128;

		if ( value )
			error_string->append("\n").append( parser_bag->start_position );

		if ( offset < record_len )
		{
			error_string->append("\n").append( parser_bag->Offset(), ' ' );
			error_string->push_back('^');
		}
	}
}

void XEPL::Evaluate_Inner_Scripts ( Nucleus* _nucleus, Gene* _gene, Cord* _expr, String* _into )
{
	if ( !_expr )
		return;

	Text*  cur_pos   = _expr->c_str();
	Text*  bo_script = nullptr;

	while ( ( bo_script = strstr ( cur_pos, "{{" ) ) != nullptr )
	{
		_into->append ( cur_pos, bo_script - cur_pos );
		Text*  eo_script = strstr ( bo_script, "}}" );

		if ( eo_script )
		{
			String inner_expr;
			inner_expr.assign ( bo_script + 2, eo_script - bo_script - 2 );
			cur_pos = eo_script + 2;

			Script ( _nucleus->Host(), _gene, &inner_expr, _into, nullptr, nullptr, true );
		}
		else
			break;
	}
	_into->append ( cur_pos );
}

void XEPL::Script::Process_Gene ( Gene* _gene )
{
	if ( !_gene )
		return;

	GeneScope hold_active ( gene, _gene );

	switch ( *rna->current_position )
	{
		case ' ' :
		case '\t':
		case '\n':
			++rna->current_position;
			rna->Skip_Whitespace();
			Process_Gene ( _gene );
			break;

		case '.' :
			do Mutate_Value();
			while ( *rna->current_position == '.' );

			Process_Gene ( gene );
			break;

		case '>' :
			++rna->current_position;
			Serialize();
			break;

		case '/' :
			++rna->current_position;
			Enter_Child_Gene();
			break;

		case '\'' :
			Select_Attribute();
			break;

		case '|' :
			++rna->current_position;
			Declare_Content();
			break;
	}
}

void XEPL::Script::Select_Attribute()
{
	const char wastip = *rna->current_position;

	++rna->current_position;

	String attribute_tag;
	rna->Pull_Tag ( &attribute_tag );

	rna->Discard_Char( wastip );

	if ( attribute_tag.empty() )
		value->assign ( *gene->cell_name );
	else
	{
		if ( *rna->current_position == '=' )
		{
			++rna->current_position;
			Get_Next_Value();

			gene->Trait_Set ( &attribute_tag, value );
			return;
		}

		truth = !!gene->Trait_Get ( &attribute_tag, value );
	}
}

void XEPL::Script::Declare_Content()
{
	if ( *rna->current_position != '=' && *rna->current_position != '+' )
	{
		gene->Copy_Content(value);
		return;
	}

	bool append_content = *rna->current_position == '+';
	++rna->current_position;
	rna->Skip_Whitespace();
	Get_Next_Value();

	if ( !append_content )
	{
		gene->Assign_Content ( value );
		return;
	}

	gene->Append_Content ( value );

	value->clear();
	gene->Copy_Content(value);
}

void XEPL::Script::Enter_Child_Gene()
{
	String tag;
	if ( *rna->current_position == '{' )
	{
		++rna->current_position;
		Enter_Inner_Block();
		tag.assign(*value);
		value->clear();
	}
	else
		rna->Pull_Tag ( &tag );

	Gene* located_gene = nullptr;

	if (gene->Get_First_Gene ( &tag, &located_gene ) )
	{
		ScopeIndex active ( located_gene );
		Process_Gene      ( located_gene );
	}
	else
		Record_Error ( "Child_Gene not found: ", tag.c_str() );
}

void XEPL::Script::Change_Gene ( void )
{
	Gene* target = nullptr;

	Text* started_at = rna->current_position;

	if ( !isalpha ( *rna->current_position ) )
	{
		if ( isdigit(*rna->current_position) )
		{
			while ( isdigit ( *rna->current_position ) )
				++rna->current_position;

			target = lobe->Index( std::atoi(started_at) );
		}
		else if ( rna->Consume( '*' ) )
		{
			auto was_value = value;
			String revalue;
			value = &revalue;

			Extract_Value();
			target = Cortex::Locate_Gene( neuron, value );
			
			value = was_value;
		}
		else if ( rna->Consume ( '$' ) )
			target = lobe->outdex_link;
		else
			target = lobe->index_link;
	}
	else
	{
		String variable_tag;
		rna->Pull_Tag ( &variable_tag );

		target = Cortex::Locate_Gene( neuron, &variable_tag );
	}

	if ( !target )
	{
		Record_Error ( "RNA: Gene/Index not found ... abort", started_at );
		return;
	}

	ScopeIndex inside( target );
	Process_Gene ( target );
}

void XEPL::Script::Process_Neuron()
{
	switch ( *rna->current_position )
	{
		case '\"' :
			Extract_Property();
			break;

		case '`' :
			Extract_Feature();
			break;

		case '$' :
			++rna->current_position;
			Change_Gene();
			break;

		default:
		{
			String tag;
			Neuron* target = nullptr;

			rna->Pull_Tag ( &tag );

			if ( neuron->Find_Neuron ( &tag, &target ) )
			{
				Neuron* was = neuron;

				neuron = target;
				Process_Neuron();
				neuron = was;
			}
		}
	}

	if ( *rna->current_position == '.' )
		Mutate_Value();
}

void XEPL::Script::Extract_Property()
{
	char was_quote = *rna->current_position++;

	String tag;

	rna->Pull_Tag  ( &tag );
	rna->Discard_Char ( was_quote );

	truth = neuron->Property_Hunt ( &tag, value );
}

void XEPL::Script::Extract_Feature()
{
	char was_quote = *rna->current_position++;

	String tag;

	rna->Pull_Tag ( &tag );
	rna->Discard_Char ( was_quote );

	truth = neuron->Feature_Get ( &tag, value );
}

bool XEPL::Script::Get_Next_Value()
{
	if ( *rna->current_position && Extract_Value() && *rna->current_position && !error_string )
	{
		Mutate_Value();
		rna->Skip_Whitespace();
		return true;
	}

	return false;
}

bool XEPL::Script::Extract_Value()
{
	switch ( *rna->current_position )
	{
		case ' ' :
		case '\t' :
		case '\n' :
			++rna->current_position;
			rna->Skip_Whitespace();
			return Extract_Value();

		case '\'' :
		case '`' :
		case '"' :
			rna->Pull_String();
			return true;

		case '$' :
			++rna->current_position;
			Change_Gene();
			return  true;

		case '.' :
			Mutate_Value();
			return  true;

		case '{' :
			++rna->current_position;
			Enter_Inner_Block();
			return  true;

		case '@' :
			++rna->current_position;
			Process_Neuron();
			return  true;

		case '!' :
		case '%' :
		case '#' :
		case ';' :
			Tap_Term();
			return true;

		case '?' :
			++rna->current_position;
			Ternary_Choice();
			return  true;

		case ')' :
		case '}' :
		case '\0' :
			return false;
	}

	if ( Get_Property() )
		return true;

	if ( rna->Pull_Number() )
		return true;

	Record_Error   ( "Unexpected char: ",  rna->current_position );

	return false;
}

void XEPL::Script::Mutate_Value ( void  )
{
	while ( rna->Consume( '.' ) )
	{
		String opcode;
		rna->Pull_Tag ( &opcode );

		String* param = nullptr;
		if ( rna->Consume( '(' ) )
		{
			Extract_Parameter ( &param );
			rna->Discard_Char( ')' );
		}

		if ( ! cortex->Did_Operator ( &opcode, this, param )
			&& ! neuron->Performed_Macro ( &opcode, value, param, truth, value ) )
				Record_Error ( "Operator/Macro not found: ", &opcode );

		delete param;
	}
}

void XEPL::Script::Extract_Parameter ( String** _string )
{
	String* value_was = value;
	{
		value = new String();
		Get_Next_Value();
		*_string = value;
	}
	value = value_was;
}

void XEPL::Script::Enter_Inner_Block()
{
	while ( !error_string && Extract_Value() )
	{}

	if ( !error_string && rna->Discard_Char ( '}' ) && *rna->current_position )
		rna->Skip_Whitespace();
}

void XEPL::Script::Tap_Index ( String* _tag, String* _value  )
{
	if ( _value )
	{
		gene->Trait_Set ( _tag, _value );
		return;
	}

	if ( gene && gene->Trait_Get ( _tag, value ) )
		return;

	value->clear();
}

void XEPL::Script::Tap_Local (  String* _tag, String* _value   )
{
	Gene*& locals = lobe->locals;

	if ( _value )
	{
		if ( !locals )
			locals = new Gene ( nullptr, "Locals", nullptr );

		locals->Trait_Set ( _tag, _value );
		return;
	}

	if ( locals && locals->Trait_Get ( _tag, value ) )
		return;

	if ( value )
		value->clear();
}

void XEPL::Script::Tap_Property (  String* _tag, String* _value   )
{
	if (_value )
	{
		neuron->Property_Set ( _tag, _value );
		return;
	}

	if ( neuron->Property_Get ( _tag, value ) )
		return;

	if ( value )
		value->clear();
}

void XEPL::Script::Tap_Short_Term ( String* _tag, String* _value )
{
	auto terms = lobe->short_terms;

	if ( _value )
	{
		terms->Set ( _tag, _value );
		return;
	}

	if ( terms->Get_Into ( _tag, value ) )
		return;

	if ( value )
		value->clear();
}

void XEPL::Script::Tap_Term( void )
{
	String tag;
	String* revalue = nullptr;

	char was_tip = *rna->current_position++;

	rna->Pull_Tag ( &tag );

	if ( *rna->current_position == '=' )
	{
		++rna->current_position;
		value->clear();
		Get_Next_Value();
		revalue = value;
	}

	switch( was_tip )
	{
		case '!' :
			Tap_Property( &tag, revalue );
			break;
		case '%' :
			Tap_Local( &tag, revalue );
			break;
		case '#' :
			Tap_Index( &tag, revalue );
			break;
		case ';' :
			Tap_Short_Term( &tag, revalue );
			break;
	}
}

bool XEPL::Script::Hunt_Property( String* _tag )
{
	if ( _tag->empty() )
		return false;

	if ( lobe->short_terms->Get_Into ( _tag, value ) )
		return true;

	if ( lobe->locals && lobe->locals->Trait_Get ( _tag, value ) )
		return true;

	if ( neuron && neuron->Property_Hunt ( _tag, value ) )
		return true;

	Record_Error( "Can't find trait", _tag );

	return true;
}

bool XEPL::Script::Get_Property()
{
	bool indirection = rna->Consume('*');

	if ( ! rna->Starts_Tag() )
		return false;

	String tag;
	rna->Pull_Tag ( &tag );

	bool found = Hunt_Property(&tag);

	if ( found && indirection )
		found = Hunt_Property(value);

	return found;
}

void XEPL::Script::Ternary_Choice()
{
	rna->Skip_Whitespace();

	bool was_truth    = truth;
	bool return_truth = truth;

	String result_value_string;
	String previous_value_string ( value );

	if ( *rna->current_position != ':' )
	{
		Get_Next_Value();

		if ( was_truth )
		{
			return_truth = truth;
			result_value_string.assign ( *value );
		}
		else
			value->assign ( previous_value_string );
	}

	if ( rna->Consume ( ':' ) )
	{
		value->assign ( previous_value_string );
		Get_Next_Value();

		if ( was_truth )
		{
			truth = return_truth;
			value->assign ( result_value_string );
		}
	}
}

void XEPL::Script::Serialize()
{
	int depth = 0;

	while ( rna->Consume('>'))
		++depth;

	gene->Print_Into ( value, depth );
}

//    888888ba                     888888ba
//    88    `8b                    88    `8b
//   a88aaaa8P' 88d888b. .d8888b. a88aaaa8P' .d8888b. .d8888b.
//    88   `8b. 88'  `88 88'  `88  88   `8b. 88'  `88 88'  `88
//    88     88 88    88 88.  .88  88    .88 88.  .88 88.  .88
//    dP     dP dP    dP `88888P8  88888888P `88888P8 `8888P88
//                                                         .88
//                                                     d8888P

XEPL::RnaBag::RnaBag ( Cord* _bag, Script* _script )
	: ParserBag   ( _bag, _script )
	, script      ( _script )
{}

bool XEPL::RnaBag::Starts_Tag  ( void )
{
	return isalpha ( *current_position ) || *current_position == '_';
}

bool XEPL::RnaBag::In_Tag ( void )
{
	return isalnum ( *current_position ) || *current_position == '_';
}

void XEPL::RnaBag::Pull_Tag ( String* _string )
{
	Skip_Whitespace();

	if ( !Consume('*') )
	{
		Text* start_position = current_position;

		while ( In_Tag() )
			++current_position;

		_string->assign ( start_position, current_position-start_position );
	}
	else
	{
		String was_value( script->value );

		if ( script->Get_Property() )
			_string->assign( *script->value );

		script->value->assign(was_value);
	}

	Skip_Whitespace();
}

bool XEPL::RnaBag::Pull_Number()
{
	Text* start_position = current_position;

	if ( *current_position == '-' )
	{
		if ( !isdigit ( *(current_position+1) ) )
			return false;
		++current_position;
	}
	if ( !isdigit ( *current_position ) )
		return false;

	while ( isdigit ( *current_position ) )
		++current_position;

	if ( *current_position == '.' && isdigit(*(current_position+1)) )
	{
		current_position+=2;
		while ( isdigit ( *current_position ) )
			++current_position;
	}

	script->value->assign ( start_position, current_position-start_position );

	return true;
}

void XEPL::RnaBag::Pull_String ( void )
{
	char   quote          = *current_position;
	Text*  start_position = ++current_position;
	Text*  position       = start_position;

	while ( *position && *position != quote )
		++position;

	script->value->assign ( start_position, position-start_position );

	if ( !*position )
		Parse_Error("Non-Terminated string");
	else
		++position;

	current_position = position;
}

//   dP    dP            dP  888888ba
//   Y8.  .8P            88  88    `8b
//    Y8aa8P  88d8b.d8b. 88 a88aaaa8P' .d8888b. 88d888b. .d8888b. .d8888b. 88d888b.
//   d8'  `8b 88'`88'`88 88  88        88'  `88 88'  `88 Y8ooooo. 88ooood8 88'  `88
//   88    88 88  88  88 88  88        88.  .88 88             88 88.  ... 88
//   dP    dP dP  dP  dP dP  dP        `88888P8 dP       `88888P' `88888P' dP

XEPL::XmlParser::~XmlParser()
{
	delete xml_bag;
}

XEPL::XmlParser::XmlParser ( Gene* _gene, Cord* _bag )
	: Parser()
	, xml_bag       ( new XmlBag ( _bag, this ) )
	, active_node   ( nullptr )
	, root_gene     ( _gene )
{
	parser_bag = xml_bag;
}

void XEPL::XmlParser::Xml_Make_Element ( Gene* _host, Cord* _name, Cord* _space, Gene** _gene )
{
	Gene* gene = new Gene ( _host, _name, _space );

	gene->Attach();

	*_gene = gene;
}

void XEPL::XmlParser::Xml_DocType       ( String* ) {}
bool XEPL::XmlParser::Xml_New_Comment   ( String* ) { return true; }
bool XEPL::XmlParser::Xml_New_Element   ( Gene*   ) { return true; }
bool XEPL::XmlParser::Xml_End_Element   ( Gene*   ) { return true; }
bool XEPL::XmlParser::Xml_New_Content   ( String* ) { return true; }
bool XEPL::XmlParser::Xml_New_Attribute ( String*, String*, char ) { return true; }

void XEPL::XmlParser::Xml_Undo_Element ( Gene* _gene )
{
	if ( _gene && _gene->owner_link )
		_gene->owner_link->Remove_Gene ( _gene );
}

bool XEPL::XmlParser::do_DocType()
{
	if ( xml_bag->Consume ( '<', '!', 'D' ) )
	{
		String doctype;
		xml_bag->Extract_DocType ( &doctype );
		Xml_DocType ( &doctype );
		return true;
	}
	return false;
}

bool XEPL::XmlParser::do_Instruction()
{
	if ( xml_bag->Consume ( '<', '?' ) )
	{
		xml_bag->Extract_Instruction (nullptr );
		xml_bag->Skip_Whitespace();
		return true;
	}
	return false;
}

bool XEPL::XmlParser::do_BeginNode()
{
	if ( xml_bag->Consume ( '<' ) )
	{
		xml_bag->Skip_Whitespace();
		XmlNode fresh ( this );

		while ( !error_string && xml_bag->at_xmlTag() )
		{
			String spacetag;
			xml_bag->Extract_Attribute_Name ( &spacetag );
			xml_bag->Skip_Whitespace();

			if ( xml_bag->Discard_Char ( '=' ) )
			{
				xml_bag->Skip_Whitespace();
				String quoted_term;
				char quote = xml_bag->Extract_Quoted_Value ( &quoted_term );

				if ( !error_string )
				{
					xml_bag->Skip_Whitespace();

					if ( Xml_New_Attribute ( &spacetag, &quoted_term, quote ) )
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

bool XEPL::XmlParser::do_CloseNode()
{
	if ( xml_bag->Consume ( '/', '>' ) )
	{
		xml_bag->Skip_Whitespace();
		return true;
	}
	return false;
}

bool XEPL::XmlParser::do_NodeSplit ( void )
{
	if ( xml_bag->Consume ( '>' ) )
	{
		xml_bag->Skip_Whitespace();

		ParserSelect choices ( this );

		choices.Add_Option (  ParserSelect::Completes, [] (Parser* _parser ) /* "</tag >" */
							{ return static_cast<XmlParser*> ( _parser )->do_CloseSplit(); } );

		choices.Add_Option (  ParserSelect::Can_Repeat, [] (Parser* _parser ) /* "<!-- -->" */
							{ return static_cast<XmlParser*> ( _parser )->do_Comment(); } );

		choices.Add_Option (  ParserSelect::Can_Repeat, [] (Parser* _parser ) /* "<?.?>" */
							{ return static_cast<XmlParser*> ( _parser )->do_Instruction(); }  );

		choices.Add_Option (  ParserSelect::No_Flags, [] (Parser* _parser ) /* "<![CDATA[ ]]>" */
							{ return static_cast<XmlParser*> ( _parser )->do_CData(); } );

		choices.Add_Option (  ParserSelect::Can_Repeat, [] (Parser* _parser ) /* "<tag>" */
							{ return static_cast<XmlParser*> ( _parser )->do_BeginNode(); } );

		choices.Add_Option (  ParserSelect::Can_Repeat, [] (Parser* _parser ) /* >text< */
							{ return static_cast<XmlParser*> ( _parser )->do_Content(); } );
		return true;
	}
	return false;
}

bool XEPL::XmlParser::do_CloseSplit()
{
	if ( xml_bag->Consume ( '<', '/' ) )
	{
		xml_bag->Skip_Whitespace();

		String name;
		Cord* optional_string = xml_bag->Extract_Space_Tag ( &name );

		xml_bag->Skip_Whitespace();
		xml_bag->Discard_Char ( '>' );
		xml_bag->Skip_Whitespace();

		if ( active_node->element_gene->cell_name->compare ( name ) != 0
				|| ( active_node->element_gene->space_string
					 && active_node->element_gene->space_string->compare ( *optional_string ) != 0 ) )
			Record_Error ( "Bad closing tag:", name.c_str() );

		delete optional_string;

		return true;
	}
	return false;
}

bool XEPL::XmlParser::do_Content()
{
	if ( xml_bag->at_xmlPCData() )
	{
		String new_content;
		xml_bag->Extract_PCData( &new_content );

		if ( Xml_New_Content ( &new_content) )
			active_node->element_gene->Assign_Content(&new_content);

		return true;
	}
	return false;
}

bool XEPL::XmlParser::do_CData()
{
	if ( xml_bag->Consume ( '<', '!', '[' ) )
	{
		String CData;
		xml_bag->Extract_CData ( &CData );
		active_node->element_gene->Append_Content ( &CData );
		return true;
	}
	return false;
}

bool XEPL::XmlParser::do_Comment()
{
	if ( xml_bag->Consume ( '<', '!', '-', '-' ) )
	{
		String comment;
		xml_bag->Extract_Comment ( &comment );
		Xml_New_Comment ( &comment );
		xml_bag->Skip_Whitespace();
		return true;
	}
	return false;
}

bool XEPL::XmlParser::ParseIt ( void )
{
	if ( !xml_bag->xml_parser )
		return false;

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

	return !error_string;
}

//   dP    dP            dP  888888ba
//   Y8.  .8P            88  88    `8b
//    Y8aa8P  88d8b.d8b. 88 a88aaaa8P' .d8888b. .d8888b.
//   d8'  `8b 88'`88'`88 88  88   `8b. 88'  `88 88'  `88
//   88    88 88  88  88 88  88    .88 88.  .88 88.  .88
//   dP    dP dP  dP  dP dP  88888888P `88888P8 `8888P88
//                                                   .88
//                                               d8888P

XEPL::XmlBag::XmlBag ( Cord* _bag, XmlParser* _parser )
	: ParserBag ( _bag, _parser )
	, xml_parser( _parser )
{}

void XEPL::XmlBag::Discard_Shell_Directive()
{
	if ( *current_position=='#' )
	{
		++current_position;

		while ( !xml_parser->error_string
				&& *current_position
				&& ( *current_position != '\n' && *current_position != '\r' ) )
			++current_position;
	}
}

XEPL::Cord* XEPL::XmlBag::Extract_Space_Tag ( String* _name )
{
	if ( !at_xmlTag() )
	{
		xml_parser->Record_Error ( "Not at tag", current_position );
		return nullptr;
	}
	Text* start_chars = current_position;
	Text* last_colon_position = nullptr;

	while ( in_xmlTag () )
	{
		if ( *current_position == ':' )
			last_colon_position = current_position;

		++current_position;
	}

	if ( last_colon_position )
	{
		_name->append ( last_colon_position + 1, current_position - last_colon_position - 1 );
		return new Cord ( start_chars, last_colon_position - start_chars );
	}
	else
	{
		_name->append ( start_chars, current_position - start_chars );
		return nullptr;
	}
}

void XEPL::XmlBag::Extract_Attribute_Name ( String* _into )
{
	Text* start = current_position;

	if ( at_xmlTag() )
	{
		++current_position;

		while ( in_xmlTag () )
			++current_position;

		_into->append ( start, current_position-start );
	}
	else
		xml_parser->Record_Error ( "Not at attribute tag: ", start );
}

void XEPL::XmlBag::Extract_CData ( String* _into )
{
	long  length = 0;
	Text* start = nullptr;

	if ( *current_position != 'C'          ||
			* ( current_position+1 ) != 'D' ||
			* ( current_position+2 ) != 'A' ||
			* ( current_position+3 ) != 'T' ||
			* ( current_position+4 ) != 'A' ||
			* ( current_position+5 ) != '[' )
		xml_parser->Record_Error ( "Invalid CDATA start", current_position );
	else
		current_position += 5;

	start = current_position+1;

	while ( !xml_parser->error_string && Remaining() >= 2 &&
			! ( * ( ++current_position ) == ']' &&
				* ( current_position+1 ) == ']' &&
				* ( current_position+2 ) == '>' ) )
		++length;

	current_position += 3;

	if ( length && _into )
	{
		_into->assign ( start, length );
		String scratch;
	}

	Skip_Whitespace();
}

void XEPL::XmlBag::Extract_PCData ( String* _into )
{
	Text* start    = current_position;
	Text* position = start;

	while ( in_xmlPCData ( *position ) )
		++position;

	current_position = position;

	Text* tail = position-1;
	while ( iswspace ( *tail ) )
		--tail;

	if ( _into )
		_into->append ( start, tail-start+1 );
}

void XEPL::XmlBag::Extract_Comment ( String* _into )
{
	Text* start = current_position;

	while ( *current_position && !Consume ( '-', '-', '>' ) )
		++current_position;

	if ( _into )
		_into->append ( start, current_position-start-3 );
}

void XEPL::XmlBag::Extract_DocType ( String* _into )
{
	if (* ( current_position+0 ) != 'O' ||
		* ( current_position+1 ) != 'C' ||
		* ( current_position+2 ) != 'T' ||
		* ( current_position+3 ) != 'Y' ||
		* ( current_position+4 ) != 'P' ||
		* ( current_position+5 ) != 'E' )
	{
		xml_parser->Record_Error ( "Invalid DOCTYPE start", current_position );
	}

	current_position+=6;

	Text* start = current_position;
	int nest  = 1;

	while ( *current_position && !xml_parser->error_string && nest )
	{
		if ( *current_position == '<' )
			++nest;
		else if ( *current_position == '>' )
		{
			if ( !--nest )
				break;
		}
		++current_position;
	}
	if ( _into )
		_into->assign ( start, current_position-start );

	Skip_Whitespace();
}

void XEPL::XmlBag::Extract_Instruction ( String* _into )
{
	Text* start = current_position;

	while ( !xml_parser->error_string && Remaining() >= 2 && !Consume ( '?', '>' ) )
		++current_position;

	if ( _into )
		_into->append ( start, current_position-start );
}

char XEPL::XmlBag::Extract_Quoted_Value ( String* _into )
{
	char  quote    = *current_position;
	Text* start    = ++current_position;
	Text* position = start;

	while ( !xml_parser->error_string && *position && *position != quote )
		++position;

	if ( *position == quote )
	{
		_into->append ( start, position-start );

		current_position = position+1;

		return quote;
	}
	current_position = position;
	xml_parser->Record_Error ( "Missing closing quote: ", start );
	return 0;
}

//   dP    dP            dP 888888ba                 dP
//   Y8.  .8P            88 88    `8b                88
//    Y8aa8P  88d8b.d8b. 88 88     88 .d8888b. .d888b88 .d8888b.
//   d8'  `8b 88'`88'`88 88 88     88 88'  `88 88'  `88 88ooood8
//   88    88 88  88  88 88 88     88 88.  .88 88.  .88 88.  ...
//   dP    dP dP  dP  dP dP dP     dP `88888P' `88888P8 `88888P'

XEPL::XmlNode::~XmlNode()
{
	xml_parser->active_node = parent_node;

	if ( element_gene )
	{
		element_gene->Release();

		parser_wants_it = xml_parser->Xml_End_Element ( element_gene ) && parser_wants_it;

		if ( !parser_wants_it )
			xml_parser->Xml_Undo_Element ( element_gene );
	}
}

XEPL::XmlNode::XmlNode ( XmlParser* _parser )
	: xml_parser ( _parser )
	, parent_node                ( nullptr )
	, element_gene               ( nullptr )
	, parser_wants_it            ( true )
{
	Gene* parent = nullptr;

	if ( xml_parser->active_node )
		parent = xml_parser->active_node->element_gene;
	else
		parent = xml_parser->root_gene;

	String tag;
	Cord* space = xml_parser->xml_bag->Extract_Space_Tag ( &tag );

	xml_parser->Xml_Make_Element ( parent, &tag, space, &element_gene );

	delete space;

	parser_wants_it  = xml_parser->Xml_New_Element ( element_gene );

	parent_node = xml_parser->active_node;
	xml_parser->active_node = this;

	xml_parser->xml_bag->Skip_Whitespace();
}

//   dP    dP            dP  888888ba           oo dP       dP
//   Y8.  .8P            88  88    `8b             88       88
//    Y8aa8P  88d8b.d8b. 88 a88aaaa8P' dP    dP dP 88 .d888b88 .d8888b. 88d888b.
//   d8'  `8b 88'`88'`88 88  88   `8b. 88    88 88 88 88'  `88 88ooood8 88'  `88
//   88    88 88  88  88 88  88    .88 88.  .88 88 88 88.  .88 88.  ... 88
//   dP    dP dP  dP  dP dP  88888888P `88888P' dP dP `88888P8 `88888P' dP

XEPL::XmlBuilder::~XmlBuilder()
{
	Close();
}

XEPL::XmlBuilder::XmlBuilder ( Text* _name, Text* _attrs, String* _bag )
	: tag_n_space( new String ( _name ) )
	, build_string          ( _bag )
	, attributes_closed     ( true )
	, build_closed          ( false )
{
	build_string->push_back (  '<' );
	build_string->append    ( *tag_n_space );
	build_string->push_back ( ' ' );
	build_string->append    ( _attrs );
	build_string->push_back (  '>' );
}

XEPL::XmlBuilder::XmlBuilder ( Text* _name, String* _bag )
	: tag_n_space ( new String ( _name ) )
	, build_string          ( _bag )
	, attributes_closed     ( false )
	, build_closed          ( false )
{
	build_string->push_back (  '<' );
	build_string->append    ( *tag_n_space );
}

XEPL::XmlBuilder::XmlBuilder ( Cord* _name, String* _bag, Cord* _space )
	: tag_n_space ( new String ( _space ) )
	, build_string          ( _bag )
	, attributes_closed     ( false )
	, build_closed          ( false )
{
	if ( _space )
		tag_n_space->push_back (  ':' );

	tag_n_space->append     ( *_name );
	build_string->push_back (  '<' );
	build_string->append    ( *tag_n_space );
}

void XEPL::XmlBuilder::Absorb_Traits ( Nucleus* _nucleus, Gene* _gene )
{
	if ( !_gene )
		return;

	if ( !_gene->traits )
		return;

	Cord*   name = nullptr;
	String* term = nullptr;
	String  term_evaluated;

	StableTraits stable_gene ( _gene );

	while ( stable_gene.Next_Trait ( &name, &term ) )
	{
		term_evaluated.clear();
		Evaluate_Inner_Scripts ( _nucleus, _gene, term, &term_evaluated );
		Attribute_Set ( name, &term_evaluated );
	}
}

void XEPL::XmlBuilder::Attribute_Set ( Text* _name, Text* _term )
{
	build_string->push_back ( ' '   );
	build_string->append    ( _name );
	build_string->push_back ( '='   );

	String term ( _term );
	Escape_Quotes ( &term, build_string );
}

void XEPL::XmlBuilder::Attribute_Set ( Cord* _name, Cord* _term )
{
	build_string->push_back (  ' '   );
	build_string->append    ( *_name );
	build_string->push_back (  '='   );

	Escape_Quotes ( _term, build_string );
}

void XEPL::XmlBuilder::Close_Attributes()
{
	if ( attributes_closed )
		return;

	attributes_closed = true;

	build_string->push_back ( '>' );
}

void XEPL::XmlBuilder::Close()
{
	if ( build_closed )
		return;

	build_closed = true;

	if ( attributes_closed )
	{
		build_string->append ( "</" );
		build_string->append ( *tag_n_space );
		build_string->push_back('>');
	}
	else
		build_string->append ( "/>" );

	delete tag_n_space;
	tag_n_space = nullptr;
}

//   dP    dP                   dP dP    dP            dP
//   Y8.  .8P                   88 Y8.  .8P            88
//    Y8aa8P  .d8888b. 88d888b. 88  Y8aa8P  88d8b.d8b. 88
//   d8'  `8b 88ooood8 88'  `88 88 d8'  `8b 88'`88'`88 88
//   88    88 88.  ... 88.  .88 88 88    88 88  88  88 88
//   dP    dP `88888P' 88Y888P' dP dP    dP dP  dP  dP dP
//                     88
//                     dP

XEPL::XeplXml::XeplXml ( Gene* _gene, Cord* _cord )
	: XmlParser ( _gene, _cord )
{
	ParseIt();
}

bool XEPL::XeplXml::Xml_New_Element ( Gene* _gene )
{
	return _gene->cell_name->front() != '_';
}

//    88888888b                                      888888ba                                        dP
//    88                                             88    `8b                                       88
//   a88aaaa    88d888b. 88d888b. .d8888b. 88d888b. a88aaaa8P' .d8888b. 88d888b. .d8888b. 88d888b. d8888P
//    88        88'  `88 88'  `88 88'  `88 88'  `88  88   `8b. 88ooood8 88'  `88 88'  `88 88'  `88   88
//    88        88       88       88.  .88 88        88     88 88.  ... 88.  .88 88.  .88 88         88
//    88888888P dP       dP       `88888P' dP        dP     dP `88888P' 88Y888P' `88888P' dP         dP
//                                                                      88
//                                                                      dP

XEPL::ErrorReport::~ErrorReport()
{
	std::unique_lock lock_output ( output_lock );

	std::cout << "ErrorReport: " << c_str() << std::endl;
}

XEPL::ErrorReport::ErrorReport ( Text* _text, Cord* _cord )
	: ErrorReport ( _text )
{
	if ( _cord )
		append( *_cord );
}

XEPL::ErrorReport::ErrorReport ( Text* _text, Text* _chars )
	: ErrorReport ( _text )
{
	append( _chars );
}

void XEPL::xeplCantFind ( Text* _type, Nucleus* _nucleus, Cord* _name )
{
	xeplCantFind( _type, _nucleus, _name->c_str() );
}

void XEPL::xeplCantFind ( Text* _type, Nucleus* _nucleus, Text* _name )
{
	String temp;
	if ( _nucleus )
	{
		_nucleus->Nucleus_Path ( &temp, '/' );
		temp.push_back (  ' ' );
	}
	ErrorReport error_message ( "Can't find " );
	error_message.append(_type).append(": ").append(temp).append(_name);
}

thread_local XEPL::Lobe* XEPL::tlsLobe = nullptr;
std::mutex   XEPL::output_lock;
