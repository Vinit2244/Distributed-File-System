#include "headers.h"

pthread_mutex_t server_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t send_buffer_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t send_signal = PTHREAD_COND_INITIALIZER;


void* send_handler(){


    //Currently work in progress


    printf("------------------------Send handler started-------------------------------\n");
    while(1){
    
        pthread_mutex_lock(&send_buffer_lock);
        pthread_cond_wait(&send_signal,&send_buffer_lock);

        packet p = send_buffer[send_count-1];

        if(p->status==0){

            int server_socket, client_socket;
            struct sockaddr_in server_addr, client_addr;
            socklen_t client_addr_len = sizeof(client_addr);


            server_socket = socket(AF_INET, SOCK_STREAM, 0);
            if (server_socket == -1) {
                perror("Socket creation failed");
                // exit(EXIT_FAILURE);
            }

            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(NS_PORT);
            server_addr.sin_addr.s_addr = inet_addr(p->ip);

            if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
                perror("Binding to port failed");
                
            }

            if (listen(server_socket, MAX_CONNECTIONS) == -1) {
                perror("Listening on port failed");
                
            }

            client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
            if (client_socket == -1) {
                perror("Accepting connection failed");
                continue;
            }

            if(send(client_socket,p->r,sizeof(p->r),0)<0){
                perror("Send error");
            }

            close(client_socket);
            close(server_socket);
            p->status=0;

        }

        pthread_mutex_unlock(&send_buffer_lock);
    }

    return NULL;
}

void* receive_handler(){
    printf("-------------------------TCP handler started-------------------------------\n");
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(NS_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        perror("Binding to port failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, MAX_CONNECTIONS) == -1) {
        perror("Listening on port failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        
        // Accept incoming connections
        
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket == -1) {
            perror("Accepting connection failed");
            continue;
        }


        request req = (request)malloc(sizeof(st_request));
        if(recv(client_socket,req,sizeof(st_request), 0) == -1){
            perror("Receiving data failed");
            continue;
        }
        
        //Send request object for processing
        process(req);
        free(req);
        close(client_socket);
    }

    close(server_socket);

    return NULL;

}

void* udp_handler(){
    
    printf("------------------------UDP handler started--------------------------------\n");
    
    int n;

    sockfd_udp = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_udp < 0){
        perror("[-]socket error");
        exit(1);
    }

    memset(&server_addr_udp, '\0', sizeof(server_addr_udp));
    server_addr_udp.sin_family = AF_INET;
    server_addr_udp.sin_port = htons(NS_PORT);
    server_addr_udp.sin_addr.s_addr = INADDR_ANY;

    n = bind(sockfd_udp, (struct sockaddr*)&server_addr_udp, sizeof(server_addr_udp));
    if (n < 0) {
        perror("[-]bind error");
        exit(1);
    }
    request prev=NULL;
    while(1){
    
    // printf("Checking for new servers!\n");
    request req = (request)malloc(sizeof(st_request));
    recvfrom(sockfd_udp,req, sizeof(st_request), 0, (struct sockaddr*)&client_addr_udp, &addr_size_udp);
    process(req);
    free(req);

    }
    close(sockfd_udp);
    return NULL;
}
