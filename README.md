# Malloc and Free Implementation
### *By Will Dickinson*

## Compiling and Running

```
make
./malloc_free.exe [TEST FLAG]
```
Test flags are as described:
- 0 - run all tests
- 0 - run free chunk reuse tests
- 1 - run sorted free list tests
- 2 - run splitting free chunks tests
- 3 - run coalescing tests
- 4 - run alternating sequence tests
- 5 - run worst fit tests
- 6 - run malloc return null tests

or if you want to open an interactive shell just have no flags
```
make run
```
or
```
./malloc_free.exe
```

# Tests

I am using worst-fit allocation, and when freeing I am inserting in a sorted position.

I am using pragmas to collapse my code and make it more readable so I used the flag `-Wno-unknown-pragmas`.

All of these tests will at some point call the audit function which will print a diagram of the chunks similar to what is seen in [chapter 17 of the 3ep book](http://pages.cs.wisc.edu/~remzi/OSTEP/vm-freespace.pdf). This function will also verify the integrity of the magic number for each allocated chunk.

Unless otherwise specified, all allocated chunks will be 1/20 of the heap size.

## Free chunk reuse tests

- Allocates 2 chunks and then frees the first one. Allocates another chunk with size less than the first. Verifies that the address of thre first chunk is at the start of the heap.
- Allocates 3 chunks and then frees the first 2. Allocates another chunk with size less than the sum of the first 2. Verifies that the address of thre first chunk is at the start of the heap.
- Allocates 3 chunks and frees the middle chunk. Allocates a chunk with size greater than the previous middle chunk. Verifies that the address of this new chunk is at the end of the allocated chunks. Then allocates 1 chunk with size less than the previous middle chunk. Verifies that the address of new the middle chunk is at the end of the first allocated chunk. 

## Sorted free list tests

- Allocates 5 chunks. Frees the first, third, and fifth chunk, in that order. Verifies that the each nodes next pointer address is greater than the node's current address.
- Allocates 10 chunks and then frees every other chunk in a random order. Verifies that the each nodes next pointer address is greater than the node's current address.

## Splitting free chunks tests

- Allocates 1 chunk. Verifies that the address of the free list head has moved up by the size of the allocated chunk.
- Do something else IDK.

## Coalescing tests

- Allocates 5 chunks and then frees them all. Verifies that there is exactly one node in the free list.
- Allocates 5 chunks and then frees the first 2 and the last 2. Verifies that there are exactly 2 nodes in the free list.
- Allocates 5 chunks and then frees chunks 1, 3 and 4. Verifies that there are exactly 3 nodes in the free list.

## Alternating sequence tests

After each of these tests, verifies that no free chunks are next to each other.

- Allocates 2 chunks. Frees the second chunk.
- Allocates 7 chunks. Frees every other chunk.
- Allocates 3 chunks. The middle chunk will be a size equal to half the heap size. Frees the middle chunk. Allocates 4 chunks.

## Worst fit tests

- Allocates 2 chunks. Frees the first. Allocates a new chunk of size less than the first chunk. Verifies that this chunk was not allocated at the start of the heap, but rather after the second chunk, thereby taking the free space that was biggest.
- Allocates 3 chunks. The middle chunk will be a size equal to half the heap size. Frees the middle chunk. Allocates 4 chunks. Verifies that the first 3 chunks were allocated in the middle and the last chunk was allocated at the end (when the size of the middle free chunk became less than the last free chunk)


## Malloc returns null tests

- Request 1 chunk that is twice the heap size. Verifies that the return is NULL.
- Allocated 2 chunks that come very close to totaling the heap size. Request 1 chunk that will make the total greater than the heap size. Verifies that the return is NULL.


# NOTES

Implement malloc and free. You must coalesce free blocks, and you must include a tool to audit the heap. When called, it should display a complete list of free and allocated blocks, and verify that all memory is properly accounted for. See chapter 17 of the 3ep book for suggestions.

Submit your source files here. Please do NOT zip them up.

You should include your source code, a Makefile, and instructions for how to repeat each of your tests in a README file.

You should allocate a single range of memory to act as your heap using the mmap system call (see 3ep chapter 17 for information and an example).

You should include tests that show each of the following:

Free blocks are re-used when possible
Free blocks are split and coalesced appropriately
The free-block list is in sorted order and includes all free blocks on the heap
The heap is always an alternating sequence of 1 free block and 1 or more allocated blocks, with the entire heap accounted for
You are correctly implementing either worst-fit or next-fit allocation
Your README file should give instructions for compiling your code and running each of the tests. You should also submit a video where you demonstrate your tests and explain how each test proves that your code is working as expected.