#include "vm/frame.h"
#include <list.h>
#include "threads/synch.h"

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

    list_push_back (&lru_list, &(page->lru));

    lock_release(&lru_list_lock);
  }
}

void del_page_from_lru_list(struct page* page){
  if(page != NULL){
    if(lru_clock == page){

      lru_clock = list_entry(list_next(&(page->lru)), struct page, lru);
      list_remove(&(page->lru));

    }else{
      list_remove(&page->lru);
    }
  }
}

struct page* alloc_page(enum palloc_flags flags){
  if((flags & PAL_USER) == 0)
    return NULL;

  void *kaddr = palloc_get_page(flags);
  while(kaddr == NULL){
    try_to_free_pages();
    kaddr = palloc_get_page(flags);
  }

  struct page *new_pg = malloc(sizeof(struct page));

  if(new_pg == NULL){
    palloc_free_page(kaddr);
    return NULL;
  }
  new_pg->kaddr = kaddr;
  new_pg->thread = thread_current();
  add_page_to_lru_list(new_pg);
  return new_pg;
}

void free_page(void *kaddr){
  lock_acquire(&lru_list_lock);
  for(struct list_elem *elem = list_begin(&lru_list); elem != list_end(&lru_list); elem = list_next(elem)){
    struct page *pg = list_entry(elem, struct page, lru);
    if(pg->kaddr == kaddr){
      __free_page(pg);
      break;
    }
  }
  lock_release(&lru_list_lock);
}

void __free_page(struct page* page){
  palloc_free_page(page->kaddr);
  del_page_from_lru_list(page);
  free(page);
}

static struct list_elem* get_next_lru_clock(void){
  struct list_elem *elem;
  if(lru_clock == NULL){
    elem = list_begin(&lru_list);
    if(elem != list_end(&lru_list)){
      lru_clock = list_entry(elem, struct page, lru);
      return elem;
    }
    else
      return NULL;
  }
  elem = list_next(&lru_clock->lru);
  if(elem == list_end(&lru_list)){
    if(&lru_clock->lru == list_begin(&lru_list))
      return NULL;
    else
      elem = list_begin(&lru_list);
  }
  lru_clock = list_entry(elem, struct page, lru);
  return elem;
}

void try_to_free_pages(enum palloc_flags flags){
  lock_acquire(&lru_list_lock);
  // 이부분 빼도 될 것 같음.
  if(list_empty(&lru_list)){
    lock_release(&lru_list_lock);
    return;
  }
  while(true){
    struct list_elem *e = get_next_lru_clock();
    if(e == NULL){
      break;
    }
    struct page *lru_page = list_entry(e, struct page, lru);
    if(lru_page->vme->pinned == true){
      continue;
    }
    struct thread *page_thread = lru_page->thread;
    if(pagedir_is_accessed(page_thread->pagedir, lru_page->vme->vaddr)){
      pagedir_set_accessed(page_thread->pagedir, lru_page->vme->vaddr, false);
      continue;
    }
    if(pagedir_is_dirty(page_thread->pagedir, lru_page->vme->vaddr) || lru_page->vme->type == VM_ANON)
		{
			/* if vm_entry is mmap file, don't call swap out.*/
			if(lru_page->vme->type == VM_FILE)
			{
				file_write_at(lru_page->vme->file, lru_page->kaddr ,lru_page->vme->read_bytes, lru_page->vme->offset);
			}
			/* if not mmap_file, change type to ANON and call swap_out function */
			else
			{
				lru_page->vme->type = VM_ANON;
				lru_page->vme->swap_slot = swap_out(lru_page->kaddr);
 			}
		}
		lru_page->vme->is_loaded = false;
		pagedir_clear_page(page_thread->pagedir, lru_page->vme->vaddr);
		__free_page(lru_page);
		break;
  }
  lock_release(&lru_list_lock);
  return;
}
