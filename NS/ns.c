#include "headers.h"

ss ss_list[100];         // List of all storage servers
int server_count = 0;    // Number of storage servers
packet send_buffer[100]; // Buffer to store packets to be sent
int send_count = 0;      // Number of packets in buffer                           // Size of UDP address

// Helper function to split string into tokens (n tokens)
char **processstring(char data[], int n)
{

    char **tokens = (char **)malloc(sizeof(char *) * n);
    for (int i = 0; i < n; i++)
    {
        tokens[i] = (char *)malloc(sizeof(char) * MAX_DATA_LENGTH);
    }

    int i = 0;
    char *token = strtok(data, "|");
    int j = 0;
    while (token != NULL && j < n)
    {
        j++;
        strcpy(tokens[i], token);
        token = strtok(NULL, "|");
        i++;
    }

    return tokens;
}

// Code to initialise nfs
void init_nfs()
{

    // nothing as of now but any global pointers declared will be malloced here

    return;
}

// Client requests handled here
void client_handler(char data[])
{

    // yet to work on based on client request format

    char **tokens = (char **)malloc(sizeof(char *) * 3);
    tokens = processstring(data, 3);
    char *path = (char *)malloc(sizeof(char) * MAX_DATA_LENGTH);
    strcpy(path, tokens[0]);
}

void make_backup(){

    

}
// Code to add a new storage server in naming server list
void init_storage(char data[])
{
    // tokenise the string and create a new server object with extracted attributes



    char **tokens = (char **)malloc(sizeof(char *) * 5);

    for (int i = 0; i < 5; i++)
    {
        tokens[i] = (char *)malloc(sizeof(char) * MAX_DATA_LENGTH);
    }

    tokens = processstring(data, 4);
    ss new_ss = (ss)malloc(sizeof(ss_info));

    strcpy(new_ss->ip, tokens[1]);
    strcpy(new_ss->port, tokens[3]);
    strcpy(new_ss->client_port, tokens[2]);
    new_ss->path_count = 0;
    pthread_mutex_init(&new_ss->lock, NULL);
    // printf("%s\n",new_ss->port);
    // printf("hi\n");

    pthread_mutex_lock(&server_lock);
    ss_list[server_count] = new_ss;
    server_count++;
    pthread_mutex_unlock(&server_lock);

    pthread_t server_thread;
    pthread_create(&server_thread, NULL, &server_handler, (void *)new_ss);

    int backup_flag=0;
    pthread_mutex_lock(&status_lock);

    for(int i=0;i<connection_count;i++){
        if(strcmp(connections[i].port,tokens[3])==0){
            backup_flag=1;
            break;
        }
    }

    if(backup_flag==0)make_backup();

    pthread_mutex_unlock(&status_lock);
    return;
}

int main()
{

    init_nfs(); // initialises ns server

    // declaring thread variables
    pthread_t send_thread;
    pthread_t receive_thread;
    // pthread_t udp_thread;

    // TCP socket to check for new requests

    // constructing threads for listening to TCP sockets
    pthread_create(&send_thread, NULL, &send_handler, NULL);
    pthread_create(&receive_thread, NULL, &receive_handler, NULL);

    // joining threads
    pthread_join(send_thread, NULL);
    pthread_join(receive_thread, NULL);
    // pthread_join(udp_thread, NULL);

    return 0;
}
