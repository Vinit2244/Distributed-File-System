#include "headers.h"

pthread_mutex_t server_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t send_buffer_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t send_signal = PTHREAD_COND_INITIALIZER;
pthread_mutex_t status_lock = PTHREAD_MUTEX_INITIALIZER;

int server_socket_tcp, client_socket_tcp;
struct sockaddr_in server_addr_tcp, client_addr_tcp;
socklen_t client_addr_len_tcp = sizeof(client_addr_tcp);

server_status connections[100];
int connection_count=0;

void *send_handler()
{

    // Currently work in progress

    printf("------------------------Send handler started-------------------------------\n");
    while (1)
    {

        pthread_mutex_lock(&send_buffer_lock);
        pthread_cond_wait(&send_signal, &send_buffer_lock);

        packet p = send_buffer[send_count - 1];

        if (p->status == 0)
        {

            int server_socket, client_socket;
            struct sockaddr_in server_addr, client_addr;
            socklen_t client_addr_len = sizeof(client_addr);

            server_socket = socket(AF_INET, SOCK_STREAM, 0);
            if (server_socket == -1)
            {
                perror("Socket creation failed");
                // exit(EXIT_FAILURE);
            }

            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(atoi(p->port));
            server_addr.sin_addr.s_addr = inet_addr(p->ip);

            if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
            {
                perror("Binding to port failed");
            }

            if (listen(server_socket, MAX_CONNECTIONS) == -1)
            {
                perror("Listening on port failed");
            }

            client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
            if (client_socket == -1)
            {
                perror("Accepting connection failed");
                continue;
            }

            if (send(client_socket, p->r, sizeof(p->r), 0) < 0)
            {
                perror("Send error");
            }

            close(client_socket);
            close(server_socket);
            p->status = 0;
        }

        pthread_mutex_unlock(&send_buffer_lock);
    }

    return NULL;
}

void *receive_handler()
{
    printf("-------------------------TCP handler started-------------------------------\n");

    server_socket_tcp = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_tcp == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr_tcp.sin_family = AF_INET;
    server_addr_tcp.sin_port = htons(NS_PORT);
    server_addr_tcp.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket_tcp, (struct sockaddr *)&server_addr_tcp, sizeof(server_addr_tcp)) == -1)
    {
        perror("Binding to port failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket_tcp, MAX_CONNECTIONS) == -1)
    {
        perror("Listening on port failed");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        client_socket_tcp = accept(server_socket_tcp, (struct sockaddr *)&client_addr_len_tcp, &client_addr_len_tcp);
        if (client_socket_tcp == -1)
        {
            perror("Accepting connection failed");
        }
        request req = (request)malloc(sizeof(st_request));
        int x=recv(client_socket_tcp,req,sizeof(st_request),0);
        printf("%s\n",req->data);
        // if(x>0){
        //     printf("%s\n",req->data);
        // }

        process(req);
        free(req);
        printf("Request served\n");
        close(client_socket_tcp);
        
    }

    close(server_socket_tcp);

    return NULL;
}

void *server_handler(void *p)
{

    ss pack = (ss)p;
    
    pack->server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (pack->server_socket == -1)
    {
        perror("Socket creation failed");
    }
    memset(&pack->server_addr, '\0', sizeof(pack->server_addr));
    pack->server_addr.sin_family = AF_INET;
    pack->server_addr.sin_port = htons(atoi(pack->port));
    pack->server_addr.sin_addr.s_addr = INADDR_ANY;


    int x=connect(pack->server_socket, (struct sockaddr *)&pack->server_addr, sizeof(pack->server_addr));
    if(x==0)printf("Connected to server succesfully!\n");
    
    while (1)
    {

        request r=(request)malloc(sizeof(st_request));
        r->request_type=PING;
        strcpy(r->data,"");
        send(pack->server_socket,r,sizeof(st_request),0);
        recv(pack->server_socket,r,sizeof(st_request),MSG_DONTWAIT);
        sleep(3);
        if(strcmp(r->data,"")==0){
            printf("Server %s disconnected!\n",pack->port);
            pthread_mutex_lock(&status_lock);
            for(int i=0;i<connection_count;i++){
                if(strcmp(pack->port,connections[i].port)==0 && connections[i].status==1){
                    connections[i].status=0;
                    break;
                }
            }
            pthread_mutex_unlock(&status_lock);
            break;
        }
        else{
            sleep(7);
        }
        free(r);


        //yet to write
    }
    close(pack->client_socket);
    close(pack->server_socket);
    pthread_exit(NULL);
}
