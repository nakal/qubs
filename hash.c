
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "hash.h"
#include "memory.h"

static size_t hashsum(const hash_t *hash, const char *key);
static void hash_destroylist(hash_element_list_t *elem);

hash_t * hash_create(size_t size) {

	hash_t *hash;

#ifdef DEBUG
	printf("Creating hash with size %u.\n", size);
#endif
	hash=mem_malloc(sizeof(hash_t));
	if (hash==NULL) return NULL;

	hash->karray=(hash_element_list_t **)
		mem_calloc(1, size*sizeof(hash_element_list_t *));
	if (hash->karray==NULL) {
		mem_free(hash);
		return NULL;
	}
	/* memset(hash->karray, 0, size*sizeof(hash_element_list_t *)); */

	hash->size=size;
	hash->element_count=0;

#ifdef DEBUG
	printf("Creating done.\n");
#endif
	return hash;
}

/* inserts a copy of val into the hash */
int hash_put(hash_t *hash, const char *key, void *val, size_t valsize) {

	hash_element_list_t *elem;
	size_t sum;
	int retval;

#ifdef DEBUG
	printf("Inserting element with key [%s].\n", key);
#endif
	/* make an element */
	elem=mem_malloc(sizeof(hash_element_list_t));
	if (elem==NULL) return -1;
	memset(elem, 0, sizeof(hash_element_list_t));

	/* make a copy of key */
	elem->key=mem_malloc(strlen(key)+1);
	if (elem->key==NULL) {
		mem_free(elem);
		return -1;
	}
	strcpy(elem->key, key);

	/* make a copy of value */
	elem->data=mem_malloc(valsize);
	if (elem->data==NULL) {
		mem_free(elem->key);
		mem_free(elem);
		return -1;
	}
	memcpy(elem->data, val, valsize);

	/* calculate hashsum */
	sum=hashsum(hash, key);
	assert ( sum < hash->size );
	retval=hash_appendtolist(&hash->karray[sum], elem);
	if (retval==0) hash->element_count++;

#ifdef DEBUG
	printf("%s element finished (%d).\n", retval==0 ?
		"Inserting" : "Replacing", retval);
#endif

	if (retval==1) retval=0; /* just replacing, but everything ok */
	return retval;
}

void * hash_get(hash_t *hash, const char *key) {

	size_t sum;

#ifdef DEBUG
	printf("Retrieving element with key [%s].\n", key);
#endif
	sum=hashsum(hash, key);
	assert(sum < hash->size);
	return hash_getlistelement(hash->karray[sum], key);
}

void hash_destroy(hash_t *hash) {

	size_t i;

#ifdef DEBUG
	printf("Destroying hash.\n");
#endif
	for (i=0; i<hash->size; i++) {
		if (hash->karray[i]!=NULL) {
			hash_destroylist(hash->karray[i]);
		}
	}

	mem_free(hash->karray);
	mem_free(hash);
}

static void hash_destroylist(hash_element_list_t *elem) {

	hash_element_list_t *next;

#ifdef DEBUG
	printf("Destroying list element [%s].\n", elem->key);
#endif
	next=elem->next;
	mem_free(elem->key);
	mem_free(elem->data);
	mem_free(elem);
	if (next!=NULL) hash_destroylist(next);
}

int hash_appendtolist(hash_element_list_t **list, hash_element_list_t *elem) {

	if (*list!=NULL && strcmp((*list)->key, elem->key)!=0)
		return hash_appendtolist(&(*list)->next, elem);
	else {

#ifdef DEBUG
		printf("Appending list element [%s] (%s key).\n", elem->key,
			*list==NULL ? "new" : "replacing");
#endif

		if (*list==NULL) {

			/* no element with this key exists in the list */

			*list=elem;
			elem->next=NULL;

			return 0;
		} else {
			/* element with the specified key is already in the list */

			mem_free((*list)->data);
			(*list)->data=elem->data;
			mem_free(elem->key);
			mem_free(elem);

			return 1;
		}
	}
}

void * hash_getlistelement(hash_element_list_t *list, const char *key) {

	if (list==NULL) return NULL;
	if (strcmp(list->key, key))
		return hash_getlistelement(list->next, key);
	else return list->data;
}

static size_t hashsum(const hash_t *hash, const char *key) {
	size_t k;
	size_t len, i;

	k=0xfeed;
	len=strlen(key);
	for (i=0; i<len; i++) {
		k=(k>>1)^(k<<1);
		k^=key[i];
	}

	return k & (hash->size-1);
}

void hash_dump(hash_t *hash) {

	size_t i;

	for (i=0; i<hash->size; i++) {
		if (hash->karray[i]!=NULL) {
			printf("%zu:\n", i);
			hash_dumplist(hash->karray[i]);
		}
	}
}

void hash_dumplist(hash_element_list_t *elem) {

	printf("\t[%s] => \"%s\"\n", elem->key, (char *)elem->data);

	if (elem->next!=NULL) {
		assert(strcmp(elem->next->key, elem->key));
		hash_dumplist(elem->next);
	}
}

#ifdef UNIT_TEST
int main(void) {
	hash_t *h;

	mem_init;

	/* simple key test */
	h=hash_create(256);
	assert(h!=NULL);
	hash_put(h, "test1", "test1", 6);
	hash_put(h, "test2", "test2", 6);
	hash_put(h, "test3", "test3", 6);
	hash_dump(h);
	assert(h->element_count==3);
	assert(strcmp(hash_get(h, "test1"), "test1")==0);
	assert(strcmp(hash_get(h, "test2"), "test2")==0);
	assert(strcmp(hash_get(h, "test3"), "test3")==0);
	hash_destroy(h);

	/* same key test */
	h=hash_create(256);
	assert(h!=NULL);
	hash_put(h, "test1", "test1", 6);
	hash_put(h, "test2", "test2", 6);
	hash_put(h, "test1", "test3", 6);
	hash_dump(h);
	assert(h->element_count==2);
	hash_destroy(h);

	/* testing collisions */
	h=hash_create(2);
	assert(h!=NULL);
	hash_put(h, "test1", "test1", 6);
	hash_put(h, "test2", "test2", 6);
	hash_put(h, "test3", "test3", 6);
	hash_put(h, "test4", "test4", 6);
	hash_put(h, "test5", "test5", 6);
	hash_put(h, "test6", "test6", 6);
	hash_dump(h);
	assert(h->element_count==6);
	assert(strcmp(hash_get(h, "test1"), "test1")==0);
	assert(strcmp(hash_get(h, "test2"), "test2")==0);
	assert(strcmp(hash_get(h, "test3"), "test3")==0);
	assert(strcmp(hash_get(h, "test4"), "test4")==0);
	assert(strcmp(hash_get(h, "test5"), "test5")==0);
	assert(strcmp(hash_get(h, "test6"), "test6")==0);
	hash_destroy(h);

	/* testing replacement */
	h=hash_create(2);
	assert(h!=NULL);
	hash_put(h, "test1", "test1", 6);
	hash_put(h, "test2", "test2", 6);
	hash_put(h, "test3", "test3", 6);
	hash_put(h, "test4", "test4", 6);
	hash_put(h, "test5", "test5", 6);
	hash_put(h, "test6", "test6", 6);
	hash_put(h, "test6", "test7", 6);
	hash_dump(h);
	assert(h->element_count==6);
	assert(strcmp(hash_get(h, "test1"), "test1")==0);
	assert(strcmp(hash_get(h, "test2"), "test2")==0);
	assert(strcmp(hash_get(h, "test3"), "test3")==0);
	assert(strcmp(hash_get(h, "test4"), "test4")==0);
	assert(strcmp(hash_get(h, "test5"), "test5")==0);
	assert(strcmp(hash_get(h, "test6"), "test7")==0);
	hash_destroy(h);

	/* testing multi-replacement */
	h=hash_create(1);
	assert(h!=NULL);
	hash_put(h, "test3", "test5", 6);
	hash_put(h, "test2", "test5", 6);
	hash_put(h, "test1", "test1", 6);
	hash_put(h, "test1", "test4", 6);
	hash_put(h, "test3", "test6", 6);
	hash_put(h, "test2", "test5", 6);
	hash_put(h, "test1", "test2", 6);
	hash_put(h, "test3", "test4", 6);
	hash_put(h, "test2", "test6", 6);
	hash_put(h, "test1", "test3", 6);
	hash_put(h, "test2", "test7", 6);
	hash_dump(h);
	assert(h->element_count==3);
	assert(strcmp(hash_get(h, "test1"), "test3")==0);
	assert(strcmp(hash_get(h, "test2"), "test7")==0);
	assert(strcmp(hash_get(h, "test3"), "test4")==0);
	hash_destroy(h);

	/* testing replacement in the middle */
	h=hash_create(2);
	assert(h!=NULL);
	hash_put(h, "test1", "test1", 6);
	hash_put(h, "test2", "test2", 6);
	hash_put(h, "test3", "test3", 6);
	hash_put(h, "test4", "test4", 6);
	hash_put(h, "test5", "test5", 6);
	hash_put(h, "test6", "test6", 6);
	hash_put(h, "test4", "test7", 6);
	hash_dump(h);
	assert(h->element_count==6);
	assert(strcmp(hash_get(h, "test1"), "test1")==0);
	assert(strcmp(hash_get(h, "test2"), "test2")==0);
	assert(strcmp(hash_get(h, "test3"), "test3")==0);
	assert(strcmp(hash_get(h, "test4"), "test7")==0);
	assert(strcmp(hash_get(h, "test5"), "test5")==0);
	assert(strcmp(hash_get(h, "test6"), "test6")==0);
	hash_destroy(h);

	/* testing replacement at start */
	h=hash_create(2);
	assert(h!=NULL);
	hash_put(h, "test1", "test1", 6);
	hash_put(h, "test2", "test2", 6);
	hash_put(h, "test3", "test3", 6);
	hash_put(h, "test4", "test4", 6);
	hash_put(h, "test5", "test5", 6);
	hash_put(h, "test6", "test6", 6);
	hash_put(h, "test1", "test7", 6);
	hash_dump(h);
	assert(h->element_count==6);
	assert(strcmp(hash_get(h, "test1"), "test7")==0);
	assert(strcmp(hash_get(h, "test2"), "test2")==0);
	assert(strcmp(hash_get(h, "test3"), "test3")==0);
	assert(strcmp(hash_get(h, "test4"), "test4")==0);
	assert(strcmp(hash_get(h, "test5"), "test5")==0);
	assert(strcmp(hash_get(h, "test6"), "test6")==0);
	hash_destroy(h);

	mem_done;
	return 0;
}
#endif
