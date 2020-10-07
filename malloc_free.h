#ifndef _MALLOC_FREE_H_
#define _MALLOC_FREE_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <inttypes.h>
#include <assert.h>

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

// Pointer to start of heap
void *heap_pointer;
// Pointer to first node in free list
node *free_list_head;
// Offset for displaying understandable values
uint64_t offset;

size_t align();
void coalesce();
void *my_malloc(size_t size);
void my_free(void *ptr);
void init_heap();

#endif