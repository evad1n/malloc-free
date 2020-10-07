# Malloc and Free Implementation
### *By Will Dickinson*

## Compiling and Running


### Run Interactive Shell
```
make run
```

### Run Tests
```
make test
```

Tests can also be run from the interactive shell.


# Details

I am using an 8 byte alignment, typical for a 64-bit word.

I am using worst-fit allocation, and when freeing I am inserting in a sorted position.

When allocating chunks, size 0 will not be accepted. I looked up what the typical case was with the official malloc, and it is allowed to either return NULL or return the address. I decided to return NULL as it made more sense to me. When allocating negative sizes, the behavior is the same as the official malloc and the size_t type will overflow to the max value and it will exceed the allowed size.

The audit function will print a diagram of the chunks similar to what is seen in [chapter 17 of the 3ep book](http://pages.cs.wisc.edu/~remzi/OSTEP/vm-freespace.pdf). This function will also verify the integrity of the magic number for each allocated chunk, and make sure all chunks are aligned to the 64-bit word size.

# Tests

Unless otherwise specified, all allocated chunks in tests will be 1/20 of the heap size. For a heap of size 4096, this means 204.8, which will translate to 224 after alignment and adding the size of the header.

All of these test cases will call the audit function before verifying, thereby having an extra layer of verification and also printing out the heap state at the time of verification. If it is desired, one can also manually verify the tests by examining each heap state.

All test cases start and end with an empty heap.

Some test cases may seem to have an excessive amount of steps. This is necessary due to the nature of worst-fit allocation. All tests take this into account and make sure allocated chunks go where they are needed in order to correctly test functionality.

## 1. Free chunk reuse tests

- Allocates 1 chunk with size of 1/3 of the heap. Allocates a second chunk with size slightly greater than the first. Frees the first chunk. Allocates another chunk with size less than the first. Verifies that the address of the first chunk is at the start of the heap.
- Allocates 2 chunks each with size 1/3 of the heap. Allocates another chunk of standard size. Frees the first 2 chunks. Allocates another chunk of standard size. Verifies that the address of the first chunk is at the start of the heap.
- Allocates 3 chunks each of size 1/4 of the heap. Allocates another chunk of standard size. Frees the second chunk. Verifies that the free list head is at the end of the first allocated chunk. Allocates a chunk of standard size. Verifies that the address of this new chunk is at the end of the first allocated chunk.

## 2. Sorted free list tests

- Allocates 5 chunks. Frees the first, third, and fifth chunk, in that order. Verifies that the each free chunk's next pointer address is greater than the node's current address.
- Allocates 10 chunks and then frees every other chunk in a non-sequential order. Verifies that the each free chunk's next pointer address is greater than the node's current address.

## 3. Splitting free chunks tests

- Allocates 1 chunk. Verifies that the address of the free list head has moved up by the size of the allocated chunk.
- Allocates 1 chunk of size 1/2 of the heap size. Allocates another chunk of standard size. Verifies that the free list head has moved up by the total size of the allocated chunks. Frees the first chunk. Allocates another chunk of standard size. Verifies that the address of the free list head has moved up by the size of the allocated chunk.
- Allocates 1 chunk that is the size of the heap. Verifies that the free list head pointer is NULL.
- Allocates 1 chunk of size 1/2 of the heap size. Allocates another chunk of standard size. Frees the first chunk. Allocates another chunk of size 1/2 of the heap size. Verifies that there is only 1 free chunk.

## 4. Coalescing tests

- Allocates 5 chunks and then frees them all. Verifies that there is exactly one node in the free list.
- Allocates 5 chunks and then frees the first 2 and the last 2. Verifies that there are exactly 2 chunks in the free list.
- Allocates 5 chunks and then frees chunks 1, 3 and 4. Verifies that there are exactly 3 chunks in the free list.

## 5. Alternating sequence tests

After each of these tests, verifies that no free chunks are next to each other.

- Allocates 2 chunks. Frees the second chunk.
- Allocates 7 chunks. Frees every other chunk.
- Allocates 3 chunks. The middle chunk will be a size equal to half the heap size. Frees the middle chunk. Allocates 4 more chunks.

## 6. Worst fit tests

- Allocates 2 chunks. Frees the first. Allocates a new chunk of size less than the first chunk. Verifies that this chunk was not allocated at the start of the heap, but after the second chunk, thereby taking the free space that was biggest.
- Allocates 3 chunks. The middle chunk will be a size equal to half the heap size. Frees the middle chunk. Allocates 4 chunks. Verifies that the first 3 chunks were allocated in the middle and the last chunk was allocated at the end (when the size of the middle free chunk became less than the last free chunk)


## 7. Malloc bad size tests

- Request 1 chunk that is twice the heap size. Verifies that the return is NULL.
- Allocated 2 chunks that come very close to totaling the heap size. Request 1 chunk that will make the total greater than the heap size. Verifies that the return is NULL.
- Request a chunk of size -1. Verifies that the return is NULL.
- Request a large (but still less than the size of the heap) negative chunk size. Verifies that the return is NULL.
- Request a chunk of size 0. Verifies that the return is NULL.