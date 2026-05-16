#include <stdbool.h>
#include <stdio.h>
#include <sys/mman.h>

typedef struct Block {
  size_t size;
  bool is_free;
  struct Block *prev;
  struct Block *next;
} Block;

Block *head = NULL;
Block *tail = NULL;

void print_heap() {
  printf("------------\n");
  Block *cur = head;
  int idx = 0;
  while (cur != NULL) {
    printf("[%d] Add: %p | size : %4zu bytes | status : %s\n", idx++,
           (void *)cur, cur->size, cur->is_free ? "FREE" : "USED");
    cur = cur->next;
  }
  printf("------------\n");
}

void *my_malloc(size_t size) {
  if (size <= 0)
    return NULL;

  Block *cur = head;
  Block *found_block = NULL;

  // 1. try to find memory space among already borrowed.
  while (cur != NULL) {
    if (cur->is_free && cur->size >= size) {
      found_block = cur;
      break;
    }
    // if not, cur is tail of linked list.
    cur = cur->next;
  }

  // TODO: re-use case
  if (found_block != NULL) {
  }

  size_t total_size = size + sizeof(Block);
  Block *new_block = (Block *)mmap(NULL, total_size, PROT_READ | PROT_WRITE,
                                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  // fail to borrow space
  if (new_block == MAP_FAILED) {
    return NULL;
  }

  new_block->is_free = false;
  new_block->size = size;
  new_block->next = NULL;
  new_block->prev = tail;

  // update pointers
  if (head == NULL) {
    head = new_block;
  }

  if (tail != NULL) {
    tail->next = new_block;
  }

  tail = new_block;

  // actual location of size
  return (void *)(new_block + 1);
}

void my_free(void *ptr) {
  if (ptr == NULL) {
    return;
  }
  Block *block = ((Block *)ptr) - 1;
  if (block == NULL) {
    return;
  }

  if (block->prev != NULL) {
    block->prev->next = block->next;
  } else {
    head = block->next;
  }

  if (block->next != NULL) {
    block->next->prev = block->prev;
  } else {
    tail = block->prev;
  }

  munmap(block, sizeof(Block) + block->size);
  return;
}

typedef struct Point {
  int x;
  int y;
} Point;

int main() {

  Point *p1 = my_malloc(sizeof(Point));
  print_heap();
  Point *p2 = my_malloc(sizeof(Point));
  print_heap();

  p1->x = 99;
  p1->y = 88;

  p2->x = 30;
  p2->y = 40;

  my_free(p2);
  print_heap();

  printf("p1 is %d, %d\n", p1->x, p1->y);
  // printf("p2 is %d, %d\n", p2->x, p2->y);

  return 0;
}
