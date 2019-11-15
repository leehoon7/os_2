#include <page.h>
#include <list.h>
#include <synch.h>

struct list lru_list;

void lru_list_init(void){
  list_init(&lru_list);

}
