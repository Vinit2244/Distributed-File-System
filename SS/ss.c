#include "headers.h"

// Initiating global variables
pthread_mutex_t accessible_paths_mutex;
pthread_mutex_t pending_requests_mutex;
pthread_mutex_t threads_arr_mutex;
pthread_cond_t update_paths_txt_cond_var;
pthread_cond_t pending_requests_cond_var;

char**  accessible_paths         = NULL;                // Stores the RELATIVE PATH (relative to the directory in which the storage server c file resides) of all the files that are accessible by clients on this storage server
int     num_of_paths_stored      = 0;                   // Initially no paths are stored
int     nfs_registrations_status = NOT_REGISTERED;      // Stores the status whether our server has been registered with NFS or not
int     nfs_socket_fd;                                  // Socket to communicate with NFS server
int     num_of_pending_requests = 0;                    // Stores the number of pending requests stored in the pending_buffer
int     read_head_idx_pending_requests_buffer  = 0;     // Index where to read the next pending request from
int     write_head_idx_pending_requests_buffer = 0;     // Index where to write the next pending request
struct  sockaddr_in nfs_address;                        // IPv4 address struct
pending_request_node pending_requests_buffer = NULL;    // Buffer to store all the pending request to be served to clients
pthread_t* requests_serving_threads_arr;                // Holds the threads when a request is being served in some thread
int*    thread_slot_empty_arr;                              // 1 = thread is running, 0 = thread slot is free and can be used to create a new thread

int main(int argc, char *argv[])
{
    // Allocating memory to the pending requests buffer
    pending_requests_buffer = (pending_request_node) malloc(MAX_PENDING * sizeof(st_pending_request_node));
    requests_serving_threads_arr = (pthread_t*) malloc(MAX_PENDING * sizeof(pthread_t));
    thread_slot_empty_arr = (int*) calloc(MAX_PENDING, sizeof(int));    // Initializing all the values to 0 since no thread is running

    // Initializing all the accessible paths
    accessible_paths_init();

    printf(GREEN("\nPaths read from the text file into the accessible paths 2D array.\n"));

    // Initializing mutexes, condition variables and semaphores
    /*========== MUTEX ==========*/
    pthread_mutex_init(&accessible_paths_mutex, NULL);
    pthread_mutex_init(&pending_requests_arr_mutex, NULL);
    pthread_mutex_init(&threads_arr_mutex, NULL);
    /*========== COND VARS ==========*/
    pthread_cond_init(&update_paths_txt_cond_var, NULL);
    pthread_cond_inti(&pending_requests_cond_var, NULL);
    /*========== SEMAPHORES ==========*/

    // Bind to port and connect to NFS
    start_nfs_port();
    // Bind to port and start listening for client requests
    start_client_port();

    // Creating the thread that would keep updating the paths.txt file with the current state of the accessible paths array regularly after some time interval
    pthread_t store_filepaths_thread;
    pthread_create(&store_filepaths_thread, NULL, &store_filepaths, NULL);

    // Waiting for threads to complete
    pthread_join(store_filepaths_thread, NULL);

    // Destroying mutexes, condition variables and semaphores
    /*========== MUTEX ==========*/
    pthread_mutex_destroy(&accessible_paths_mutex);
    pthread_mutex_destroy(&pending_requests_arr_mutex);
    pthread_mutex_destroy(&threads_arr_mutex);
    /*========== COND VARS ==========*/
    pthread_cond_destroy(&update_paths_txt_cond_var);
    pthread_cond_destroy(&pending_requests_cond_var);
    /*========== SEMAPHORES ==========*/

    // Freeing Memory
    free(pending_requests_buffer);

    return 0;
}

