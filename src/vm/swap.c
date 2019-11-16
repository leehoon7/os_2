#include "vm/swap.h"
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
}
