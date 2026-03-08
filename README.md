# RedFS: reduced filesystem for low power platform and os implementations


RedFS a filesystem designed to be capable of indexing a maximum of 4gb of storage 
of any type, offering a modern and easy to use API to allow a simple integration 
with custom OS or Runtime Environments. It offer modern-like partitioning while 
maintaning a simple and easy to modify architecture. 

It was designed first to work with microcontroller or low power system like 16bit 
microcomputer and so on, for that reason many design choice were influenced by limitations 
that forced a simple design and optimized architecture, easy to modify in case 
of possible new addition and easy to maintain.



### Custom Disk management system for simple partitioning 


RedFS it's not only a file system implementation, it ships an entire partitioning system 
that manage the boot sector, the partition table and the partition offset and allocation 
inside the device. It's not "standard" and compatible with existing system, at least that 
was not the intent. 

The boot sector is responsible for the execution of custom code, generally a bootloader.
The default size is 512 byte which is enough to store a small text section to boot a 
specific partition or doing whatever the system require. The bootloader is not managed 
by the redFs library, it only provide a small space for the system.


The partition table is a small datastructure immediately after the boot sector and store 
the partition count, the partition addresses and the partition sizes. It's used to map 
and keep track of changes inside the device. It's inserted after the boot sector to easily 
calculate an offset and grep the address of one or more partition.

TODO: to complete

