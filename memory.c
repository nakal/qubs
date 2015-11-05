#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"

#ifndef NDEBUG

#define DEBUG_MALLOC_OUT    stderr

typedef struct {
	void *		ptr;
	size_t		size;
	const char *	srcname;
	int		srcline;
} memory_alloc_history_entry_t;

static memory_alloc_history_entry_t *memory_alloc_history = NULL;
static size_t memory_alloc_history_size = 0;
static size_t memory_alloc_history_pos = 0;

static memory_alloc_history_entry_t memory_remove_entry(void *ptr,
    const char *srcname,
    int srcline);
static void memory_add_entry(void *ptr, size_t size, const char *srcname,
    int srcline);

void
memory_init(void)
{
	memory_alloc_history = NULL;
	memory_alloc_history_size = 0;
	memory_alloc_history_pos = 0;

#ifdef DEBUG_MALLOC_VERBOSE
	fprintf(DEBUG_MALLOC_OUT, "DEBUGGER: memory_init()\n");
#endif
}

void *
memory_malloc(size_t size, const char *srcname, int line)
{
	void *ptr;

	ptr = compat_malloc(size);

#ifdef DEBUG_MALLOC_VERBOSE
	fprintf(DEBUG_MALLOC_OUT,
	    "malloc():\tin %s:%d %d bytes (-> %p)\n", srcname, line,
	    size, ptr);
	if (ptr == NULL) fprintf(DEBUG_MALLOC_OUT,
	    "WARNING: malloc returned NULL\n");
#endif
	memory_add_entry(ptr, size, srcname, line);

	return ptr;
}

void *
memory_calloc(size_t nelem, size_t size, const char *srcname, int line)
{
	void *ptr;

	ptr = compat_calloc(nelem, size);

#ifdef DEBUG_MALLOC_VERBOSE
	fprintf(DEBUG_MALLOC_OUT,
	    "calloc():\tin %s:%d %d bytes (-> %p)\n", srcname, line,
	    nelem*size, ptr);
	if (ptr == NULL) fprintf(DEBUG_MALLOC_OUT,
	    "WARNING: calloc returned NULL\n");
#endif
	memory_add_entry(ptr, size, srcname, line);

	return ptr;
}

void *
memory_realloc(void *ptr, size_t size, const char *srcname, int line)
{
	void *p;


	p = compat_realloc(ptr, size);

#ifdef DEBUG_MALLOC_VERBOSE
	fprintf(DEBUG_MALLOC_OUT,
	    "realloc():\tin %s:%d %d bytes (%p -> %p)\n", srcname, line,
	    size, ptr, p);
	if (p == NULL) fprintf(DEBUG_MALLOC_OUT,
	    "WARNING: realloc returned NULL\n");
#endif

	memory_remove_entry(ptr, srcname, line);
	memory_add_entry(p, size, srcname, line);

	return p;
}

void
memory_free(void *ptr, const char *srcname, int line)
{
#ifdef DEBUG_MALLOC_VERBOSE
	memory_alloc_history_entry_t entry;

	entry =
#endif
	    memory_remove_entry(ptr, srcname, line);


#ifdef DEBUG_MALLOC_VERBOSE
	fprintf(DEBUG_MALLOC_OUT,
	    "free():\tin %s:%d %d bytes (%p allocated in %s:%d)\n",
	    srcname, line, entry.size, ptr, entry.srcname, entry.srcline);
#endif

	free(ptr);
}

void
memory_done(void)
{
	if (memory_alloc_history_pos != 0) {
#ifdef DEBUG_MALLOC_VERBOSE
		fprintf(DEBUG_MALLOC_OUT,
		    "WARNING: have still %d malloc'ed chunks.\n",
		    memory_alloc_history_pos);
#endif
		memory_list_allocated();
	} else {
#ifdef DEBUG_MALLOC_VERBOSE
		fprintf(DEBUG_MALLOC_OUT,
		    "OK: memory has been cleared.\n");
#endif
	}

	free(memory_alloc_history);
	memory_alloc_history = NULL;
	memory_alloc_history_size = 0;
	memory_alloc_history_pos = 0;
}

void
memory_list_allocated(void)
{
	size_t i;

	fprintf(DEBUG_MALLOC_OUT,
	    "\n****************************************\n");
	fprintf(DEBUG_MALLOC_OUT, "Memory pointer list:\n");

	for (i = 0; i < memory_alloc_history_pos; i++)
		fprintf(DEBUG_MALLOC_OUT,
		    "\tfrom %s:%d have %zu bytes (at %p)\n",
		    memory_alloc_history[i].srcname,
		    memory_alloc_history[i].srcline,
		    memory_alloc_history[i].size,
		    memory_alloc_history[i].ptr
		    );
	fprintf(DEBUG_MALLOC_OUT,
	    "\n****************************************\n");
}

static void
memory_add_entry(void *ptr, size_t size, const char *srcname,
    int srcline)
{
	if (ptr == NULL) return;

	if (memory_alloc_history_pos >= memory_alloc_history_size) {
		memory_alloc_history_entry_t *mh;

		mh = (memory_alloc_history_entry_t *)
		    realloc(memory_alloc_history,
			(memory_alloc_history == NULL ? 32 :
			memory_alloc_history_size<<1)*
			sizeof(memory_alloc_history_entry_t));
		assert(mh != NULL);

		memory_alloc_history_size = (memory_alloc_history == NULL ? 32 :
		    memory_alloc_history_size<<1);
		memory_alloc_history = mh;
	}

	assert(memory_alloc_history_pos < memory_alloc_history_size);
	memory_alloc_history[memory_alloc_history_pos].ptr = ptr;
	memory_alloc_history[memory_alloc_history_pos].size = size;
	memory_alloc_history[memory_alloc_history_pos].srcname = srcname;
	memory_alloc_history[memory_alloc_history_pos].srcline = srcline;

	memory_alloc_history_pos++;
}

static memory_alloc_history_entry_t
memory_remove_entry(void *ptr,
    const char *srcname,
    int srcline)
{
	size_t i;
	memory_alloc_history_entry_t cp;

	assert(memory_alloc_history_pos <= memory_alloc_history_size);

	for (i = 0; i < memory_alloc_history_pos; i++)
		if (memory_alloc_history[i].ptr == ptr) break;

	if (i >= memory_alloc_history_pos) {
		fprintf(DEBUG_MALLOC_OUT,
		    "ERROR: tried to free junk (%p) in %s:%d.\n", ptr,
		    srcname, srcline);
		abort();
	}

	/* i pointing to to-be-removed entry */
	memcpy(&cp, memory_alloc_history+i,
	    sizeof(memory_alloc_history_entry_t));

	if (i+1 < memory_alloc_history_size)
		memmove(memory_alloc_history+i, memory_alloc_history+i+1,
		    (memory_alloc_history_size-i-1)*
		    sizeof(memory_alloc_history_entry_t));
	memset(&memory_alloc_history[memory_alloc_history_size-1], 0,
	    sizeof(memory_alloc_history_entry_t));

	memory_alloc_history_pos--;

	return cp;
}

void
memory_hexdump(void *ptr, const char *srcname, int srcline)
{
	size_t i;

	for (i = 0; i < memory_alloc_history_pos; i++)
		if (memory_alloc_history[i].ptr == ptr) break;

	if (i >= memory_alloc_history_pos) {
		fprintf(DEBUG_MALLOC_OUT,
		    "ERROR: tried to hexdump junk in %s:%d.\n"
		    "\tPlease use memory_hexdump_alien(), "
		    "if you think that it's correct.\n",
		    srcname, srcline);
		abort();
	}

	fprintf(DEBUG_MALLOC_OUT,
	    "\n****************************************\n");
	fprintf(DEBUG_MALLOC_OUT, "\tfrom %s:%d have %zu bytes (at %p)\n",
	    memory_alloc_history[i].srcname,
	    memory_alloc_history[i].srcline,
	    memory_alloc_history[i].size,
	    memory_alloc_history[i].ptr
	    );

	memory_hexdump_alien(ptr, memory_alloc_history[i].size);
	fprintf(DEBUG_MALLOC_OUT,
	    "\n****************************************\n");
}

void
memory_hexdump_alien(void *ptr, size_t len)
{
	size_t pos;

	for (pos = 0; pos < len; pos++) {
		if ((pos&0x0F) == 0)
			fprintf(DEBUG_MALLOC_OUT, "%06zX:  ", pos);

		fprintf(DEBUG_MALLOC_OUT, "%02X ", ((char *)ptr)[pos]);

		if ((pos&0x0F) == 0x0F)
			fprintf(DEBUG_MALLOC_OUT, "\n");
	}

	if ((pos&0x0F)) fprintf(DEBUG_MALLOC_OUT, "\n");
}
#endif

void *
compat_malloc(size_t size)
{
	if (size == 0) size++;

	return malloc(size);
}

void *
compat_calloc(size_t nelem, size_t size)
{
	if (size == 0) size++;

	return calloc(nelem, size);
}

void *
compat_realloc(void *ptr, size_t size)
{
	if (size == 0) size++;

	return realloc(ptr, size);
}
