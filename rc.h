#ifndef TREE_REF
#define TREE_REF

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define RC_INVALID_REF (0xFFFFFFFFFFFFFFFF)
#define RC_INIT_SZ (8)
#define RC_GROWTH_RT (2)
#define RC_REF(type,obj) ((type*)obj->ptr)
#define RC_DEREF(type,obj) (*RC_REF(type,obj))

struct rc_entry {
    size_t count;
    size_t* dep_list;
    size_t n_deps;
    size_t dep_capacity;
};

// direct access to the reference counter entry and the memory allocation
struct strong_ref {
    void* ptr;
    struct rc_entry entry;
};

// need to retrieve the strong reference (upgrade to strong reference).
struct weak_ref {
    size_t entry_id;
};

struct strong_ref* rc_alloc(void* ptr, size_t n, struct strong_ref* dep);
struct weak_ref rc_downgrade(struct strong_ref* ref);
struct strong_ref* rc_upgrade(struct weak_ref weak);
void rc_cleanup();

// // own functions
struct strong_ref* ptr_exist_return_strong(void* a);
int strong_exist_return_bool(struct strong_ref* a);
int find_index_of_strong(struct strong_ref* ref);
void add_graph(struct strong_ref* ref);
void add_entry(struct strong_ref* ref);
void add_ref_to_dep(struct strong_ref* dep, struct strong_ref* ref);
struct strong_ref* ptr_exist_return_strong(void* a);
int strong_exist_return_bool(struct strong_ref* a);
int find_index_of_strong(struct strong_ref* a);
void dfs_to_remove_count(struct strong_ref* ref);

#endif
