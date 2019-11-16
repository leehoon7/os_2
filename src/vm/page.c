#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <threads/malloc.h>
#include <threads/palloc.h>
#include "filesys/file.h"
#include "vm/page.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "userprog/syscall.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "lib/kernel/list.h"

static void destruct(struct hash_elem *e, void *aux UNUSED);

void vm_init(struct hash *vm){
  hash_init(vm, vm_hash_func, vm_less_func, NULL);
}

static unsigned vm_hash_func(const struct hash_elem *e, void *aux){
  struct vm_entry *h = hash_entry(e, struct vm_entry, elem);
  return hash_int((int)h->vaddr);
  // return hash_int(h->vaddr);
}

static bool vm_less_func(const struct hash_elem *a, const struct hash_elem *b){
  struct vm_entry *ha = hash_entry(a, struct vm_entry, elem);
  struct vm_entry *hb = hash_entry(b, struct vm_entry, elem);
  return (int)(ha->vaddr) < (int)(hb->vaddr);
  // return (ha->vaddr < hb->vaddr);
}

bool insert_vme(struct hash *vm, struct vm_entry *vme){
  return hash_insert(vm, &vme->elem) == NULL;
}

bool delete_vme(struct hash *vm, struct vm_entry *vme){
  return hash_delete(vm, &vme->elem) == NULL;
}

struct vm_entry *find_vme(void *vaddr){
  struct vm_entry vme;
  vme.vaddr = pg_round_down(vaddr);
  struct hash_elem *helem = hash_find(&thread_current()->vm, &vme.elem);
  if(helem != NULL)
    return hash_entry(helem, struct vm_entry, elem);
  return NULL;
}

void vm_destroy(struct hash *vm){
  hash_destroy(vm, destruct);
}

//미완성 free_page
static void destruct(struct hash_elem *e, void *aux UNUSED){
  struct vm_entry *vme = hash_entry(e, struct vm_entry, elem);
  void* phys_addr;
  if(vme->is_loaded){
    phys_addr = pagedir_get_page(thread_current()->pagedir, vme->vaddr);
    palloc_free_page(phys_addr);
    pagedir_clear_page(thread_current()->pagedir, vme->vaddr);
  }
  free(vme);
}

bool load_file(void *kaddr, struct vm_entry *vme)
{
	bool result = false;
  // int file_bytes = file_read_at(vme->file, kaddr, vme->read_bytes, vme->offset);
  // int check_bytes = (int) (vme->read_bytes);
  // if(check_bytes == file_bytes){
  //    memset(kaddr + vme->read_bytes, 0, vme->zero_bytes);
  //    return true;
  // }
  // return false;
	if((int)vme->read_bytes == file_read_at(vme->file, kaddr, vme->read_bytes, vme->offset))
	{
		result = true;
		memset(kaddr + vme->read_bytes, 0, vme->zero_bytes);
	}
	return result;
}
