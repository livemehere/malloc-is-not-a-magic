#include <stdio.h>
#include <stdlib.h>

typedef struct Point {
  int x;
  int y;
} Point;

int main() {

  Point *p = (Point *)malloc(sizeof(Point));
  p->x = 10;
  p->y = 20;

  printf("Value: %d\n", p->x);
  printf("Value: %d\n", p->y);

  free(p);

  printf("Value: %d\n", p->x);
  printf("Value: %d\n", p->y);

  return 0;
}
