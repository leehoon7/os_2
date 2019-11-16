#ifndef SWAP_H
#define SWAP_H

#include "vm/page.h"
#include "threads/synch.h"
#define SECTORS_PER_PAGE 8

void swap_init(void);
void swap_in(size_t used_index, void* kaddr);
size_t swap_out(void* kaddr);

#endif
