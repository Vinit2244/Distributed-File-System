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
    // Preparing the request to be sent
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

    // Connecting to the NFS through TCP
    memset(&address, 0, sizeof(address));

    socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) 
    { 
        // Some error occured while creating socket
        fprintf(stderr, RED("socket : %s\n"), strerror(errno));
        exit(EXIT_FAILURE);
    }

    address.sin_port    = htons(NFS_SERVER_PORT_NO);        // port on which server side process is listening
    address.sin_family  = AF_INET;

    if (inet_pton(AF_INET, NFS_IP, &address.sin_addr.s_addr) <= 0) 
    {   
        fprintf(stderr, RED("inet_pton : %s\n"), strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Waiting for us to connect to the NFS, it may happen that the NFS port might be busy and could not accept the connection request so sending it again and again until once it connects
    while(1)
    {
        if (connect(socket_fd, (struct sockaddr *) &address, sizeof(address)) == -1) 
        {
            // Could not connect
            continue;
        }
        // Connected
        break;
    }

    // Sending the registration request
    int sent_msg_size;
    if ((sent_msg_size = sendto(socket_fd, (request)&registration_request_st, sizeof(st_request), 0, (struct sockaddr *)&address, sizeof(address))) < 0)
    {
        fprintf(stderr, RED("sendto : %s\n"), strerror(errno));
        exit(1);
    }

    // Send the registration request (since we are using TCP we are sure that it would have reaced the NFS so now we can change the registration status to register assuming that the NFS also registers our ss successfully)
    nfs_registrations_status = REGISTERED;

    // Closing the socket as the communication is done
    if (close(socket_fd) < 0) 
    {
        fprintf(stderr, RED("close : failed to close the socket!\n"));
        exit(EXIT_FAILURE);
    }

    return;
}
