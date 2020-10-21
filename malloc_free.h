#if !defined(MALLOC_FREE_H)
#define MALLOC_FREE_H

#include <inttypes.h>

// Represents an allocated chunk header
typedef struct header_t
{
    size_t size;
    int magic;
} header;

// Represents a free chunk
typedef struct node_t
{
    size_t size;
    struct node_t *next;
} node;

extern const size_t HEAP_SIZE;
extern const int MAGIC_NUMBER;
extern const size_t ALIGN_TO;
extern void *heap_pointer;
extern node *free_list_head;
extern uint64_t offset;

size_t align(size_t raw);
void coalesce();
void *my_malloc(size_t size);
void my_free(void *ptr);
void init_heap();

#endif // MALLOC_FREE_H