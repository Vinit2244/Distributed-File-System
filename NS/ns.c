#include "headers.h"

int curr_cache_write_index;
cache_array cache = NULL;
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

// Handles Ctrl + Z signal and prints all the log when ctrl+z is pressed
void handleCtrlZ(int signum)
{
    // Print all the logs

    FILE *fptr = fopen("logs.txt", "r");
    if (fptr == NULL)
    {
        fprintf(stderr, RED("fopen : %s\n"), strerror(errno));
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE] = {0};

    printf(YELLOW_COLOR);
    printf("\n");
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, fptr)) > 0)
    {
        // Print content out on the screen
        fwrite(buffer, 1, bytesRead, stdout);
    }
    printf(RESET_COLOR);
    printf("\n");

    fclose(fptr);
}

// First argument should be whether the communication was with storage server or client (SS or CLIENT)
// Second argument is storage server id with which the communication is happening, if communication is with a client and not a storage server then by default pass value 0 there
// Third argument should be the port number of client or storage server that is involved in the communication whose log is being inserted
// Fourth argument is the request type
// Fifth argument is the request data
// Sixth argumnet is the status code of the request processed (Yet to implement like error codes like path found or path not found or successfully completed or not successfully completed etc)
// This function returns 1 if log is successfully logged or else 0
int insert_log(const int type, const int ss_id, const int ss_or_client_port, const int request_type, const char* request_data, const int status_code)
{
    FILE* fptr = fopen("logs.txt" , "a");
    if (fptr == NULL)
    {
        fprintf(stderr, RED("fopen : %s\n"), strerror(errno));
        return 0;
    }

    if (type == SS)
    {
        fprintf(fptr, "Communicating with Storage Server : %d\n", ss_id);
        fprintf(fptr, "NFS Port number                   : %d\n", NS_PORT);
        fprintf(fptr, "Storage Server Port number        : %d\n", ss_or_client_port);
        fprintf(fptr, "Request type                      : %d\n", request_type);
        fprintf(fptr, "Request data                      : %s\n", request_data);
        fprintf(fptr, "Status                            : %d\n", status_code);
        fprintf(fptr, "\n");
    }
    else
    {
        fprintf(fptr, "Communicating with Client\n");
        fprintf(fptr, "NFS Port number                   : %d\n", NS_PORT);
        fprintf(fptr, "Client Port number                : %d\n", ss_or_client_port);
        fprintf(fptr, "Request type                      : %d\n", request_type);
        fprintf(fptr, "Request data                      : %s\n", request_data);
        fprintf(fptr, "Status                            : %d\n", status_code);
        fprintf(fptr, "\n");
    }

    fclose(fptr);

    return 1;
}

int main()
{
    // Initialise the cache
    init_cache();

    // Initialising signal handlers
    // Handling Ctrl + z (SIGTSTP) signal to print the logging (book keeping) output
    struct sigaction sa;
    sa.sa_handler = &handleCtrlZ;    // Ctrl + Z (Windows/Linux/Mac)
    sa.sa_flags = SA_RESTART;        // Automatically restart the system call
    sigaction(SIGTSTP, &sa, NULL);   // Prints the log output  onto the screen

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
    
    while(1) {}

    return 0;
}
