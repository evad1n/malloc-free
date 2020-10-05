#include <stdbool.h>
#include "malloc_free.h"
#include "main.h"
#include "tests.h"

#define CHUNK_SIZE (HEAP_SIZE / 20)
#define MAX_CHUNKS 10

#pragma region Test_Helpers

/* Adds emphasis */
void emphasis(char *message)
{
    char top[100] = {0};
    char bot[100] = {0};
    for (size_t i = 0; i < strlen(message); i++)
    {
        strcat(top, "=");
        strcat(bot, "=");
    }
    printf("\n%s\n%s\n%s\n\n", top, message, bot);
}

/* Just colors it green to indicate success */
void success(char *message)
{
    printf("\x1b[32m");
    emphasis(message);
    printf("\x1b[0m");
}

/* Just so if I want to edit this it is easier */
void passed()
{
    printf("\nTEST PASSED\n\n");
}

/* Frees any allocated chunks on the heap. */
void free_all_chunks()
{
    void *chunks_to_free[MAX_CHUNKS];
    int num_allocated_chunks = 0;

    void *address = heap_pointer;
    node *last_free = free_list_head;
    while (address < heap_pointer + HEAP_SIZE)
    {
        // If it is free
        // check if it is in free list
        if (address == last_free)
        {
            node *chunk = (node *)address;

            // next free chunk
            last_free = last_free->next;
            // next chunk
            address += (chunk->size + sizeof(node));
        }
        // else it must be allocated
        else
        {
            header *chunk = (header *)address;
            // check magic number is right
            assert(chunk->magic == MAGIC_NUMBER);

            // can't free while inside this loop, so store the address for later
            chunks_to_free[num_allocated_chunks] = chunk + 1;
            num_allocated_chunks++;

            // next chunk
            address += (chunk->size + sizeof(header));
        }
    }
    // Free them all
    for (size_t i = 0; i < num_allocated_chunks; i++)
    {
        my_free(chunks_to_free[i]);
    }
}

bool check_allocated_chunks()
{
    printf("Walking through allocated chunks...\n");

    void *address = heap_pointer;
    node *last_free = free_list_head;

    int num_allocated_chunks = 0;

    while (address < heap_pointer + HEAP_SIZE)
    {
        // If it is free
        // check if it is in free list
        if (address == last_free)
        {
            node *chunk = (node *)address;

            // next free chunk
            last_free = last_free->next;
            // next chunk
            address += (chunk->size + sizeof(node));
        }
        // else it must be allocated
        else
        {
            header *chunk = (header *)address;
            // check magic number is right
            assert(chunk->magic == MAGIC_NUMBER);

            num_allocated_chunks++;

            // print out allocated chunk info
            printf("Allocated chunk at %ld with size %ld and magic %d\n", (uint64_t)chunk - offset, chunk->size, chunk->magic);

            // next chunk
            address += (chunk->size + sizeof(header));
        }
    }
    printf("There %s %d allocated chunk%s\n", num_allocated_chunks == 1 ? "is" : "are", num_allocated_chunks, num_allocated_chunks == 1 ? "" : "s");

    return true;
}

/* Verifies that the each free list node's next pointer address is greater than the node's current address. */
bool verify_sorted()
{
    bool sorted = true;
    node *curr = free_list_head;
    while (curr)
    {
        if (curr->next && curr->next <= curr)
        {
            sorted = false;
        }

        curr = curr->next;
    }

    return sorted;
}

/* Verifies that no free list nodes next pointer is equal to the address of the node plus it's size, which would mean they are adjacent. */
bool verify_alternating()
{
    bool alternating = true;
    node *curr = free_list_head;
    while (curr)
    {
        if ((uint64_t)curr + curr->size + sizeof(node) == (uint64_t)curr->next)
        {
            alternating = false;
        }

        curr = curr->next;
    }

    return alternating;
}

#pragma endregion Test_Helpers

#pragma region Tests

void test_free_chunk_reuse()
{
    emphasis("TESTING FREE CHUNKS BEING REUSED AS MUCH AS POSSIBLE");

    void *chunks[MAX_CHUNKS];

    printf("ALLOCATING 2 CHUNKS\n");
    chunks[0] = my_malloc(HEAP_SIZE / 3);
    chunks[1] = my_malloc((HEAP_SIZE / 3) + CHUNK_SIZE);
    printf("FREEING THE FIRST CHUNK\n");
    my_free(chunks[0]);
    printf("ALLOCATING ANOTHER CHUNK\n");
    chunks[0] = my_malloc(CHUNK_SIZE);
    assert(heap_pointer == (header *)chunks[0] - 1);
    free_all_chunks();
    passed();

    success("ALL FREE CHUNK REUSE TESTS PASSED");
}

void test_sorted_free_list()
{
    emphasis("TESTING FREE LIST IS SORTED IN IN-MEMORY ORDER");

    void *chunks[MAX_CHUNKS];

    printf("ALLOCATING 5 CHUNKS\n");
    chunks[0] = my_malloc(CHUNK_SIZE);
    chunks[1] = my_malloc(CHUNK_SIZE);
    chunks[2] = my_malloc(CHUNK_SIZE);
    chunks[3] = my_malloc(CHUNK_SIZE);
    chunks[4] = my_malloc(CHUNK_SIZE);
    printf("FREEING EVERY OTHER CHUNK\n");
    my_free(chunks[0]);
    my_free(chunks[2]);
    my_free(chunks[4]);
    assert(verify_sorted());
    free_all_chunks();
    passed();

    printf("ALLOCATING 10 CHUNKS\n");
    chunks[0] = my_malloc(CHUNK_SIZE);
    chunks[1] = my_malloc(CHUNK_SIZE);
    chunks[2] = my_malloc(CHUNK_SIZE);
    chunks[3] = my_malloc(CHUNK_SIZE);
    chunks[4] = my_malloc(CHUNK_SIZE);
    chunks[5] = my_malloc(CHUNK_SIZE);
    chunks[6] = my_malloc(CHUNK_SIZE);
    chunks[7] = my_malloc(CHUNK_SIZE);
    chunks[8] = my_malloc(CHUNK_SIZE);
    chunks[9] = my_malloc(CHUNK_SIZE);
    printf("FREEING EVERY OTHER CHUNK IN A NON-SEQUENTIAL ORDER\n");
    my_free(chunks[4]);
    my_free(chunks[6]);
    my_free(chunks[2]);
    my_free(chunks[0]);
    my_free(chunks[8]);
    assert(verify_sorted());
    free_all_chunks();
    passed();

    success("ALL SORTED FREE LIST TESTS PASSED");
}

void test_splitting_free_chunks()
{
    emphasis("TESTING FREE CHUNKS ARE SPLIT PROPERLY WHEN ALLOCATING");

    void *chunks[MAX_CHUNKS];

    printf("ALLOCATING 1 CHUNK\n");
    node *prev_head_address = free_list_head;
    chunks[0] = my_malloc(CHUNK_SIZE);
    uint64_t expected = (uint64_t)prev_head_address + ((header *)chunks[0] - 1)->size + sizeof(header);
    printf("CHECKING ADDRESS OF FREE LIST HEAD\n");
    printf("Expected: %ld, Actual: %ld\n", expected - offset, (uint64_t)free_list_head - offset);
    assert((uint64_t)free_list_head == expected);
    free_all_chunks();
    passed();

    success("ALL SPLITTING FREE CHUNKS TESTS PASSED");
}

void test_coalesce()
{
    emphasis("TESTING FREE CHUNKS BEING COALESCED PROPERLY");

    void *chunks[MAX_CHUNKS];

    printf("ALLOCATING 5 CHUNKS\n");
    chunks[0] = my_malloc(CHUNK_SIZE);
    chunks[1] = my_malloc(CHUNK_SIZE);
    chunks[2] = my_malloc(CHUNK_SIZE);
    chunks[3] = my_malloc(CHUNK_SIZE);
    chunks[4] = my_malloc(CHUNK_SIZE);
    printf("FREEING ALL CHUNKS...\n");
    free_all_chunks();
    printf("MAKING SURE THERE IS ONLY 1 CHUNK\n");
    assert(free_list_head->next == NULL);
    passed();

    printf("ALLOCATING 5 CHUNKS\n");
    chunks[0] = my_malloc(CHUNK_SIZE);
    chunks[1] = my_malloc(CHUNK_SIZE);
    chunks[2] = my_malloc(CHUNK_SIZE);
    chunks[3] = my_malloc(CHUNK_SIZE);
    chunks[4] = my_malloc(CHUNK_SIZE);
    printf("FREEING SOME CHUNKS...\n");
    my_free(chunks[0]);
    my_free(chunks[1]);
    my_free(chunks[3]);
    my_free(chunks[4]);
    printf("MAKING SURE THERE ARE ONLY 2 CHUNKS\n");
    assert(free_list_head->next->next == NULL);
    free_all_chunks();
    passed();

    success("ALL COALESCING TESTS PASSED");
}

void test_alternating_sequence()
{
    emphasis("TESTING HEAP IS IN ALTERNATING SEQUENCE OF 1 FREE CHUNK AND 1 OR MORE ALLOCATED CHUNKS");

    void *chunks[MAX_CHUNKS];

    printf("ALLOCATING 2 CHUNKS\n");
    chunks[0] = my_malloc(CHUNK_SIZE);
    chunks[1] = my_malloc(CHUNK_SIZE);
    printf("FREEING SECOND CHUNK\n");
    my_free(chunks[1]);
    assert(verify_alternating());
    free_all_chunks();
    passed();

    printf("ALLOCATING 7 CHUNKS\n");
    chunks[0] = my_malloc(CHUNK_SIZE);
    chunks[1] = my_malloc(CHUNK_SIZE);
    chunks[2] = my_malloc(CHUNK_SIZE);
    chunks[3] = my_malloc(CHUNK_SIZE);
    chunks[4] = my_malloc(CHUNK_SIZE);
    chunks[5] = my_malloc(CHUNK_SIZE);
    chunks[6] = my_malloc(CHUNK_SIZE);
    printf("FREEING EVERY OTHER CHUNK\n");
    my_free(chunks[0]);
    my_free(chunks[2]);
    my_free(chunks[4]);
    my_free(chunks[6]);
    assert(verify_alternating());
    free_all_chunks();
    passed();

    printf("ALLOCATING 3 CHUNKS\n");
    chunks[0] = my_malloc(CHUNK_SIZE);
    chunks[1] = my_malloc(HEAP_SIZE / 2);
    chunks[2] = my_malloc(CHUNK_SIZE);
    printf("FREEING MIDDLE CHUNK\n");
    my_free(chunks[1]);
    printf("ALLOCATING 4 MORE CHUNKS\n");
    chunks[1] = my_malloc(CHUNK_SIZE);
    chunks[3] = my_malloc(CHUNK_SIZE);
    chunks[4] = my_malloc(CHUNK_SIZE);
    chunks[5] = my_malloc(CHUNK_SIZE);
    assert(verify_alternating());
    free_all_chunks();
    passed();

    success("ALL ALTERNATING SEQUENCE TESTS PASSED");
}

void test_worst_fit()
{
    emphasis("TESTING WORST FIRST ALLOCATION");

    void *chunks[MAX_CHUNKS];

    printf("ALLOCATING 2 CHUNKS\n");
    chunks[0] = my_malloc(CHUNK_SIZE);
    chunks[1] = my_malloc(CHUNK_SIZE);
    printf("FREEING FIRST CHUNK\n");
    my_free(chunks[0]);
    printf("ALLOCATING CHUNK OF SMALLER SIZE\n");
    chunks[0] = my_malloc(CHUNK_SIZE / 2);
    assert(chunks[0] > chunks[1]);
    free_all_chunks();
    passed();

    printf("ALLOCATING 3 CHUNKS. MIDDLE CHUNK IS HALF THE HEAP SIZE\n");
    chunks[0] = my_malloc(CHUNK_SIZE);
    chunks[1] = my_malloc(HEAP_SIZE / 2);
    chunks[2] = my_malloc(CHUNK_SIZE);
    printf("FREEING MIDDLE CHUNK\n");
    my_free(chunks[1]);
    printf("ALLOCATING 4 CHUNKS\n");
    chunks[1] = my_malloc(CHUNK_SIZE);
    assert(chunks[2] > chunks[0]);
    chunks[3] = my_malloc(CHUNK_SIZE);
    assert(chunks[2] > chunks[3]);
    chunks[4] = my_malloc(CHUNK_SIZE);
    assert(chunks[2] > chunks[4]);
    chunks[5] = my_malloc(CHUNK_SIZE);
    assert(chunks[2] < chunks[5]);
    free_all_chunks();
    passed();

    success("ALL WORST FIT ALLOCATION TESTS PASSED");
}

void test_malloc_bad_size()
{
    emphasis("TESTING MALLOC RETURNS NULL ON BAD VALUE SIZE REQUESTS");

    void *chunks[MAX_CHUNKS];

    printf("REQUESTING 1 CHUNK THAT IS TWICE THE SIZE OF HEAP\n");
    chunks[0] = my_malloc(2 * HEAP_SIZE);
    assert(chunks[0] == NULL);
    passed();

    printf("ALLOCATING 2 CHUNKS WITH SUM SLIGHTLY SMALLER THAN HEAP SIZE\n");
    chunks[0] = my_malloc((HEAP_SIZE / 2) - (CHUNK_SIZE / 2));
    chunks[1] = my_malloc((HEAP_SIZE / 2) - (CHUNK_SIZE / 2));
    printf("REQUESTING ADDITIONAL CHUNK WITH SIZE GREATER THAN REMAINING HEAP SIZE\n");
    chunks[2] = my_malloc(CHUNK_SIZE);
    assert(chunks[2] == NULL);
    free_all_chunks();
    passed();

    printf("REQUESTING 1 CHUNK OF SIZE -1\n");
    chunks[0] = my_malloc(-1);
    assert(chunks[0] == NULL);
    passed();

    printf("REQUESTING LARGE NEGATIVE SIZE CHUNK\n");
    chunks[0] = my_malloc(-HEAP_SIZE / 2);
    assert(chunks[0] == NULL);
    passed();

    printf("REQUESTING SIZE 0 CHUNK\n");
    chunks[0] = my_malloc(0);
    assert(chunks[0] == NULL);
    passed();

    success("ALL MALLOC BAD SIZE TESTS PASSED");
}

void test_all()
{
    emphasis("RUNNING ALL TESTS");
    test_free_chunk_reuse();
    test_sorted_free_list();
    test_splitting_free_chunks();
    test_coalesce();
    test_alternating_sequence();
    test_worst_fit();
    test_malloc_bad_size();
    success("ALL TESTS PASSED");
}

#pragma endregion Tests