#include "headers.h"

// Writes onto the paths.txt file the current state of the accessible paths buffer regularly from time to time
void *store_filepaths(void *args)
{
    pthread_mutex_lock(&accessible_paths_mutex);
    while(1)
    {
        // Wait till there is some update done to the accessible paths 2D array and as soon as some thing is updated write it onto the text file
        pthread_cond_wait(&update_paths_txt_cond_var, &accessible_paths_mutex);

        FILE* fptr = fopen("paths.txt", "w");

        // First write the number of paths
        fprintf(fptr, "%d\n", num_of_paths_stored);
        
        // Then write all the paths (one in each line)
        for (int i = 0; i < num_of_paths_stored; i++)
        {
            fprintf(fptr, "%s\n", accessible_paths[i]);
        }

        fclose(fptr);
    }
    pthread_mutex_unlock(&accessible_paths_mutex);

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
    st_request request;
    memset(&(request.data), 0, MAX_DATA_LENGTH);

    int recvd_msg_size;
    if ((recvd_msg_size = recv(sock_fd, &request, sizeof(st_request), 0)) <= 0)
    {
        fprintf(stderr, RED("recv : %s\n"), strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Process request
    printf(BLUE("\nRequest received : %s\n"), request.data);

    // Send acknowledgement
    st_request ack_st;
    ack_st.request_type = ACK;
    // Write something on the data like the request id (unique to each request made and the success code/failure code/ or somthing to tell what has happened)
    strcpy(ack_st.data, "write the message here to be copied");
    
    int sent_msg_size;
    if ((sent_msg_size = send(sock_fd, &ack_st, sizeof(st_request), 0)) <= 0)
    {
        fprintf(stderr, RED("send : %s\n"), strerror(errno));
        exit(EXIT_FAILURE);
    }

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

// Send registration request
void *send_reg_req(void* args)
{
    st_request registration_request_st;
    registration_request_st.request_type = REGISTRATION_REQUEST;
    memset(registration_request_st.data, 0, MAX_DATA_LENGTH);

    // Creating paths string array to be 1000 less than the max size because the data would also contain ip and ports info so not all space is used by the paths
    char paths_string[MAX_DATA_LENGTH - 1000] = {0};

    pthread_mutex_lock(&accessible_paths_mutex);
    for (int i = 0; i < num_of_paths_stored; i++)
    {
        // Attach the path to the end of the string
        strcat(paths_string, accessible_paths[i]);

        // If the current path is not the last one then attach a pipe '|' after the current path
        if (i != num_of_paths_stored - 1)
        {
            strcat(paths_string, "|");
        }
    }
    pthread_mutex_unlock(&accessible_paths_mutex);

    // Printing my SS details like port number and ip address and all the paths that are stored currently with me on the query
    sprintf(registration_request_st.data, "%d|%s|%d|%d|%d|%s", MY_SS_ID, MY_IP, MY_CLIENT_PORT_NO, MY_NFS_PORT_NO, num_of_paths_stored, paths_string); // <My ss_id>|<My ip>|<client port>|<nfs port>|<no of paths>|[<paths>]

    // Sending the registration request until acknowledgement is not received
    while (nfs_registrations_status == NOT_REGISTERED)
    {
        int sent_msg_size;
        // printf("%d\n",sizeof(&registration_request_st))
        if ((sent_msg_size = sendto(socket_fd, (request)&registration_request_st, sizeof(st_request), 0, (struct sockaddr *)&address, sizeof(address))) < 0)
        {
            fprintf(stderr, RED("sendto : %s\n"), strerror(errno));
            exit(1);
        }
        usleep(10000); // Try again every 10 miliseconds
    }

    return NULL;
}

// Receive registration acknowledgement
void *receive_reg_ack(void* args)
{
    // Receiving registration acknowledgement
    st_request registration_ack_st;
    while (1)
    {
        memset(&registration_ack_st, 0, MAX_DATA_LENGTH);

        int msg_size_recvd;
        if ((msg_size_recvd = recvfrom(socket_fd, &registration_ack_st,sizeof(st_request),0,(struct sockaddr*)&address,&addr_size)) <= 0)
        {
            fprintf(stderr, RED("recv : %s\n"), strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (registration_ack_st.request_type == REGISTRATION_ACK)
        {
            printf(GREEN("Registratin acknowledgement received!\n"));
            nfs_registrations_status = REGISTERED;
            break;
        }
    }

    // Closing NFS socket
    if (close(socket_fd) < 0)
    {
        fprintf(stderr, RED("close : failed to close the socket!\n"));
        exit(EXIT_FAILURE);
    }

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

