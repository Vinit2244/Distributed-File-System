//Code for initialising and working with NFS for the project
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

//Relevant Macros
#define NS_PORT 2000
#define NS_IP "127.0.0.1"
#define MAX_DATA_LENGTH 100000
#define MAX_CONNECTIONS 10

//Request types
#define ACK                  1
#define REQ                  2
#define RES                  3
#define WRITE_REQ            4
#define READ_REQ             5
#define DELETE_REQ           6
#define CREATE_REQ           7
#define REGISTRATION_REQUEST 8
#define REGISTRATION_ACK     9

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

//Concurrency locks and conditional variables
extern pthread_mutex_t server_lock;
extern pthread_mutex_t send_buffer_lock;
extern pthread_cond_t send_signal;

//Request packet structure
typedef struct st_request
{
    int request_type;
    char data[MAX_DATA_LENGTH];
} st_request;

//Typedef of request and server pointer objects
typedef struct st_request* request;
typedef struct ss_info* ss;

//Storage server info
typedef struct ss_info
{
    char ip[20];
    char port[10];
    char client_port[10];
    char paths[1000][100];
    int path_count;

} ss_info;

//Send packet used for convenience of sending request packets to client/SS
typedef struct send_packet{

    request r;      //main request packet
    int send_to;    //Whether a client or server
    char ip[20];    //IP address where to send the request body
    int status;     
    char port[10]; 

} send_packet;

typedef send_packet* packet;



//Global variables
extern ss ss_list[100];
extern int server_count;
extern packet send_buffer[100];
extern int send_count;
extern int sockfd_udp;
extern struct sockaddr_in server_addr_udp, client_addr_udp;
extern socklen_t addr_size_udp;


//Defined functions
char** processstring(char data[],int n);
void init_nfs();
void client_handler(char data[]);
void init_storage(char data[]);
void process(request req);
void* send_handler();
void* receive_handler();
void* udp_handler();
