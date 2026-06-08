> [!WARNING]  
> This project must be modified before using it. It's a simplified design that may not be compatible with 
> already existing implementation of known filesystems.

> [!TIP]
> **The documentation about REDAPI functions is located inside the header files of the library**.
> This readme is a mere introduction to the library and to the design of it, the documentation dedicated 
> to how to use this library is located inside *"src/include/\*.h"* and is suggested to check it along 
> side the example inside *"src/test/format.c"*, *"src/test/file.c"* and *"src/test/dir.c"* for possible usage. 

# RedFS: reduced filesystem for low power platform and OS implementations

RedFS is a simple filesystem implementation for **low power** and **simple operative systems** built to work 
with small devices like microcomputers or microcontrollers, or generally in a limited computational environment. 

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


* **The boot sector of 512** byte wide starting from the base of the disk.


* **Max addressable space** ( 32bit pointer limit ): 4gb.


* **Max of 256 partition** supported by the partition table
 

* **Custom partition table** located in the first bytes of the disk immediately after the boot sector.


* **Auto-caching system:** after a predefined threshold redFs can automatically synch back changes to the disk.


* **Simplified API system** 

<br>

---

# RedFs API structure 


RedFs API functions are flagged with "REDAPI" preprocessor flag to simplify finding the required functions 
from only the header file. Each API functions is preceded by a summary documentation explaining the behaviour 
and oftentimes providing example and suggestions on how to use them properly. 

<br>

One of the main target of **redFs is maintaining simplicity, thus the library is designed to be self-explorable** 
without any additional documentation other than the source code itself. The header files provide a summary description
and the source code provide a local source to understand how a certain function works under the hood. 

<br>

The file structure is divided by hierarchy, with files that implements base functionality used by other functions in other 
file to construct further abstractions. All of redFs condense the input/output inside **"src/lib/redFs_io.c"**, the 
real point where all the upcoming traffics is redirected to a physical device ( *more on that in the following chapter* ).<br>
 
Then the core of RedFs is build inside **"src/lib/redFs.c"** which use the I/O functions provided to construct the partitioning system of the library and implements all the necessary functions to **initialise disks**, **create partitions**, **erase partitions** and so on. In the same level **"src/lib/redFs_node.c"** create the abstraction layer to **create and link nodes** for each partition, **link nodes with other nodes**, **dealloc nodes** and so on. <br>
On top of redFs_node.c two files, namely **"src/lib/redFs_folder.c"** and **"src/lib/redFs_file.c"** implement specific function (starting from the generic ones inside the node implementation) to work with *files* and *folder*, especially for a simple 
**folder creation**, **folder nesting** or **file creation**, **file read action**, **write actions**, **research option**.

<br>

All of this with **unix-like compatible path description** that can be accepted by file and folder functions, **it will be 
parsed and a tree navigation will be performed automatically**.

<br>


> [!NOTE]
> *"redFs_node.c"* does not provide API-ready functions like *"redFs.c"*, those are built and provided by the 
> specific abstraction constructed to implement files and folder.

---

# How to build your RedFs library

RedFs comes with the makefile which can build the test program and redFs as a static library. <br> 
The static library target is provided to create a *modern system compatible* library that can be linked 
by compilers in Unix base environment. 

<br>

### To build the test program:

```bash

make test

```

<br>

The test executable is located inside *"src/test"* folder. It spawn three different processes to test redFs 
on things such **disk formatting**, **general allocation**, **file manipulation**, **directory creation** 
and so on.
It's suggested to launch the test suit every release to see if the library behaviour is predicable and stable and 
test new feature before porting the new version. 

<br>

For design choice each testing programs is divided by categories and for each one it implements a real use case of redFs 
by following the instructions and best practice from the library's header file. 

<br>

> [!TIP]
> **It's suggested to look at the implementation *"file.c"*, *"dir.c"* and *"format.c"* inside *"src/test"* for examples 
> of a possible usage of REDAPI functions.**

---

### How to build the library target

As stated redFs can be compiled as a static library that can be linked with different executable during compilation 
time. This target is designed for modern system and does not require any special adaptations **except for the endpoint**
used by redFs to read and write onto a support:

<br>

```bash

make lib 

```

<br>

> What it means 'compiling a static library' with custom integrations and custom systems.

<br>

The **lib** target it's meant to produce a static library compatible with modern unix-like systems such 
all distribution of *linux*, *BSD* or *MacOs*, or generally *anything that can read a \*.a* library and link 
it with an existing executable compatible with POSIX standard, but this **is not an usable target for dedicated scenario** 
which may differs a lot from modern systems, even at the level of compilation or linking process. 

<br>

**It's obvious that it's not possible to take in account different scenarios. This is the reason for why 
the entire library depend on just two standard point-of-failure for reading and writing files** and the 
same can be applied to custom compilation process for different systems that may require different stages 
to produce a working version of redFs. 

<br>

> [!IMPORTANT]
> It goes without saying that if you are planning on using this library for your dedicated environment 
> ( which can be a small OS or runtime environment ) you certainly already know that and you already know 
> how to adapt the building process for your needs.


---

### Example on a possible non-compatible scenario 


If you want to compile redFs as a static library for a generic **microcontroller** ( such as arduino  ) 
then you can follow a similar process for what is already inside the makefile. 
You *just need to customise the I/O (further discussed in the next section)* and select the right compiler 
for your microcontroller. Then you can just produce a single library that can be used to link with 
the executable of your system. 

<br>

By starting from what is already given, **you just need to update the compiler and the I/O implementation.**

---

### Example of a more complex scenario 

> Let's imagine a simple microcomputer, with a simple processor with discrete power, and with a limited amount 
> of resources. Let's imagine this hardware running a simple runtime environment which is meant to work with 
> simple applications, even in parallel. 

<br>

The runtime environment does not support a compatible executable format like the one used for linux, and does 
not have a compatible linker as it's present on linux, but it has a simple C compiler that can produce 
object files in a custom format and compile it down to a working "library" by using a simple referencing system 
for functions call inside the objects file. 
To work with this system redFs must include the header files of the system to communicate with memory devices. 

<br>

This scenario require a custom building process of redFs that must follow this limitations, and of course the output 
cannot be compatible with modern systems, not only on a binary level ( where CPU instructions may differs ) but 
also on the entire executable structure or library structure. 

<br>

This is still a relatively simple scenario, many times you may need more workaround to fix the building process 
on whatever you are trying to port redFs.

> [!IMPORTANT]
> **Each system which is not unix-based will differs inevitably from the standard compilation structure described 
> inside the makefile, for this reason it's not possible to use the lib target for each and every platform**

---

# Working with the I/O: how to integrate RedFs with your system 

RedFs is based on two main functions that communicate with one memory drive, handling the **READ** and **WRITE**
operations. The library section related to the I/O operations is located inside *src/lib/redFs_io.c* and the 
header inside *src/include/redFs_io.h*. 

<br>

> [!CAUTION]
> You **MUST** customise those functions based on the system where you want to integrate redFs and handling the 
> disk access is nor part of redFs and must be handled by the host's systems.. 

This design choice was made to allow a quick and simplified interaction with any system, isolating the main functions 
where all the traffics of redFs pass through, and allowing a customisable way to integrate redFs in different systems, even 
with virtual drives as it is shown already inside the implementation compilable with VIRTIO flag during compilation 
time. 

<br>

> [!TIP]
> See inside **makefile** for the compilation of target **test**.

### The main functions on which redFs is build on top of:

Each endpoint is declared as follow:

```C

// WRITE
int redFs_disk_action_write(RED_PTR address, uint8_t data, ...);

// READ
int redFs_disk_action_read(RED_PTR address, uint8_t* data, ...);

```

<br>

Those functions accept the address of memory location alongside the data (1 byte) that need to be written or read. <br>
It's possible to add as many auxiliary functions as desired to connect redFs with your system, but you should remember that the WRITE and READ function must be integrated with what you're implementing.<br> 
**You can check out the existing example inside the redFs_io.h header for more informations, or directly check out the implementation inside redFs_io.c to see how the virtual tunnelling with the testing suit was made.**

<br>

> [!TIP]
> For further details check *src/include/redFs_io.h* header file. 

---

# Main desing


The library use 32bit pointers to organise information inside the drive, this was mainly a 
simplification choice since it would've required a more flexible design for each fstab 
depending on the individual size of the partition to avoid extreme oversize 
for each fstab without introducing a dynamic allocation system for it. 

<br>

Instead we have a fixed array of memory block that define the **page** of each partition, it's an array 
with fixed size where the length is the **maximum supported size / block size**. 
**Each block size is 32K** and has a *start address* and a *"fragment map"* which is a 32bit number. <br>
The *"fragment map"* is used to keep track of the fragmentation inside each memory block, and to offset the right amount of 
data from *the starting point address* to allocate a new node, *node that is 1024 byte long*.<br> 
The position of each bit of the fragment map define the *offset* from the *start address*, and each 
*flagged bit ( with value == 1 ) define an allocated page*.

<br>

This simple bitmap is used to track movement and allocation inside each block of memory without 
wasting space and resources that would ended up increasing the size of the fstab. <br>
**Since each node is 1024 byte long, and each memory block has a fragment map of 32bit, *every memory 
block can index a total of 32K of memory, or 32 node for each memory block.***

## Allocation

With the use of the bitmap for each memory block each partition can dynamically allocate and deallocate 
new node of size 1024 byte. <br>
*If the block is empty ( bitmap/fragment map == 0 ) then the block is flagged as* **FREE_BLOCK**, *otherwise is flagged 
with* **ACTIVE_BLOCK** to indicate that it's used by the filesystem. <br>
*If the fragment map* of one block *is equal to the maximum value it can store* ( each bit is flagged with 1, which means each offset is already used ) then the block is flagged as FULL_BLOCK. <br>
During the allocation the library will search for every block that is **ACTIVE_BLOCK** or **FREE_BLOCK** ( selecting first the 
active ones ), avoid **FULL_BLOCK and RESERVED_BLOCK**. <br>
**Every block that is flagged as RESERVED_BLOCK is dedicated to map the space used by the fstab of the partition**, space 
that cannot be overwritten of freed.

---

## Fstab structure 

The Fstab for each partition store the array of memory block and the relative array to indicate the state of the memory 
block, the partition id ( reference from the partition table ), the name associated with the partition along side the 
*redFs_id* ( unique magic number used to verify the integrity of the fstab when is fetched from the drive ), the **block 
limit** which define the size of the partition along side the free block count, **the version of the fstab** which is **the 
same as the library** to flag avoid operating with incompatible major version and the address of the entry point 
which is the root folder of the partition. 

<br>  

### Red_Header for active partition 

To operate with the library it's necessary to have a quick way o interact with the fstab of each partition, for this reason 
**every functions**, different from the partitioning ones, **that perform any kind of memory manipulation and tree navigation** are based on the data structure *Red_Header* which store the current *active state* of the fstab from the partition. 

<br>

*A partition is considered active* when there is a instance of Red_Header obtained by the dedicated functions ( *check redFs.h 
for more informations* ) still used by the system. <br>
When a function accept Red_Header and perform a memory manipulation ( allocating a node or removing a node ) it will 
write the necessary node inside the partition but **it will update the cached version of the fstab inside Red_Header.** <br>
With that it's possible to use a caching system associated with the active fstab; **it is synched automatically by redFs when a certain amount of write operations is reached** or manually if you invoke the dedicated function ( see redFs.h for more informations ).

<br>

**Every modification to the filesystem that is not synched back to the drive will be lost if not cached by the autosynch.**

<br>

> [!NOTE]
> RedFs does not directly synch different instance of the same partition fetched as Red_Header by two separate process, it 
> is left to the system the managing of different cached version of the partition to synch and avoid memory integrity 
> problems and data dependency problems between different processes.

--- 

## Node Structure 

Datas and folder are substructure obtained by a standardised node structure which methods are implemented inside *"src/lib/redFs_node.c"*.<br>
Each node type can fit inside 1024 bit, each memory block of 32Kb can store and map up to 32 node.<br>

```C

typedef struct Red_File Red_File;
typedef struct Red_Node;

```

**Red_Node** is the generalised structure which can directly act as a folder, the main use is to organise the tree structure 
and store files.<br>
**Red_File** is similar to Red_Node but it can store the stream of data associated with the file content.

<br>

They both share a *type* ( to identify if the node is a folder or a file ), the **father node** ( f_node to perform a tree navigation from the relative position of a folder or a file), *name* of the node, the *permissions* of the node and the ***chained flag***. <br>
The ***chained flag*** identify the node when extended with another one to allow a **greater storage of raw data** ( if the node is file ) **or node pointer** ( if the node is a folder ). When a folder or a file is chained it use the *"prev_page"* and *"next_page"* address to link with different node.<br>
**This feature is enabled by default and cannot be disabled**, it allow an unlimited storage of data inside 
virtual elements, namely folder or file, *without the limit of the 1024 byte for each node*.

<br>

The method's implementation relative to file and folder manipulation is located inside *"src/lib/redFs_file.c"* and *"redFs_folder.c"*, while the documentation is provided inside *"src/include/redFs_file.h"* and *"src/include/redFs_folder.h"* like 
for the entirety of the library. <br>
Functions located inside *"src/lib/redFs_node.c"* and declaration inside *"src/include/redFs_node.h"* can be used by more 
specific usage of the library although they are not part of the API meant to be used directly, every functions associated
with folders and files are based on general functions located in the node.c implementation.

<br>

---

# Partition table

redFs follow his standard to map partitions inside the drive and it use a data structure named **Red_ptable** to store 
different array to define the disk structure. <br>
The partition table compatible with redFs **must** store the *maximum disk size* of the initialised disk, the *partition 
count* which function as a index for the partition table's array, and *arrays of pointers of each partition* to indicate the 
address where the fstab of the partition is located, *the size of each partition*, the *partition id* of each partition and 
the *partition name*.

<br>

During the allocation of a new partition a **unique ID** is generated to identify the partition with the name given. 
**The generation of the partition id start from 1000 and increase by 1 starting from the greater partition id found 
inside the partition table**. <br>
If for example the partition table is empty the first valid partition id is *1000*, but if the partition table is 
populated by for example three partitions and the greatest partition id inside is *1011* then the new partition id 
will be *1012* and so on. <br>
If a partition is *erased* and/or *reallocated* it will **NOT** maintain the same partition id, same if the partition is formatted, the partition id will still be obtained by following the same criteria, so if you delete the partition *1001* and 
the maximum partition id present inside the partition table is *1024* then the new id for the new partition will be 
*1025*.

<br>

> [!NOTE]
> The maximum amount of partition limit is 256 for each disk.
> **The partition table is ordered by pointers and not by priority or declaration or id, this to simplify searching for 
> a new free space for a new partition between existing ones.**

--- 

# Boot sector 

**The first 512 byte available to read and write of the disk are reserved to raw code that can be executed by the system
as a boot sector** to initialise a boot process. <br>
It's possible to write this boot sector by using the dedicated function inside redFs.

<br>

It's possible to change the boot sector size although is not recommended since it diverge from the original design 
of the library. To do so you can change the preprocessor macro **BOOT_SECTOR_SIZE** and recompile the library. <br>
**It's not suggested as operation since it is not compatible with standard redFs versions** and if the boot sector 
execute code from redFs but not compiled to address the new increased size it will cause unpredictable behaviour. <br>
***If you increase the size be sure to use the right redFs version modified to work with the new bootsector size.***

---

# Error system

RedFs use his own error system to keep track of errors inside the library and use his own **strerror** and **errno** to 
grep information about the error of the immediately used functions. 

<br>

Every redFs API function that return a number generally means that it can return an error code compatible with *"redFs_strerror()"* and *"redFs_print_strerror()"* functions, **for more information every API functions explain the return process if not 
immediately understandable by the function signature.** <br>
Each error is stored during runtime inside *"redFs_errno"* which is a static variable that store the latest exit code of the 
most recently called REDAPI function. The return code of the functions is generally equal to the exit code inside *redFs_errno*.

<br>

The error type is an enumerator that map a specific string inside the header *redFs.h*, the error code start from 
0 indicating the normal operation status and for value greater than 0 it indicate the error. 

<br>

