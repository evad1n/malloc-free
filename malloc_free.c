#include "malloc_free.h"

// Size of heap
const size_t HEAP_SIZE = 4096;
// Magic number to verify integrity of allocated chunk
const int MAGIC_NUMBER = 123456789;
// Align to 64-bit word which is 8 bytes
const size_t ALIGN_TO = 8;

/* Given a requested size, returns the total aligned size needed. */
size_t align(size_t raw)
{
    size_t aligned = ALIGN_TO * ((raw - 1 + ALIGN_TO + sizeof(header)) / ALIGN_TO);
    return aligned;
}

/* Coalesces all free chunks that are next to each other in memory. */
void coalesce()
{
    // Walk through free list and while there are 2 next to each other keep merging
    node *curr = free_list_head;
    while (curr)
    {
        // Check to see if they are next to each other
        if ((uint64_t)curr + sizeof(node) + curr->size == (uint64_t)curr->next)
        {
            // printf("Merging chunk at %ld with size %ld and next %ld\n", (uint64_t)curr - offset, curr->size, curr->next ? (uint64_t)curr->next - offset : 0);
            // printf("with next at %ld with size %ld and next %ld\n", (uint64_t)curr->next - offset, curr->next->size, curr->next->next ? (uint64_t)curr->next->next - offset : 0);

            node *temp = curr->next;

            curr->next = temp->next;
            curr->size = curr->size + temp->size + sizeof(node);

            // printf("Final chunk at %ld with size %ld and next %ld\n", (uint64_t)curr - offset, curr->size, curr->next ? (uint64_t)curr->next - offset : 0);
        }
        // Skip to next node if we didn't merge anything
        else
        {
            curr = curr->next;
        }
    }
}

/* Returns pointer to memory. Returns NULL if there is not enough space. */
void *my_malloc(size_t size)
{
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
    // printf("requested size: %ld\n", size);
    // printf("allocating size: %ld\n", needed_size - sizeof(header));
    // printf("total needed size: %lu\n", needed_size);

    // printf("Free list start: %ld\n", (uint64_t)free_list_head - offset);

    // WORST FIT
    // Search for biggest chunk for worst fit
    node *curr = free_list_head;
    node *biggest_chunk_prev = curr;
    node *biggest_chunk = curr;
    while (curr)
    {
        // printf("Checking chunk at %ld with size %ld\n", (uint64_t)curr - offset, curr->size);
        if (curr->next && curr->next->size > biggest_chunk->size)
        {
            biggest_chunk_prev = curr;
            biggest_chunk = curr->next;
        }
        curr = curr->next;
    }

    // If there is no chunk big enough return NULL
    if (needed_size > biggest_chunk->size)
    {
        printf("NO CHUNK BIG ENOUGH\n");
        return NULL;
    }

    size_t prev_size = biggest_chunk->size;
    node *prev_next = biggest_chunk->next;

    // printf("Biggest size chunk found at %ld with size %ld\n", (uint64_t)biggest_chunk - offset, biggest_chunk->size);

    // printf("free list chunk should still be at %ld\n", (uint64_t)biggest_chunk - offset);

    // Create header
    header *allocated_header = (header *)biggest_chunk;
    allocated_header->size = needed_size - sizeof(header);
    allocated_header->magic = MAGIC_NUMBER;

    // Cut big chunk down to size
    header *allocated_address = (header *)biggest_chunk + 1;

    // printf("Allocated header at %ld, with size %ld and magic %d\n", (uint64_t)allocated_header - offset, allocated_header->size, allocated_header->magic);

    // printf("Allocated memory will be returned at %ld\nShould be %ld above allocated header address\n", (uint64_t)allocated_address - offset, sizeof(header));

    // Split free chunk
    // If the chunk to be split is the head
    if (biggest_chunk == free_list_head)
    {
        free_list_head = (node *)((char *)biggest_chunk + needed_size);
        free_list_head->size = prev_size - needed_size;
        free_list_head->next = prev_next;
    }
    else
    {
        node *split_free_chunk = (node *)((char *)biggest_chunk + needed_size);
        split_free_chunk->size = prev_size - needed_size;
        biggest_chunk_prev->next = split_free_chunk;
    }

    // printf("Free list chunk now at %ld, with size %ld and next %ld\n", (uint64_t)free_list_head - offset, free_list_head->size, (uint64_t)free_list_head->next);

    return (void *)allocated_address;
}

/* Frees the allocated chunk starting at the pointer passed in. Orders and coalesces the free list afterwards. */
void my_free(void *ptr)
{
    header *hptr = (header *)ptr - 1;
    assert(hptr->magic == MAGIC_NUMBER);
    // printf("Attempting to free at address %ld\n", (uint64_t)ptr - offset);

    node *new_free_chunk = (node *)hptr;
    // printf("Creating free chunk at address %ld\n", (uint64_t)new_free_chunk - offset);

    // Insert into free list at the right place so need for reordering
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
        new_free_chunk->size = hptr->size - sizeof(header) + sizeof(node);
        free_list_head = new_free_chunk;
    }
    else
    {
        prev->next = new_free_chunk;
        new_free_chunk->next = curr;
        new_free_chunk->size = hptr->size - sizeof(header) + sizeof(node);
    }

    // Try to coalesce
    coalesce();

    // printf("Free list head now at address %ld\n", (uint64_t)free_list_head - offset);
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