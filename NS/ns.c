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


// Code to add a new storage server in naming server list
void init_storage(char data[])
{
    // tokenise the string and create a new server object with extracted attributes

    
    char** tokens = processstring(data, 4);
    ss new_ss = (ss)malloc(sizeof(ss_info));
    strcpy(new_ss->ip, tokens[1]);
    strcpy(new_ss->port, tokens[3]);
    strcpy(new_ss->client_port, tokens[2]);
    new_ss->path_count = 0;
    
    pthread_mutex_init(&new_ss->lock, NULL);

    pthread_mutex_lock(&server_lock);

    int check_flag=0;
    int id=-1;
    for(int i=0;i<server_count;i++){
        if(strcmp(ss_list[i]->port,new_ss->port)==0){
            check_flag=1;
            id=i;
            break;
        }
    }
    if(check_flag==1){


        printf(GREEN("%s is back online!\n"),new_ss->port);
        
        new_ss->status=1;


        ss_list[id]->status=1;
        ss_list[id]->path_count=0;
        
           
    }
    else{

    new_ss->backup_path_count=0;

    new_ss->is_backedup=0;
    new_ss->has_backup=0;
    ss_list[server_count] = new_ss;
    server_count++;
    id=server_count-1;
    
    }
    pthread_mutex_unlock(&server_lock);
    
    pthread_t server_thread;
    pthread_create(&server_thread, NULL, &server_handler, (void *)ss_list[id]);
    pthread_t sync_backup_thread;
    pthread_create(&sync_backup_thread,NULL,&sync_backup,(void*)ss_list[id]);
    
    
    return;
}

int main()
{

    init_nfs(); // initialises ns server

    // declaring thread variables
    
    pthread_t receive_thread;
    pthread_t backup_thread_idx;

    // TCP socket to check for new requests

    // constructing threads for listening to TCP sockets
    
    pthread_create(&receive_thread, NULL, &receive_handler, NULL);
    pthread_create(&backup_thread_idx, NULL, &backup_thread, NULL);
    // joining threads
    pthread_join(receive_thread, NULL);
    pthread_join(backup_thread_idx, NULL);
    
    while(1){}

    return 0;
}
