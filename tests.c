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

void free_chunks(void *chunks[], int num_to_free)
{
    int index = 0;
    int num_freed = 0;
    while (num_freed < num_to_free)
    {
        header *hptr = (header *)chunks[index] - 1;
        if (hptr->magic == MAGIC_NUMBER)
        {
            my_free(chunks[index]);
            num_freed++;
        }
        index++;
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
        if (curr->next <= curr)
        {
            sorted = false;
        }

        curr = curr->next;
    }

    return sorted;
}

#pragma endregion Test_Helpers

#pragma region Tests

void test_free_chunk_reuse()
{
    emphasis("TESTING FREE CHUNKS BEING REUSED AS MUCH AS POSSIBLE");

    void *chunks[MAX_CHUNKS];

    chunks[0] = my_malloc(CHUNK_SIZE);

    free_chunks(chunks, 1);

    success("ALL FREE CHUNK RESUSE TESTS PASSED");
}

void test_sorted_free_list()
{
    emphasis("TESTING FREE LIST IS SORTED IN IN-MEMORY ORDER");

    void *chunks[MAX_CHUNKS];

    printf("ALLOCATING CHUNKS\n");
    chunks[1] = my_malloc(CHUNK_SIZE);
    chunks[2] = my_malloc(CHUNK_SIZE);
    chunks[3] = my_malloc(CHUNK_SIZE);
    chunks[4] = my_malloc(CHUNK_SIZE);
    chunks[5] = my_malloc(CHUNK_SIZE);
    // audit();
    printf("FREEING SOME CHUNKS IN ORDER TO SCREW IT UP\n");
    my_free(chunks[1]);
    audit();
    my_free(chunks[3]);
    audit();
    my_free(chunks[5]);
    printf("SHOULD STILL BE IN ORDER\n");
    audit();

    assert(verify_sorted());

    // Free everything
    my_free(chunks[2]);
    my_free(chunks[4]);

    success("ALL SORTED FREE LIST TESTS PASSED");
}

void test_splitting_free_chunks()
{
    emphasis("TESTING FREE CHUNKS ARE SPLIT PROPERLY WHEN ALLOCATING");

    void *chunks[MAX_CHUNKS];

    chunks[0] = my_malloc(CHUNK_SIZE);

    free_chunks(chunks, 1);

    success("ALL SPLITTING FREE CHUNKS TESTS PASSED");
}

void test_coalesce()
{
    emphasis("TESTING FREE CHUNKS BEING COALESCED PROPERLY");

    void *chunks[MAX_CHUNKS];

    printf("ALLOCATING 5 CHUNKS\n");
    chunks[1] = my_malloc(CHUNK_SIZE);
    chunks[2] = my_malloc(CHUNK_SIZE);
    chunks[3] = my_malloc(CHUNK_SIZE);
    chunks[4] = my_malloc(CHUNK_SIZE);
    chunks[5] = my_malloc(CHUNK_SIZE);
    audit();
    printf("FREEING ALL CHUNKS...\n");
    my_free(chunks[1]);
    my_free(chunks[2]);
    my_free(chunks[3]);
    my_free(chunks[4]);
    my_free(chunks[5]);
    printf("Making sure there is only 1 chunk\n");
    assert(free_list_head->next == NULL);
    audit();
    walk_free_list();
    printf("\nTEST PASSED\n\n");

    printf("ALLOCATING 5 CHUNKS\n");
    chunks[1] = my_malloc(CHUNK_SIZE);
    chunks[2] = my_malloc(CHUNK_SIZE);
    chunks[3] = my_malloc(CHUNK_SIZE);
    chunks[4] = my_malloc(CHUNK_SIZE);
    chunks[5] = my_malloc(CHUNK_SIZE);
    audit();
    printf("FREEING SOME CHUNKS...\n");
    my_free(chunks[1]);
    my_free(chunks[2]);
    my_free(chunks[4]);
    my_free(chunks[5]);
    printf("Making sure there are only 2 chunks\n");
    assert(free_list_head->next->next == NULL);
    audit();
    walk_free_list();
    printf("\nTEST PASSED\n\n");

    success("ALL COALESCING TESTS PASSED");
}

void test_alternating_sequence()
{
    emphasis("TESTING HEAP IS IN ALTERNATING SEQUENCE OF 1 FREE CHUNK AND 1 OR MORE ALLOCATED CHUNKS");

    void *chunks[MAX_CHUNKS];

    chunks[0] = my_malloc(CHUNK_SIZE);
    chunks[1] = my_malloc(CHUNK_SIZE);

    audit();
    free_chunks(chunks, 2);
    audit();

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
    free_chunks(chunks, 2);
    printf("\nTEST PASSED\n\n");

    audit();

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
    free_chunks(chunks, 6);
    printf("\nTEST PASSED\n\n");

    success("ALL WORST FIT ALLOCATION TESTS PASSED");
}

void test_malloc_bad_size()
{
    emphasis("TESTING MALLOC RETURNS NULL ON BAD VALUE SIZE REQUESTS");

    void *chunks[MAX_CHUNKS];

    printf("REQUESTING 1 CHUNK THAT IS TWICE THE SIZE OF HEAP\n");
    chunks[0] = my_malloc(2 * HEAP_SIZE);
    assert(chunks[0] == NULL);
    printf("\nTEST PASSED\n\n");

    printf("ALLOCATING 2 CHUNKS WITH SUM SLIGHTLY SMALLER THAN HEAP SIZE\n");
    chunks[0] = my_malloc((HEAP_SIZE / 2) - (CHUNK_SIZE / 2));
    chunks[1] = my_malloc((HEAP_SIZE / 2) - (CHUNK_SIZE / 2));
    printf("REQUESTING ADDITIONAL CHUNK WITH SIZE GREATER THAN REMAINING HEAP SIZE\n");
    chunks[2] = my_malloc(CHUNK_SIZE);
    assert(chunks[2] == NULL);
    free_chunks(chunks, 2);
    printf("\nTEST PASSED\n\n");

    printf("REQUESTING 1 CHUNK OF SIZE -1\n");
    chunks[0] = my_malloc(-1);
    assert(chunks[0] == NULL);
    printf("\nTEST PASSED\n\n");

    printf("REQUESTING LARGE NEGATIVE SIZE CHUNK\n");
    chunks[0] = my_malloc(-HEAP_SIZE / 2);
    assert(chunks[0] == NULL);
    printf("\nTEST PASSED\n\n");

    printf("REQUESTING SIZE 0 CHUNK\n");
    chunks[0] = my_malloc(0);
    assert(chunks[0] == NULL);
    printf("\nTEST PASSED\n\n");

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