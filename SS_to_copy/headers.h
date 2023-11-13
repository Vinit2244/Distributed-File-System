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
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

// ==================== User defined header files  ====================
#include "utils.h"
#include "threads.h"

// ========================== Useful Macros ==========================
#define MAX_DATA_LENGTH     10000       // Maximum number of characters data being sent can have (query/file data)
#define MAX_NO_OF_REQ       10          // At max it can handle 10 pending requests, if the request buffer is full then all the other incoming requests will be rejected
#define MAX_FILES           30          // Maximum number of files that can be stored in the storage server
#define MAX_PATH_LEN        1024        // Maximum length the relative path of a file can have
#define NFS_SERVER_PORT_NO  2000        // Port on which NFS server listens
#define MY_NFS_PORT_NO      3000        // Port number used to communicate with NFS server
#define MY_CLIENT_PORT_NO   4000        // Port number used to communicate with client
#define MY_IP               "127.0.0.1" // Ip address of this storage server
#define NFS_IP              "127.0.0.1" // IP address of the naming server
#define MAX_PENDING         10          // Maximum number of connections the TCP socket can have in queue waiting
#define MY_SS_ID            1           // Each storage server is assigned a unique SS_ID (used to distinguish between different servers)
#define PWD                 "/Users/vinitmehta/Desktop/IIITH_Main/Sem3/OSN/mini_projects/final-project-43/SS_to_copy"

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
#define ACK                  1
#define REQ                  2
#define RES                  3
#define WRITE_REQ            4
#define READ_REQ             5
#define DELETE_REQ           6
#define CREATE_REQ           7
#define APPEND_REQ           8
#define REGISTRATION_REQUEST 9
#define REGISTRATION_ACK     10
#define STOP_REQ             11
#define READ_REQ_DATA        12
#define ADD_PATHS            13
#define DELETE_PATHS         14
#define PASTE                15
#define COPY                 16
#define COPY_REQUEST         17
#define RETRIEVE_INFO        19
#define INFO                 20
#define DATA_TO_BE_COPIED    21
#define PING                 22
#define CREATE_FILE          23
#define CREATE_FOLDER        24
#define DELETE_FILE          25
#define DELETE_FOLDER        26

// ============================= Statuses =============================
#define NOT_REGISTERED    0
#define REGISTERED        1
#define WRITE_SUCCESSFUL  2
#define APPEND_SUCCESSFUL 2
#define SUCCESSFUL        2    // Keep all three successes to have the same value

// ============================ Structures ============================
// All the network communication happens in this structure form
typedef struct st_request 
{
    int  request_type;              // Request type would determine whether it is an acknowledgement, query, response, data etc.
    char data[MAX_DATA_LENGTH];     // All the communication happens in the form of strings
} st_request;

typedef struct st_request* request;

// Used to pass data to request serving threads
typedef struct st_thread_data
{
    int thread_idx;
    int client_sock_fd;
} st_thread_data;

typedef struct st_thread_data* thread_data;

// ========================= Global variables =========================
extern char**  accessible_paths;                // Stores all the RELATIVE PATHS (relative to the directory in which the storage server c file resides) of all the files that are accessible by clients on this storage server
extern int     num_of_paths_stored;             // Stores the number of paths which are currently stored in the accessible_paths array
extern int     nfs_registrations_status;        // Stores the status whether our server has been registered with NFS or not
extern int     client_server_socket_fd;         // Socket file descriptor to receive client requests
extern int     nfs_server_socket_fd;            // Socket file descriptot to receive NFS requests
extern struct  sockaddr_in ss_address_nfs;      // IPv4 address struct for TCP communication between ss and nfs (requests)
extern struct  sockaddr_in ss_address_client;   // IPv4 address struct for TCP communication between ss and client (requests)
extern socklen_t addr_size;                     // IPv4 address struct for TCP communication between ss and nfs (register)
extern int*    thread_slot_empty_arr;           // 1 = thread is running, 0 = thread slot is free and can be used to create a new thread
extern pthread_t* requests_serving_threads_arr; // Holds the threads when a request is being served in some thread

#endif
