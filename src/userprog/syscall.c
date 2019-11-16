#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

/* my include */
#include "devices/shutdown.h" // for SYS_HALT
#include "userprog/process.h"
#include "threads/vaddr.h" // for check_address : is_user_vaddr
#include "filesys/filesys.h" // for SYS_CREATE, SYS_REMOVE
#include "vm/page.h"
static void syscall_handler (struct intr_frame *);

struct lock file_lock;
struct file
  {
    struct inode *inode;        /* File's inode. */
    off_t pos;                  /* Current position. */
    bool deny_write;            /* Has file_deny_write() been called? */
  };

/* functions for system call*/
void my_halt();
void my_exit(int status);
pid_t my_exec(const char *cmd_line);
int my_wait(pid_t pid);
bool my_create(const char *file, unsigned initial_size);
bool my_remove(const char *file);
int my_open(const char *file);
int my_filesize(int fd);
int my_read(int fd, void *buffer, unsigned size);
int my_write(int fd, const void *buffer, unsigned size);
void my_seek(int fd, unsigned position);
unsigned my_tell(int fd);
void my_close(int fd);

/* help function */
void check_address(void *addr, void *esp);
void check_valid_buffer(void* buffer, unsigned size, void* esp, bool to_write);
void check_valid_string(const void *str, void *esp);

int mmap(int fd, void *addr);
void munmap(mapid_t mapid);
void do_munmap(struct mmap_file* mmap_file);

void
syscall_init (void)
{
  lock_init(&file_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED)
{
  uint32_t *esp = f->esp;
//  printf ("system call! %d \n", *esp);
//  hex_dump(esp, esp, 300, true);

  if(*esp == SYS_HALT){ // 0 : 핀토스 끄기
    my_halt();
  }
  else if(*esp == SYS_EXIT){ // 1 :
    int status = (int)*(uint32_t *)(f->esp+4);
    check_address(f->esp+4, esp);
    my_exit(status);
    thread_exit();
  }else if(*esp == SYS_EXEC){ // 2
    const char *cmd = (const char*)*(uint32_t *)(f->esp+4);
//    printf("here");

//    check_address(f->esp+4, esp);
    check_valid_string((const void*)cmd, f->esp);

//    printf("here %p \n", cmd);
//    printf("our command : %s \n\n", cmd);
//    printf("hello.. \n\n");
    f->eax = my_exec(cmd);
  }else if(*esp == SYS_WAIT){ // 3
    int return_code;
    pid_t pid = (pid_t)*(uint32_t *)(f->esp+4);
    check_address(f->esp+4, esp);
    return_code = my_wait(pid);
    f->eax = return_code;
  }else if(*esp == SYS_CREATE){ // 4
    bool return_code;

    check_address(f->esp+4, esp);
    check_address(f->esp+8, esp);

    const char *file = (const char*)*(uint32_t *)(f->esp+4);
    unsigned initial_size = (unsigned)*(uint32_t *)(f->esp+8);

    return_code = my_create(file, initial_size);
    f->eax = return_code;
  }else if(*esp == SYS_REMOVE){ // 5
    bool return_code;
    check_address(f->esp+4, esp);

    const char *file = (const char*)*(uint32_t *)(f->esp+4);
    return_code = my_remove(file);
    f->eax = return_code;
  }else if(*esp == SYS_OPEN){ // 6
    int return_code;
    const char *file = (const char*)*(uint32_t *)(f->esp+4);
//    check_address(f->esp+4, esp);
    check_valid_string(file, f->esp);
    return_code = my_open(file);
    f->eax = return_code;
  }else if(*esp == SYS_FILESIZE){ // 7
    int return_code;
    int fd = (int)*(uint32_t *)(f->esp+4);
    check_address(f->esp+4, esp);
    return_code = my_filesize(fd);
    f->eax = return_code;
  }else if(*esp == SYS_READ){ // 8
    int fd = (int)*(uint32_t *)(f->esp+4);
    void *buf = (void *)*(uint32_t *)(f->esp+8);
    unsigned size = (unsigned)*(uint32_t *)(f->esp+12);
    // check_address(f->esp+4, esp);
    // check_address(f->esp+8, esp);
    // check_address(f->esp+12, esp);
    check_valid_buffer((void*)buf, (unsigned)size, f->esp, true);
    f->eax = my_read(fd, buf, size);
  }else if(*esp == SYS_WRITE){ // 9
    int fd = (int)*(uint32_t *)(f->esp+4);
    const void *buf = (void *)*(uint32_t *)(f->esp+8);
    unsigned size = (unsigned)*(uint32_t *)(f->esp+12);
    // check_address(f->esp+4, esp);
    // check_address(f->esp+8, esp);
    // check_address(f->esp+12, esp);
    check_valid_buffer((void*)buf, (unsigned)size, f->esp, false);
    f->eax = my_write(fd, buf, size);
  }else if(*esp == SYS_SEEK){ // 10
    int fd = (int)*(uint32_t *)(f->esp+4);
    unsigned position = (unsigned)*(uint32_t *)(f->esp+8);
    check_address(f->esp+4, esp);
    check_address(f->esp+8, esp);
    my_seek(fd, position);
  }else if(*esp == SYS_TELL){ // 11
    unsigned return_code;
    int fd = (int)*(uint32_t *)(f->esp+4);
    check_address(f->esp+4, esp);
    return_code = my_tell(fd);
    f->eax = return_code;
  }else if(*esp == SYS_CLOSE){ // 12
    int fd = (int)*(uint32_t *)(f->esp+4);
    check_address(f->esp+4, esp);
    my_close(fd);
  }else if(*esp == SYS_MMAP){
    int fd = (int)*(uint32_t *)(f->esp+4);
    void *addr = (void *)*(uint32_t *)(f->esp+8);
    check_address(f->esp+4, esp);
    check_address(f->esp+8, esp);
    f->eax = mmap(fd, addr);
  }else if(*esp == SYS_MUNMAP){
    mapid_t mapid = (mapid_t)*(uint32_t *)(f->esp+4);
    check_address(f->esp+4, esp);
    munmap(mapid);
  }
  struct vm_entry *vme = find_vme(f->esp);
  if(vme)
    vme->pinned = false;
}

void my_halt(){
  shutdown_power_off();
}

void my_exit(int status){
  printf("%s: exit(%d)\n", thread_name(), status);
  thread_current() -> exit_status = status;
  for(int i = 3; i < 128; i++){
    if(thread_current()->fd[i] != NULL){
      my_close(i);
    }
  }
  thread_exit();
}

pid_t my_exec(const char *cmd_line){
  pid_t return_code = process_execute(cmd_line);
  return return_code;
}

int my_wait(pid_t pid){
  int return_code = process_wait(pid);
  return return_code;
}

bool my_create(const char *file, unsigned initial_size){
  if(file == NULL) my_exit(-1);
  //check_address(file);
  lock_acquire(&file_lock);
  bool return_code = filesys_create(file, initial_size);
  lock_release(&file_lock);
  return return_code;
}

bool my_remove(const char *file){
  if(file == NULL) my_exit(-1);
  //check_address(file);
  lock_acquire(&file_lock);
  bool return_code = filesys_remove(file);
  lock_release(&file_lock);
  return return_code;
}

int my_open(const char *file){
  if(file == NULL) my_exit(-1);
  //check_address(file);
  lock_acquire(&file_lock);
  struct file *fp = filesys_open(file);
  if (fp == NULL){
    lock_release(&file_lock);
    return -1;
  }
  if(!strcmp(thread_current()->name, file)){
    file_deny_write(fp);
  }
  for(int i = 3; i< 128; i++){
    if(thread_current()->fd[i] == NULL){
      if (!strcmp(thread_current()->name, file)){
        file_deny_write(fp);
      }
      thread_current()->fd[i] = fp;
      lock_release(&file_lock);
      return i;
    }
  }
  lock_release(&file_lock);
  return -1;
}

int my_filesize(int fd){
  struct file *fp = thread_current()->fd[fd];
  lock_acquire(&file_lock);
  if (fp == NULL){
    lock_release(&file_lock);
    my_exit(-1);
  }
  int return_code = file_length(fp);
  lock_release(&file_lock);
  return return_code;
}

int my_read(int fd, void *buffer, unsigned size){
  struct file *fp = thread_current()->fd[fd];
  int i;
  //check_address(buffer);
  lock_acquire(&file_lock);
  if (fd == 0){
    for(i = 0; i < size; i++){
      if(((char *)buffer)[i] == NULL){
      	lock_release(&file_lock);
      	return ++i;
      }
    }
  } else if (fd > 2){
    if (fp == NULL){
      lock_release(&file_lock);
      my_exit(-1);
    }
    lock_release(&file_lock);
    return file_read(fp, buffer, size);
  }
  lock_release(&file_lock);
  return i;
}

int my_write(int fd, const void *buffer, unsigned size){
  struct file *fp = thread_current()->fd[fd];
  //check_address(buffer);
  lock_acquire(&file_lock);
  if (fd == 1){
    putbuf(buffer, size);
    lock_release(&file_lock);
    return size;
  } else if (fd > 2){
    if (fp == NULL){
      lock_release(&file_lock);
      my_exit(-1);
    }else if (fp->deny_write){
      file_deny_write(fp);
    }
    lock_release(&file_lock);
    return file_write(fp, buffer, size);
  }
  lock_release(&file_lock);
  return -1;
}

void my_seek(int fd, unsigned position){
  struct file *fp = thread_current()->fd[fd];
  if (fp == NULL){
    my_exit(-1);
  }
  file_seek(fp, position);
}

unsigned my_tell(int fd){
  struct file *fp = thread_current()->fd[fd];
  if (fp == NULL){
    my_exit(-1);
  }
  return file_tell(fp);
}

void my_close(int fd){
  struct file *fp = thread_current()->fd[fd];
  if (fp == NULL){
    my_exit(-1);
  }
  struct file *temp = thread_current()->fd[fd];
  thread_current()->fd[fd] = NULL;
  return file_close(temp);
}

// 유저가 이 주소를 사용할 수 없으면 : -1 status로 exit.
void check_address(void *addr, void *esp){
//  printf("in check address : %p\n", addr);
  //printf("vs ")
  if (addr < (void *)0x08048000 || addr >= (void *)0xc0000000){
    my_exit(-1);
  }

  struct vm_entry *vme = find_vme(addr);
  if(vme == NULL){
    my_exit(-1);
  }

  // 완벽하지는 않음.
  /*
  if(find_vme(addr) == NULL){
    exit(-1);
  }
  */


  /*
  if(!is_user_vaddr(addr)){
    my_exit(-1);
  }*/
}

void check_valid_buffer(void* buffer, unsigned size, void* esp, bool to_write){
  struct vm_entry* vme;
  char* buf = (char*)buffer;
  for(int i=0; i<size; i++){
    check_address((void*)buf, esp);
    vme = find_vme((void*)buf);
    if(vme != NULL && to_write && !vme->writable)
      my_exit(-1);
    buf++;
  }
}

void check_valid_string(const void *str, void *esp){
  char *str_ = (char*)str;
  check_address((void*)str_, esp);
  while(*str_){
    str_++;
    check_address(str_, esp);
  }
}

int mmap(int fd, void *addr){
  int32_t offset = 0;
  void *virtual_address = addr;

  if((uint32_t)addr%PGSIZE != 0 || addr == NULL)
    return -1;
  struct mmap_file* mmap_file_entry = malloc(sizeof(struct mmap_file));
  if(mmap_file_entry == NULL)
    return -1;

  //if(cur->fd[fd] != NULL)
  struct thread *cur = thread_current();
  struct file *mmap_file = file_reopen(cur->fd[fd]);
  if(mmap_file == NULL){
    printf("File reopen fail!\n");
    return -1;
  }

  cur->mapid += 1;
  mmap_file_entry->mapid = cur->mapid;

  list_init(&(mmap_file_entry->vme_list));

  mmap_file_entry->file = mmap_file;
  uint32_t file_len = file_length(mmap_file);

  while(file_len > 0){
      struct vm_entry *vme = malloc(sizeof(struct vm_entry));
    if(vme == NULL)
      return -1;

    size_t page_read_bytes = file_len < PGSIZE ? file_len : PGSIZE;
    size_t page_zero_bytes = PGSIZE - page_read_bytes;

    vme->type      = VM_FILE;
    vme->vaddr     = virtual_address;
    vme->writable  = true;
    vme->is_loaded = false;
    vme->pinned    = false;
    vme->file      = mmap_file;
    vme->offset    = offset;
    vme->read_bytes= page_read_bytes;
    vme->zero_bytes= page_zero_bytes;

    if(!insert_vme(&cur->vm, vme))
       return -1;

    list_push_back(&(mmap_file_entry->vme_list), &(vme->mmap_elem));
    file_len -= page_read_bytes;
    offset += page_read_bytes;
    virtual_address += PGSIZE;
  }

  list_push_back(&cur->mmap_list,&mmap_file_entry->elem);
  return cur->mapid;
}

void munmap(mapid_t mapid){
  struct mmap_file *map_file;
  struct thread *cur = thread_current();
  struct list_elem *element;
  struct list_elem *tmp;

  for(element = list_begin(&cur->mmap_list) ; element != list_end(&cur->mmap_list) ; element = list_next(element)){
    map_file = list_entry(element, struct mmap_file, elem);

    if(mapid == CLOSE_ALL || map_file->mapid == mapid){
      do_munmap(map_file);

      file_close(map_file->file);

      tmp = list_prev(element);
      list_remove(element);
      element = tmp;

      free(map_file);
      if(mapid != CLOSE_ALL)
        break;
    }
  }
}

void do_munmap(struct mmap_file* mmap_file){
  struct thread *cur = thread_current();
	struct list_elem *element;
	struct list_elem *tmp;
	struct list *vm_list = &(mmap_file->vme_list);
	struct vm_entry *vme;
	void *physical_address;

	for(element = list_begin(vm_list); element != list_end(vm_list); element = list_next(element)){
		vme = list_entry(element, struct vm_entry, mmap_elem);

		if(vme->is_loaded){
			physical_address = pagedir_get_page(cur->pagedir, vme->vaddr);

			if(pagedir_is_dirty(cur->pagedir, vme->vaddr)){
				lock_acquire(&file_lock);
				file_write_at(vme->file, vme->vaddr, vme->read_bytes, vme->offset);
				lock_release(&file_lock);
			}

			pagedir_clear_page(cur->pagedir, vme->vaddr);
      free_page(physical_address);
			//palloc_free_page(physical_address);
		}

		tmp = list_prev(element);
		list_remove(element);
		element = tmp;

		delete_vme(&cur->vm, vme);
	}
}
