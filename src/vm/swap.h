#include "vm/page.h"
#include <list.h>
#include "threads/synch.h"

void swap_init(size_t used_index, void* kaddr);
void swap_in(size_t used_index, void* kaddr);
size_t swap_out(void* kaddr);
