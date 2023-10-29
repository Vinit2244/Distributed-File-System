#ifndef __THREADS_H__
#define __THREADS_H__

#include <pthread.h>
#include <semaphore.h>

// Concurrency handling mechanisms (locks, condition variables and semaphores)
extern pthread_mutex_t accessible_paths_mutex;
extern pthread_mutex_t pending_requests_mutex;
extern pthread_mutex_t threads_arr_mutex;

extern pthread_cond_t update_paths_txt_cond_var;
extern pthread_cond_t pending_requests_cond_var;

// Thread functions
void* store_filepaths(void* args);
void* send_data_nfs(void* args);
void* receive_data_nfs(void* args);
void* send_data_client(void* args);
void* receive_data_client(void* args);
void* server_request(void* args);

#endif