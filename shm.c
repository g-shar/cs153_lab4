#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct shm_page {
    uint id;
    char *frame;
    int refcnt;
  } shm_pages[64];
} shm_table;

void shminit() {
  int i;
  initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));
  for (i = 0; i< 64; i++) {
    shm_table.shm_pages[i].id = 0;
    shm_table.shm_pages[i].frame = 0;
    shm_table.shm_pages[i].refcnt = 0;
  }
  release(&(shm_table.lock));
}

int shm_open(int id, char **pointer) {
  int i;
  int emptySlot = 0;
  char *mem;
  //initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));
  //pde_t* pgdir = myproc()->pgdir;
  for (i = 0; i< 64; i++) {
    if (shm_table.shm_pages[i].id == 0 && !emptySlot) {
      //keep track of the first empty slot in case the shared memory segment does not exist
      emptySlot = i;
    } 
    
    if(shm_table.shm_pages[i].id == id) {
      //shared memory segment exists
      cprintf("shared memory segment exists\n");
      uint va = PGROUNDUP(myproc()->sz);
      mappages(myproc()->pgdir, (void*) va, PGSIZE, V2P(shm_table.shm_pages[i].frame), PTE_W|PTE_U);
      myproc()->sz = va + PGSIZE;
      shm_table.shm_pages[i].refcnt += 1;
      *pointer = (char*) va;
      release(&(shm_table.lock));
      //cprintf("exiting shm_open\n");
      return 0;
    }
  }
  //shared memory segment does not exist
  shm_table.shm_pages[emptySlot].id = id;
  if((mem = kalloc()) == 0)
    return 0;
  cprintf("shared memory segment does not exist\n");
  uint va = PGROUNDUP(myproc()->sz);
  mappages(myproc()->pgdir, (void*) va, PGSIZE, V2P(mem), PTE_W | PTE_U);
  myproc()->sz = PGROUNDUP(myproc()->sz) + PGSIZE;
  shm_table.shm_pages[emptySlot].frame = mem;
  shm_table.shm_pages[emptySlot].refcnt = 1;
  *pointer = (char*) va;
  release(&(shm_table.lock));
  //cprintf("exiting shm_open\n");
  return 0;
}


int shm_close(int id) {
//you write this too!




return 0; //added to remove compiler warning -- you should decide what to return
}
