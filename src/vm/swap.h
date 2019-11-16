<<<<<<< HEAD
#include "vm/page.h"
#include <list.h>
#include "threads/synch.h"
=======
// include???
#define SECTORS_PER_PAGE 8
>>>>>>> a2f73721231dec89c66dab4abb2a90480ecf2704

void swap_init(size_t used_index, void* kaddr);
void swap_in(size_t used_index, void* kaddr);
size_t swap_out(void* kaddr);
