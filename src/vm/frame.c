#include <page.h>
#include <list.h>
#include <synch.h>

struct list lru_list;
struct lock lru_list_lock; // add, del 할때 lock 걸어야 할거같음!
struct page* lru_clock;

void lru_list_init(void){
  list_init(&lru_list);
  lock_init(&lru_list_lock);
  lru_clock = NULL;
}

void add_page_to_lru_list(struct page* page){
  if(page != NULL){
    lock_acquire(&lru_list_lock);

    list_push_back (lru_list, &(page->lru));

    lock_release(&lru_list_lock);
  }
}

void del_page_from_lru_list(struct page* page){
  if(page != NULL){
    if(lru_clock == page){
      lock_acquire(&lru_list_lock);

      lru_clock = list_entry(list_next(&(page->lru)), struct page, lru);
      list_remove(&(page->lru));

      lock_release(&lru_list_lock);
    }
  }
}
