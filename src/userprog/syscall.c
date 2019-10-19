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
  if(*esp == 1){
    printf("%s: exit(0)\n", thread_name());
    //printf("thread exit...\n");
    thread_exit();
  }
//  printf("%d", *esp);
//  printf(" : %p \n", esp);
//  printf ("system call!\n");
  if(*esp == 9){
//    hex_dump(esp, esp, 1000, true);

/*
    int fd;
    const void *buf;
    unsigned size;
    printf("%d", *(esp+1));
    printf("%s", *(esp+2));
    printf("%u", *(esp+3));
    printf("\n\n, %p", (int)*(uint32_t *)(esp+4));
    printf("\n\n, %p", (void *)*(uint32_t *)(esp+8));
    printf("\n\n, %p", *(esp+12));
    printf("good..");*/
    f->eax = write((int)*(uint32_t *)(f->esp+20),
(void *)*(uint32_t *)(f->esp+24),
(unsigned)*((uint32_t *)(f->esp+28)));
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
