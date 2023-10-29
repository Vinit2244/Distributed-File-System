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

// Thread which keeps sending data to Clients/NFS
void* send_data_nfs(void* args)
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

        if (i != num_of_paths_stored - 1)
        {   // If the current path is not the last one then attach a pipe '|' after the current path
            strcat(paths_string, "|");
        }
    }
    pthread_mutex_unlock(&accessible_paths_mutex);

    // Printing my SS details like port number and ip address and all the paths that are stored currently in me on the query
    sprintf(registration_request_st.data, "%s|%d|%d|%d|%s", MY_IP, MY_CLIENT_PORT_NO, MY_NFS_PORT_NO, num_of_paths_stored, paths_string);  // <My ip>|<client port>|<nfs port>|<no of paths>|[<paths>]
    
    // Sending the registration request
    int sent_msg_size;
    while (nfs_registrations_status == NOT_REGISTERED)
    {
        if ((sent_msg_size = send_to(nfs_socket_fd, (request) &registration_request_st, sizeof(st_request), 0, (struct sockaddr *)&nfs_address, sizeof(nfs_address))) < 0)
        {
            fprintf(stderr, RED("sendto : %s\n"), strerror(errno));
            exit(1);
        }
        usleep(100000); // Sleep for 10 miliseconds before trying again
    }

    // Our server has been registered with NFS

    return NULL;
}

// Thread which keeps receiving data/acknowledgement from Clients/NFS
void* receive_data_nfs(void* args)
{
    int nfs_server_address_size = sizeof(nfs_address);

    int recvd_msg_size;
    while (1)
    {
        st_request request_recvd;
        memset(request_recvd.data, 0, MAX_DATA_LENGTH);

        if ((recvd_msg_size = recvfrom(nfs_socket_fd, (request) &request_recvd, sizeof(st_request), 0, (struct sockaddr *) &nfs_address, (unsigned int*) &nfs_server_address_size)) < 0)
        {
            fprintf(stderr, RED("recvfrom : %s\n"), strerror(errno));
            exit(1);
        }

        // If registration acknowledgement is received
        if (request_recvd.request_type == REGISTRATION_ACK)
        {
            // If the registration was successfull
            if (strcmp(request_recvd.data, "successfull") == 0)
            {
                nfs_registrations_status = REGISTERED;
            }
        }
    }

    return NULL;
}

// Processes clients request one by one and sends response to clients
void* send_data_client(void* args)
{
    while (1)
    {
        pthread_mutex_lock(&pending_requests_mutex);
        // Checking if some request is pending or not
        if (num_of_pending_requests == 0)
        {
            // If no requests are present to be served then just skip and move on
            pthread_mutex_unlock(&pending_requests_mutex);
        }
        else
        {
            // There are pending requests to be served
            pending_request_node req = (pending_request_node) malloc(sizeof(st_pending_request_node));
            *req = pending_requests_buffer[read_head_idx_pending_requests_buffer];

            read_head_idx_pending_requests_buffer = (read_head_idx_pending_requests_buffer + 1) % MAX_PENDING;
            num_of_pending_requests--;

            pthread_mutex_unlock(&pending_requests_mutex);

            pthread_mutex_lock(&threads_arr_mutex);

            for (int i = 0; i < MAX_PENDING; i++)
            {
                if (thread_slot_empty_arr[i] == 0)
                {
                    req->idx_thread_alloted = i;
                    thread_slot_empty_arr[i] = 1;
                    pthread_create(&requests_serving_threads_arr[i], NULL, &server_request, req);
                }
            }

            pthread_mutex_unlock(&threads_arr_mutex);
        }
    }
    return NULL;
}

// Keeps receiving data requests from clients
void* receive_data_client(void* args)
{
    int ss_socket_fd = *((int*) args);
    int addr_size = sizeof(struct sockaddr_in);
    int data_len_recvd;
    while (1)
    {
        // Receiving client information and its request
        struct sockaddr_in client_addr;
        
        int client_socket_fd = accept(ss_socket_fd, (struct sockaddr *)&client_addr, (socklen_t *)&addr_size);
        if (client_socket_fd == -1)
        {
            fprintf(stderr, RED("accept : %s\n"), strerror(errno));
            exit(EXIT_FAILURE);
        }

        st_request recvd_request;

        if ((data_len_recvd = recv(client_socket_fd, &recvd_request, sizeof(st_request), 0)) < 0)
        {
            fprintf(stderr, RED("recv : %s\n"), strerror(errno));
            exit(EXIT_FAILURE);
        }

        // Storing the request onto the buffer
        // If the buffer is full don't store and just ignore the request
        pthread_mutex_lock(&pending_requests_mutex);
        if (num_of_pending_requests == MAX_PENDING)
        {
            // Ignore the request and move on
        }
        else
        {
            memcpy(&pending_requests_buffer[write_head_idx_pending_requests_buffer].client_socket_fd, &client_socket_fd, sizeof(int));
            memcpy(&pending_requests_buffer[write_head_idx_pending_requests_buffer].recvd_request, &recvd_request, sizeof(recvd_request));
            write_head_idx_pending_requests_buffer = (write_head_idx_pending_requests_buffer + 1) % MAX_PENDING;
            num_of_pending_requests++;
        }
        pthread_mutex_unlock(&pending_requests_mutex);
    }
    
    free(args);
    return NULL;
}

// Processes the request allocated to it in the allocated thread and then returns
void* server_request(void* args)
{
    pending_request_node req = (pending_request_node)args;

    int client_socket_fd = req->client_socket_fd;
    st_request recvd_request = req->recvd_request;
    int thread_idx = req->idx_thread_alloted;

    st_request data_to_send_to_client;

    /*=======================================================================================*/
    // Process the request to get the relevant data (Code this part to process the request)
    /*=======================================================================================*/

    // Sending processed data
    int result = send_to(client_socket_fd, data_to_send_to_client, sizeof(st_request), 0);

    // If there is some error in sending the data
    if (result == 0)
    {
        fprintf(stderr, RED("send : %s\n"), strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Close the connection with this client once the request is served
    if (close(client_socket_fd) < 0)
    {
        fprintf(stderr, RED("close : failed to close the client socket!\n"));
        exit(EXIT_FAILURE);
    }
    free(args);

    // Once this request is served the the thread_slot which was being used for serving this request becomes free
    pthread_mutex_lock(&threads_arr_mutex);
    thread_slot_empty_arr[thread_idx] = 0;
    pthread_mutex_unlock(&threads_arr_mutex);

    return NULL;
}
