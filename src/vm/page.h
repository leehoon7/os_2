#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
//#include "threads/thread.h"

#define VM_BIN 0
#define VM_FILE 1
#define VM_ANON 2


struct vm_entry{
  uint8_t type;
  void *vaddr;
  bool writable;

  bool is_loaded;
  struct file* file;

  struct list_elem mmap_elem;

  size_t offset;
  size_t read_bytes;
  size_t zero_bytes;

  size_t swap_slot;

  struct hash_elem elem;
};

void vm_init(struct hash *vm);
static unsigned vm_hash_func(const struct hash_elem *e, void *aux);
static bool vm_less_func(const struct hash_elem *a, const struct hash_elem *b);

bool insert_vme(struct hash *vm, struct vm_entry *vme);
bool delete_vme(struct hash *vm, struct vm_entry *vme);

struct vm_entry *find_vme(void *vaddr);
void vm_destroy(struct hash *vm);
static void destruct(struct hash_elem *e, void *aux UNUSED);
#endif
