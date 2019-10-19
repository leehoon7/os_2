#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

/* my include */
#include "devices/shutdown.h" // for SYS_HALT
#include "userprog/process.h"
static void syscall_handler (struct intr_frame *);

void my_halt();
int my_exit(int status);
pid_t my_exec(const char *cmd_line);
int my_wait(pid_t pid);
bool my_create(const char *file, unsigned initial_size);
bool my_remove(const char *file);
bool my_open(const char *file);
bool my_filesize(int fd);
int my_read(int fd, void *buffer, unsigned size);
int my_write(int fd, const void *buffer, unsigned size);
void my_seek(int fd, unsigned position);
unsigned my_tell(int fd);
void my_close(int fd);

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED)
{
  uint32_t *esp = f->esp;
  printf ("system call! %d \n", *esp);
  hex_dump(esp, esp, 300, true);

  if(*esp == SYS_HALT){ // 0
    my_halt();
  }
  else if(*esp == SYS_EXIT){ // 1
    int status = (int)*(uint32_t *)(f->esp+4);

    my_exit(status);
    thread_exit();
  }else if(*esp == SYS_EXEC){ // 2

  }else if(*esp == SYS_WAIT){ // 3

  }else if(*esp == SYS_CREATE){ // 4

  }else if(*esp == SYS_REMOVE){ // 5

  }else if(*esp == SYS_OPEN){ // 6

  }else if(*esp == SYS_FILESIZE){ // 7

  }else if(*esp == SYS_READ){ // 8

  }else if(*esp == SYS_WRITE){ // 9
    int fd = (int)*(uint32_t *)(f->esp+4);
    const void *buf = (void *)*(uint32_t *)(f->esp+8);
    unsigned size = (unsigned)*(uint32_t *)(f->esp+12);

    f->eax = my_write(fd, buf, size);
  }else if(*esp == SYS_SEEK){ // 10

  }else if(*esp == SYS_TELL){ // 11

  }else if(*esp == SYS_CLOSE){ // 12

  }
}

void my_halt(){
  shutdown_power_off();
}

int my_exit(int status){
  printf("%s: exit(%d)\n", thread_name(), status);
  thread_exit();
}

pid_t my_exec(const char *cmd_line){

}

int my_wait(pid_t pid){

}

bool my_create(const char *file, unsigned initial_size){

}

bool my_remove(const char *file){

}

bool my_open(const char *file){

}

bool my_filesize(int fd){

}

int my_read(int fd, void *buffer, unsigned size){

}

int my_write(int fd, const void *buffer, unsigned size){
  if (fd == 1){
    putbuf(buffer, size);
    return size;
  }
  return -1;
}

void my_seek(int fd, unsigned position){

}

unsigned my_tell(int fd){

}

void my_close(int fd){

}
