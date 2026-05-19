
> [!WARNING]  
> This project require custom integration of simple endpoints. It's a simplified design that may not be compatible with 
> already existing implementation of known filesystem. It's suggested to use redFs if you need a simple and fast filesystem 
> implementation for simple operative systems, runtime environment and so on. 

# RedFS: reduced filesystem for low power platform and OS implementations

RedFS a filesystem designed to be capable of indexing a maximum of 4gb of storage 
of any type, offering a modern and easy to use API to allow a simple integration 
with custom OS or Runtime Environments. It offer modern-like partitioning while 
maintaning a simple and easy to modify architecture. 


This was initially designed for low power machines like microcontrollers or old 
microcomputer system that needed some kind of integration with bigger memory support 
without introducing too much complexity. 


## Simplified API to allow an easy integration


This filesystem is designed to be extremely simple and easy to integrate in different 
kind of scenarios due to the base API it uses. Since it's not possible to create a 
universal junction between different hardware and software system the library 
uses two general read and write functions located inside "redFs_io.c", those are the 
main endpoints of the library from which every physical request is sent by the library.

Essentially to integrate this project with your situation you need to modify the read and 
write implementation to adapt them to your kind of scenario. For example, inside an operative 
system you may find different sofware implementation that serve as node to connect different 
type of device call and abstract them as a general entry point for an higher development level, 
the read and write function will be modified to use this abstraction layer and then is the OS 
to do the job, selecting the device and useing the driver to properly communicate with it, the 
filesystem is only there to translate and write it own data structure inside the device, the how 
and when is decided by the abstraction layer connected inside the end point of the library. 

Another example is the virtual implementation of the testbench file. The makefile uses two 
different commands to compile the library: 

- make testbench
- make lib

The difference is that inside the testbench the endpoint of the library are modified to work 
with a virtual drive interpreted by a single file in the project directory. By doing so the 
testbench can run all the necessary test function and operate ( from the redFs perspective ) 
like on a normal drive, while the endpoint ( the main read and write function ) redirect the 
request and directly translate it to work with the virtual file stream. 


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
