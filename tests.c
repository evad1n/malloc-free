#include <stdbool.h>
#include "malloc_free.h"
#include "main.h"
#include "tests.h"

#define CHUNK_SIZE (HEAP_SIZE / 20)

#pragma region Test_Helpers

/* Adds emphasis */
void test_formatter(char *message)
{
    printf("\n===========================================================================================\n");
    printf("%s\n", message);
    printf("===========================================================================================\n");
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
    test_formatter("TESTING FREE CHUNKS BEING REUSED AS MUCH AS POSSIBLE");
}

void test_splitting_free_chunks()
{
    test_formatter("TESTING FREE CHUNKS ARE SPLIT PROPERLY WHEN ALLOCATING");
}

void test_coalesce()
{
    printf("TESTING FREE CHUNKS BEING COALESCED PROPERLY\n");
    printf("====================================================\n");

    int *data_1;
    int *data_2;
    int *data_3;
    int *data_4;
    int *data_5;

    printf("ALLOCATING 5 CHUNKS\n");
    data_1 = (int *)my_malloc(CHUNK_SIZE);
    data_2 = (int *)my_malloc(CHUNK_SIZE);
    data_3 = (int *)my_malloc(CHUNK_SIZE);
    data_4 = (int *)my_malloc(CHUNK_SIZE);
    data_5 = (int *)my_malloc(CHUNK_SIZE);
    audit();
    printf("FREEING ALL CHUNKS...\n");
    my_free(data_1);
    my_free(data_2);
    my_free(data_3);
    my_free(data_4);
    my_free(data_5);
    printf("Making sure there is only 1 chunk\n");
    assert(free_list_head->next == NULL);
    audit();
    walk_free_list();
    printf("\nTEST PASSED\n");

    printf("ALLOCATING 5 CHUNKS\n");
    data_1 = (int *)my_malloc(CHUNK_SIZE);
    data_2 = (int *)my_malloc(CHUNK_SIZE);
    data_3 = (int *)my_malloc(CHUNK_SIZE);
    data_4 = (int *)my_malloc(CHUNK_SIZE);
    data_5 = (int *)my_malloc(CHUNK_SIZE);
    audit();
    printf("FREEING SOME CHUNKS...\n");
    my_free(data_1);
    my_free(data_2);
    my_free(data_4);
    my_free(data_5);
    printf("Making sure there are only 2 chunks\n");
    assert(free_list_head->next->next == NULL);
    audit();
    walk_free_list();
    printf("\nTEST PASSED\n");

    printf("\nALL COALESCING TESTS PASSED\n");
}

void test_sorted_free_list()
{
    printf("\n===========================================================================================\n");
    printf("\nTESTING FREE LIST IS SORTED IN IN-MEMORY ORDER\n");
    printf("===========================================================================================\n");

    int *data_1;
    int *data_2;
    int *data_3;
    int *data_4;
    int *data_5;

    printf("ALLOCATING CHUNKS\n");
    data_1 = (int *)my_malloc(CHUNK_SIZE);
    data_2 = (int *)my_malloc(CHUNK_SIZE);
    data_3 = (int *)my_malloc(CHUNK_SIZE);
    data_4 = (int *)my_malloc(CHUNK_SIZE);
    data_5 = (int *)my_malloc(CHUNK_SIZE);
    // audit();
    printf("FREEING SOME CHUNKS IN ORDER TO SCREW IT UP\n");
    my_free(data_1);
    audit();
    my_free(data_3);
    audit();
    my_free(data_5);
    printf("SHOULD STILL BE IN ORDER\n");
    audit();

    assert(verify_sorted());

    // Free everything
    my_free(data_2);
    my_free(data_4);
}

void test_alternating_sequence()
{
    printf("\n===========================================================================================\n");
    printf("TESTING HEAP IS IN ALTERNATING SEQUENCE OF 1 FREE CHUNK AND 1 OR MORE ALLOCATED CHUNKS\n");
    printf("===========================================================================================\n");
}

void test_worst_fit()
{
    printf("\n===========================================================================================\n");
    printf("TESTING WORST FIRST ALLOCATION\n");
    printf("===========================================================================================\n\n");

    void *ptrs[10];

    printf("ALLOCATING 2 CHUNKS\n");
    ptrs[0] = my_malloc(CHUNK_SIZE);
    ptrs[1] = my_malloc(CHUNK_SIZE);
    printf("FREEING FIRST CHUNK\n");
    my_free(ptrs[0]);
    printf("ALLOCATING CHUNK OF SMALLER SIZE\n");
    ptrs[0] = my_malloc(CHUNK_SIZE / 2);
    assert(ptrs[0] > ptrs[1]);
    my_free(ptrs[0]);
    my_free(ptrs[1]);
    printf("\nTEST PASSED\n");

    printf("ALLOCATING 2 CHUNKS\n");
    ptrs[0] = my_malloc(CHUNK_SIZE);
    ptrs[1] = my_malloc(CHUNK_SIZE);
    printf("FREEING FIRST CHUNK\n");
    my_free(ptrs[0]);
    printf("ALLOCATING CHUNK OF SMALLER SIZE\n");
    ptrs[0] = my_malloc(CHUNK_SIZE / 2);
    assert(ptrs[0] > ptrs[1]);
    my_free(ptrs[1]);
    printf("\nTEST PASSED\n");

    printf("\nALL WORST FIT ALLOCATION TESTS PASSED\n");
}

void test_malloc_bad_size()
{
    printf("\n===========================================================================================\n");
    printf("TESTING MALLOC RETURNS NULL ON BAD VALUE SIZE REQUESTS\n");
    printf("===========================================================================================\n\n");

    void *ptrs[10];

    printf("REQUESTING 1 CHUNK THAT IS TWICE THE SIZE OF HEAP\n");
    ptrs[0] = my_malloc(2 * HEAP_SIZE);
    assert(ptrs[0] == NULL);
    printf("\nTEST PASSED\n");

    printf("ALLOCATING 2 CHUNKS WITH SUM SLIGHTLY SMALLER THAN HEAP SIZE\n");
    ptrs[0] = my_malloc((HEAP_SIZE / 2) - (CHUNK_SIZE / 2));
    ptrs[1] = my_malloc((HEAP_SIZE / 2) - (CHUNK_SIZE / 2));
    printf("REQUESTING ADDITIONAL CHUNK WITH SIZE GREATER THAN REMAINING HEAP SIZE\n");
    ptrs[2] = my_malloc(CHUNK_SIZE);
    assert(ptrs[2] == NULL);
    my_free(ptrs[0]);
    my_free(ptrs[1]);
    printf("\nTEST PASSED\n");

    printf("REQUESTING 1 CHUNK OF SIZE -1\n");
    ptrs[0] = my_malloc(-1);
    assert(ptrs[0] == NULL);
    printf("\nTEST PASSED\n");

    printf("REQUESTING LARGE NEGATIVE SIZE CHUNK\n");
    ptrs[0] = my_malloc(-HEAP_SIZE / 2);
    assert(ptrs[0] == NULL);
    printf("\nTEST PASSED\n");

    printf("REQUESTING SIZE 0 CHUNK\n");
    ptrs[0] = my_malloc(0);
    assert(ptrs[0] == NULL);
    printf("\nTEST PASSED\n");

    printf("\nALL MALLOC RETURNS NULL ON BAD SIZE REQUEST TESTS PASSED\n");
}

void test_all()
{
    test_formatter("RUNNING ALL TESTS\n\n");
    test_free_chunk_reuse();
    test_sorted_free_list();
    test_splitting_free_chunks();
    test_coalesce();
    test_alternating_sequence();
    test_worst_fit();
    test_malloc_bad_size();
}

#pragma endregion Tests