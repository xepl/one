
# XEPL Operating Environment C++

XEPL is a software operating environment for applications to use multi-threading.

When anything can be connected with ease, programming takes on a whole new aunce (nuanced, different, and much easier).

XEPL builds into a software Operating Environment for applications to use multi-threading on a common world view of data.  In contrast, the Operating System is trying to be all that for your application, and so much more.  

The Operating Environment isn't trying to replace your Operating System (XEPL uses the OS to be portable); Instead, XEPL is just trying to be the best Environment for your software to execute in.  An environment tailor maid by you, where every XEPL noun and verb is unique to the problem you are trying to solve.

What that means is that, you build tiny c++ widgets, then the OE handles the low-level data organization, sharing and inter-neuron signal activations, while you write pure, cooperating Neurons and interactions in XEPL XML.

Meaning, the complexity of writing parallel programs in C++ is gone, replaced only ... by the next problem you have to wrap your head around ... learning to solve problems with parallel agents ... and in this case, by using Neurons and Signals to coordinate your work on a Common World View of data.

So, I guess ... We'll be building on those solutions you come with ... for a long time to come.

Every beginning ... has a ... beginning, even if it takes 20 years.  Welcome to XEPL.

![Beautiful Multi-Threading](https://xepl.net/images/concept/DALL·E%202024-02-19%2012.43.56.jpg)

## Print ... Hello, World!

	#include "xepl.hpp"
	
	int main ( int, char**, char** )
	{
	    // Instantiate your cortex
	    XEPL::Cortex cortex ( "brain", std::cout );
	    
	    // Add the keyword
	    cortex.Register_Keyword ( "Print", []
	    ( XEPL::Neuron* _neuron, 
	      XEPL::Gene*   _gene, 
	      XEPL::String* _param ) 
	    {
	       std::cout << *_param << std::endl;
	       _neuron->Process_Inner_Genes ( _gene );
	    } );
	  
	    // Invoke the keyword
      cortex.Execute_As_Xml("<Print>Hello, World!</Print>");
	}

// The above code prints: Hello, World!

## Building XEPL

XEPL is just as simple as compiling Hello, World!

Be sure that you have a C++ Compiler. XEPL uses C++17.

First get the source code (copy/paste in cli)

`mkdir xepl`

`cd xepl`

`git clone https://github.com/xepl/one.git one`

`git clone https://github.com/xepl/kits.git kits`

`cd one`

`g++ -std=c++17 simple.cpp -o simple`

## Multi-Threading confused with multi-processing

Multi-processing was born out of exclusion, to protect one process from another ... where Multi-threading is natural inclusion.

In multi-threading, we want our threads to share all the information, and sharing the processing workload without system barriers, and without unnecessary blocking.  In XEPL, it is clear that the System architect can know what needs protection, and what doesn't.

How, sharing is bigger than the language itself.  Multi-threading Sharing is an integrated concept all its own; while it may use the primitives of the language ... the act of sharing is something in the Environment, not a part of the language.

XEPL OE is a core/kernel/cortex for safely sharing multi-threaded data.

## Multi-Agent designs

What is a multi-agent design? ... and what is Distributed Computing ... really?

I can write a Bank Application single threaded, where there is a queue at the door that lets one customer in at a time, and the teller handles the customer, even physically accesses the vault, in their behalf.

Its a simple application, that is guaranteed to produce accurate results, all of the time.

But, its a bit slow.  Multiple tellers could prepare transactions, and runners could make the deposits/withdrawal's.

It's an improvement that comes with complexity, and, even it can be slow, when bogged down.

## Multi-threaded complexity

Building the parallel bank in native C++ requires you to use low level primitives to create threads, mutex for critical sections, and semaphores for signaling.

Then you build queues using those primitives, and find a way to block/terminate your threads.  Memory is still a linear layout, so every thread has an API, and you build message passing.  Of course, you could just lean on the Operating System ... but, why leave the comfort of your own application.

Once you decide on all the mechanics of all that, get your glue code up and running to marshal in/our of the APIs ... then, you can start writing your parallel banking application.

You have to wonder, why?  Is it worth it? ... because before XEPL ... Multi-threading was hard ... very hard, and the likelihood of coming out of integration ... was slim.

When Multi-Cores arrived, the walled in process dominated, and developers were never really able to scratch the surface of what is available.

## C++17

C++ is the best language, bar none.  C++ delivers Top performance, extensible, flexible, portable, and zero active runtime. Better yet, you can connect C++ to anything.

XEPL is 100% Pure C++17.  Runs on my Mac, and on my Pi.  The XEPL OE is platform independent, portable C++.

The XEPL Kits are where you add binary Components to XEPL.  Once registered, your Kits are available to XEPL XML.  How easy is that?  Better yet, you pick the words ... making your XML Dialect ... obscure.

XEPL uses almost everything C++ has to offer ... except for leaning on templates and shared_ptr.  XEPL is a pure inheritance based environment, and there are only a few unique objects in XEPL.  

Meaning, In XEPL there are only a few base classes the OE needs (combinations are powerful); Hence, you can write more application logic, and less, unnecessarily error prone, data manipulation logic.

## XEPL Lobes vs Neurons

Multi-threading is hard, and there are a lot of ideas out there on how to do it.  They all share a common vernacular, so when I branched out to solve the problem, I needed new terms to encapsulate my ideas.

The first such is that a Thread is too low level.  In XEPL a Lobe is a Thread.  Now, everything that happens inside the Lobe is single threaded, thread-safe.  Neurons are thread-safe, and data is processed serially and sequentially by the Lobe.

The Neuron is a symbolic-computer, where every Neuron can synapse with any Axon, and is scheduled to run by the OE whenever the signal is Triggered.  Its simple and elegant.

Recall, that Neurons can be composed of Neurons.  Signal Relays are present in every Neuron when inner Neurons Synapse.  All the way up to the Lobe.

Where it is the Lobe, that is a Neuron with special signaling powers.  When an inner Neuron extends an Axon that is synapsed by another Neuron, the Lobe that Triggers the Signal places an Action in each Lobe ActionList.  The ActionList wakes its Lobe, and the inner_neurons can process and/or Relay the Signal to deeper synapsed, inner-neurons.

## Binary XML is the Gene database

To continue with the neuroscience theme, XEPL is controlled by Genes.  Genes contain all the DNA required to construct and signal complex systems, using the base Keywords and Operators you defined for your system.

XML is the structure of the Gene.  Similar to XML the Gene has a Tag (name), a list of properties (attributes), a collection of inner_genes (elements), and a many-to-many connected wire (content).

The XEPL XML Parser seamlessly moves strings into a DNA gene, and you can then access that Gene via the RNA::Script. {$gene/path/to/inner_gene'attribute'}.

## XEPL Ins and Outs

XEPL uses C++11 thread_local storage to provide a Common Operating Environment for the Lobes.

The Common Operating Environment is a Shared, Binary XML Database.  What that means is that everything, the XML code and the data, are stored in DNA::Genes.

Even the system state, lobe state, neuron state, and scope state are also in Genes.

Binary XML is like Text XML all laid out in memory.  You know XML as Text XML ... a 2D flat-file (tag, attributes, elements, content), while Binary XML is 3D connected (any element can contain or be contained by any other element), and in XEPL OE, Binary Data is 4D as its moving through a pipeline.

But, here's the thing .. data doesn't move.  XEPL uses pointer magic (its main reason for not preferring references), to swap the value of the expected pointers, kind of like a state based thread_local storage.

### XEPL XML Parsing

XEPL has its own XML parser integrated into directly into the Operating Environment.

The XML Parser is a fast recursive descent Gene builder.

The parsing capabilities are conformant to standards, your extensions need not be ... obscurity is good.

That extends to the RNA::Script parser as well. RNA is a simple string based machine, but as an inner scripting language ... you can extend it to do so much more.



## Primitive Shell

In the simple.cpp I coded XML using C++ Strings.  Its for simplicity in explanation.

XEPL can dynamically load and unload its Neurons; 
XEPL has a primitive shell in the KITS::CLI you can used to inspect and alter a running cortex.

However, typing Neurons in at the Shell is highly error prone.

I recommend you keep your XEPL XML code in .xml files, and find a nice XML editor.  Remember, XML must always be well-formed.

# XEPL is Smart Dynamic Memory

Real-time C++ has always been challenged by new/delete.  Threads going to the Operating System for memory, can really bog things down.

XEPL is smart; it uses the XEPL::Lobe to Recycle previous allocated memory, avoiding unnecessary deletes.

How, XEPL adds a per-thread backpack to recycle previous allocations (places them in the Lobes backpack), skipping the delete, to, later return the  allocation as new (all without blocking).

Simply derive a class from the XEPL::Recycler and ... wallah ... recycled memory blocks.  Memory blocks  can be allocated in one thread as one class instance, and deleted by another representing a different instance of a different class.  (blocks are allocated by size, not by class type)

## The Atom and the Cell

Above the Recycler is the Atom and the Cell.

The Atom is a reference counted block of memory. And the Cell adds a "name".  Names are only unique within a collection.

Above that, there are many kinds of Cells, and like the Atom, anything can be Recycled.

In a nutshell, that does away with using shared_ptr, as XEPL logic is aware of when a pointer is being retained.

Overall, the Atom determines the lifetime.

## The Recycler

It isn't a typically class recycler, it recycles the underlying block of memory of xx bytes, and shares that between classes of xx bytes.

Neurons are for processing DNA, and DNA is a Data Structure that can be composed of cross-connected Genes of cross-connected DNA, offering a three dimensional, instant access no schema database.

DNA powers Neurons with instructions to build and refine the Operating Environment that signal and all co-operate on a common world view of data.

## What is XEPL

Never before, has anyone, gone back this far and challenged the rationale, that a distributed system ... need, in any way ... be limited. 

Or that C++ ... is more about getting dynamic behavior out of inheritance, and less about templates solving static problems.

Both are good ... XEPL uses Inheritance.

# The Example

	#include "xepl.hpp"
	
	int main ( int, char**, char** )
	{
	    XEPL::Cortex cortex ( "brain", std::cout );
	    
	    cortex.Register_Operator ( "space", []
	    ( XEPL::Script* _script, 
	      XEPL::Cord*   _rhs )
	    {
	    	_script->value->push_back ( ' ' );
	     	if ( _rhs )
			     _script->value->append ( *_rhs );
	    } );
	    
	    cortex.Execute_As_Rna(
		    "'RNA'.space('Xepl')");
	}
	
// Your first RNA.operator - it just adds a space


	#include "xepl.hpp"
	
	int main ( int, char**, char** )
	{
	    XEPL::Cortex cortex ( "brain", std::cout );

	  // Register_Keyword("Print");
	  // Register_Operator("space");

	   cortex.Register_Keyword ( "Method", []
	   ( XEPL::Neuron* _neuron, 
	     XEPL::Gene*   _call_gene, 
	     XEPL::String* )
	   {
	     XEPL::String name;
	     if ( _call_gene->Trait_Get("name", &name ) )
	     {
	        _neuron->Register_Method ( &name,
	           &XEPL::Nucleus::Method_Execute,
	           _call_gene );
	     }
	  } );
	  
	  cortex.Execute_As_Xml(
	    "<Method name='Speak'>"
	       "<Print>{'Speak'.space(to)}</Print>"
	    "</Method>");

		cortex.Execute_As_Xml(
			"<brain.Speak to='method param'/>");
	}


### The Source Code

C++ is a language of Inheritance.  XEPL makes extreme use of Inheritance; as such, the XEPL Core is implemented in under 5,000 lines of C++ source code.

Using another 5,000 LOC, the XEPL Engine (XE) offers functional Brain.Kits. XEPL is complete and independent of the brain.Kits.
