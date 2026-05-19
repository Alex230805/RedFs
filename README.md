
> [!WARNING]  
> This project require custom integration of simple endpoints. It's a simplified design that may not be compatible with 
> already existing implementation of known filesystem. It's suggested to use redFs if you need a simple and fast filesystem 
> implementation for simple operative systems, runtime environment and so on. 

# RedFS: reduced filesystem for low power platform and OS implementations

---

RedFS is a simple filesystem implementation for **low power** and **simple operative systems** built to work 
witth small devices like microcomputers or microcontrollers, or generally in a limited computational environment. 


RedFs embed different functions to easily interact with the filesystem and the disk generally, giving 
a simplified access to the disk partition table, boot sector and each partition, allowing a simplified 
tree navigation and file manipulation. 


Due to the fact that it's desinged for small and simple systems the structure speaking of disk partitioning 
and how it's built could be different from the *modern way of doing things* (whatever that means), but be 
compatible with existing system it's not one of redFs main goals, it's designed to be as simple as possible 
without wasting resources while still providing an easy-to-understand structure with accomodating functions 
to interact with the filesystem. 
If you need backward compatibility with different filesystems ... then you should find another project. 

#### Quick vews on the main redFs specs:


* **The boot sector is 512** byte wide starting from the base of the disk.

* **Max addressable space** ( 32bit pointer limit ): 4gb

* **Max of 256 partition** supported by the partition table 

* **Custom partition table** located in the first bytes of the disk immediately after the boot sector  

* **Auto-caching system:** after a predefined threshold redFs can automatically sinch back changes to the disk 

---

## How to build your RedFs library


RedFs comes with the makefile which can build a test program (which test the redFs functions) and redFs as 
a static library. The static library target is provided to create a *modern system compatible* library that 
can be linked with the compilers in Unix based system. 


To build the test program:


```markdown

make test

```

The test executable can be found inside *src/test* folder. It spawn three different processes that test redFs 
on different aspect such **disk dormatting**, **general allocation**, **file manipulation**, **directory creation** 
and so on. 


#### It's suggested to look inside the testing program's source code for practicle example on how to use redFs 

---

It's possible to use the files located inside *src/test* for practicle example on how to use redFs functions to 
operate on the disk. Each test file, namely **file.c** **dir.c** and **format.c**, divides the three possible 
operations that you may want to integrate with existing applications in your system; inside those files you 
can find and take suggestion on what's possible to do with redFs and what it looks like to work with redFs. 
For more informations you can directly consults the header files inside *src/include* which provide a description for 
each API functions that are designed to be used by applications or by the system. 

---

#### How to build the library target


As stated the library compile redFs as a static library that can be linked with different executable during compilation 
time, it's suggested to read first **[how to integrate redFs with your system](/#how_to_integrate)**.

```markdown

make lib 

```

> What it means 'compiling a statis library' really with custom integrations and custom systems?


The library target it's meant to produce a static library compatible with moder unix-like systems such 
all the distribution of linux, BSD or MacOs, but of couse **it is not an usable target for dedicated scenario** 
which may differs a lot from unix like system, even at the level of compilation or linking process. 


It's obvious that it's not possible to take in account different scenarios and that was one reason for why 
the entire library depend on just two standard end point for reading and writing files. The same can be 
applied to custom compilation for different systems that may require different process or stages to produce 
a working version of redFs. 

---

#### Example on a possible scenario 


If you want to compile redFs as a static library for a **microcontroller** ( such arduinos ) then you can 
follow a similar process of what's is shown inside the makefile. You just need to customize the I/O ( further 
discussed in the next section ) and select the right compiler for your microcontroller. Then you can just 
produce a single library that can be used to link with the executable of your system. 


From what's it's already present, you just need to update the compiler and the I/O implementation. 


#### Example of a more complex scenario 


Let's set a simple microcomputer, with a simple processor with descrete power, and with a limited amount 
of resourced. Let's imagine that this hardware has a simple runtime environment which is meant to work with 
simple applications, even in parallel. 


This runtime environment does not have a compatible executable format like the one used for linux, and does 
not have a compatible linker as it's present on linux, but it has a simple C compiler that can produce 
object files in a custom format and compile it in a working "library" by using a simple referencing system 
for functions call inside the objects. 
To work with this system redFs must include the header files of the system to communicate with memory devices. 


This scenario require a custom building process of redFs that must follow this limits, and of course the output 
cannot be compatible with modern systems, not only on a binary level ( where CPU instructions may differs ) but 
also on the entire executable structure or library structure. 


> **Each system which is not unix-based will differs inevitably from the standard compilation structure described 
inside the makefile, for this reason it's not possible to use the lib target for each and every platform**

---

## How to integrate RedFs with your system {#how_to_integrate}


RedFs is based on two main functions that communicate with one memory drive, handling the **READ** and **WRITE**
operations. The library's section related to the I/O operations are located inside *src/lib/redFs_io.c* and the 
header inside *src/include/redFs_io.h*. 


You **MUST** customize those functions based on the system where you want to integrate redFs. This design choise 
was made to allow a quick and simplified interaction with the system, isolating the main functions where all the 
traffics of redFs pass through, and this allow a customizable way to integrate redFs in different system, even 
with virtual drives as it is shown alreay inside the implementation compliable with VIRTIO flag during compilation 
time. 

#### The main functions on which redFs is build

```C

// WRITE
int redFs_disk_action_write(RED_PTR address, uint8_t data, ...);

// READ
int redFs_disk_action_read(RED_PTR address, uint8_t* data, ...);

```

Those accept the address of the memory location alongside the data (1 byte) that need to be written or read. It's possible 
to add as many auxilary functions as desired to connect redFs with your system, but you should remember that the WRITE and 
READ function must be integrated with what you're implementing. **You can check out the existing example inside the redFs_io.h 
header for more informations, or directly check out the implementation inside redFs_io.c to see how the virtual tunneling 
with the testing suit was made.**


For further details check *src/include/redFs_io.h* header file. 

---

## Main desing


The libray uses 32bit pointers to organize information inside the drive, this was mainly a 
semplification choise since it would have required a more flexible design for each fstab 
depending on the individual size of the partition, so instead of having a fixed block size 
array of pointer it would have had a flexible block size array of pointer to reduce and adapt 
the fstab size without wasting too much space with small partition, but this would have required 
too many operation that could raise the complexity and the disk usage for the drive, and since 
it's not meant to be used with enormous files and since it's meant to be used with low power and 
simple system this solution was not adopted. 

Instead we have a fixed array of memory block that define the page of each partition, this array 
have a fixed size and the legth is the maximum supported size / block size. 
Each block size is 32K bit due to a simple reason: it's easier to map nodes inside each block. 
Each block has a start address and a "fragment map" which is just a 32bit number. This is used 
to keep track of the fragmentation inside each memory block, and to offset the right amount of 
data from the starting point to allocate a new node, node that is 1024 byte long. For each 
bit every node is allocated and each bit keep track of the offsetted zone from the starting 
point. If one bit is set to 1 then the location corresponding to that bit is occupied by a node, if 
the bit is set to 0 then the zone is free and ready to be allocated. 

This simple bitmap is used to easily track movement and allocation inside each block of memory without 
wasting space and resources that would ended up increasing the size of the fstab for nothing. 


Since each node is 1024 byte long, and each memory block has a fragment map of 32bit, every memory 
block can index a total of 32K of memory, or 32 node for each memory block. 


## Disk management

redFs include a simple implementation of a partition table and a boot sector on the first memory 
zone of the disk. 
It's used to map the partition and their state, is used to deallocate and allocate new partition 
and to switch between one and another easily. 

Inside the main redFs.h you can find function to manipulate or initialize the partition table inside 
the disk which find it place after the 512 byte of boot sector that has no specific structure and 
can be used as preferred by the bootloader or any other program. 
The partition table is only used during the allocation of new partition and for erasing them from 
the disk, it's important to consider to touch as less as possible this part since you can damage and 
lose data from one or another partition if something is not done correctly with a specific order. For 
that reason inside "redFs.h" ( the main header ) there is a section for the "interal" functions used to 
compose more complex operations from least complex ones, and a section of "public" functions that 
integrate more complex operations. If you need to create a new partition or to initialize or operate with 
the partition table is suggested to use the public function unless you know what could happen if you 
use one or another specific "internal" functions. 

Inside the "redFs_testbench.c" there are examples and testing environment to see which function does what 
and to understand how to use the library.

To operate on one or more partition you can require a partition header for each partition which has a copy 
of the partition fstab with some informations about the partition in general. Every operation on file, 
folders and so on are done with the use of the partition header and the fstab copy, this is done to increase 
the speed of the operations since the allocatio/deallocation need to use the fstab to find the right 
place inside the first available memory block and also to allow different interaction window for different 
partitions, so if this is used inside a complex os it's possible to have multiple activity with different 
partitions all active at once, you just need to provide the right header connected to one specific partition 
and nothing more. 

But it's important to synch back new modifications on the fstab from time to time do avoid losing data, so each 
header has a cache timing variable which is increased every time each functions that accept a partition header 
require a node action ( which can be an allocation or deallocation, a search a remove and so on ), and when the 
roof is reached the fstab is synched back to the partition. 
Before closing the interaction and "umount" the partition it's important to synch any remaining data inside the 
partition header after the latest autosynch to the drive, so there is a dedicated function to do that and it's 
important to remember that every single time, no matter what, you need to call this function before dumping 
your partition header and start working on something different. 

Again inside the testbench file you can find example on how this can be done.

## File and Folder

Each file or folder is a node, a node is a little data structure 1024 byte long which contain little informations 
about the current node, pointers for a continuation of the node and an array of content. You may need file or folder 
bigger than the limit of the internal array inside each node, maybe you need a folder with 1024 files inside. 
To allow this each node has a chained flag and a next_page pointer which can connect the current node with a second 
one like an extension to increase the amount of information that can be stored as a single entity, a file or a folder. 

File and folders are similar structure, with a difference in the content array: the folder stores pointers and the 
file stores byte of data. 
The folder store pointers to other elements like other file and folder. The file store information directly from the 
source provided by the specific functions and if needed it will extend the node with other nodes to fit the required 
data. 

Those specific node are sub-implementation of a more generalized node implementation that can be found inside "redFs_node.c". 
There many functions are used to allocate node, deallocate them, list them, append content and so on. They are not meant to 
be used directly by the system or by the user, those are specific functions created to construct the more specific 
behaviour of files and folder that find their implementation inside "redFs_file.c" and "redFs_folder.c" respectively.

Again there are public and not public functions inside "redFs_file.c" and "redFs_folder.c" and the difference is in the 
path dependent and independed behaviour. The partition header have a current_node pointer which is used to define the 
current position inside the partition ( intended as a folder, for example "/" or "/home" ), every action on file or 
folder are designed to work with the current_node pointer and not with a general path given as input. That's because 
those are internal functions used next with a wrapper to implement the path independent functions which navigate 
the partition three and then call the path dependend version on the latest entry to complete the job.

Generally speaking the path independent functions are the one usable by the user to implement something or to operate on 
the drive, just for a matter of simplicity. 

Inside of the "redFs_testbench.c" is possible to see some example for file manipulation and folder manipulation.
