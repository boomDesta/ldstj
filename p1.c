#include <stdio.h>
#include <stdlib.h>

struct S{
    struct D* a;
    struct D* b;
};
struct D{
    int c;
    int d;
};
struct S* d0;

int main(int argc,char** argv) {
    struct S* d0 = malloc(sizeof(struct S));
    d0->b = malloc(sizeof(struct D));
    d0->a = malloc(sizeof(struct D));
    d0->a->c = 1;
    d0->a->d = 2;
    d0->b->c = 3;
    d0->b->d = 4;

    d0->b->c = d0->a->d;
    d0->a->c = d0->b->d;
    printf("%d\n%d\n%d\n%d",d0->a->c,d0->a->d,d0->b->c,d0->b->d);
    return 0;
}
