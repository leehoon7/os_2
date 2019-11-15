#ifndef FRAME_H
#define FRAME_H

#include "vm/page.h"

void lru_list_init(void);
void add_page_to_lru_list(struct page* page);
void del_page_from_lru_list(struct page* page);

#endif
