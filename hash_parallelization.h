/*
 * Owner: Mehmet Saraçoğlu
 * Prepared for CMPE322 course in Bogazici University.
 * DO NOT EDIT
 */

#ifndef HASH_PARALLELIZATION_H
#define HASH_PARALLELIZATION_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <stdint.h>

struct entry_struct {
	int value; // Unique ID.
	int64_t timestamp; // The time that the entry is placed on hash.
};

struct entry_struct **hash_array; // Hash array.
struct entry_struct *entry_list; // Array allocated according to the CL parameters.
pthread_mutex_t *lock_list; // Array of locks allocated according to the CL parameters.
double h_1_speedup; // Speedup value which h_1 provides.

// n -> Size of the hash array.
// m -> Number of objects to be placed.
// value_list -> Values of the objects.
// t -> Number of threads to be used.
// k -> Special value that will be used in h_2.
// random_seed -> Seed value to be used in random function.

int n, m, t, k, random_seed;

// PROJECT PREDEFINED FUNCTIONS //

// Evaluation function. Do not edit.
int check_entry_list() {
	return 0;
}

// Creates a random integer value smaller than k. The rand() function is thread safe. Check about thread safety.
int get_random_val() {
	return rand() % (n / k);
}

// Call this function after giving values in your program.
int init(
	int n_,
	int m_,
	int t_,
	int k_,
	int random_seed_)
{
	n = n_;
	m = m_;
	t = t_;
	k = k_;
	random_seed = random_seed_;

	srand(random_seed);
	return 0;
}

// END OF PROJECT PREDEFINED FUNCTIONS //


// IMPLEMENT GIVEN FUNCTIONS IN .c FILE //

int array_allocation();
int array_deallocation();
int parallel_h_1();
int parallel_h_2();
int sequential_h_1();
int speedup_comparison_h_1();

// END OF IMPLEMENT SECTION //


#endif // HASH_PARALLELIZATION_H
