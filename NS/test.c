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

#define NS_PORT 2049
#define NS_IP "127.0.0.1"

#define MAX_DATA_LENGTH 100000
#define MAX_CONNECTIONS 10

// Request types
#define ACK 1
#define REQ 2
#define RES 3
#define REGISTRATION_REQUEST 4
#define REGISTRATION_ACK 5

pthread_mutex_t server_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t send_buffer_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t send_signal = PTHREAD_COND_INITIALIZER;
// Request packet structure
typedef struct st_request
{
  int request_type;
  char data[MAX_DATA_LENGTH];
} st_request;

typedef struct st_request *request;
typedef struct ss_info *ss;

// Storage server info
typedef struct ss_info
{
  char ip[20];
  char port[10];
  char client_port[10];
  char paths[1000][100];
  int path_count;

} ss_info;

typedef struct send_packet
{

  request r;   // main request packet
  int send_to; // Whether a client or server
  char ip[20]; // IP address where to send the request body
  int status;
  char port[10];

} send_packet;

typedef send_packet *packet;

int main()
{

  int sockfd;
  struct sockaddr_in addr;
  char buffer[1024];
  socklen_t addr_size;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  memset(&addr, '\0', sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(NS_PORT);
  addr.sin_addr.s_addr = inet_addr(NS_IP);

  //   bzero(buffer, 1024);
  //   strcpy(buffer, "Hello, World!");
  request r = (request)malloc(sizeof(st_request));
  r->request_type = REGISTRATION_REQUEST;
  strcpy(r->data, "127.0.0.1|2049|8080|1|/home");

  int x = sendto(sockfd, r, 1024, 0, (struct sockaddr *)&addr, sizeof(addr));
  printf("%d\n", x);
  //   printf("[+]Data send: %s\n", buffer);

  //   bzero(buffer, 1024);
  //   addr_size = sizeof(addr);

  recvfrom(sockfd, r, 1024, 0, (struct sockaddr *)&addr, &addr_size);
  printf("%d\n", r->request_type);
  //   printf("[+]Data recv: %s\n", buffer);
}
