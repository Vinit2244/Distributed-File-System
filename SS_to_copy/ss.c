#include "headers.h"

// Initiating global variables
pthread_mutex_t accessible_paths_mutex;
pthread_mutex_t threads_arr_mutex;
pthread_cond_t update_paths_txt_cond_var;

char**  accessible_paths         = NULL;           // Stores the RELATIVE PATH (relative to the directory in which the storage server c file resides) of all the files that are accessible by clients on this storage server
int     num_of_paths_stored      = 0;              // Initially no paths are stored
int     nfs_registrations_status = NOT_REGISTERED; // Stores the status whether our server has been registered with NFS or not
int     client_server_socket_fd;                   // TCP Socket file descriptor to receive client requests
int     nfs_server_socket_fd;                      // TCP Socket file descriptor to receive NFS requests
int     socket_fd;                                 // UDP Socket used for communication with NFS to register my SS
struct  sockaddr_in ss_address_nfs;                // IPv4 address struct for ss and nfs TCP communication (requests)
struct  sockaddr_in ss_address_client;             // IPv4 address struct for ss and client TCP communication (requests)
struct  sockaddr_in address;         
socklen_t addr_size;              // IPv4 address struct for ss and nfs USP communication (register)
int*    thread_slot_empty_arr;                     // 1 = thread is running, 0 = thread slot is free and can be used to create a new thread
pthread_t* requests_serving_threads_arr;           // Holds the threads when a request is being served in some thread

int main(int argc, char *argv[])
{
    // Allocating memory
    requests_serving_threads_arr = (pthread_t*) malloc(MAX_PENDING * sizeof(pthread_t));
    thread_slot_empty_arr        = (int*) calloc(MAX_PENDING, sizeof(int));    // 0 indicates slot is empty and 1 indicates slot is busy

    // Initializing all the accessible paths (Reading all the paths stored in paths.txt and storing it in accessible paths buffer)
    accessible_paths_init();

    printf(GREEN("\nPaths read from the text file into the accessible paths 2D array.\n"));

    // Initializing mutexes, condition variables and semaphores
    /*========== MUTEX ==========*/
    pthread_mutex_init(&accessible_paths_mutex, NULL);
    pthread_mutex_init(&threads_arr_mutex, NULL);
    /*========== COND VARS ==========*/
    pthread_cond_init(&update_paths_txt_cond_var, NULL);
    /*========== SEMAPHORES ==========*/

    // First start the NFS and Client TCP servers to listen to their requests
    if(nfs_registrations_status==NOT_REGISTERED)register_ss();
    start_nfs_port();
    start_client_port();
    
    // Register my SS with NFS

    // Creating the thread that would keep updating the paths.txt file with the current state of the accessible paths array regularly after some time interval
    pthread_t store_filepaths_thread;
    pthread_create(&store_filepaths_thread, NULL, &store_filepaths, NULL);

    // Waiting for threads to complete
    pthread_join(store_filepaths_thread, NULL);

    // Destroying mutexes, condition variables and semaphores
    /*========== MUTEX ==========*/
    pthread_mutex_destroy(&accessible_paths_mutex);
    pthread_mutex_destroy(&threads_arr_mutex);
    /*========== COND VARS ==========*/
    pthread_cond_destroy(&update_paths_txt_cond_var);
    /*========== SEMAPHORES ==========*/

    // Freeing Memory
    free(requests_serving_threads_arr);
    free(thread_slot_empty_arr);

    return 0;
}

