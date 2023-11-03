#include "headers.h"

// Periodically keeps checking for all the file paths and if some new file path is created/deleted then immediately signals the NFS to add/delete that path
void* check_and_store_filepaths(void* args)
{
    while (1)
    {
        pthread_mutex_lock(&accessible_paths_mutex);
        
        char base_dir_path[MAX_PATH_LEN] = {0};
        sprintf(base_dir_path, "%s/storage", PWD);

        // Linked list to store the paths found as we don't know in advance how many paths will be found
        linked_list_head paths = create_linked_list_head();

        // Searching the SS_test_dir recursively to obtain the absolute paths of all the files
        seek(base_dir_path, paths);

        // Print the number of paths found for debugging
        printf("\nChecked filepaths : %d\n", paths->number_of_nodes);

        // Number of paths found
        int num_paths_found = paths->number_of_nodes;

        // Storing the copy of accessible paths array to match with the files found
        char** accessible_paths_copy = (char**) malloc(MAX_FILES * sizeof(char*));

        for (int i = 0; i < MAX_FILES; i++)
        {
            accessible_paths_copy[i] = NULL;
        }
        for (int k = 0; k < num_of_paths_stored; k++)
        {
            accessible_paths_copy[k] = calloc(MAX_PATH_LEN, sizeof(char));
            strcpy(accessible_paths_copy[k], accessible_paths[k]);
        }

        // Storing relative paths of the files found in the found_paths array
        char** found_paths = (char**) malloc(num_paths_found * sizeof(char*));
        // Storing a copy of found_paths as well so after comparing if there are some new paths or some paths are deleted then we can create a new accessible paths array
        char** found_paths_copy = (char**) malloc(paths->number_of_nodes * sizeof(char*));

        linked_list_node n = paths->first;
        int idx = 0;
        while (n != NULL)
        {
            found_paths[idx] = (char*) calloc(MAX_PATH_LEN, sizeof(char));
            found_paths_copy[idx] = (char*) calloc(MAX_PATH_LEN, sizeof(char));
            strcpy(found_paths_copy[idx], ".");
            strcat(found_paths_copy[idx], &n->path[strlen(PWD)]);
            strcpy(found_paths[idx], ".");
            strcat(found_paths[idx++], &n->path[strlen(PWD)]);
            n = n->next;
        }
        // Have copied all the found paths in the array so now we can free the linked list
        free_linked_list(paths);

        // Now go and match each paths in found_paths and accessible_paths
        for (int k = 0; k < num_paths_found; k++)
        {
            char* curr_found_path = found_paths[k];
            for (int j = 0; j < num_of_paths_stored; j++)
            {
                char* curr_accessible_path = accessible_paths_copy[j];
                if (curr_found_path != NULL && curr_accessible_path != NULL)
                {
                    if (strcmp(curr_found_path, curr_accessible_path) == 0)
                    {
                        free(found_paths[k]);
                        free(accessible_paths_copy[j]);
                        found_paths[k] = NULL;
                        accessible_paths_copy[j] = NULL;
                        break;
                    }
                }
            }
        }

        // Now all the not null paths in found_paths are the paths that are newly added while all the not null paths in accessible paths copy are deleted paths
        // Checking for new paths
        int num_new_paths = 0;
        char new_paths[MAX_DATA_LENGTH - 1000] = {0};
        for (int i = 0; i < num_paths_found; i++)
        {
            if (found_paths[i] != NULL)
            {
                strcat(new_paths, found_paths[i]);
                strcat(new_paths, "|");
            }
        }
        // Removing the last | from the concatenation of paths
        if (strlen(new_paths) > 0)
        {
            new_paths[strlen(new_paths) - 1] = '\0';
            send_update_paths_request(ADD_PATHS, new_paths);
        }

        // Checking for deleted paths
        char deleted_paths[MAX_DATA_LENGTH - 1000] = {0};
        for (int i = 0; i < num_of_paths_stored; i++)
        {
            if (accessible_paths_copy[i] != NULL)
            {
                strcat(deleted_paths, accessible_paths_copy[i]);
                strcat(deleted_paths, "|");
            }
        }
        // Removing the last | from the concatenation of paths
        if (strlen(deleted_paths) > 0)
        {
            deleted_paths[strlen(deleted_paths) - 1] = '\0';
            send_update_paths_request(DELETE_PATHS, deleted_paths);
        }

        // Freeing all the memory allocated except the found paths copy array
        for (int i = 0; i < num_paths_found; i++)
        {
            if (found_paths[i] != NULL)
            {
                free(found_paths[i]);
                found_paths[i] = NULL;
            }
        }
        free(found_paths);

        for (int i = 0; i < num_of_paths_stored; i++)
        {
            if (accessible_paths_copy[i] != NULL)
            {
                free(accessible_paths_copy[i]);
                accessible_paths_copy[i] = NULL;
            }
        }
        free(accessible_paths_copy);

        // Copying all the found paths into the accessible paths array 
        for (int i = 0; i < num_paths_found; i++)
        {
            memset(accessible_paths[i], 0, MAX_PATH_LEN);
            strcpy(accessible_paths[i], found_paths_copy[i]);
            free(found_paths_copy[i]);
        }

        num_of_paths_stored = num_paths_found;
        free(found_paths_copy);

        pthread_mutex_unlock(&accessible_paths_mutex);

        // Keep checking every 5 seconds
        sleep(5);
    }
    
    return NULL;
}

// Processes the request allocated to it in the allocated thread and then returns
void* serve_request(void* args)
{
    st_thread_data meta_data = *((thread_data) args);
    int sock_fd = meta_data.client_sock_fd;             // Socket id for communicating with the node which has sent the request
    int thread_index = meta_data.thread_idx;            // Index of the thread on which this is running

    // Freeing arguments as all the information is extracted
    free(args);

    // Accepting the request
    st_request recvd_request;

    memset(&(recvd_request.data), 0, MAX_DATA_LENGTH);

    // Receiving the request
    int recvd_msg_size;
    if ((recvd_msg_size = recv(sock_fd, &recvd_request, sizeof(st_request), 0)) <= 0)
    {
        fprintf(stderr, RED("recv : %s\n"), strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Process request
    printf(BLUE("\nRequest received : %s\n"), recvd_request.data);
    char** request_tkns = tokenize(recvd_request.data, '|');
    /*
        READ data format : <path>
        WRITE data format : <path>|<content to write> (Keep sending write data in this format and when all the data to be written is sent send the stop request)
        APPEND data format : same as write
    */

    // Selecting the type of request sent
    if (recvd_request.request_type == READ_REQ)
    {
        printf("Read request received\n");
        // Read the data onto the specified file break it in parts and send each part in chunks followed by stop request at last
        char* path_to_read = request_tkns[0];
        
        FILE* fptr;
        fptr = fopen(path_to_read, "r");

        st_request send_read_data;
        send_read_data.request_type = READ_REQ_DATA;
        memset(send_read_data.data, 0, MAX_DATA_LENGTH);

        int bytes_read = fread(send_read_data.data, 1, MAX_DATA_LENGTH, fptr);

        int sent_msg_size;
        if ((sent_msg_size = send(sock_fd, (request) &send_read_data, sizeof(st_request), 0)) <= 0)
        {
            fprintf(stderr, RED("send : %s\n"), strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    else if (recvd_request.request_type == WRITE_REQ)
    {
        char* path_to_write = request_tkns[0];
        char* data_to_write = request_tkns[1];

        FILE* fptr;
        fptr = fopen(path_to_write, "w");
        fprintf(fptr, "%s", data_to_write);
        fclose(fptr);

        send_ack(WRITE_SUCCESSFUL, sock_fd);
    }
    else if (recvd_request.request_type == APPEND_REQ)
    {
        char* path_to_write = request_tkns[0];
        char* data_to_write = request_tkns[1];

        FILE* fptr;
        fptr = fopen(path_to_write, "a");
        fprintf(fptr, "%s", data_to_write);
        fclose(fptr);

        send_ack(APPEND_SUCCESSFUL, sock_fd);
    }

    free_tokens(request_tkns);

    // Closing client socket as all the communication is done
    if (close(sock_fd) < 0) {
        fprintf(stderr, RED("close : failed to close the client socket!\n"));
        exit(EXIT_FAILURE);
    }

    // Freeing thread slot
    pthread_mutex_lock(&threads_arr_mutex);
    thread_slot_empty_arr[thread_index] = 0;
    pthread_mutex_unlock(&threads_arr_mutex);

    return NULL;
}

// This function starts and binds to the port which listens for communication with NFS
void* start_nfs_port(void* args)
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
    
    printf(GREEN("Started listening to NFS requests.\n"));

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

    return NULL;
}

// This function binds to the port listening for client requests and starts listening for requests made by clients (SS acts as TCP server)
void* start_client_port(void* args)
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

    printf(GREEN("Started listening to Client requests.\n"));

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

    return NULL;
}

