> [!WARNING]  
> This project must be integrated before using it. It's a simplified design that may not be compatible with 
> already existing implementation of known filesystems.


# RedFS: reduced filesystem for low power platform and OS implementations

<br>

RedFS is a simple filesystem implementation for **low power** and **simple operative systems** built to work 
with small devices like microcomputers or microcontrollers, or generally in a limited computational environment. 

<br>

<br>

RedFs embed different functions to easily interact with the filesystem and the disk generally, giving 
a simplified access to the disk partition table, boot sector and each partition, allowing a simplified 
tree navigation and file manipulation. 

<br>

Due to the fact that it's designed for small and simple systems the structure speaking of disk partitioning 
and how it's built could be different from the *modern way of doing things* (whatever it means), but be 
compatible with existing system it's not one of redFs main goals, it's designed to be as simple as possible 
without wasting resources while still providing an easy-to-understand structure and functions with to interact with the filesystem. 
If you need backward compatibility with different filesystems ... then you should find another project. 

<br>

---

<br>

## Quick views on the main redFs specs:

<br>

* **The boot sector of 512** byte wide starting from the base of the disk.

<br>

* **Max addressable space** ( 32bit pointer limit ): 4gb.

<br>

* **Max of 256 partition** supported by the partition table
 
<br>

* **Custom partition table** located in the first bytes of the disk immediately after the boot sector.

<br>

* **Auto-caching system:** after a predefined threshold redFs can automatically synch back changes to the disk.

<br>

* **Simplified API system** 

<br>

---

<br>

# RedFs API structure 

<br>

RedFs API functions are flagged with "REDAPI" preprocessor flag to simplify finding the required functions 
from only the header file. Each API functions is preceded by a summary documentation explaining the behaviour 
and oftentimes providing example and suggestions on how to use them properly. 

<br>

One of the main target of **redFs is maintaining simplicity, thus the library is designed to be self-explorable** 
without any additional documentation other than the source code itself. The header files provide a summary description
and the source code provide a local source to understand how a certain function works under the hood. 

<br>

The file structure is divided by hierarchy, with files that implements base functionality used by other functions in other 
file to construct further abstractions. All of redFs condense the input/output inside **"src/lib/redFs_io.c"**, the real point 
where all the upcoming traffics is redirected to a physical device ( *more on that in the following chapter* ). 
Then the core of RedFs is build inside **"src/lib/redFs.c"** which use the I/O functions provided to construct the partitioning 
system of the library and implements all the necessary functions to **initialise disks**, **create partitions**, **erase partitions** 
and so on. In the same level **"src/lib/redFs_node.c"** create the abstraction layer to **create and link nodes** for 
each partition, **link nodes with other nodes**, **dealloc nodes** and so on. 
On top of redFs_node.c two files, namely **"src/lib/redFs_folder.c"** and **"src/lib/redFs_file.c"** implement specific function 
(starting from the generic ones inside the node implementation) to work with files and folder, especially for a simple 
**folder creation**, **folder nesting** or **file creation**, **file read action**, **write actions**, **research option** and so 
on. All of this with **unix-like compatible path description** that cn be accepted by file and folder functions, **it will be 
parsed and a tree navigation will be performed automatically**.

<br>

<br>

#### NOTE: "redFs_node.c" does not provide API-ready functions like "redFs.c", those are built and provided by the specific abstraction for files and folder.

<br>

# How to build your RedFs library

<br>

RedFs comes with the makefile which can build a test program (which test the redFs functions) and redFs as 
a static library. The static library target is provided to create a *modern system compatible* library that 
can be linked by the compilers in Unix based system. 

<br>

### To build the test program:

<br>

```bash

make test

```

<br>


The test executable can be found inside *"src/test"* folder. It spawn three different processes that test redFs 
on different aspect such **disk formatting**, **general allocation**, **file manipulation**, **directory creation** 
and so on. 

<br>

For design choice, the testing programs divided by categories implements a real use case of redFs with the practice 
suggested by the library. **It's suggested to look inside *"src/test"* for practical example on how to implement 
a program that uses redFs's API functions.**

<br>

---

<br>

### How to build the library target

<br>

As stated redFs can be compiled as a static library that can be linked with different executable during compilation 
time.

<br>

```bash

make lib 

```

<br>

#### But a reasonable amount of doubts might emerge, and one is:

<br>

> What it means 'compiling a static library' really with custom integrations and custom systems?

<br>

The library target it's meant to produce a static library compatible with modern unix-like systems such 
all distribution of linux, BSD or MacOs, or generally anything that can read a \*.a library and link 
it with an existing executable, but of course **it is not an usable target for dedicated scenario** 
which may differs a lot from unix like system, even at the level of compilation or linking process. 

<br>

**It's obvious that it's not possible to take in account different scenarios and that was one reason for why 
the entire library depend on just two standard end point-of-failure for reading and writing files**. 
The same can be applied to custom compilation for different systems that may require different process 
or stages to produce a working version of redFs. 

<br>

---

<br>

### Example on a possible non-compatible scenario 

<br>

If you want to compile redFs as a static library for a generic **microcontroller** ( such arduino ) 
then you can follow a similar process of what's is shown inside the makefile. 
You just need to customise the I/O ( further discussed in the next section ) and select the right compiler 
for your microcontroller. Then you can just produce a single library that can be used to link with 
the executable of your system. 

<br>

From what's it's already present, **you just need to update the compiler and the I/O implementation.**

<br>

---

<br>

### Example of a more complex scenario 

<br>

Let's set a simple microcomputer, with a simple processor with discrete power, and with a limited amount 
of resourced. Let's imagine that this hardware has a simple runtime environment which is meant to work with 
simple applications, even in parallel. 

<br>

This runtime environment does not have a compatible executable format like the one used for linux, and does 
not have a compatible linker as it's present on linux, but it has a simple C compiler that can produce 
object files in a custom format and compile it in a working "library" by using a simple referencing system 
for functions call inside the objects. 
To work with this system redFs must include the header files of the system to communicate with memory devices. 

<br>

This scenario require a custom building process of redFs that must follow this limits, and of course the output 
cannot be compatible with modern systems, not only on a binary level ( where CPU instructions may differs ) but 
also on the entire executable structure or library structure. 

<br>

**Each system which is not unix-based will differs inevitably from the standard compilation structure described 
inside the makefile, for this reason it's not possible to use the lib target for each and every platform**

<br>

# Working with the I/O: how to integrate RedFs with your system 

<br>

RedFs is based on two main functions that communicate with one memory drive, handling the **READ** and **WRITE**
operations. The library's section related to the I/O operations are located inside *src/lib/redFs_io.c* and the 
header inside *src/include/redFs_io.h*. 

<br>

You **MUST** customise those functions based on the system where you want to integrate redFs. This design choice 
was made to allow a quick and simplified interaction with the system, isolating the main functions where all the 
traffics of redFs pass through, and this allow a customisable way to integrate redFs in different system, even 
with virtual drives as it is shown already inside the implementation compilable with VIRTIO flag during compilation 
time. 

<br>

## The main functions on which redFs is build on top of:

<br>

```C

// WRITE
int redFs_disk_action_write(RED_PTR address, uint8_t data, ...);

// READ
int redFs_disk_action_read(RED_PTR address, uint8_t* data, ...);

```

<br>

Those accept the address of the memory location alongside the data (1 byte) that need to be written or read. It's possible 
to add as many auxiliary functions as desired to connect redFs with your system, but you should remember that the WRITE and 
READ function must be integrated with what you're implementing. **You can check out the existing example inside the redFs_io.h 
header for more informations, or directly check out the implementation inside redFs_io.c to see how the virtual tunneling 
with the testing suit was made.**

<br>

For further details check *src/include/redFs_io.h* header file. 

<br>

---

<br>

## Main desing


The library uses 32bit pointers to organise information inside the drive, this was mainly a 
simplification choice since it would have required a more flexible design for each fstab 
depending on the individual size of the partition, so instead of having a fixed block size 
array of pointer it would have had a flexible block size array of pointer to reduce and adapt 
the fstab size without wasting too much space with small partition, but this would have required 
too many operation that could raise the complexity and the disk usage for the drive, and since 
it's not meant to be used with enormous files and since it's meant to be used with low power and 
simple system this solution was not adopted. 

Instead we have a fixed array of memory block that define the page of each partition, this array 
have a fixed size and the length is the maximum supported size / block size. 
Each block size is 32K bit due to a simple reason: it's easier to map nodes inside each block. 
Each block has a start address and a "fragment map" which is just a 32bit number. This is used 
to keep track of the fragmentation inside each memory block, and to offset the right amount of 
data from the starting point to allocate a new node, node that is 1024 byte long. For each 
bit every node is allocated and each bit keep track of the offset zone from the starting 
point. If one bit is set to 1 then the location corresponding to that bit is occupied by a node, if 
the bit is set to 0 then the zone is free and ready to be allocated. 

This simple bitmap is used to easily track movement and allocation inside each block of memory without 
wasting space and resources that would ended up increasing the size of the fstab for nothing. 


Since each node is 1024 byte long, and each memory block has a fragment map of 32bit, every memory 
block can index a total of 32K of memory, or 32 node for each memory block. 

// TBF
