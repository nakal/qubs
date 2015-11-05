#ifndef HASH_INCLUDED
#define HASH_INCLUDED

/* Hash size. Use power of 2! */
#define HASH_SIZE    256

typedef struct _hash_element_list_t {
	char *				key;
	void *				data;
	struct _hash_element_list_t *	next;
} hash_element_list_t;

typedef struct {
	/* key array */
	hash_element_list_t **	karray;
	/* key count */
	size_t			size;

	/* current number of elements */
	size_t			element_count;
} hash_t;

extern hash_t *hash_create(size_t size);
extern int hash_put(hash_t *hash, const char *key, void *val, size_t valsize);
extern void *hash_get(hash_t *hash, const char *key);
extern void hash_destroy(hash_t *hash);
extern int hash_appendtolist(hash_element_list_t **list,
    hash_element_list_t *elem);
extern void *hash_getlistelement(hash_element_list_t *list, const char *key);
extern void hash_dump(hash_t *hash);
extern void hash_dumplist(hash_element_list_t *elem);
#endif
