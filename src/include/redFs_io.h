#ifndef REDFS_IO_H
#define REDFS_IO_H

#include "redFs.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>


/*
 *	redFs_io.c and redFs_io.h provide the simplest access point to a phisical drive. 
 *	Every functions of redFs are based on top of a predefined READ and WRITE functions provided 
 *	by this library section to simplify integrating it with different systems and scenarios, even 
 *	with virtual peripherals like what was implemented for running the testing program.
 *
 *	READ: 
 *		redFs_disk_action_read(RED_PTR address, uint8_t* data, ...);
 *
 *	WRITE:
 *		redFs_disk_action_write(RED_PTR address, uint8_t data, ...);
 *
 *	Thoe are the main functions. For any implementation it's possible to add custom functions which 
 *	initialized the connection for the physical drive, but keep in mind that every single traffic 
 *	pass through those two main function. 
 *
 *	As an example, the testing sequence initialized by the programs inside "src/test" use redFs library 
 *	with the redFs_io object section compiled with  VIRTIO defined,  which activate two auxilary 
 *	functions that provides an initialization point for a virtual memory ( a File )  and a closing function 
 *	to terminate the I/O stream. The two "redFs_disk_action_write(RED_PTR address, uint8_t data, ...)" and 
 *	"redFs_disk_action_read(RED_PTR address, uint8_t* data, ...)" contain a section, which is compiled only 
 *	with VIRTIO defined, with a custom implementation to work with this virtual system ( by using a global FILE* ).
 *
 *	NOTE:
 *
 *	You MUST provide an implementation for the READ and WRITE access point, it's not possible to operate with redFs 
 *	without providing a connection to those two access points. 
 *
 *	EXAMPLE OF A POSSIBLE IMPLEMENTATION:
 *
 *	In a simple runtime environment, you may have a memory structure which abstract the interaction with every single 
 *	device, including memory. This memory node is the main junction point to interact, throughout the system's kernel and drivers, 
 *	with the device. Since redFs just need a connection with READ and WRITE operation you can create auxiliary functions 
 *	to interact with the kernel's node system and get access to only the selected devices. Once this connection is enstablished 
 *	the system can invoke redFs and, which a good implementation of the READ and WRITE function, it can take controll over 
 *	the system's driver to communicate with those abstract node, writing to and reading from the device. 
 *
 */


#ifdef VIRTIO

/*	NOTE: those functions are related to the virtual drive implementation used by the testing program. You DON'T need this 
 *	section if you are not planning to compile a testing version of redFs 
 */

/*
 *	Static file pointer, it's used when redFs is compiled with the VIRTIO flag abilitated and provide a standard end point, 
 *	or a simulated drive, to work with the custom implementations of the READ and WRITE functions of redFs.
 *	You don't need to compile redFs with VIRTIO.
 *
 */

static FILE* fp; 

/*
 *	Before using redFs with a simulated drive, you need to call this function to open a file stream which is used 
 *	to simulate a drive interaction, alongside custom implementations designed for operating with virtual memory 
 *	located inside the READ and WRITE function of redFs. 
 *
 */

void redFs_open_static_virtual_memory(const char* name);

/*
 *	Before finishing testing the library, it's mandatory to close the file stream opened before with 
 *	redFs_open_static_virtual_memory(const char* name). You need to call redFs_close_static_virtual_memory()
 *	to close it and proceed to conclude a test program. 
 *
 */

void redFs_close_static_virtual_memory();

#endif

/*
 *	redFs_disk_action_write(RED_PTR address, uint8_t data, ...) is the functions who provide a connection 
 *	from redFs and a physical drive for writing data. 
 *	redFs has simple functions that are built on top of this function to write things such the partition table, 
 *	entire partitions, folders, files and so on. 
 *	You need to provide a custom implementation of this function to write to a destination drive.
 *
 */

REDAPI int redFs_disk_action_write(RED_PTR address, uint8_t data, ...);

/*
 *	redFs_disk_action_read(RED_PTR address, uint8_t data, ...) is the functions who provide a connection 
 *	from redFs and a physical drive for writing data. 
 *	redFs has simple functions that are built on top of this function to read things such the partition table, 
 *	entire partitions, folders, files and so on. 
 *	You need to provide a custom implementation of this function to read from a destination drive.
 *
 */

REDAPI int redFs_disk_action_read(RED_PTR address, uint8_t* data, ...);


#ifndef REDFS_IO_IMP
#define REDFS_IO_IMP

#endif // REDFS_IO_IMP
#endif// REDFS_IO_H
