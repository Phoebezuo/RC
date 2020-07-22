# RC

You are to develop a reference counting garbage collector library that can be used by other developers. The overall goal of this library is to make memory allocation and deallocation easier for programmers. Assuming they abide by the rules of the library and do not make any allocations outside of this library, allocations should be completely managed by this library.

This library will be used as a replacement for manual memory allocation. One drawback of this library used in a context of C, is that all calls are explicit, so allocations can potentially be lost or you could wind up with an invalid count. Your library will have explicit calls to alloc, upgrade and downgrade.

Your references are broken up into two different types, Strong references and Weak references. A strong reference has direct access to the reference counter entry and the memory allocation, while a weak reference will need to retrieve the strong reference (upgrade to strong reference). A strong reference can be downgraded to a weak reference via `downgrade` and a weak reference can be upgraded to a strong reference via `upgrade`. 

WARNING: If your `.bss` section is found to be larger than 256 bytes, your program will instantly fail this task.

## Structures

You must use the following structs, no modifications can be made to the structures. However, you are expected to have at least 1 *static* variable.

```
struct rc_entry {
    size_t count;
    size_t* dep_list;
    size_t n_deps;
    size_t dep_capacity;
}
struct strong_ref {
    void* ptr;
    struct rc_entry entry;
}
struct weak_ref {
    size_t entry_id;
};
```

## Functions and documentation

Use the following documentation to assist with your implementation.

```
/**
 * Returns an allocation of n bytes and creates an internal rc entry.
 *
 * If the ptr argument is NULL and deps is NULL, it will return a new 
 * allocation
 * 
 * If the ptr argument is not NULL and an entry exists, it will increment
 *  the reference count of the allocation and return a strong_ref pointer
 *
 * If the ptr argument is NULL and deps is not NULL, it will return 
 * a new allocation but the count will correlate to the dependency
 * if the dependency is deallocated the reference count on the object will decrement
 *
 * If the ptr argument is not NULL and an entry exists and dep is not
 * NULL, it will increment the count of the strong reference but the count
 * will be related to the dependency, if the dependency is deallocated the
 * reference count on the object will decrement
 */
strong_ref* rc_alloc(void* ptr, size_t n, struct strong_ref* dep);



/**
 * Downgrades a strong reference to a weak reference, this will decrement the reference count by 1
 * If ref is NULL, the function will return an invalid weak ref object
 * If ref is a value that does not exist in the reference graph, it will return an weak_ref object 
 *  that is invalid
 * If ref is a value that does exist in the reference graph, it will return
 *    a valid weak_ref object
 * 
 * An invalid weak_ref object is where its entry_id field is set to 0xFFFFFFFFFFFFFFFF
 *
 * @param strong_ref* ref (reference to allocation)
 * @return weak_ref (reference with an entry id)
 */
weak_ref rc_downgrade(strong_ref* ref);


/**
 * Upgrdes a weak reference to a strong reference.
 * The weak reference should check that the entry id is valid (bounds check)
 * If a strong reference no longer exists or has been deallocated, the return 
 *   result should be null.
 */
strong_ref* rc_upgrade(weak_ref ref);


/**
 * Cleans up the reference counting graph.
 */
void rc_cleanup();
```

## Examples

### Example 1

```
void example() {
    struct strong_ref* m = rc_alloc(NULL, sizeof(int), NULL);

    *((int*)m->ptr) = 100;
    printf("%d\n", *((int*)m->ptr));

    struct weak_ref w = rc_downgrade(m); <-- Deallocated here
    //check if w is invalid
    struct strong_ref* p = rc_upgrade(w);
    if(p == NULL) {
        puts("Reference has been removed");
    }
}
```

### Example 2

```
void example() {
    struct strong_ref* m = rc_alloc(NULL, sizeof(int), NULL);
    struct strong_ref* a = rc_alloc(m->ptr, 0, NULL); <-- Increments count by 1
    *((int*)m->ptr) = 2;
    rc_downgrade(m); <-- Decrements
    *((int*)a->ptr) = 0;
    rc_downgrade(a); <-- Deallocates
}
```

### Example 3

```
void example() {
    struct strong_ref* m = rc_alloc(NULL, sizeof(struct obj), NULL);
    ((struct obj*) m->ptr)->p = rc_alloc(NULL, sizeof(int), m); <-- p is dependent on m

    rc_downgrade(m); <-- Deallocates m and m->ptr->p, should return an invalid weak_ref
}
```

### Example 4

```
void example() {
    struct strong_ref* m = rc_alloc(NULL, sizeof(int), NULL);
    struct strong_ref* a = rc_alloc(m->ptr, 0, NULL);
    
    struct weak_ref w = rc_downgrade(a); <-- reduces the count by 1
    
    a = rc_upgrade(w); <--- Increments the count by 1
    rc_downgrade(a);
    rc_downgrade(m); <-- Deallocates m and a, should return an invalid weak_ref
}
```

### Run the Program

``` shell
make all
```

### Clean the Program

``` shell
make clean
```

### Sample Output

``` shell
example1:
100

example2:
100
18446744073709551615
Reference has been removed

example3:
m = 0
Reference has been removed

example4:
1
Reference has been removed

example5:
1 2 0
Reference has been removed
1

example6:
1 2
0 1 1

example7:
1 1 0
1 1 0
0 0 0
```

