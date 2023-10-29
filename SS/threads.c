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
    int sock_fd = meta_data.client_sock_fd;
    int thread_index = meta_data.thread_idx;

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
    printf(BLUE("\nRequest received : %d\n"), request.data);

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
    sprintf(registration_request_st.data, "%s|%d|%d|%d|%s", MY_IP, MY_CLIENT_PORT_NO, MY_NFS_PORT_NO, num_of_paths_stored, paths_string); // <My ip>|<client port>|<nfs port>|<no of paths>|[<paths>]

    // Sending the registration request until acknowledgement is not received
    while (nfs_registrations_status == NOT_REGISTERED)
    {
        int sent_msg_size;
        if ((sent_msg_size = send_to(socket_fd, (request)&registration_request_st, sizeof(st_request), 0, (struct sockaddr *)&address, sizeof(address))) < 0)
        {
            fprintf(stderr, RED("sendto : %s\n"), strerror(errno));
            exit(1);
        }
        usleep(100000); // Try again every 10 miliseconds
    }

    return NULL;
}

// Receive registration acknowledgement
void *receive_reg_ack(void* args)
{
    // Receiving registration acknowledgement
    st_request registration_ack_st;
    memset(&registration_ack_st, 0, MAX_DATA_LENGTH);

    int msg_size_recvd;
    if ((msg_size_recvd = recv(socket_fd, &registration_ack_st, sizeof(st_request), 0)) <= 0)
    {
        fprintf(stderr, RED("recv : %s\n"), strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Closing NFS socket
    if (close(socket_fd) < 0)
    {
        fprintf(stderr, RED("close : failed to close the socket!\n"));
        exit(EXIT_FAILURE);
    }

    return NULL;
}

