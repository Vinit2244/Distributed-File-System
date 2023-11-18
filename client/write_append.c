#include "headers.h"

void communicate_with_ss_write(char *ipaddress, char *port, char *path,int f)
{
    int client_socket = connect_with_ss(ipaddress, port);
    printf("-----Storage     Server      Connected------\n");
    char input = 'a'; // Junk to initialise so that goes into loop
    int dataread = 0;
    st_request *packet = malloc(sizeof(st_request));
    if(f==1)
    {
        packet->request_type = WRITE_REQ;
    }
    else
    {
        packet->request_type = APPEND_REQ;
    }
    snprintf(packet->data, sizeof(packet->data), "%s|", path);
    printf(BLUE("ENTER INPUT TO BE WRITTEN \n"));
    while (input != '\n')
    {
        input = fgetc(stdin);
        packet->data[strlen(packet->data)] = input;
        dataread++;
        packet->data[strlen(packet->data) + 1] = '\0';
        if (dataread == MAX_DATA_LENGTH - 1)
        {
            ssize_t bytes_sent = send(client_socket, packet, sizeof(st_request), 0);
            if (bytes_sent == -1)
            {
                perror("Send failed");
            }
            free(packet);
            st_request *packet = malloc(sizeof(st_request));
            packet->request_type = WRITE_REQ;
            snprintf(packet->data, sizeof(packet->data), "%s|", path);
            dataread = 0;
        }
    }
    if (dataread > 0)
    {
        ssize_t bytes_sent = send(client_socket, packet, sizeof(st_request), 0);
        if (bytes_sent == -1)
        {
            perror("Send failed");
        }
    }
    free(packet);
    close(client_socket);
}

void writing_append_operation(char *path,int f)
{
    int client_socket = connect_with_ns();
    st_request *readerpacket = malloc(sizeof(st_request));
    if(f==1)
    {
        readerpacket->request_type = WRITE_REQ;
    }
    else
    {
        readerpacket->request_type = APPEND_REQ;
    }
    strcpy(readerpacket->data, path);
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
        ipaddress = strtok(response->data, "|");
        port = strtok(NULL, "|");
    }
    printf("%s %s \n", ipaddress, port);
    close(client_socket);
    // communicate_with_ss(ipaddress,port,path);
    communicate_with_ss_write(ipaddress, port, path,f);

    int client_socket_two = connect_with_ns();
    st_request *readerpacket2 = malloc(sizeof(st_request));
    readerpacket2->request_type = WRITE_APPEND_COMP;
    strcpy(readerpacket2->data, path);
    bytes_sent = send(client_socket_two, readerpacket2, sizeof(st_request), 0);
    if (bytes_sent == -1)
    {
        perror("Send failed");
    }
    printf(GREEN("-----Write/Append Completed------\n"));
}