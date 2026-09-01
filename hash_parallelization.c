#include "hash_parallelization.h"
#include <string.h>
#include <time.h> // For clock_gettime
#include <stdint.h> // For int64_t


// Helper struct for passing arguments to threads
typedef struct {
    int thread_id;
} thread_arg_t;

// ARRAY ALLOCATION
int array_allocation() {
    // We allocate memory for our three main arrays here.
    // This is done once at the beginning.
    
    // First, we create the hash table itself.
    hash_array = (struct entry_struct **)malloc(n * sizeof(struct entry_struct *));
    if (hash_array == NULL) return -1;
    
    // It is important to set all slots to NULL so we know they are empty.
    for (int i = 0; i < n; i++) {
        hash_array[i] = NULL;
    }

    // Next, we create the list of items we want to insert.
    entry_list = (struct entry_struct *)malloc(m * sizeof(struct entry_struct));
    if (entry_list == NULL) return -1;

    // Finally, we generate the locks. One lock for each slot in the table.
    lock_list = (pthread_mutex_t *)malloc(n * sizeof(pthread_mutex_t));
    if (lock_list == NULL) return -1;

    // Initialize every lock so they are ready to use.
    for (int i = 0; i < n; i++) {
        pthread_mutex_init(&lock_list[i], NULL);
    }
    
    return 0;
}

// ARRAY DEALLOCATION
int array_deallocation() {
    // We need to clean up the locks first.
    if (lock_list != NULL) {
        for (int i = 0; i < n; i++) {
            pthread_mutex_destroy(&lock_list[i]);
        }
    }

    // Free the hash table memory.
    if (hash_array != NULL) {
        free(hash_array);
        hash_array = NULL;
    }

    // Free the list of items.
    if (entry_list != NULL) {
        free(entry_list);
        entry_list = NULL;
    }

    // Free the lock array memory.
    if (lock_list != NULL) {
        free(lock_list);
        lock_list = NULL;
    }
    
    return 0;
}

// SEQUENTIAL H1
int sequential_h_1() {
    // This is the standard, single-threaded version.
    
    // S1: i <- First entry
    // We go through every item in the list one by one.
    for (int j = 0; j < m; j++) {
        struct entry_struct *entry = &entry_list[j];
        
        // S2: c <- 0
        int c = 0;
        
        // S3: While not processed entries are existing:
        while (1) {
            // S4: Check hash_array[(i + c) mod n].
            int index = (entry->value + c) % n;
            
            // S5: If empty:
            if (hash_array[index] == NULL) {
                // S6: Put i there.
                hash_array[index] = entry;
                // We record the time when the item was placed.
                // Using clock_gettime for monotonic wall-clock time.
                struct timespec ts;
                clock_gettime(CLOCK_MONOTONIC, &ts);
                entry->timestamp = (int64_t)ts.tv_sec * 1000000000L + ts.tv_nsec;
                
                // S10: Mark entry i as processed.
                // S11: i <- next entry (Handled by for loop)
                // S12: c <- 0
                break;
            } else {
                // S7: Else:
                // S8: c <- c + 1
                c++;
                // S9: Go to state S4.
                continue;
            }
        }
    }
    // S13: Finish the procedure.
    return 0;
}

// PARALLEL H1
void *runner_h1(void *arg) {
    int thread_id = ((thread_arg_t *)arg)->thread_id;
    
    // We figure out which part of the list this thread is responsible for.
    int entries_per_thread = m / t;
    int start_index = thread_id * entries_per_thread;
    int end_index = start_index + entries_per_thread;
    
    // Now we start the main loop for this thread.
    
    // S1: i <- First entry of thread's possessed entry group
    for (int j = start_index; j < end_index; j++) {
        struct entry_struct *entry = &entry_list[j];
        
        // S2: c <- 0
        int c = 0;

        // S3: While not processed entries in thread's possessed entry group are existing:
        while (1) {
            int index = (entry->value + c) % n;

            // S4: Try acquiring Locks[(i + c) mod n]
            // We use trylock so we don't get stuck if someone else has it.
            if (pthread_mutex_trylock(&lock_list[index]) == 0) {
                 
                 // S6: Check hash_array[(i + c) mod n].
                 // Detailed Check:
                 if (hash_array[index] == NULL) {
                     // S8: Put i there.
                     hash_array[index] = entry;
                     
                     // Using clock_gettime for monotonic wall-clock time.
                     struct timespec ts;
                     clock_gettime(CLOCK_MONOTONIC, &ts);
                     entry->timestamp = (int64_t)ts.tv_sec * 1000000000L + ts.tv_nsec; 
                     
                     // S9: Release Locks[(i + c) mod n].
                     pthread_mutex_unlock(&lock_list[index]);
                     
                     // S17: Mark entry i as processed.
                     // S18 i <- next entry
                     // S19 c <- 0
                     break; // Move to the next item.
                 } else {
                     // S10: Else:
                     // S11: Release Locks[(i + c) mod n].
                     pthread_mutex_unlock(&lock_list[index]);
                     
                     // S12: c <- c + 1
                     c++;
                     // S13: Go to state S4
                     continue;
                 }

            } else {
                // S15: Else (Lock failed):
                // S16: Go to state S4.
                // We retry the SAME index until we get the lock (Spin-wait behavior).
                continue;
            }
        }
    }
    // S18: Finish the procedure.
    pthread_exit(NULL);
}

int parallel_h_1() {
    pthread_t threads[t];
    thread_arg_t args[t];

    for (int i = 0; i < t; i++) {
        args[i].thread_id = i;
        pthread_create(&threads[i], NULL, runner_h1, (void *)&args[i]);
    }

    for (int i = 0; i < t; i++) {
        pthread_join(threads[i], NULL);
    }
    return 0;
}

// PARALLEL H2
void *runner_h2(void *arg) {
    int thread_id = ((thread_arg_t *)arg)->thread_id;
    int entries_per_thread = m / t;
    int start_index = thread_id * entries_per_thread;
    int end_index = start_index + entries_per_thread;
    
    // We need an array to remember which locks we have taken, so we can release them later.
    // The maximum number of locks we might need is n divided by k.
    int *acquired_locks_indices = (int *)malloc((n / k) * sizeof(int));

    // Now we iterate through the items this thread owns.
    // S1: i <- First entry of thread's possessed entry group
    for (int j = start_index; j < end_index; j++) {
        struct entry_struct *entry = &entry_list[j];
        
        // S2: c <- 0
        int c = 0;

        // S3: While not processed entries in thread's possessed entry group are existing:
        while (1) {
            // S4: rand_val <- get_random_val() // rand_val < (n / k).
            // We pick a random starting point.
            int rand_val = get_random_val();

            // S5: Set up a counter cnt.
            // S6: cnt <- 0
            int cnt = 0;
            int locks_acquired_count = 0;
            int acquisition_failed = 0;

            // S7: Do (n / k) times:
            // We try to grab every lock in our assigned set.
            for (cnt = 0; cnt < (n / k); cnt++) {
                // S8: Try acquiring Locks
                // We calculate which lock to grab. We jump by 'k' steps each time.
                // This formula ensures we only touch specific locks assigned to this group.
                int index = ((entry->value + c) % k) + ((rand_val + cnt) % (n / k)) * k;
                
                if (pthread_mutex_trylock(&lock_list[index]) == 0) {
                    // Success, we got the lock.
                    acquired_locks_indices[locks_acquired_count++] = index;
                } else {
                    // S9: If failed:
                    // Someone else has this lock. We cannot wait because that might cause a deadlock.
                    acquisition_failed = 1;
                    
                    // S10: Release all acquired locks.
                    // So we give back everything we took so far.
                    for (int x = 0; x < locks_acquired_count; x++) {
                        pthread_mutex_unlock(&lock_list[acquired_locks_indices[x]]);
                    }
                    locks_acquired_count = 0;
                    
                    // S11: Go to state S4
                    break; // We stop trying and go back to the start.
                }
            }

            if (acquisition_failed) {
                // S11 continuation: Go to state S4 (loop back to start of while(1))
                continue;
            }

            // If we are here, we have acquired all (n/k) locks successfully.
            // Now we own all the slots we need to check.
            
            // S13: cnt <- 0
            cnt = 0;
            int placed = 0;

            // S14: While cnt < (n / k):
            // Now we assume the role of searching for an empty slot.
            for (cnt = 0; cnt < (n / k); cnt++) {
                // S15: Check hash_array
                int index = ((entry->value + c) % k) + ((rand_val + cnt) % (n / k)) * k;
                
                // S16: If empty:
                if (hash_array[index] == NULL) {
                    // S17: Put i there.
                    hash_array[index] = entry;
                    
                    // Using clock_gettime for monotonic wall-clock time.
                    struct timespec ts;
                    clock_gettime(CLOCK_MONOTONIC, &ts);
                    entry->timestamp = (int64_t)ts.tv_sec * 1000000000L + ts.tv_nsec;

                    // S18: Release all acquired locks.
                    // We are done. We can release all the locks now.
                    for (int x = 0; x < locks_acquired_count; x++) {
                        pthread_mutex_unlock(&lock_list[acquired_locks_indices[x]]);
                    }
                    placed = 1;
                    // S19: cnt <- cnt + 1 (Implicitly handled by loop, but we break here)
                    break;
                }
                // S19: cnt <- cnt + 1
            }

            if (placed) {
                // S24: Mark entry i as processed.
                // S25: i <- next entry
                // S26: c <- 0
                break; // Break while(1) to process next entry
            } else {
                // S20: If no available slot is found in the previous loop:
                // S21: Release all acquired locks.
                // The table was full in all the positions we checked.
                for (int x = 0; x < locks_acquired_count; x++) {
                    pthread_mutex_unlock(&lock_list[acquired_locks_indices[x]]);
                }
                
                // S22: c <- c + 1
                c++;
                
                // S23: Go to state S4.
                continue; // Continue while(1) loop
            }
        }
    }
    
    // Clean up our helper array.
    free(acquired_locks_indices);
    
    // S27: Finish the procedure.
    pthread_exit(NULL);
}

int parallel_h_2() {
    pthread_t threads[t];
    thread_arg_t args[t];

    for (int i = 0; i < t; i++) {
        args[i].thread_id = i;
        pthread_create(&threads[i], NULL, runner_h2, (void *)&args[i]);
    }

    for (int i = 0; i < t; i++) {
        pthread_join(threads[i], NULL);
    }
    return 0;
}

// SPEEDUP COMPARISON H1
int speedup_comparison_h_1() {
    // We measure how much faster the parallel version is compared to the sequential one.
    // We store the final ratio in 'h_1_speedup'.
    // We use clock_gettime because it gives the real wall-clock time, which is what we want.
    
    struct timespec start, end;
    
    // Part 1: Run the Sequential Version
    // We must clear the table first so it starts empty.
    for (int i = 0; i < n; i++) hash_array[i] = NULL;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    sequential_h_1();
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    double time_sequential = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;

    // Part 2: Run the Parallel Version
    // We clear the table again for a fair test.
    for (int i = 0; i < n; i++) hash_array[i] = NULL;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    parallel_h_1();
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    double time_parallel = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;

    // Part 3: Calculate the Ratio
    if (time_parallel > 0) {
        h_1_speedup = time_sequential / time_parallel;
    } else {
        h_1_speedup = 0; // Avoid crashing if time is zero.
    }
    
    return 0;
}
