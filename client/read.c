#include "headers.h"

//ERROR HANDLING LEFT WITH SS


void communicate_with_ss(char *ipaddress, char *port, char *path)
{
    int client_socket = connect_with_ss(ipaddress, port);
    printf("-----Storage     Server      Connected------\n");
    st_request *readerpacket = malloc(sizeof(st_request));
    readerpacket->request_type = READ_REQ;
    strcpy(readerpacket->data, path);
    ssize_t bytes_sent = send(client_socket, readerpacket, sizeof(st_request), 0);
    if (bytes_sent == -1)
    {
        perror("Send failed");
    }

    // Reader Packet sent

    // while(1)
    // {
    request req = (request)malloc(sizeof(st_request));
    while(1)
    {
        memset(req,0,sizeof(struct st_request));
        if (recv(client_socket, req, sizeof(st_request), 0) == -1)
        {
            perror("Receiving data failed");
            // continue;
        }
        if(req->request_type==ACK)
        {
            break;
        }
        else if(req->request_type==READ_FAILED)
        {
            printf("%s",req->data);
            break;
        }
        else
        {
            printf("%s", req->data);
        }
    }


    free(req);
    close(client_socket);
}

void communicate_with_ss_backup(char *ipaddress, char *port, char *path)
{
    int client_socket = connect_with_ss(ipaddress, port);
    printf("-----Storage     Server      Connected------\n");
    st_request *readerpacket = malloc(sizeof(st_request));
    readerpacket->request_type = BACKUP_READ_REQ;
    strcpy(readerpacket->data, path);
    ssize_t bytes_sent = send(client_socket, readerpacket, sizeof(st_request), 0);
    if (bytes_sent == -1)
    {
        perror("Send failed");
    }

    // Reader Packet sent

    // while(1)
    // {
    request req = (request)malloc(sizeof(st_request));
    while(1)
    {
        memset(req,0,sizeof(struct st_request));
        if (recv(client_socket, req, sizeof(st_request), 0) == -1)
        {
            perror("Receiving data failed");
            // continue;
        }
        if(req->request_type==ACK)
        {
            break;
        }
        else if(req->request_type==READ_FAILED)
        {
            printf("%s",req->data);
            break;
        }
        else
        {
            printf("%s", req->data);
        }
    }

    free(req);
    close(client_socket);
}

void reading_operation(char *path)
{
    int client_socket = connect_with_ns();
    st_request *readerpacket = malloc(sizeof(st_request));
    readerpacket->request_type = READ_REQ;
    strcpy(readerpacket->data, path);
    // printf("%s\n",path);

    ssize_t bytes_sent = send(client_socket, readerpacket, sizeof(st_request), 0);
    if (bytes_sent == -1)
    {
        perror("Send failed");
    }
    st_request *response = (st_request *)malloc(sizeof(st_request));
    ssize_t bytes_received = recv(client_socket, response, sizeof(st_request), 0);
    char *port;
    char *ipaddress;
    if (bytes_received == -1)
    {
        perror("Receive failed");
    }
    else
    {
        if (response->request_type == FILE_NOT_FOUND)
        {
            printf(RED("File Not Found \n")); // Error Not Found File
            return;
        }
        else if(response->request_type == TIMEOUT){
            printf(RED("File currently unavailable please try again\n"));
            return;
        }
        else{
        ipaddress = strtok(response->data, "|");
        port = strtok(NULL, "|");
        }
    }
    printf("%s %s \n", ipaddress, port);
    close(client_socket);
    if(response->request_type==RES){

    communicate_with_ss(ipaddress, port, path);
    }
    else if(response->request_type==BACKUP_READ_REQ){

        communicate_with_ss_backup(ipaddress, port, path);


    }

    
}