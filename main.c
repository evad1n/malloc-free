#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "malloc_free.h"
#include "tests.h"

#pragma region Helpers

/* Take an integer and print it in constant size format [* prefix:    number *] */
void print_formatted(char *prefix, uint64_t number)
{
    size_t format_size = strlen("***********************");
    char num_string[20];
    sprintf(num_string, "%ld", number);
    int spaces = (format_size - 4) - strlen(num_string) - strlen(prefix);

    char formatted[50] = {0};

    formatted[0] = '*';
    formatted[1] = ' ';
    strcat(formatted, prefix);
    for (size_t i = 0; i < spaces; i++)
    {
        strcat(formatted, " ");
    }
    strcat(formatted, num_string);
    strcat(formatted, " *");

    printf("%s\n", formatted);
}

/* Walks through free list and prints out info */
void walk_free_list()
{
    printf("Walking through free list...\n");

    node *curr = free_list_head;
    int num_free_chunks = 0;

    while (curr)
    {
        num_free_chunks++;
        printf("Free chunk at %ld with size %ld and next %ld\n", (uint64_t)curr - offset, (uint64_t)curr->size, curr->next ? (uint64_t)curr->next - offset : 0);
        curr = curr->next;
    }
    printf("There %s %d free chunk%s\n", num_free_chunks == 1 ? "is" : "are", num_free_chunks, num_free_chunks == 1 ? "" : "s");
}

/* Walks through allocated chunks and prints out info */
void walk_allocated_chunks()
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
}

/* Walk through heap and print everything in an ascii diagram. Verifies all memory is accounted for.*/
void audit()
{
    printf("\n================\n");
    printf("==  AUDITING  ==\n");
    printf("================\n");

    printf("Heap start: %ld\n", (uint64_t)heap_pointer - offset);
    printf("Heap size: %ld\n", HEAP_SIZE);
    printf("Free list start: %ld\n\n", (uint64_t)free_list_head - offset);

    // WALK BY CHUNK
    // SIMULTANEOUSLY WALK FREE LIST WHEN FINDING A FREE CHUNK
    // Check for in memory order, coalescing

    void *address = heap_pointer;
    node *last_free = free_list_head;

    int num_allocated_chunks = 0;
    int num_free_chunks = 0;

    while (address < heap_pointer + HEAP_SIZE)
    {
        // If it is free
        // check if it is in free list
        if (address == last_free)
        {
            num_free_chunks++;
            node *chunk = (node *)address;

            // print data
            printf("***********************\n");
            printf("*      FREE CHUNK     *\n");
            print_formatted("Address: ", (uint64_t)address - offset);
            printf("***********************\n");
            print_formatted("Size: ", (uint64_t)chunk->size);
            print_formatted("Next: ", chunk->next ? (uint64_t)chunk->next - offset : 0);
            printf("*                     *\n");
            printf("***********************\n");

            // next free chunk
            last_free = last_free->next;
            // next chunk
            address += (chunk->size + sizeof(node));
        }
        // else it must be allocated
        else
        {
            num_allocated_chunks++;
            header *chunk = (header *)address;
            // check magic number is right
            assert(chunk->magic == MAGIC_NUMBER);

            // print data
            printf("***********************\n");
            printf("*   ALLOCATED CHUNK   *\n");
            print_formatted("Address: ", (uint64_t)address - offset);
            printf("***********************\n");
            print_formatted("Size: ", chunk->size);
            print_formatted("Magic: ", chunk->magic);
            printf("*                     *\n");
            printf("***********************\n");

            // next chunk
            address += (chunk->size + sizeof(header));
        }
        // Make it more legible
        if (address < heap_pointer + HEAP_SIZE)
        {
            printf("        |    |        \n");
            printf("        |    |        \n");
        }
    }

    assert((uint64_t)address - offset == HEAP_SIZE);
    printf("Accounted for %ld of %ld bytes in heap\n", (uint64_t)address - offset, HEAP_SIZE);
    printf("There %s %d allocated chunk%s\n", num_allocated_chunks == 1 ? "is" : "are", num_allocated_chunks, num_allocated_chunks == 1 ? "" : "s");
    printf("There %s %d free chunk%s\n\n", num_free_chunks == 1 ? "is" : "are", num_free_chunks, num_free_chunks == 1 ? "" : "s");
}

#pragma endregion Helpers

#pragma region Shell

/* Walks through the heap till finding an allocated chunk at the specified index. Prints an error message if there is no such allocated chunk. */
void free_at_index(int index)
{
    // Return if index is less than 1
    if (index < 1)
    {
        printf("Index must be at least 1\n");
        return;
    }

    // WALK BY CHUNK
    // SIMULTANEOUSLY WALK FREE LIST WHEN FINDING A FREE CHUNK

    void *address = heap_pointer;
    node *last_free = free_list_head;

    int allocated_chunk_index = 0;

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

            allocated_chunk_index++;
            // If this is the index to free then break out of loop
            if (allocated_chunk_index == index)
                break;

            // next chunk
            address += (chunk->size + sizeof(header));
        }
    }

    if (index > allocated_chunk_index)
    {
        printf("Index is greater than the number of allocated chunks!\n");
        printf("There are only %d allocated chunks\n", allocated_chunk_index);
    }
    else
    {
        printf("Freeing allocated chunk at index %d\n", allocated_chunk_index);
        printf("Freeing allocated chunk at address %ld\n", (uint64_t)address - offset);

        my_free(address + sizeof(header));
    }
}

/* Show a list of commands for the interactive shell */
void show_commands()
{
    printf("\naudit - Audits the heap and displays it in diagram format\n");
    printf("walk free - Walks through the free list and prints out info\n");
    printf("walk allocated - Walks through the allocated chunks and prints out info\n");
    printf("malloc - Allocates a chunk of a user specified size\n");
    printf("free - Frees the allocated chunk at the index specified by the user\n");
    printf("help - Displays this list of commands\n");
    printf("quit - End the session\n\n");
}

/* Start the interactive shell */
void start_shell()
{
    char command[100];

    show_commands();

    while (strcmp(command, "quit"))
    {
        printf("> ");
        scanf("%s", command);

        // Which commands to execute, switch statements only accept int values :C
        if (!strcmp(command, "audit"))
        {
            audit();
        }
        else if (!strcmp(command, "walk"))
        {
            char which[20];
            scanf("%s", which);
            if (!strcmp(which, "free"))
            {
                walk_free_list();
            }
            else if (!strcmp(which, "allocated"))
            {
                walk_allocated_chunks();
            }
            else
            {
                printf("Invalid command for 'walk'. Type 'help' to see the list of commands\n");
            }
        }
        else if (!strcmp(command, "malloc"))
        {
            int size;
            printf("Size of chunk to allocate: ");
            scanf("%d", &size);
            printf("You requested to allocate a chunk of size %d\n", size);
            my_malloc(size);
        }
        else if (!strcmp(command, "free"))
        {
            int index;
            printf("Index of allocated chunk to free (the first allocated chunk is index 1): ");
            scanf("%d", &index);
            printf("You requested to free allocated chunk at index %d\n", index);
            free_at_index(index);
        }
        else if (!strcmp(command, "help"))
        {
            show_commands();
        }
        else if (strcmp(command, "quit"))
        {
            printf("Unrecognized command. Type 'help' to see the list of commands\n");
        }
    }
    printf("\nSession ended\n");
}

#pragma endregion Shell

int main(int argc, char const *argv[])
{
    init_heap();

    // Which test to run
    if (argv[1])
    {
        switch (atoi(argv[1]))
        {
        case 0:
            test_all();
            break;
        case 1:
            test_free_chunk_reuse();
            break;
        case 2:
            test_sorted_free_list();
            break;
        case 3:
            test_splitting_free_chunks();
            break;
        case 4:
            test_coalesce();
            break;
        case 5:
            test_alternating_sequence();
            break;
        case 6:
            test_worst_fit();
            break;
        case 7:
            test_malloc_bad_size();
            break;
        default:
            printf("Unrecognized flag. Please consult the following usage.\n");
            printf("0 - run all tests in below order.\n");
            printf("1 - run free chunk reuse tests.\n");
            printf("2 - run sorted free list tests.\n");
            printf("3 - run splitting free chunks tests.\n");
            printf("4 - run coalescing tests.\n");
            printf("5 - run alternating sequence tests.\n");
            printf("6 - run worst fit tests.\n");
            printf("7 - run malloc bad value tests.\n");
            printf("\nOr do not specify any flags and it will run an interactive shell.\n");
            break;
        }
    }
    // No arguments defaults to starting the interactive shell
    else
    {
        start_shell();
    }
}