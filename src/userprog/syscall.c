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
static void syscall_handler (struct intr_frame *);

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
void check_address(void *addr);

void
syscall_init (void)
{
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
    check_address(f->esp+4);
    my_exit(status);
    thread_exit();
  }else if(*esp == SYS_EXEC){ // 2
    const char *cmd = (const char*)*(uint32_t *)(f->esp+4);
//    printf("here");
    check_address(f->esp+4);
//    printf("here %p \n", cmd);
//    printf("our command : %s \n\n", cmd);
//    printf("hello.. \n\n");
    f->eax = my_exec(cmd);
  }else if(*esp == SYS_WAIT){ // 3
    pid_t pid = (pid_t)*(uint32_t *)(f->esp+4);
    check_address(f->esp+4);
    f->eax = my_wait(pid);
  }else if(*esp == SYS_CREATE){ // 4
    bool return_code;

    check_address(f->esp+4);
    check_address(f->esp+8);

    const char *file = (const char*)*(uint32_t *)(f->esp+4);
    unsigned initial_size = (unsigned)*(uint32_t *)(f->esp+8);

    return_code = my_create(file, initial_size);
    f->eax = return_code;
  }else if(*esp == SYS_REMOVE){ // 5
    bool return_code;
    check_address(f->esp+4);

    const char *file = (const char*)*(uint32_t *)(f->esp+4);
    return_code = my_remove(file);
    f->eax = return_code;
  }else if(*esp == SYS_OPEN){ // 6
    int return_code;
    const char *file = (const char*)*(uint32_t *)(f->esp+4);
    check_address(f->esp+4);
    return_code = my_open(file);
    f->eax = return_code;
  }else if(*esp == SYS_FILESIZE){ // 7
    int return_code;
    int fd = (int)*(uint32_t *)(f->esp+4);
    check_address(f->esp+4);
    return_code = my_filesize(fd);
    f->eax = return_code;
  }else if(*esp == SYS_READ){ // 8
    int fd = (int)*(uint32_t *)(f->esp+4);
    void *buf = (void *)*(uint32_t *)(f->esp+8);
    unsigned size = (unsigned)*(uint32_t *)(f->esp+12);
    check_address(f->esp+4);
    check_address(f->esp+8);
    check_address(f->esp+12);
    f->eax = my_read(fd, buf, size);
  }else if(*esp == SYS_WRITE){ // 9
    int fd = (int)*(uint32_t *)(f->esp+4);
    const void *buf = (void *)*(uint32_t *)(f->esp+8);
    unsigned size = (unsigned)*(uint32_t *)(f->esp+12);
    check_address(f->esp+4);
    check_address(f->esp+8);
    check_address(f->esp+12);
    f->eax = my_write(fd, buf, size);
  }else if(*esp == SYS_SEEK){ // 10
    int fd = (int)*(uint32_t *)(f->esp+4);
    unsigned position = (unsigned)*(uint32_t *)(f->esp+8);
    check_address(f->esp+4);
    check_address(f->esp+8);
    my_seek(fd, position);
  }else if(*esp == SYS_TELL){ // 11
    unsigned return_code;
    int fd = (int)*(uint32_t *)(f->esp+4);
    check_address(f->esp+4);
    return_code = my_tell(fd);
    f->eax = return_code;
  }else if(*esp == SYS_CLOSE){ // 12
    int fd = (int)*(uint32_t *)(f->esp+4);
    check_address(f->esp+4);
    my_close(fd);
  }
}

void my_halt(){
  shutdown_power_off();
}

void my_exit(int status){
  printf("%s: exit(%d)\n", thread_name(), status);
  thread_current() -> exit_status = status;
  for(int i = 3; i < 128; i++){
    if(thread_current()->fd[i] != NULL){
      close(i);
    }
  }
  thread_exit();
}

pid_t my_exec(const char *cmd_line){
  return process_execute(cmd_line);
}

int my_wait(pid_t pid){
  return process_wait(pid);
}

bool my_create(const char *file, unsigned initial_size){
  if(file == NULL) my_exit(-1);
  check_address(file);
  return filesys_create(file, initial_size);
}

bool my_remove(const char *file){
  if(file == NULL) my_exit(-1);
  check_address(file);
  return filesys_remove(file);
}

int my_open(const char *file){
  struct file *fp = filesys_open(file);
  if (fp == '\0'){
    return -1;
  }
  for(int i = 3; i< 128; i++){
    if(thread_current()->fd[i] == NULL){
      thread_current()->fd[i] = fp;
      return i;
    }
  }
  return -1;
}

int my_filesize(int fd){
  return file_length(thread_current()->fd[fd]);
}

int my_read(int fd, void *buffer, unsigned size){
  int i;
  if (fd == 0){
    for(i = 0; i < size; i++){
      if(((char *)buffer)[i] == '\0'){
        break;
      }
    }
  } else if (fd > 2){
    return file_read(thread_current()->fd[fd], buffer, size);
  }
  return i;
}

int my_write(int fd, const void *buffer, unsigned size){
  if (fd == 1){
    putbuf(buffer, size);
    return size;
  } else if (fd > 2){
    return file_write(thread_current()->fd[fd], buffer, size);
  }
  return -1;
}

void my_seek(int fd, unsigned position){
  file_seek(thread_current()->fd[fd], position);
}

unsigned my_tell(int fd){
  return file_tell(thread_current()->fd[fd]);
}

void my_close(int fd){
  return file_close(thread_current()->fd[fd]);
}

// 유저가 이 주소를 사용할 수 없으면 : -1 status로 exit.
void check_address(void *addr){
//  printf("in check address : %p\n", addr);
  //printf("vs ")
  if(!is_user_vaddr(addr)){
    my_exit(-1);
  }
}
