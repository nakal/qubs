#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED

/*
 * List elements for list below.
 */
typedef struct _listentry_t {
	void *			element;
	struct _listentry_t *	next;
} listentry_t;

/*
 * List of linked elements.
 */
typedef struct {
	listentry_t *	first;
	listentry_t *	last;

	size_t		count;
} list_t;

extern list_t *list_create(void);
extern void list_destroy(list_t *list);
extern void list_destroywithelements(list_t *list, void (*freefunc)(void *));
extern int list_append(list_t *list, void *element);
extern int list_prepend(list_t *list, void *element);
extern int list_remove(list_t *list, void *element);
extern void *list_removefirst(list_t *list);
extern void list_empty(list_t *list);
extern void list_list_append(list_t *dst, list_t *src);
#endif
