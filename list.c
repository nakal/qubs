#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "memory.h"
#include "list.h"

#ifdef UNIT_TEST
#include <stdio.h>
#include <string.h>
#endif

list_t *
list_create(void)
{
	return mem_calloc(1, sizeof(list_t));
}

void
list_destroy(list_t *list)
{
	list_empty(list);
	mem_free(list);
}

void
list_destroywithelements(list_t *list, void (*freefunc)(void *))
{
	listentry_t *e;

	e = list->first;
	while (e != NULL) {
		(*freefunc)(e->element);
		e = e->next;
	}

	list_destroy(list);
}

int
list_append(list_t *list, void *element)
{
	listentry_t *listentry;

	listentry = (listentry_t *)mem_malloc(sizeof(listentry_t));
	if (listentry == NULL) return -1;

	listentry->next = NULL;
	listentry->element = element;

#ifdef UNIT_TEST
	printf("Created element with %p\n", listentry->element);
	printf("First element in list %p\n",
	    list->first == NULL ? NULL : list->first->element);
	printf("Last element in list %p\n",
	    list->last == NULL ? NULL : list->last->element);
#endif

	if (list->last == NULL) {
		assert(list->last == list->first);
		list->first = list->last = listentry;
	} else {
		list->last->next = listentry;
		list->last = listentry;
	}

	list->count++;
	return 0;
}

/*
 *      TODO: unit tests
 */
int
list_prepend(list_t *list, void *element)
{
	listentry_t *listentry;

	listentry = (listentry_t *)mem_malloc(sizeof(listentry_t));
	if (listentry == NULL) return -1;

	listentry->next = list->first;
	listentry->element = element;

#ifdef UNIT_TEST
	printf("Created element with %p\n", listentry->element);
	printf("First element in list %p\n",
	    list->first == NULL ? NULL : list->first->element);
	printf("Last element in list %p\n",
	    list->last == NULL ? NULL : list->last->element);
#endif

	if (list->last == NULL) {
		assert(list->last == list->first);
		list->first = list->last = listentry;
	} else
		list->first->next = listentry;

	list->count++;
	return 0;
}

int
list_remove(list_t *list, void *element)
{
	listentry_t *cur;
	listentry_t *prev;

	prev = NULL;
	cur = list->first;
	while (cur != NULL && cur->element != element) {
		prev = cur;
		cur = cur->next;
	}

	if (cur != NULL) {
		/* unlink entry */
		if (prev != NULL)
			prev->next = cur->next;
		else list->first = cur->next;

		if (cur == list->last)
			list->last = prev;

		mem_free(cur);

		list->count--;
		return 0;
	}

	return -1;
}

void *
list_removefirst(list_t *list)
{
	if (list->first != NULL) {
		listentry_t *toremove;
		void *element;

		toremove = list->first;
		list->first = list->first->next;
		if (list->first == NULL) {
			list->last = NULL;
			assert(list->count == 1);
		}
		element = toremove->element;
		mem_free(toremove);
		list->count--;
		return element;
	} else assert(list->first == list->last);

	return NULL;
}

void
list_empty(list_t *list)
{
#ifdef UNIT_TEST
	printf("Destroying list with %u elements.\n", list->count);
#endif
	while (list->count > 0) {
		list_removefirst(list);
	}
	assert(list->first == NULL && list->last == NULL);
}

void
list_list_append(list_t *dst, list_t *src)
{
	assert(src != NULL);
	assert(dst != NULL);

	if (src->first != NULL) {
		if (dst->first == NULL)
			dst->first = src->first;
		else
			dst->last->next = src->first;
		dst->last = src->last;
	}
}

#ifdef UNIT_TEST

int
main(void)
{
	char *AAA = "AAA";
	char *BBB = "BBB";
	char *CCC = "CCC";
	/*char *DDD="DDD";*/

	list_t list;

	memset(&list, 0, sizeof(list_t));

	printf(">> Testing list_append().\n");
	list_append(&list, AAA);
	list_append(&list, BBB);
	list_append(&list, CCC);

	assert(strcmp(list.first->element, AAA) == 0);
	assert(strcmp(list.first->next->element, BBB) == 0);
	assert(strcmp(list.first->next->next->element, CCC) == 0);
	assert(list.first->next->next->next == NULL);
	assert(list.count == 3);
	printf("List Test (inserted 3 elements)\n");

	printf(">> Testing list_removefirst().\n");
	list_removefirst(&list);
	assert(strcmp(list.first->element, BBB) == 0);
	assert(strcmp(list.first->next->element, CCC) == 0);
	assert(list.first->next->next == NULL);
	assert(list.count == 2);
	printf("List Test (removed top element; 2 left)\n");

	list_removefirst(&list);
	assert(strcmp(list.first->element, CCC) == 0);
	assert(list.first->next == NULL);
	assert(list.count == 1);
	printf("List Test (removed top element; 1 left)\n");

	list_removefirst(&list);
	assert(list.first == NULL);
	assert(list.last == NULL);
	assert(list.count == 0);
	printf("List Test (removed top element; list empty now)\n");

	printf(">> Testing list_append() in used list.\n");
	list_append(&list, AAA);
	list_append(&list, BBB);
	list_append(&list, CCC);
	assert(strcmp(list.first->element, AAA) == 0);
	assert(strcmp(list.first->next->element, BBB) == 0);
	assert(strcmp(list.first->next->next->element, CCC) == 0);
	assert(list.first->next->next->next == NULL);
	assert(list.count == 3);
	printf("List Test (inserted 3 elements)\n");

	printf(">> Testing list_remove().\n");
	list_remove(&list, BBB);
	assert(strcmp(list.first->element, AAA) == 0);
	assert(strcmp(list.first->next->element, CCC) == 0);
	assert(list.first->next->next == NULL);
	assert(list.count == 2);
	printf("List Test (removed middle element; 2 left)\n");

	list_append(&list, BBB);
	assert(strcmp(list.first->element, AAA) == 0);
	assert(strcmp(list.first->next->element, CCC) == 0);
	assert(strcmp(list.first->next->next->element, BBB) == 0);
	assert(list.first->next->next->next == NULL);
	assert(list.count == 3);
	printf("List Test (inserted back)\n");

	printf(">> Testing list_remove().\n");
	list_remove(&list, BBB);
	assert(strcmp(list.first->element, AAA) == 0);
	assert(strcmp(list.first->next->element, CCC) == 0);
	assert(list.first->next->next == NULL);
	assert(list.count == 2);
	printf("List Test (removed last element; 2 left)\n");

	printf(">> Testing list_remove().\n");
	list_remove(&list, AAA);
	assert(strcmp(list.first->element, CCC) == 0);
	assert(list.first->next == NULL);
	assert(list.count == 1);
	printf("List Test (removed first element; 1 left)\n");

	list_append(&list, BBB);
	assert(strcmp(list.first->element, CCC) == 0);
	assert(strcmp(list.first->next->element, BBB) == 0);
	assert(list.first->next->next == NULL);
	assert(list.count == 2);
	printf("List Test (inserted 1 element back)\n");

	printf(">> Testing list_remove().\n");
	list_remove(&list, BBB);
	assert(strcmp(list.first->element, CCC) == 0);
	assert(list.first->next == NULL);
	assert(list.count == 1);
	printf("List Test (removed element again; 1 left)\n");

	printf(">> Testing list_remove().\n");
	list_remove(&list, CCC);
	assert(list.first == NULL);
	assert(list.count == 0);
	printf("List Test (removed remaining element; 0 left)\n");
}
#endif
