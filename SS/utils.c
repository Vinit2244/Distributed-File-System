#include "headers.h"

// Reads all the paths accessible to the server (which are stored in the file paths.txt and stores them in cached buffer in the program 2D array)
void accessible_paths_init(void)
{
    // First allocating memory to the file paths
    accessible_paths = (char **)calloc(MAX_FILES, sizeof(char *)); // Memory to individual paths will be allocated when the path is stored

    pthread_mutex_lock(&accessible_paths_mutex);
    FILE *fptr;

    if ((fptr = fopen("paths.txt", "r")) == NULL)
    {
        // File do not exist yet so we need to create the file
        fptr = fopen("paths.txt", "w");

        if (fptr == NULL) // Error in opening file in write mode
            fprintf(stderr, RED("fopen : %s\n"), strerror(errno));
        else
        {
            // Write 0 in the file on the initial line as no path is stored and the file is opened for the first time
            fprintf(fptr, "%d\n", 0);
            fclose(fptr);
        }
    }
    else
    {
        // File already exists so we just need to read the already stored paths
        int num_of_paths;
        fscanf(fptr, "%d", &num_of_paths); // Reading the number of paths that is stored on the first line in the file

        // Reading all the paths that is stored afterwards and storing it in the accessible paths 2D array
        for (int i = 0; i < num_of_paths; i++)
        {
            char path[1024] = {0};
            fscanf(fptr, "%s", path);

            accessible_paths[num_of_paths_stored] = (char *)calloc(MAX_PATH_LEN, sizeof(char));
            strcpy(accessible_paths[num_of_paths_stored++], path);
        }
        fclose(fptr);
    }
    pthread_mutex_unlock(&accessible_paths_mutex);

    return;
}

// This function tokenises the provided string on given character and returns a 2D character array broken at ch
char **tokenize(const char *str, const char ch)
{
    // Counting the number of delimiters
    int num_of_ch = 0;
    for (int i = 0; i < strlen(str); i++)
    {
        if (str[i] == ch)
            num_of_ch++;
    }

    // Number of tokens would be 1 more than the number of delimiters present in the string
    int num_of_tokens = num_of_ch + 1;

    // Allocating num_of_tokens + 1 memory because we need to store last token as NULL token to mark the end of tokens
    char **tokens = (char **)malloc((num_of_tokens + 1) * sizeof(char *));
    for (int i = 0; i < num_of_tokens; i++)
    {
        tokens[i] = (char *)calloc(MAX_PATH_LEN, sizeof(char));
    }
    // The last token will be kept null so that when traversing we would know when the tokens end by checking for NULL token
    tokens[num_of_tokens] = NULL;

    int token_idx = 0;     // Index of the token being stored
    int token_str_idx = 0; // Index where the next character is to be stored on token
    for (int i = 0; i < strlen(str); i++)
    {
        // If the delimiter character is encountered increment the token index by 1 to start storing the next token and reset the token string index to 0 to start storing from the starting of the string
        if (str[i] == ch)
        {
            token_idx++;
            token_str_idx = 0;
            continue;
        }
        else
        {
            tokens[token_idx][token_str_idx++] = str[i];
        }
    }

    return tokens;
}

// Frees the memory allocated to the 2D tokens array
void free_tokens(char **tokens)
{
    // Looping through all the tokens untill the NULL token is encountered which marks the end of the tokens array
    int i = 0;
    while (tokens[i] != NULL)
    {
        free(tokens[i]);
        i++;
    }
    free(tokens);
    return;
}

// This function first adds the path to the accessible array if there is space available and then signals the update thread to update the paths.txt file with the latest modifications
void add_path(const char *path)
{
    pthread_mutex_lock(&accessible_paths_mutex);
    if (num_of_paths_stored == MAX_FILES) // Checking if the storage limit is full or not
    {
        printf(RED("You cannot add new files. File limit exceeded.\n"));
    }
    else
    {
        // Allocating memory before copying the path
        accessible_paths[num_of_paths_stored] = (char *)calloc(MAX_PATH_LEN, sizeof(char));
        // Copying the path in the correct location
        strcpy(accessible_paths[num_of_paths_stored], path);
        num_of_paths_stored++;
        // Signalling to update the paths.txt
        pthread_cond_signal(&update_paths_txt_cond_var);
    }
    pthread_mutex_unlock(&accessible_paths_mutex);
    return;
}

// This function removes the specified path from the accessible paths array
void remove_path(const char *path)
{
    pthread_mutex_lock(&accessible_paths_mutex);
    int flag = 1; // Flag to keep track of whether the specified path was found or not
    // Looping through all the paths to find required path
    for (int i = 0; i < num_of_paths_stored; i++)
    {
        // If required path is found
        if (strcmp(accessible_paths[i], path) == 0)
        {
            // Freeing the memory allocated to that path
            free(accessible_paths[i]);
            // Shifting all the remaining paths ahead of this path back by one place
            for (int j = i + 1; j < num_of_paths_stored; j++)
            {
                accessible_paths[j - 1] = accessible_paths[j];
            }
            accessible_paths[num_of_paths_stored - 1] = NULL;
            num_of_paths_stored--;
            flag = 0;                                        // Path was found
            pthread_cond_signal(&update_paths_txt_cond_var); // Signalling update thread to update the contents in the paths.txt file
            break;
        }
    }
    // If specified path was not found
    if (flag)
    {
        printf(RED("No such path available.\n"));
    }
    pthread_mutex_unlock(&accessible_paths_mutex);
    return;
}

// Registers my SS with NFS using UDP uses two threads one for sending and other for receiving acknowledgement
void register_ss(void)
{
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0)
    { 
        // Some error occured while creating socket
        fprintf(stderr, RED("socket : %s"), strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Stores address information about the client side address
    memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_port = htons(NFS_SERVER_PORT_NO); // port on which server side process is listening

    if (inet_pton(AF_INET, NFS_IP, &address.sin_addr.s_addr) <= 0)
    { 
        // inet presentation to network - converts the ip string to unsigned integer
        fprintf(stderr, RED("inet_pton : %s\n"), strerror(errno));
        return 0;
    }

    // Two threads one to send the registration request and another to accept the registration request because we are using UDP socket
    // Registration request sending and registration acknowledgement receiving thread
    pthread_t reg_req_sending_thread, reg_ack_receiving_thread;

    pthread_create(&reg_req_sending_thread, NULL, &send_reg_req, NULL);
    pthread_create(&reg_ack_receiving_thread, NULL, &receive_reg_ack, NULL);

    pthread_join(reg_req_sending_thread, NULL);
    pthread_join(reg_ack_receiving_thread, NULL);

    return;
}

// This function starts and binds to the port which listens for communication with NFS
void start_nfs_port(void)
{
    memset(&ss_address_nfs, 0, sizeof(ss_address_nfs));

    // This socket id is never used for sending/receiving data, used by server just to get new sockets (clients)
    nfs_server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (nfs_server_socket_fd < 0)
    {
        // Some error occured while creating socket
        fprintf(stderr, RED("socket : %s\n"), strerror(errno));
        exit(EXIT_FAILURE);
    }

    ss_address_nfs.sin_family = AF_INET;
    ss_address_nfs.sin_port = htons(MY_NFS_PORT_NO); // My port on which I am listening to NFS communication
    ss_address_nfs.sin_addr.s_addr = htonl(INADDR_ANY);

    // Binding to the port
    if (bind(nfs_server_socket_fd, (struct sockaddr *)&ss_address_nfs, sizeof(ss_address_nfs)) == -1)
    {
        fprintf(stderr, RED("bind : %s\n"), strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Listening for incoming requests for communication
    if (listen(nfs_server_socket_fd, MAX_PENDING) == -1)
    {
        fprintf(stderr, RED("listen : %s\n"), strerror(errno));
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        // Keep accepting incoming requests
        // Accepting NFS requests
        struct sockaddr_in nfs_addr;
        int addr_size = sizeof(struct sockaddr_in);
        int nfs_sock_fd = accept(nfs_server_socket_fd, (struct sockaddr *)&nfs_addr, (socklen_t *)&addr_size);
        if (nfs_sock_fd == -1)
        {
            fprintf(stderr, RED("accept : %s\n"), strerror(errno));
            exit(EXIT_FAILURE);
        }

        pthread_mutex_lock(&threads_arr_mutex);
        for (int i = 0; i < MAX_PENDING; i++)
        {
            if (thread_slot_empty_arr[i] == 0)
            {
                thread_slot_empty_arr[i] = 1;
                thread_data args = (thread_data)malloc(sizeof(st_thread_data));
                args->client_sock_fd = nfs_sock_fd;
                args->thread_idx = i;

                // If this thread is still running then first wait for it to complete, it is about to complete as it has already made it's slot empty to 0
                pthread_join(requests_serving_threads_arr[i], NULL);

                // Create a new thread on the same position
                pthread_create(&requests_serving_threads_arr[i], NULL, &serve_request, args);
                break;
            }
        }
        pthread_mutex_unlock(&threads_arr_mutex);
    }

    if (close(nfs_server_socket_fd) < 0)
    {
        fprintf(stderr, RED("close : failed to close the server socket!\n"));
        exit(EXIT_FAILURE);
    }

    return;
}

// This function binds to the port listening for client requests and starts listening for requests made by clients (SS acts as TCP server)
void start_client_port(void)
{
    memset(&ss_address_client, 0, sizeof(ss_address_client));

    // This socket id is never used for sending/receiving data, used by server just to get new sockets (clients)
    client_server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_server_socket_fd < 0)
    {
        // Some error occured while creating socket
        fprintf(stderr, RED("socket : %s\n"), strerror(errno));
        exit(EXIT_FAILURE);
    }

    ss_address_client.sin_family = AF_INET;
    ss_address_client.sin_port = htons(MY_CLIENT_PORT_NO); // My port on which I am listening to Client communication
    ss_address_client.sin_addr.s_addr = htonl(INADDR_ANY);

    // Binding to the port
    if (bind(client_server_socket_fd, (struct sockaddr *)&ss_address_client, sizeof(ss_address_client)) == -1)
    {
        // Error while binding to the port
        fprintf(stderr, RED("bind : %s\n"), strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Listening for incoming requests for communication
    if (listen(client_server_socket_fd, MAX_PENDING) == -1)
    {
        fprintf(stderr, RED("listen : %s\n"), strerror(errno));
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        // Keep accepting incoming requests
        // Accepting client requests
        struct sockaddr_in client_addr;
        int addr_size = sizeof(struct sockaddr_in);

        // This socket is used for communication with the particular client
        int client_socket_fd = accept(client_server_socket_fd, (struct sockaddr *)&client_addr, (socklen_t *)&addr_size);
        if (client_socket_fd == -1)
        {
            fprintf(stderr, RED("accept : %s\n"), strerror(errno));
            exit(EXIT_FAILURE);
        }

        // If the threads array is full then the request won't be served and just ignored => No ack will be sent for that request so the NFS should request the same again if ack is not received in some time
        pthread_mutex_lock(&threads_arr_mutex);
        for (int i = 0; i < MAX_PENDING; i++)
        {
            if (thread_slot_empty_arr[i] == 0)
            {
                // This thread slot will now get busy so setting it's value to 1 (busy)
                thread_slot_empty_arr[i] = 1;
                // Storing the data that is to be passed to the thread in a struct
                thread_data args = (thread_data)malloc(sizeof(st_thread_data));
                args->client_sock_fd = client_socket_fd;
                args->thread_idx = i;

                // If this thread is still running then first wait for it to complete, it is about to complete as it has already made it's slot empty to 0
                pthread_join(requests_serving_threads_arr[i], NULL);

                // Create a new thread on the same position
                pthread_create(&requests_serving_threads_arr[i], NULL, &serve_request, args);
                break;
            }
        }
        pthread_mutex_unlock(&threads_arr_mutex);
    }

    // Close the server
    if (close(client_server_socket_fd) < 0)
    {
        // Error while closing the socket
        fprintf(stderr, RED("close : failed to close the server socket!\n"));
        exit(EXIT_FAILURE);
    }

    return;
}

