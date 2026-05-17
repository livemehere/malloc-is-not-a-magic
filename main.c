#include <stdbool.h>
#include <stdio.h>
#include <sys/mman.h>

const int PAGE_SIZE = 4096;

typedef struct Block {
  size_t size;
  bool is_free;
  struct Block *prev;
  struct Block *next;
} Block;

Block *head = NULL;
Block *tail = NULL;

void print_heap() {
  printf("---------Heap---------\n");
  Block *cur = head;
  int idx = 0;
  while (cur != NULL) {
    printf("[%d] Addr: %p | size: %4zu bytes | status: %s\n", idx++,
           (void *)cur, cur->size, cur->is_free ? "🟢 FREE" : "🔴 USED");
    cur = cur->next;
  }
  printf("----------------------\n\n");
}

void split_block(Block *block, size_t size) {
  // 1. has capacity
  if (block->size > size + sizeof(Block)) {
    // 2. ptr addr of splited block
    Block *rest_block = (Block *)((char *)(block + 1) + size);
    rest_block->size = block->size - size - sizeof(Block);
    rest_block->is_free = true;

    rest_block->next = block->next;
    rest_block->prev = block;

    if (rest_block->next != NULL) {
      rest_block->next->prev = rest_block;
    } else {
      tail = rest_block;
    }

    block->next = rest_block;
    block->size = size;
  }
}

void *my_malloc(size_t size) {
  if (size <= 0)
    return NULL;

  Block *cur = head;
  Block *found_block = NULL;

  // 1. find re-usable space
  while (cur != NULL) {
    if (cur->is_free && cur->size >= size) {
      found_block = cur;
      break;
    }
    cur = cur->next;
  }

  // 2. if exits split that block with what we needed and return.
  if (found_block != NULL) {
    found_block->is_free = false;
    split_block(found_block, size);
    return (void *)(found_block + 1);
  }

  // 3. if not, we need to request to get memory from os
  size_t request_size = size + sizeof(Block);
  size_t mmap_size = PAGE_SIZE;

  while (mmap_size < request_size) {
    mmap_size += PAGE_SIZE;
  }

  Block *new_block = (Block *)mmap(NULL, mmap_size, PROT_READ | PROT_WRITE,
                                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  // if fail return null
  if (new_block == MAP_FAILED)
    return NULL;

  new_block->is_free = true;
  new_block->size = mmap_size - sizeof(Block);
  new_block->next = NULL;

  if (head == NULL)
    head = new_block;
  if (tail != NULL)
    tail->next = new_block;
  tail = new_block;

  new_block->is_free = false;
  split_block(new_block, size);
  return (void *)(new_block + 1);
}

typedef struct Point {
  int x;
  int y;
} Point;

int main() {
  print_heap();

  Point *p1 = my_malloc(sizeof(Point));
  p1->x = 80;
  p1->y = 90;
  print_heap();
  /*
    ---------Heap---------
    [0] Addr: 0x102564000 | size:    8 bytes | status: 🔴 USED
    [1] Addr: 0x102564028 | size: 4024 bytes | status: 🟢 FREE
    ----------------------
   */

  return 0;
}
