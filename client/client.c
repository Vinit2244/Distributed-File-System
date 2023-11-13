#include "headers.h"

int connect_with_ss(char *ipaddress, char *port)
{
    int client_socket;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(port));
    server_address.sin_addr.s_addr = inet_addr(ipaddress);
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("Connection failed");
        return 0;
    }
    return client_socket;
}

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
    if (recv(client_socket, req, sizeof(st_request), 0) == -1)
    {
        perror("Receiving data failed");
        // continue;
    }
    printf("%s\n", req->data);
    free(req);
    close(client_socket);
}
void communicate_with_ss_info(char *ipaddress, char *port, char *path)
{
    int client_socket = connect_with_ss(ipaddress, port);
    printf("-----Storage     Server      Connected------\n");
    st_request *readerpacket = malloc(sizeof(st_request));
    readerpacket->request_type = RETRIEVE_INFO;
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
    if (recv(client_socket, req, sizeof(st_request), 0) == -1)
    {
        perror("Receiving data failed");
        // continue;
    }
    if(req->request_type==INFO)
    {
        printf("%s\n", req->data);
    }
    else
    {
        printf("WRONG DATA RECEIVED\n");
    }
    free(req);
    close(client_socket);
}
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
void client_to_ns()
{
    int client_socket;
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
    // recv(client_socket, initialization, sizeof(st_request), 0);

    close(client_socket);
    printf("-----Communication with NS established-----\n");
}

int connect_with_ns()
{
    int client_socket;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(NS_PORT);
    server_address.sin_addr.s_addr = inet_addr(NS_IP);
    while (1)
    {
        if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
        {
            perror("Connection failed");
            continue;
        }
        else
        {
            break;
        }
    }
    return client_socket;
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
            perror("File Not Found \n"); // Error Not Found File
            return;
        }
        ipaddress = strtok(response->data, "|");
        port = strtok(NULL, "|");
    }
    printf("%s %s \n", ipaddress, port);
    close(client_socket);
    communicate_with_ss(ipaddress, port, path);
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
            perror("File Not Found \n"); // Error Not Found File
            return;
        }
        ipaddress = strtok(response->data, "|");
        port = strtok(NULL, "|");
    }
    printf("%s %s \n", ipaddress, port);
    close(client_socket);
    // communicate_with_ss(ipaddress,port,path);
    communicate_with_ss_write(ipaddress, port, path,f);
}

void create_operation(char *path, char *name,int macro)
{
    int client_socket = connect_with_ns();
    st_request *packet = malloc(sizeof(st_request));
    packet->request_type = macro;
    snprintf(packet->data, sizeof(packet->data), "%s|%s", path, name);
    ssize_t bytes_sent = send(client_socket, packet, sizeof(st_request), 0);
    if (bytes_sent == -1)
    {
        perror("Send failed");
    }
    st_request *response = (st_request *)malloc(sizeof(st_request));
    ssize_t bytes_received = recv(client_socket, response, sizeof(st_request), 0);
    if (response->request_type==ACK)
    {
        printf("Deletion of Directory or File succesfull \n");
    }
    else
    {
        printf("Deletion of Directory or File not succesfull \n"); // Error Not succesfull
    }
    close(client_socket);
}
void delete_operation(char *path,int macro)
{
    int client_socket = connect_with_ns();
    st_request *packet = malloc(sizeof(st_request));
    packet->request_type = macro;
    snprintf(packet->data, sizeof(packet->data), "%s", path);
    ssize_t bytes_sent = send(client_socket, packet, sizeof(st_request), 0);
    if (bytes_sent == -1)
    {
        perror("Send failed");
    }
    st_request *response = (st_request *)malloc(sizeof(st_request));
    ssize_t bytes_received = recv(client_socket, response, sizeof(st_request), 0);
    if (response->request_type==ACK)
    {
        printf("Deletion of Directory or File succesfull \n");
    }
    else
    {
        printf("Deletion of Directory or File not succesfull \n"); // Error Not succesfull
    }
    close(client_socket);
}

void copy_operation(char *path1, char *path2)
{
    int client_socket = connect_with_ns();
    st_request *packet = malloc(sizeof(st_request));
    packet->request_type = COPY_REQ;
    snprintf(packet->data, sizeof(packet->data), "%s|%s", path1, path2);
    ssize_t bytes_sent = send(client_socket, packet, sizeof(st_request), 0);
    if (bytes_sent == -1)
    {
        perror("Send failed");
    }
    st_request *response = (st_request *)malloc(sizeof(st_request));
    ssize_t bytes_received = recv(client_socket, response, sizeof(st_request), 0);
    if (strcmp(response->data, "STOP") == 0)
    {
        printf("Copy of Directory or File succesfull \n");
    }
    else
    {
        printf("Copy of Directory or File not succesfull \n"); // Error Not succesfull
    }
    close(client_socket);
}

void info(char *path)
{
    int client_socket = connect_with_ns();
    st_request *readerpacket = malloc(sizeof(st_request));
    readerpacket->request_type = RETRIEVE_INFO;
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
            perror("File Not Found \n"); // Error Not Found File
            return;
        }
        ipaddress = strtok(response->data, "|");
        port = strtok(NULL, "|");
    }
    printf("%s %s \n", ipaddress, port);
    close(client_socket);
    communicate_with_ss_info(ipaddress, port, path);
}
int main()
{
    // client_to_ns();
    char input[3000];
    while (1)
    {
        printf("----Enter operation you want to do -------\n");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';
        char *operation = strtok(input, " ");
        if (strcmp("READ", operation) == 0)
        {
            char *path = strtok(NULL, " ");
            reading_operation(path); // Reading Function Gets Called
        }
        else if (strcmp("WRITE", operation) == 0)
        {
            char *path = strtok(NULL, " ");
            writing_append_operation(path,1); // Writing Function Gets Called
        }
        else if (strcmp("APPEND", operation) == 0)
        {
            char *path = strtok(NULL, " ");
            writing_append_operation(path,0); // Writing Function Gets Called
        }
        else if (strcmp("CREATE", operation) == 0)
        {
            char *type = strtok(NULL, " ");
            char *path = strtok(NULL, " ");
            char *name = strtok(NULL, " ");
            if(strcmp(type,"FOLDER")==0)
            {
                create_operation(path, name,CREATE_FOLDER);
            }
            else if(strcmp(type,"FILE")==0)
            {
                create_operation(path, name,CREATE_FILE);
            }
            
        }
        else if (strcmp("DELETE", operation) == 0)
        {
            char *type = strtok(NULL, " ");
            char *path = strtok(NULL, " ");
            if(strcmp(type,"FOLDER")==0)
            {
                delete_operation(path,DELETE_FOLDER);
            }
            else if(strcmp(type,"FILE")==0)
            {
                delete_operation(path,DELETE_FILE);
            }
            
        }
        else if (strcmp("COPY", operation) == 0)
        {
            char *path1 = strtok(NULL, " ");
            char *path2 = strtok(NULL, " ");
            printf("%s %s\n", path1, path2);
            copy_operation(path1, path2);
        }
        else if (strcmp("INFO", operation) == 0)
        {
            char *path = strtok(NULL, " ");
            info(path); 
        }
        else if (strcmp("EXIT", operation) == 0)
        {
            break;
        }
        else
        {
            printf("Invalid Operation do again\n");
        }
    }
    // close(client_socket);
}