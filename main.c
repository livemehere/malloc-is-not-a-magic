#include <stdbool.h>
#include <stdio.h>
#include <sys/mman.h>

const int PAGE_SIZE = 4096;

// size : 32bytes
typedef struct Block {
  size_t size;        // 8
  bool is_free;       // 1 + 7(padding)
  struct Block *prev; // 8
  struct Block *next; // 8
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

void my_free(void *ptr) {
  if (ptr == NULL)
    return;

  // 1. mark as free
  Block *block = ((Block *)ptr) - 1;
  block->is_free = true;

  // 2. merge with next free block
  if (block->next != NULL && block->next->is_free) {
    // check both address right next to each other
    if ((char *)block->next == (char *)block + sizeof(Block) + block->size) {
      block->size += sizeof(Block) + block->next->size;
      block->next = block->next->next;
      if (block->next != NULL) {
        block->next->prev = block;
      } else {
        tail = block;
      }
    }
  }

  // 3. merge with prev free block
  if (block->prev != NULL && block->prev->is_free) {
    if ((char *)block->prev + sizeof(Block) + block->prev->size ==
        (char *)block) {
      block->prev->size += sizeof(Block) + block->size;
      block->prev->next = block->next;
      if (block->next != NULL) {
        block->next = block->prev;
      } else {
        tail = block->prev;
      }
    }
  }
}

// size : 8bytes
typedef struct Point {
  int x;
  int y;
} Point;

void print_point(Point *p) { printf("p1 x: %d, y: %d\n", p->x, p->y); }

int main() {
  print_heap();

  // size : 8(Point) + 32(Block) = 40
  Point *p1 = my_malloc(sizeof(Point));
  p1->x = 80;
  p1->y = 90;
  print_point(p1);
  print_heap();

  // clang-format off
  /*
    ---------Heap---------
    [0] Addr: 0x102564000 | size:    8 bytes | status: 🔴 USED
    [1] Addr: 0x102564028 | size: 4024 bytes | status: 🟢 FREE = 4096 - 40(p1) - 32(Block)
    ----------------------
   */
  // clang-format on

  Point *p2 = my_malloc(sizeof(Point));
  p2->x = 1;
  p2->y = 2;
  print_point(p2);
  print_heap();
  /*
    ---------Heap---------
    [1] Addr: 0x1048cc028 | size:    8 bytes | status: 🔴 USED
    [0] Addr: 0x1048cc000 | size:    8 bytes | status: 🔴 USED
    [2] Addr: 0x1048cc050 | size: 3984 bytes | status: 🟢 FREE
    ----------------------
   */

  void *bigSize = my_malloc(5000);
  print_heap();
  // clang-format off
  /*
    ---------Heap---------
    [0] Addr: 0x100f7c000 | size:    8 bytes | status: 🔴 USED
    [1] Addr: 0x100f7c028 | size:    8 bytes | status: 🔴 USED
    [2] Addr: 0x100f7c050 | size: 3984 bytes | status: 🟢 FREE
    [3] Addr: 0x100f80000 | size: 5000 bytes | status: 🔴 USED
    [4] Addr: 0x100f813a8 | size: 3128 bytes | status: 🟢 FREE = 4096*2(mmap) - 5000(size) - 32(Block) - 32(Block)
   */
  // clang-format on

  my_free(p1);
  print_heap();
  // clang-format off
  /*
    ---------Heap---------
    [0] Addr: 0x1025f4000 | size:    8 bytes | status: 🟢 FREE = set free but not merge with next block what still used
    [1] Addr: 0x1025f4028 | size:    8 bytes | status: 🔴 USED
    [2] Addr: 0x1025f4050 | size: 3984 bytes | status: 🟢 FREE
    [3] Addr: 0x1025f8000 | size: 5000 bytes | status: 🔴 USED
    [4] Addr: 0x1025f93a8 | size: 3128 bytes | status: 🟢 FREE
   */
  // clang-format on

  my_free(p2);
  print_heap();
  // clang-format off
  /*
    ---------Heap---------
    [0] Addr: 0x1028e8000 | size: 4064 bytes | status: 🟢 FREE = merge 3 block (3984 + 40 + 40)
    [1] Addr: 0x1028ec000 | size: 5000 bytes | status: 🔴 USED
    [2] Addr: 0x1028ed3a8 | size: 3128 bytes | status: 🟢 FREE
  */
  // clang-format on

  my_free(bigSize);
  print_heap();
  // clang-format off
  /*
    ---------Heap---------
    // Two memory do not merged because their not right next each other.
    [0] Addr: 0x10539c000 | size: 4064 bytes | status: 🟢 FREE
    [1] Addr: 0x1053a0000 | size: 8160 bytes | status: 🟢 FREE
   */
  // clang-format on

  return 0;
}
