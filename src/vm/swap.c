#include "vm/swap.h"
#include "vm/page.h"
#include <list.h>
#include "threads/synch.h"
#include "devices/block.h"
#include "lib/kernel/bitmap.h"

struct bitmap *swap_bitmap;
struct block *swap_block;

void swap_init(void){
	swap_block = block_get_role(BLOCK_SWAP);
  size_t size = block_size(swap_block) / SECTORS_PER_PAGE;
	swap_bitmap = bitmap_create(size);
	bitmap_set_all(swap_bitmap, true);
}

void swap_in(size_t used_index, void* kaddr){
  // 현재 쓰이고 있으면 true, 안쓰이고 있으면 false
  bool using_now = bitmap_test(swap_map, used_index);
  if(using_now){
    return;
  }

  int i;
  for(i = 0; i < SECTORS_PER_PAGE; i++){
    block_read(swap_block, used_index * SECTORS_PER_PAGE + i, page + (BLOCK_SECTOR_SIZE * i));
  }
  bitmap_set(swap_bitmap, used_index, true);
}

size_t swap_out(void* kaddr){
  size_t used_index = bitmap_scan (swap_bitmap, 0, 1, true);
  int i;
  for(i = 0; i < SECTORS_PER_PAGE; i++){
    block_write(swap_block, used_index * SECTORS_PER_PAGE + i, page + (BLOCK_SECTOR_SIZE * i));
  }
  bitmap_set(swap_bitmap, used_index, false);
}
