#ifndef __HEADERS_H__
#define __HEADERS_H__

// =========================== Header files ===========================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// ==================== User defined header files  ====================
#include "utils.h"
#include "threads.h"

// ========================== Useful Macros ==========================
#define MAX_DATA_LENGTH     100000      // Maximum number of characters data being sent can have (query/file data)
#define MAX_NO_OF_REQ       10          // At max it can handle 10 pending requests, if the request buffer is full then all the other incoming requests will be rejected
#define MAX_FILES           10          // Maximum number of files that can be stored in the storage server
#define MAX_PATH_LEN        1024        // Maximum length the relative path of a file can have
#define NFS_SERVER_PORT_NO  2000        // Port on which NFS server listens
#define MY_NFS_PORT_NO      3000        // Port number used to communicate with NFS server
#define MY_CLIENT_PORT_NO   4000        // Port number used to communicate with client
#define MY_IP               "127.0.0.1" // Ip address of this storage server
#define NFS_IP              "0.0.0.0"   // IP address of the naming server
#define MAX_PENDING         10          // Maximum number of connections the TCP socket can have in queue waiting

// =========================== Color Codes ============================
#define RED_COLOR    "\033[0;31m"
#define GREEN_COLOR  "\033[0;32m"
#define BLUE_COLOR   "\033[0;34m"
#define YELLOW_COLOR "\033[0;33m"
#define CYAN_COLOR   "\033[0;36m"
#define ORANGE_COLOR "\e[38;2;255;85;0m"
#define RESET_COLOR  "\033[0m"

#define RED(str)    RED_COLOR    str RESET_COLOR
#define GREEN(str)  GREEN_COLOR  str RESET_COLOR
#define BLUE(str)   BLUE_COLOR   str RESET_COLOR
#define YELLOW(str) YELLOW_COLOR str RESET_COLOR
#define CYAN(str)   CYAN_COLOR   str RESET_COLOR
#define ORANGE(str) ORANGE_COLOR str RESET_COLOR

// =========================== Request Types ==========================
#define ACK 1
#define REQ 2
#define RES 3
#define REGISTRATION_REQUEST 4
#define REGISTRATION_ACK     5

// ============================= Statuses =============================
#define NOT_REGISTERED 0
#define REGISTERED     1

// ============================ Structures ============================
typedef struct st_request 
{
    int  request_type;              // Request type would determine whether it is an acknowledgement, query, response, data etc.
    char data[MAX_DATA_LENGTH];     // All the communication happens in the form of strings
} st_request;

typedef struct st_request* request;

typedef struct st_pending_request_node
{
    int idx_thread_alloted;
    int client_socket_fd;
    st_request recvd_request;
} st_pending_request_node;

typedef st_pending_request_node* pending_request_node;

// ========================= Global variables =========================
extern char**  accessible_paths;            // Stores the RELATIVE PATH (relative to the directory in which the storage server c file resides) of all the files that are accessible by clients on this storage server
extern int     num_of_paths_stored;         // Stores the number of paths which are currently stored in the accessible_paths array
extern request request_buffer;              // Buffer (Queue) to store all the incoming requests
extern int     num_of_pending_requests;     // Stores the number of pending requests in the queue
extern int     request_buffer_read_idx;     // Index from where to read the request
extern int     request_buffer_write_idx;    // Index where to write new incoming requests
extern int     nfs_registrations_status;    // Stores the status whether our server has been registered with NFS or not
extern int     nfs_socket_fd;               // NFS socket fd (used for communication with nfs)
extern struct  sockaddr_in nfs_address;     // IPv4 address struct
extern pending_request_node pending_requests_buffer;    // Buffer to store all the pending request to be served to clients
extern int     num_of_pending_requests;     // Stores the number of pending requests stored in the pending_buffer
extern int     read_head_idx_pending_requests_buffer;   // Index where to read the next pending request from
extern int     write_head_idx_pending_requests_buffer;  // Index where to write the next pending request
extern pthread_t* requests_serving_threads_arr; // Holds the threads when a request is being served in some thread
extern int*    thread_slot_empty_arr;           // 1 = thread is running, 0 = thread slot is free and can be used to create a new thread

#endif
