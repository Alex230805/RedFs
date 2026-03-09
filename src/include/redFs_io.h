#ifndef REDFS_IO_H
#define REDFS_IO_H

#include "redFs.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>


static FILE* fp; 

/*
 * Base functions on which redFs is build on. To be able to use redFs you must 
 * compile it with your own implementation for the read and write call for specific 
 * devices. From complex to simple operative system each device may be managed 
 * differently and this functions provide a standard output layer for the filesystem.
 *
 * */

#ifdef VIRTIO

void redFs_open_static_virtual_memory(const char* name);
void redFs_close_static_virtual_memory();

#endif

int redFs_disk_action_write(RED_PTR address, uint8_t data, ...);
int redFs_disk_action_read(RED_PTR address, uint8_t* data, ...);

#ifndef REDFS_IO_IMP
#define REDFS_IO_IMP

#endif // REDFS_IO_IMP

#endif// REDFS_IO_H
