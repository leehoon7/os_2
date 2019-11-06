#include "vm/page.h"

#include "hash.h"

void vm_init(struct hash *vm){
  hash_init(vm, vm_hash_func, vm_less_func, NULL);
}

static unsigned vm_hash_func(const struct hash_elem *e, void *aux){
  struct vm_entry *h = hash_entry(e, struct vm_entry, elem);
  return hash_int((int)h->vaddr);
}

static bool vm_less_func(const struct hash_elem *a, const struct hash_elem *b){
  struct vm_entry *ha = hash_entry(a, struct vm_entry, elem);
  struct vm_entry *hb = hash_entry(b, struct vm_entry, elem);
  return hash_int((int)ha->vaddr) < hash_int((int)hb->vaddr);
}

bool insert_vme(struct hash *vm, struct vm_entry *vme){
  //return 고치기
  return hash_insert(vm, &vme->elem) == NULL;
}

bool delete_vme(struct hash *vm, struct vm_entry *vme){
  //return 고치기
  return hash_delete(vm, &vme->elem) == NULL;
}

struct vm_entry *find_vme(void *vaddr){
  struct vm_entry vme;
  vme.vaddr = pg_round_down(vaddr);
  //미완성
  struct vm_entry* vme = hash_entry(vaddr, struct vm_entry, vaddr);
  struct hash_elem* helem = hash_find(&thread_current()->vm, hash_elem)
}
