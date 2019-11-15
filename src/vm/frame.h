#ifndef FRAME_H
#define FRAME_H

#include "vm/page.h"

void lru_list_init(void);
void add_page_to_lru_list(struct page* page);
void del_page_from_lru_list(struct page* page);

struct page* alloc_page(enum palloc_flags flags);
void free_page(void *kaddr);
void __free_page(struct page* page);

static struct list_elem* get_next_lru_clock(void);
#endif
