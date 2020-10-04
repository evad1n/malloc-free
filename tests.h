#ifndef _TESTS_H_
#define _TESTS_H_

void test_free_chunk_reuse();
void test_sorted_free_list();
void test_splitting_free_chunks();
void test_coalesce();
void test_alternating_sequence();
void test_worst_fit();
void test_malloc_bad_size();
void test_all();

#endif