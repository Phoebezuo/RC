#include "rc.h"
#include <assert.h>
#include <stdbool.h>

// an array of strong_ref pointers
struct ref_graph{
	struct strong_ref** refs_list;
	size_t refs_size;
	size_t refs_capacity;
};

struct ref_graph graph = { .refs_list = NULL, .refs_size = 0, .refs_capacity = RC_INIT_SZ };

void add_graph(struct strong_ref* ref) {
	if (graph.refs_list == NULL) { // initialization
		graph.refs_list = malloc(sizeof(struct strong_ref*) * graph.refs_capacity);
		*graph.refs_list = ref;
		graph.refs_size++;

	} else { // add ref to existing list
		*(graph.refs_list + graph.refs_size) = ref;
		graph.refs_size++;
	}

	// when size hit capacity, increase capacity
	if (graph.refs_size == graph.refs_capacity) {
		graph.refs_capacity *= RC_GROWTH_RT;
		graph.refs_list = realloc(graph.refs_list, sizeof(struct strong_ref*) * graph.refs_capacity);
	}
}

void add_entry(struct strong_ref* ref) {
	ref->entry.count = 1;
	ref->entry.n_deps = 0;
	ref->entry.dep_capacity = RC_INIT_SZ;
	ref->entry.dep_list = malloc(sizeof(size_t) * ref->entry.dep_capacity);
}

void add_ref_to_dep(struct strong_ref* dep, struct strong_ref* ref) {
	// add ref into dep's dep_list, dep's count not change
	int entry_id = find_index_of_strong(ref);
	*(dep->entry.dep_list + dep->entry.n_deps) = entry_id;
	dep->entry.n_deps++;

	// when size hit capacity, increase capacity
	if (dep->entry.n_deps == dep->entry.dep_capacity) {
		dep->entry.dep_capacity *= RC_GROWTH_RT;
		dep->entry.dep_list = realloc(dep->entry.dep_list, sizeof(size_t) * dep->entry.dep_capacity);
	}
}

struct strong_ref* ptr_exist_return_strong(void* a) {
	for (int i = 0; i <	(int)graph.refs_size; i++) {
		if (graph.refs_list[i]->ptr == a) {
			return graph.refs_list[i];
		}
	}
	return NULL;
}

int strong_exist_return_bool(struct strong_ref* a) {
	for (int i = 0; i < (int)graph.refs_size; i++) {
		if (graph.refs_list[i] == a) {
			return true;
		}
	}
	return false;
}

int find_index_of_strong(struct strong_ref* a) {
	for (int i = 0; i < (int)graph.refs_size; i++) {
		if (graph.refs_list[i] == a) {
			return i;
		}
	}
	return -1;
}

struct strong_ref* rc_alloc(void* ptr, size_t n, struct strong_ref* dep) {

	if (ptr == NULL) {
		if (dep == NULL) {
			// return a new allocation
			struct strong_ref* ref = malloc(sizeof(struct strong_ref));
			ref->ptr = malloc(n);

			add_graph(ref);
			add_entry(ref);

			return ref;
		} else {
			// return a new allocation but the count will correlate to the dependency
			struct strong_ref* ref = malloc(sizeof(struct strong_ref));
			ref->ptr = malloc(n);

			add_graph(ref);
			add_entry(ref);
			add_ref_to_dep(dep, ref);

			return ref;
		}
	}

	if (ptr != NULL && (ptr_exist_return_strong(ptr) != NULL)) {
		if (dep == NULL) {
			// increment the reference count of the allocation and return a strong_ref pointer
			struct strong_ref* ref = ptr_exist_return_strong(ptr);
			ref->entry.count++;
			return ref;
		} else {
			// increment the count of the strong reference
			struct strong_ref* ref = ptr_exist_return_strong(ptr);
			ref->entry.count++;

			add_graph(ref);
			add_entry(ref);
			add_ref_to_dep(dep, ref);

			// ref is not only depend on dep, but can also access with another independent variable
			ref->entry.count++;

			return ref;
		}
	}

	return NULL;
}

void dfs_to_decrease_count(struct strong_ref* ref) {
	// decrement the reference count by 1
	ref->entry.count--;

	if (ref->entry.n_deps == 0) { return; } // base case

	for (int i = 0; i < (int)ref->entry.n_deps; i++) { // recursion case
		if ((int)graph.refs_list[ref->entry.dep_list[i]]->entry.count > 0) {
			dfs_to_decrease_count(graph.refs_list[ref->entry.dep_list[i]]);
		}
	}
}

struct weak_ref rc_downgrade(struct strong_ref* ref) {
	struct weak_ref r;

	// valid case
	if (ref->entry.count > 0) { dfs_to_decrease_count(ref); }

	// invalid case
	if (ref == NULL || !strong_exist_return_bool(ref) || ref->entry.count <= 0) {
		r.entry_id = RC_INVALID_REF;
		return r;
	}

	r.entry_id = find_index_of_strong(ref);
	return r;
}

struct strong_ref* rc_upgrade(struct weak_ref ref) {
	if (ref.entry_id == RC_INVALID_REF || graph.refs_list == NULL || ref.entry_id >= graph.refs_size) { return NULL; }

	size_t index = ref.entry_id;
	struct strong_ref* ret = graph.refs_list[index];

	if (ret->entry.count == 0) {
		// a strong reference no longer exists or has been deallocated
		return NULL;
	} else {
		// upgrade weak reference to a strong reference
		ret->entry.count++;
		return ret;
	}
}

void rc_cleanup() {
	// cleans up the reference counting graph
	for (int i = 0; i < (int)graph.refs_size; i++) {
		// struct strong_ref* a =
		free(graph.refs_list[i]->entry.dep_list);
		free(graph.refs_list[i]->ptr);
		free(graph.refs_list[i]);
	}
	free(graph.refs_list);
	graph.refs_list = NULL;
	graph.refs_size = 0;
	graph.refs_capacity = 0;
}
