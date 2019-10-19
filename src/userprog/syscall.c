#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

int write(int fd, const void *buffer, unsigned size);

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
  if(*esp == 1){
    //hex_dump(esp, esp, 300, true);
    printf("%s: exit(0)\n", thread_name());
    //printf("thread exit...\n");
    thread_exit();
  }
//  printf("%d", *esp);
//  printf(" : %p \n", esp);
//  printf ("system call!\n");
  if(*esp == 9){
//    printf("esp ::: %p \n", esp);
//    printf("%d \n", (f->eax));
//    hex_dump(esp, esp, 300, true);
    int fd = (int)*(uint32_t *)(f->esp+4);
    const void *buf = (void *)*(uint32_t *)(f->esp+8);
    unsigned size = (unsigned)*(uint32_t *)(f->esp+12);
    f->eax = write(fd, buf, size);
    //write(fd, buf, size);
//    printf("%d  ", *(esp+1));
//    printf("%s  ", *(esp+2));
//    printf("%u  ", *(esp+3));
   // printf("\n\n, %", (int)*(uint32_t *)(esp+4));
   // printf("\n\n, %p", (void *)*(uint32_t *)(esp+8));
   // printf("\n\n, %p", *(esp+12));
  //  printf("good..");
  }
  //thread_exit ();
}

int write(int fd, const void *buffer, unsigned size){
  if (fd == 1){
    putbuf(buffer, size);
    return size;
  }
  return -1;
}
