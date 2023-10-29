#ifndef __THREADS_H__
#define __THREADS_H__

#include <pthread.h>
#include <semaphore.h>

// Concurrency handling mechanisms (locks, condition variables and semaphores)
extern pthread_mutex_t accessible_paths_mutex;      // Used to lock memory access to accessible_paths 2D array
extern pthread_mutex_t pending_requests_arr_mutex;  // Used to lock access to pending requests array and its read/write/count variables
extern pthread_mutex_t threads_arr_mutex;           // Used to lock access to the threads array in which we are assigning each request processing thread

extern pthread_cond_t update_paths_txt_cond_var;    // Whenever the accesible paths array is updated, this condition variable is used to signal the store_filepaths thread to store the updated file path into the text file
extern pthread_cond_t pending_requests_cond_var;    // Used to signal whenever a new request comes

// Thread functions
void* store_filepaths(void* args);
void* serve_request(void* args);
void* send_reg_req(void* args);
void* receive_reg_ack(void* args);

#endif