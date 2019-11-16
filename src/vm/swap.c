#include "vm/swap.h"
<<<<<<< HEAD

void swap_init(size_t used_index, void* kaddr){

}

void swap_in(size_t used_index, void* kaddr){

}

size_t swap_out(void* kaddr){

=======
#include "vm/page.h"
#include <list.h>
#include "threads/synch.h"
#include "devices/block.h"
#include "lib/kernel/bitmap.h"

struct bitmap *swap_bitmap;
struct block *swap_block;

void swap_init(void)
{
	swap_block = block_get_role(BLOCK_SWAP);
  size_t size = block_size(swap_block) / SECTORS_PER_PAGE;
	swap_bitmap = bitmap_create(size);
	bitmap_set_all(swap_bitmap, true);
>>>>>>> a2f73721231dec89c66dab4abb2a90480ecf2704
}
