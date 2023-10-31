#include "headers.h"

int client_socket;

void communicate_with_ss(char *ipaddress,char *port)
{
    
}

void client_to_ns()
{
    printf("-------Connection to NS started for client--------- \n");
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        perror("Socket creation failed");
        exit(1);
    }
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(NS_PORT);          
    server_address.sin_addr.s_addr = inet_addr(NS_IP); 
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("Connection failed");
        exit(1);
    }
    st_request *initialization = malloc(sizeof(st_request));
    strcpy(initialization->data, "Hello, server!");
    initialization->request_type = ACK;
    ssize_t bytes_sent = send(client_socket, initialization, sizeof(st_request), 0);
    if (bytes_sent == -1)
    {
        perror("Send failed");
    }

    printf("-----Communication with NS established-----\n");
}


void readingoperation(char *path)
{
    st_request* readerpacket=malloc(sizeof(st_request));
    readerpacket->request_type=READ_REQ;
    strcpy(readerpacket->data,path);
    ssize_t bytes_sent = send(client_socket, readerpacket, sizeof(st_request), 0);
    //printf("hi\n");
    if (bytes_sent == -1)
    {
        perror("Send failed");
    }
    // else
    // {
    //     printf("Sent %zd bytes to the server\n", bytes_sent);
    // }
    st_request* response = (st_request*)malloc(sizeof(st_request));
    ssize_t bytes_received = recv(client_socket, response, sizeof(st_request), 0);
    char *port;
    char *ipaddress;
    if (bytes_received == -1) 
    {
        perror("Receive failed");
    }
    else
    {
        ipaddress=strtok(response->data,"|");
        port=strtok(NULL, "|");
    }
    printf("%s %s \n",ipaddress,port);
    communicate_with_ss(ipaddress,port);
}

int main()
{
    client_to_ns();
    char operation[50];
    char path[3000];
    while (1)
    {
        printf("----Enter operation you want to do -------\n");
        scanf("%s %s",operation,path);
        if(strcmp("READ",operation)==0)
        {
            readingoperation(path);
        }
    }
    close(client_socket);
    
}