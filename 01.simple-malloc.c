#include <stdio.h>
#include <unistd.h>

void *simple_malloc(unsigned int size) {

  void *p = sbrk(size);

  if (p == (void *)-1) {
    return NULL;
  }

  return p;
}

typedef struct Point {
  int x;
  int y;
} Point;

int main() {

  // case 1
  Point *p = (Point *)simple_malloc(sizeof(Point));
  p->x = 10;
  p->y = 20;

  printf("Value: %d\n", p->x);
  printf("Value: %d\n", p->y);

  // case 2
  int *arr = (int *)simple_malloc(sizeof(int) * 3);
  arr[0] = 10;
  arr[1] = 20;
  arr[2] = 30;

  printf("values : %d, %d, %d\n", arr[0], arr[1], arr[2]);

  return 0;
}
