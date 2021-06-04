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
  
  initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));
  int i;
  for (i = 0; i< 64; i++) {
    shm_table.shm_pages[i].id = 0;
    shm_table.shm_pages[i].frame = 0;
    shm_table.shm_pages[i].refcnt = 0;
  }
  release(&(shm_table.lock));
}

int shm_open(int id, char **pointer) {

  acquire(&(shm_table.lock));
  int i;
  int emptySlot = 0;
  int foundId = 0;
  char *mem;
  uint va = PGROUNDUP(myproc()->sz);

  for (i = 0; i< 64; i++) {
    if (shm_table.shm_pages[i].id == 0 && !emptySlot) {
      //keep track of the first empty slot in case the shared memory segment does not exist
      emptySlot = i;
    } 
    
    if(shm_table.shm_pages[i].id == id) {
      //shared memory segment exists
      mappages(myproc()->pgdir, (void*) va, PGSIZE, V2P(shm_table.shm_pages[i].frame), PTE_W|PTE_U);
      shm_table.shm_pages[i].refcnt += 1;
      //cprintf("exiting shm_open\n");
      foundId = 1;
      break;
    }
  }

  //shared memory segment does not exist
  if (emptySlot && !foundId) {
    shm_table.shm_pages[emptySlot].id = id;
    if((mem = kalloc()) == 0)
      return 0;
    shm_table.shm_pages[emptySlot].frame = mem;
    memset(shm_table.shm_pages[emptySlot].frame, 0, PGSIZE);
    mappages(myproc()->pgdir, (void*) va, PGSIZE, V2P(mem), PTE_W | PTE_U);
    shm_table.shm_pages[emptySlot].refcnt += 1;
  }
  
  myproc()->sz = va + PGSIZE;
  *pointer = (char*) va;
  release(&(shm_table.lock));
  return 0;
}


int shm_close(int id) {
  acquire(&(shm_table.lock));
  int i;
  for (i = 0; i< 64; i++) {
    if (shm_table.shm_pages[i].id == id) {
      shm_table.shm_pages[i].refcnt -= 1;
      if (shm_table.shm_pages[i].refcnt == 0) {
        shm_table.shm_pages[i].id = 0;
        shm_table.shm_pages[i].frame = 0;
        break;
      }
    }
  }
  release(&(shm_table.lock));
  return 0; //added to remove compiler warning -- you should decide what to return
}
