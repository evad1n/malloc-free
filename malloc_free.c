#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <inttypes.h>
#include <assert.h>

#include "malloc_free.h"

// Size of heap
const size_t HEAP_SIZE = 4096;
// Magic number to verify integrity of allocated chunk
const int MAGIC_NUMBER = 123456789;
// Align to 64-bit word which is 8 bytes
const size_t ALIGN_TO = 8;
// Pointer to start of heap
void *heap_pointer;
// Pointer to first node in free list
node *free_list_head;
// Offset for displaying understandable values
uint64_t offset;

/* Given a requested size, returns the total aligned size needed. */
size_t align(size_t raw)
{
    size_t aligned = ALIGN_TO * ((raw - 1 + ALIGN_TO + sizeof(header)) / ALIGN_TO);
    return aligned;
}

/* Check for available coalescing around the recently freed chunk */
void coalesce(node *prev, node *freed)
{
    // Check chunk after freed
    if (freed->next && (uint64_t)freed + sizeof(node) + freed->size == (uint64_t)freed->next)
    {
        node *temp = freed->next;
        freed->next = temp->next;
        freed->size = freed->size + temp->size + sizeof(node);
    }

    // Check previous chunk
    if (prev && (uint64_t)prev + sizeof(node) + prev->size == (uint64_t)prev->next)
    {
        node *temp = prev->next;
        prev->next = temp->next;
        prev->size = prev->size + temp->size + sizeof(node);
    }
}

/* Returns pointer to memory. Returns NULL if there is not enough space. */
void *my_malloc(size_t size)
{
    // If there are no free chunks
    if (!free_list_head)
    {
        printf("There are no free chunks!\n");
        return NULL;
    }

    // If they enter a negative number the size will overflow to the max integer so this will fire
    if (size > HEAP_SIZE)
    {
        printf("REQUESTED SIZE EXCEEDS HEAP SIZE\n");
        printf("Did you try to allocate a negative size?\n");
        return NULL;
    }
    // Not sure if this is supposed to happen but it makes sense to deny a request of size 0
    else if (size == 0)
    {
        printf("REFUSING TO ALLOCATE SIZE 0\n");
        return NULL;
    }

    size_t needed_size = align(size);

    // WORST FIT
    // Search for biggest chunk for worst fit
    node *curr = free_list_head;
    node *biggest_chunk_prev = curr;
    node *biggest_chunk = curr;
    while (curr)
    {
        if (curr->next && curr->next->size > biggest_chunk->size)
        {
            biggest_chunk_prev = curr;
            biggest_chunk = curr->next;
        }
        curr = curr->next;
    }

    // If there is no chunk big enough return NULL
    if (needed_size > biggest_chunk->size + sizeof(node))
    {
        printf("NO CHUNK BIG ENOUGH\n");
        return NULL;
    }

    size_t prev_size = biggest_chunk->size;
    node *prev_next = biggest_chunk->next;

    // Split free chunk
    // If the chunk to be split is the head
    if (biggest_chunk == free_list_head)
    {
        // If the needed size requires the overhead space too
        if (needed_size > biggest_chunk->size)
        {
            free_list_head = biggest_chunk->next;
        }
        else
        {
            free_list_head = (node *)((void *)biggest_chunk + needed_size);
            free_list_head->size = prev_size - needed_size;
            free_list_head->next = prev_next;
        }
    }
    else
    {
        // If the needed size requires the overhead space too
        if (needed_size > biggest_chunk->size)
        {
            biggest_chunk_prev->next = biggest_chunk->next;
        }
        else
        {
            node *split_free_chunk = (node *)((void *)biggest_chunk + needed_size);
            split_free_chunk->size = prev_size - needed_size;
            biggest_chunk_prev->next = split_free_chunk;
        }
    }

    // Create header
    header *allocated_header = (header *)biggest_chunk;
    allocated_header->size = needed_size - sizeof(header);
    allocated_header->magic = MAGIC_NUMBER;

    // Cut big chunk down to size
    header *allocated_address = (header *)biggest_chunk + 1;

    return (void *)allocated_address;
}

/* Frees the allocated chunk starting at the pointer passed in. Orders and coalesces the free list afterwards. */
void my_free(void *ptr)
{
    header *hptr = (header *)ptr - 1;
    assert(hptr->magic == MAGIC_NUMBER);
    node *new_free_chunk = (node *)hptr;
    size_t new_size = hptr->size - sizeof(header) + sizeof(node);

    // Edge case where there is no free list
    if (!free_list_head)
    {
        free_list_head = new_free_chunk;
        free_list_head->next = NULL;
        free_list_head->size = new_size;
        return;
    }

    // Insert into free list at the right place so no need for reordering
    node *prev = free_list_head;
    node *curr = free_list_head;

    // Loop through list to find correct placement
    while (curr < new_free_chunk)
    {
        prev = curr;
        curr = curr->next;
    }

    // If it is at the start
    if (curr == free_list_head)
    {
        // Add new free chunk to head of free list
        new_free_chunk->next = free_list_head;
        new_free_chunk->size = new_size;
        free_list_head = new_free_chunk;
    }
    else
    {
        prev->next = new_free_chunk;
        new_free_chunk->next = curr;
        new_free_chunk->size = new_size;
    }

    // Try to coalesce around freed chunk
    coalesce(prev, new_free_chunk);
}

/* Initializes the heap and all global variables. */
void init_heap()
{
    // mmap() returns a pointer to a chunk of free space
    // Set heap pointer to start of heap
    heap_pointer = mmap(NULL, HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

    // Set offset for displaying
    offset = (uint64_t)heap_pointer;

    // Initialize free list
    // Set free_list_head to point to start of heap
    free_list_head = (node *)heap_pointer;
    free_list_head->size = HEAP_SIZE - sizeof(node);
    free_list_head->next = NULL;

    printf("\nHeap initialized with size %ld\n", HEAP_SIZE);
}