#ifndef MEMORY_INCLUDED
#define MEMORY_INCLUDED

extern void *compat_malloc(size_t size);
extern void *compat_calloc(size_t nelem, size_t size);
extern void *compat_realloc(void *ptr, size_t size);

#ifndef NDEBUG

#include <stdlib.h>
#include <assert.h>

#define mem_init		memory_init()
#define mem_done		memory_done()
#define mem_list_allocated	memory_list_allocated()
#define mem_malloc(x)		memory_malloc((x), __FILE__, __LINE__)
#define mem_calloc(x, y)						  \
    memory_calloc((x), (y), __FILE__, \
	__LINE__)
#define mem_realloc(p, x)						   \
    memory_realloc((p), (x), __FILE__, \
	__LINE__)
#define mem_free(p)			memory_free((p), __FILE__, __LINE__)
#define mem_hexdump(p)			memory_hexdump((p), __FILE__, __LINE__)
#define mem_hexdump_alien(p, x)		memory_hexdump_alien((p), (x))

extern void memory_init(void);
extern void memory_done(void);
extern void *memory_malloc(size_t size, const char *srcname, int line);
extern void *memory_calloc(size_t nelem, size_t size, const char *srcname,
    int line);
extern void *memory_realloc(void *ptr, size_t size, const char *srcname,
    int line);
extern void memory_free(void *ptr, const char *srcname, int line);
extern void memory_list_allocated(void);
extern void memory_hexdump(void *ptr, const char *srcname, int srcline);
extern void memory_hexdump_alien(void *ptr, size_t len);

#else

#define mem_init
#define mem_done
#define mem_dump_allocated
#define mem_malloc(x)		compat_malloc((x))
#define mem_calloc(x, y)	compat_calloc((x), (y))
#define mem_realloc(p, x)	compat_realloc((p), (x))
#define mem_free(p)		free((p))
#define mem_hexdump(p)
#define mem_hexdump_alien(p, x)
#endif
#endif
