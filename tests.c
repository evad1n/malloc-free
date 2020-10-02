#include <stdbool.h>
#include "malloc_free.h"
#include "main.h"
#include "tests.h"

#pragma region Test_Helpers

/* Adds emphasis */
void test_formatter(char *message)
{
    printf("\n===========================================================================================\n");
    printf("%s\n", message);
    printf("===========================================================================================\n");
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
    data_1 = (int *)my_malloc(280);
    data_2 = (int *)my_malloc(280);
    data_3 = (int *)my_malloc(280);
    data_4 = (int *)my_malloc(280);
    data_5 = (int *)my_malloc(280);
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
    data_1 = (int *)my_malloc(280);
    data_2 = (int *)my_malloc(280);
    data_3 = (int *)my_malloc(280);
    data_4 = (int *)my_malloc(280);
    data_5 = (int *)my_malloc(280);
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
    data_1 = (int *)my_malloc(280);
    data_2 = (int *)my_malloc(280);
    data_3 = (int *)my_malloc(280);
    data_4 = (int *)my_malloc(280);
    data_5 = (int *)my_malloc(280);
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
    printf("===========================================================================================\n");
}

void test_malloc_returns_null()
{
    printf("\n===========================================================================================\n");
    printf("TESTING MALLOC RETURNS NULL IF THERE IS NO CHUNK OF SUITABLE SIZE\n");
    printf("===========================================================================================\n");

    int *data_1;
    // int *data_2;
    // int *data_3;

    printf("REQUESTING 1 CHUNK THAT IS TOO BIG\n");
    data_1 = (int *)my_malloc(2 * HEAP_SIZE);
    assert(data_1 == NULL);
    printf("\nTEST PASSED\n");

    printf("\nALL MALLOC RETURNS NULL PASSED\n");
}

void test_all()
{
    test_formatter("RUNNING ALL TESTS");
    test_free_chunk_reuse();
    test_sorted_free_list();
    test_splitting_free_chunks();
    test_coalesce();
    test_alternating_sequence();
    test_worst_fit();
    test_malloc_returns_null();
}

#pragma endregion Tests