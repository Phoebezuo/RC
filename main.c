#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rc.h"

int example1() {
    struct strong_ref* m = rc_alloc(NULL, sizeof(int), NULL); // int m = new int()
    *((int*)m->ptr) = 100; // m = 100

    struct strong_ref* a = rc_alloc(m->ptr, 0, NULL); // int a = m 
    printf("%d\n", RC_DEREF(int, a)); // 100

    return 0;
}
/*
100
*/

void example2() {
    struct strong_ref* m = rc_alloc(NULL, sizeof(int), NULL); // int m = new int()
    *((int*)m->ptr) = 100;
    printf("%d\n", RC_DEREF(int, m)); // 100

    struct weak_ref w = rc_downgrade(m); // Deallocated here
    printf("%zu\n", w.entry_id); // 0
    struct strong_ref* p = rc_upgrade(w); // w is not existing anymore, so need to return NULL

    if(p == NULL) {
        puts("Reference has been removed");
    }
}
/*
100
18446744073709551615
Reference has been removed
*/


void example3() {
    struct strong_ref* m = rc_alloc(NULL, sizeof(int), NULL); // int m = new int()
    struct strong_ref* a = rc_alloc(m->ptr, 0, NULL); // a = m

    *((int*)m->ptr) = 2; // m = 2
    rc_downgrade(m); // Decrements

    *((int*)a->ptr) = 0; // a = 0
    printf("m = %d\n", *((int*)a->ptr)); // 0

    struct weak_ref w = rc_downgrade(a); // Deallocated a, m 

    struct strong_ref* p = rc_upgrade(w); // w is not existing anymore, so need to return NULL

    if(p == NULL) {
        puts("Reference has been removed");
    }
}
/*
m = 0
Reference has been removed
*/

struct obj {
	int p; 
	struct obj* k; 
};

void example4() {
    struct strong_ref* m = rc_alloc(NULL, sizeof(struct obj), NULL); // obj m = new obj()
    struct strong_ref* pp = ((struct obj*) m->ptr)->p;
    // ((struct obj*) m->ptr)->p = rc_alloc(NULL, sizeof(int), m); //int m.p = new int()
    pp = rc_alloc(NULL, sizeof(int), m); 

    struct weak_ref w = rc_downgrade(m); // Deallocated m, m.p

    printf("%d\n", w.entry_id == RC_INVALID_REF); // 1

    struct strong_ref* z = rc_upgrade(w); // w is not existing anymore, so need to return NULL

    if(z == NULL) {
        puts("Reference has been removed");
    }
}
/*
1
Reference has been removed
*/

void example5() {
    struct strong_ref* m = rc_alloc(NULL, sizeof(int), NULL); // int m = new int()
    struct strong_ref* a = rc_alloc(m->ptr, 0, NULL); // a = m
    // entry for a and m is 2 

    struct weak_ref w = rc_downgrade(a); // entry for a and m is 1 
    printf("%zu ", m->entry.count); // 1

    a = rc_upgrade(w); // entry for a and m is 2
    printf("%zu ", m->entry.count); // 2

    rc_downgrade(a); // entry for a and m is 1
    struct weak_ref k = rc_downgrade(m); // Deallocates m and a, entry for a and m is 0
    printf("%zu\n", m->entry.count); // 0

    struct strong_ref* z = rc_upgrade(k); // w is not existing anymore, so need to return NULL
    if(z == NULL) {
        puts("Reference has been removed");
    }
    rc_cleanup();
    printf("%d\n", rc_upgrade(k) == NULL);
}
/*
1 2 0
Reference has been removed
1
*/

void example6() {
	struct strong_ref* m = rc_alloc(NULL, sizeof(struct obj), NULL); // obj m = new obj()
	struct strong_ref* n = rc_alloc(NULL, sizeof(struct obj), NULL); // obj n = new obj()
	struct strong_ref* kk = ((struct obj*) m->ptr)->k;
    kk = rc_alloc(n->ptr, sizeof(struct obj), m); // obj m.k = n 

    printf("%zu ", m->entry.count); // 1
    printf("%zu\n", n->entry.count); // 2

    struct weak_ref k = rc_downgrade(m); 
    printf("%zu ", m->entry.count); // 0
    printf("%zu ", kk->entry.count); // 1
    printf("%zu\n", n->entry.count); // 1
}
/*
1 2 
0 1 1
*/

void example7() { // dependency_3 testcase 
    struct strong_ref* a = rc_alloc(NULL, sizeof(struct obj), NULL); // obj a = new obj()
    struct strong_ref* b = rc_alloc(NULL, sizeof(struct obj), a); // obj a.b = new obj()
    struct strong_ref* c = rc_alloc(NULL, sizeof(struct obj), b); // obj a.b.c = new obj()

    rc_downgrade(c);
    printf("%zu ", a->entry.count); // 1
    printf("%zu ", b->entry.count); // 1
    printf("%zu\n", c->entry.count); // 0

    rc_downgrade(c);
    printf("%zu ", a->entry.count); // 1
    printf("%zu ", b->entry.count); // 1
    printf("%zu\n", c->entry.count); // 0

    rc_downgrade(a);
    printf("%zu ", a->entry.count); // 0
    printf("%zu ", b->entry.count); // 0
    printf("%zu\n", c->entry.count); // 0
}
/*
1 1 0
1 1 0
0 0 0 
*/

int main() {
    puts("example1:");
	example1();

    puts("\nexample2:");
	example2();

    puts("\nexample3:");
	example3();

    puts("\nexample4:");
	example4(); 

    puts("\nexample5:"); 
	example5();

    puts("\nexample6:");
	example6();

    puts("\nexample7:");
	example7();
	return 0; 
}