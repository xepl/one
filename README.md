
# XEPL One C++

Welcome to tomorrow, today.  XEPL is a complete system-in-a-box, a one library does it all.  To simplify your introduction into XEPL, it is broken down into ones.

Prepare to be Dazzled !

- One header
- One implementation file
- One line installation

Here is intro1.cpp

```
#include "xepl.h"

int main ( int, const char**, char** )
{
	XEPL::Brain brain ( "Amazing" );

	new int;

	std::cout << "Hello, World!\n";
	std::cout << "XEPL detects leaks\n";
}
```

To build your XEPL Library its as easy as.

```
g++ -std=c++17 xepl.cc -o libXepl.a
```

To build your XEPL Application

```
g++ -std=c++17 -L. -lxepl intro1.cpp -o intro1
```

To run your XEPL Application

```
./intro1
```

Here is the output.

```
Hello, World!
XEPL detects leaks
Other news 6 deletes 5
 ---LEAKING new/delete: 1
Started at: Fri Sep 15 11:20:07 2023
Elapsed time: 00:00:00.000
```

There is a lot going on there.  And, That is just a taste.  It doesn't get any harder than that; it just keeps getting better.

---

> **This is not the main XEPL Project**.  This is XEPL One. THE FLAGSHIP! This unique seed can be licensed for many different purposes.

>> Over at XEPL.Tech, collectively, we are building the future of XEPL.

>>> There we will ALL share.  The DAZL makes it so.

>>>> Join the conversation ... Join https://XEPL.Tech.

---

## Description

XEPL is software: computer source code in ASCII C++ that offers a complete execution environment to anyone and everyone.

XEPL isn't complex, but it's a powerful, general-purpose software system that approaches multi-core hardware in a unique way.

Oh, and let's not forget—XEPL is easy. So easy, in fact, that even a novice programmer can start doing amazing things with their computer. (P.S., that's how I got my start—just playing around with it.)

Listen up, everyone: this is the dawn of a new software era. You're as clued in as anyone right now.

> Yeah, we absolutely learn by doing!

Guess what? Everyone's a beginner when it comes to XEPL. Even me. Sure, I wrote the Brain, but that doesn't mean I understand how to think like one. And considering XEPL's infinite possibilities, we're all just getting started.

I'm certain you'll encounter new ideas, insights, and revelations almost daily. I know I do.

> What a thrilling time to be alive!

>> Say goodbye to that dull software.

>>> Become part of something way bigger.

## Growing Forward

XEPL is a software architecture inspired by neuroscience terminology. Go ahead, look up terms like Cell, Nucleus, Neuron, Axon, Synapse, DNA, and RNA for some approximate context. 

> Ah, the greatest challenge in software? Grasping the mindset of the programmers who came before us.

> Sure, we all went to school—not just to learn how to program (you can do that here)—but to learn the *vocabulary*. The lingo of the programmers from back in the day.

> Here's the kicker: XEPL is brand new. My terminology mimics the Brain, so don't expect to find a course on it at your local university. Get ready to multitask your learning. After all, XEPL is about many-to-many relationships in numerous ways.

> Where does it all begin? With a seed. This is my seed, my one-to-many gift to the world of programming.

I've done some cool stuff, let me tell you. I integrated dynamic connections into a multi-threaded memory recycler. Transformed linear RAM into an in-memory, forest-like database. Moved those nitty-gritty C++ details into a section I call the Cortex. Oh, and I whipped up a robust RNA scripting language that's embedded everywhere in the system. You get to write or select your XML keywords and Trait operators, and then you wire up your Cortex using XML.

Within XEPL's XML framework, you can connect Neurons to Neurons and Genes to Genes. You can craft new Methods and Macros, generate renderings, and send Signals zooming along Axons. Your Genes can even self-evaluate and trigger themselves along those Axons. And yes, you guessed it, it's all in XML.

> Quick note: XEPL's C++ isn't your grandparent's C++.
- No #defines, 
- no (Type& parameters), and definitely 
- no template\<Bloat\>.

> Here's the deal: Use C++ to build small, precise, binary modules. Then watch in awe as XEPL parses its XML, constructs interconnected models with multiple threads, and completes intricate signaling tasks—all before traditional C++ can even start compiling. How's that for turnaround efficiency?

## XEPL XML

XEPL XML? It's not just any XML; it's a game-changer! Each XML record in XEPL holds Methods, Parameters, and Blobs, and can even link to other XML records. Better yet, XEPL XML is accessible concurrently to all threads.  Imagine a playground so adaptive, it can leap from a whisper-quiet computational state into an all-out, multi-core frenzy. And then, as if nothing ever happened, it settles back into its serene setting. 

In this realm of XEPL XML, the sky's the limit. You can construct robust, intricate structures designed to stand the test of time, operating 24/7 indefinitely. Or, if you're the impatient sort, whip up short-lived processors that zoom through tasks in mere microseconds. But why limit yourself? With XEPL XML, you can—and most likely will—do both. Versatility like you've never seen!

## XEPL: Where Software Meets Rock 'n' Roll

- Many-to-many data connections
- Many-to-many processing connections
- Recursive data
- Recursive processing

Let's talk multi-core. XEPL harnesses native multi-threading to turn your apps into distributed applications within a single process. Forget serialization, forget request/response, and certainly forget latency. We're talking instant access to all the recursive traits and genes.

- Single process without external dependencies
- Multi-threading without the blockade
- Dynamic loading and unloading of Neurons

Did I mention the in-memory, cross-connected binary XML database? This natural banyan tree-like lattice structure allows XEPL to tackle large volumes of structured data instantly. Dynamic connections between XML genes further offer the flexibility to adapt to a forever-changing environment and stimuli.

- Lives purely inside the CPU/RAM
- Squeezes the most out of multi-core technology
- Efficiently uses all available memory
- Operates completely asynchronously

Let's not overlook the XEPL Neurons—they're downright clever. Designed with recursion and cross-connection in mind, these Neurons can send and receive signals along Axons without breaking a sweat. And it doesn't stop there. The DNA Genes in XEPL have the ability to self-signal changes along those very same Axons.

- Quick compiles to get you moving
- Even faster execution turnaround
- Modular design for portable simplicity
- Designed for Extensibility

## What XEPL Definitely Isn't

XEPL is not some simple, single-threaded Python ordeal. Nor is it bogged down by the multi-threaded semaphore complexities of yesteryear. What XEPL is, however, is a dynamic, cross-connected, self-managed data system with context. Forget Python; XEPL is faster, easier, and soon you'll find that XEPL's C++/XML combo will be your one and only development playground.

Get started today.

Say adios to:
- Page faults
- System calls
- Message passing
- Serialization
- Operational latency
- And those pesky project delays

So why wait for tomorrow? With XEPL, the future is already here.

## License

> Licensed under the DAZL License (DAZL–1.0) (the License)

> Obtain the License at https://xepl.net/licenses/dazl–1.0.md

## Installing a C++ Build System

If you are a programming noob, you will need a C++ build system installed on your machine.

This will depend on your particular platform:

### Optional: Doxygen Installation

Doxygen, though not strictly necessary, will prove useful for exploring XEPL.

## Unleash Your Potential with XEPL

Contributing to XEPL isn't just a win for technology—it's a win for you. As a DAZL-1.0 Contributor, your smarts supercharge the project and may even carve out a path for you to commercialize your brainchildren. 

Ready to get in on the action? Head to [https://xepl.tech](https://xepl.tech).

Now, how about you say goodbye to that day job? Trust me, they don't appreciate your mad skills, and you're over the old stuff anyway. A word of caution, though: Your company might not be a fan of the license, so peek at that employee agreement, will ya?

When you contribute to XEPL, you keep your intellectual property. Share it with the XEPL community, and together, let's build something phenomenal.

Again, jump into the conversation at [https://xepl.tech](https://xepl.tech).

Joining the XEPL technology community places you right in the thick of things, shaping the software's developmental arc. I'm totally convinced that our combined talents will churn out top-notch end-products. We'll kick the billion-line-code madness to the curb and create a win-win for everyone.

### Share Ideas, Build Connections

Contributors, you're what's going to make XEPL kick major butt in the long run. By getting involved, you're part of a massive shift aimed at totally reinventing the software game for profit.

Now, don't get it twisted—contributing to XEPL isn't just about slinging code. The knowledge you bring to the table? That's the secret sauce. Blog posts, tutorials, or just throwing in some two cents, it all makes the XEPL ecosystem a heck of a lot stronger. So cheers to you for making this a dynamic, robust community.

Thanks, you're awesome.

### Get in the XEPL Ring

XEPL isn't just some code—it's a full-blown movement. We're not just creating software; we're gathering a tribe of people who know they can change the game, one contribution at a time. We're also making real-world connections that outlast the project itself. The code's just the starting line; it's the community that's the real firepower here.

# The Genesis of XEPL

For over two decades, I've been a lone wolf developer pouring my soul into XEPL. My focus may have wavered occasionally, but ultimately, I zeroed in on one ambitious venture. What emerged is computer science expressed as a heady blend of neuroscience, multi-threading, and recursion. The result? An elegantly self-actuating, recursive XML structure. Hold onto your hats, it's all contained in fewer than 20,000 lines of code, with a maximum complexity of 19 and an average complexity of just 2.

> Doing the math? That's fewer than 1000 lines of C++ per year. Each line is optimized for peak efficiency!

Here's the deal: XEPL isn't just another software—it's a game-changer. Say goodbye to the bloated, clunky programs of yesteryears. With XEPL, the future of streamlined, efficient software is happening right now. Leaner, faster, and more resilient against those irksome vulnerabilities.

# The Final Word: The DAZL-1.0 License and XEPL's Future

DAZZLE alert! XEPL rocks a brand-new, source-available license: DAZL-1.0. The why is simple: to make sure future project mavericks like you can get paid for your smarts. This license tackles the free-for-all commercial abuse of today's open-source scene, while preserving the bedrock principle of open-source: keep the source code in the public domain!

The DAZL license is poised to catch fire. Though many projects will undoubtedly be spin-offs of XEPL, DAZL is also an option for fresh, grassroots initiatives.

Here's the kicker: If you've got a vision but don't know how to realize it, XEPL is already here, getting better every second.

- For high-quality discussions, make a beeline to [XEPL.Tech](https://xepl.tech).
- For licenses and premium support, it's [XEPL.Services](https://xepl.services) for you.
- Craving some iconic swag? Head over to [XEPL.Shop](https://xepl.shop).

Welcome to the XEPL revolution.