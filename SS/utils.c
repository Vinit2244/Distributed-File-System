#include "headers.h"

// This function tokenises the provided string on given character and returns a 2D character array broken at ch
char **tokenize(const char *str, const char ch)
{
    // Counting the number of delimiters
    int num_of_ch = 0;
    for (int i = 0; i < strlen(str); i++)
    {
        if (str[i] == ch)
        {
            num_of_ch++;
        }
    }

    // Number of tokens would be 1 more than the number of delimiters present in the string
    int num_of_tokens = num_of_ch + 1;

    // Allocating num_of_tokens + 1 memory because we need to store last token as NULL token to mark the end of tokens
    char **tokens = (char **) malloc((num_of_tokens + 1) * sizeof(char*));
    for (int i = 0; i < num_of_tokens; i++)
    {
        tokens[i] = (char *) calloc(MAX_DATA_LENGTH, sizeof(char));
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

// Sends request to NFS to add or delete some paths, returns 0 if successful else returns 1
int send_update_paths_request(const int request_type, const char* paths_string)
{
    // Preparing the request to be sent
    st_request update_request;
    update_request.request_type = request_type;
    memset(update_request.data, 0, MAX_DATA_LENGTH);

    // Storing all the file paths to be updated
    sprintf(update_request.data, "%d|%s", MY_SS_ID, paths_string); // <My ss_id>|[<paths>]

    if (request_type == ADD_PATHS)
    {
        printf(CYAN("Add paths request data : "));
        printf(ORANGE("%s\n"), update_request.data);
    }
    else if (request_type == DELETE_PATHS)
    {
        printf(CYAN("Delete paths request data : "));
        printf(ORANGE("%s\n"), update_request.data);
    }
    
    // Connecting to the NFS through TCP
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));

    int socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) 
    { 
        // Some error occured while creating socket
        fprintf(stderr, RED("socket : unable to create socket for sending update paths request : %s\n"), strerror(errno));
        return 1;
    }

    address.sin_port    = htons(NFS_SERVER_PORT_NO);        // port on which server side process is listening
    address.sin_family  = AF_INET;

    if (inet_pton(AF_INET, NFS_IP, &address.sin_addr.s_addr) <= 0) 
    {   
        fprintf(stderr, RED("inet_pton : unable to convert ip to short int in send update paths request : %s\n"), strerror(errno));
        return 1;
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
    if ((sent_msg_size = send(socket_fd, (request) &update_request, sizeof(st_request), 0)) < 0)
    {
        fprintf(stderr, RED("send : could not send update paths request : %s\n"), strerror(errno));
        return 1;
    }

    // Closing the socket as the communication is done
    if (close(socket_fd) < 0) 
    {
        fprintf(stderr, RED("close : failed to close the socket of update paths request!\n"));
        return 1;
    }

    return 0;
}

// Send registration request to the NFS using TCP socket and waits for registration ack, returns 0 if successful else 1
int register_ss(void)
{
    // Preparing the request to be sent
    st_request registration_request_st;
    registration_request_st.request_type = REGISTRATION_REQUEST;
    memset(registration_request_st.data, 0, MAX_DATA_LENGTH);

    // Printing my SS details like port number and ip address and all the paths that are stored currently with me on the query
    sprintf(registration_request_st.data, "%d|%s|%d|%d", MY_SS_ID, MY_IP, MY_CLIENT_PORT_NO, MY_NFS_PORT_NO); // <My ss_id>|<My ip>|<client port>|<nfs port>

    // Connecting to the NFS through TCP
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));

    int socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) 
    { 
        // Some error occured while creating socket
        fprintf(stderr, RED("socket : could not create socket for registering SS : %s\n"), strerror(errno));
        return 1;
    }

    address.sin_port    = htons(NFS_SERVER_PORT_NO);        // port on which server side process is listening
    address.sin_family  = AF_INET;

    if (inet_pton(AF_INET, NFS_IP, &address.sin_addr.s_addr) <= 0) 
    {   
        fprintf(stderr, RED("inet_pton : could not conver ip string to short int for registering ss : %s\n"), strerror(errno));
        return 1;
    }

    // Waiting for us to connect to the NFS, it may happen that the NFS port might be busy and could not accept the connection request so sending it again and again until once it connects
    while (1)
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
    if ((sent_msg_size = send(socket_fd, (request) &registration_request_st, sizeof(st_request), 0)) < 0)
    {
        fprintf(stderr, RED("send : could not send ss registration request : %s\n"), strerror(errno));
        return 1;
    }

    // Send the registration request (since we are using TCP we are sure that it would have reaced the NFS so now we can change the registration status to register assuming that the NFS also registers our ss successfully)
    nfs_registrations_status = REGISTERED;

    printf(GREEN("SS %d registered successfully.\n"), MY_SS_ID);

    // Closing the socket as the communication is done
    if (close(socket_fd) < 0) 
    {
        fprintf(stderr, RED("close : failed to close the socket created to register ss!\n"));
        return 1;
    }

    return 0;
}

// Sends the mentioned acknowledgement to the given socket file descriptor
void send_ack(const int status_code, const int sock_fd, const char* msg)
{
    // Send acknowledgement
    st_request ack_st;
    ack_st.request_type = status_code;
    if (msg != NULL)
    {
        memset(ack_st.data, 0, MAX_DATA_LENGTH);
        strcpy(ack_st.data, msg);
    }

    // Nothing to be written onto the data as only ack is being sent
    
    int sent_msg_size;
    if ((sent_msg_size = send(sock_fd, (request) &ack_st, sizeof(st_request), 0)) <= 0)
    {
        fprintf(stderr, RED("send : could not sent acknowledgement : %s\n"), strerror(errno));
    }

    return;
}

// Searches for all files recursively
void seek(char* path_to_base_dir, linked_list_head paths) {
    struct stat dir_stat;
    stat(path_to_base_dir, &dir_stat);

    insert_in_linked_list(paths, path_to_base_dir);

    if (S_ISDIR(dir_stat.st_mode) == 0) {
        return;
    }

    DIR *dr;
    struct dirent *en;

    dr = opendir(path_to_base_dir);
    if (dr) {
        while ((en = readdir(dr)) != NULL) {
            if (en->d_name[0] != '.') {
                char* next_file_path = (char*) calloc(MAX_PATH_LEN, sizeof(char));
                strcpy(next_file_path, path_to_base_dir);
                strcat(next_file_path, "/");
                strcat(next_file_path, en->d_name);

                seek(next_file_path, paths);

                free(next_file_path);
            }
        }
        closedir(dr);
    }
    return;
}

// Creates new path with "storage" replaced by "backup"
char* replace_storage_by_backup(char* path)
{
    char *new_path = (char*) calloc(MAX_PATH_LEN, sizeof(char));

    char **tkns = tokenize(path, '/');
    int idx = 0;
    while (tkns[idx] != NULL)
    {
        if (idx == 1 && (strcmp(tkns[idx], "storage") == 0))
        {
            strcat(new_path, "backup");
        }
        else
        {
            strcat(new_path, tkns[idx]);
        }
        strcat(new_path, "/");
        idx++;
    }
    new_path[strlen(new_path) - 1] = '\0';
    free_tokens(tkns);

    return new_path;
}

// Creates the specified folder (also creates intermediate folders if required)
void create_folder(char* path)
{
    // Creating intermediate directories if not already present
    // First tokenising the file_path on "/"
    char **dirs = tokenize(path, '/');

    // Calculating the number of intermediate dirs
    int n_tkns = 0;
    while (dirs[n_tkns] != NULL)
    {
        n_tkns++;
    }
    // Final number of dirs is equal to the number of tokens
    int n_dirs = n_tkns;

    // Now creating all the intermediate dirs one by one
    for (int i = 0; i < n_dirs; i++)
    {
        if (mkdir(dirs[i], 0777) == -1)
        {
            // If the directory already exitsts do nothing
        }
        // Moving into that directory to create the next directory in hierarchy
        chdir(dirs[i]);
    }
    free_tokens(dirs);

    // Moving out of all dirs again to the home dir
    chdir(PWD);
    return;
}

// Creates absolute path PWD + relative path from the relative path provided
char* create_abs_path(char* relative_path)
{
    char* abs_path = (char*) calloc((MAX_PATH_LEN + strlen(PWD) + 10), sizeof(char));
    strcpy(abs_path, PWD);
    strcat(abs_path, "/");

    char** tkns = tokenize(relative_path, '/');
    int n_tkns = 0;
    while (tkns[n_tkns] != NULL)
    {
        n_tkns++;
    }

    if (n_tkns == 2 || n_tkns == 1)
    {
        // Trying to find abs path for the base folder or storage or backup folder which we can't allow
        return NULL;
    }
    
    for (int i = 1; i < n_tkns; i++)
    {
        strcat(abs_path, tkns[i]);
        strcat(abs_path, "/");
    }
    abs_path[strlen(abs_path) - 1] = '\0';
    return abs_path;
}

// Sends the msg string to nfs
void send_msg_to_nfs(char* msg, int req_type)
{
    // Preparing the request to be sent
    st_request msg_req_st;
    msg_req_st.request_type = req_type;
    memset(msg_req_st.data, 0, MAX_DATA_LENGTH);

    sprintf(msg_req_st.data, "%s", msg);

    // Connecting to the NFS through TCP
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));

    int socket_fd = socket(PF_INET, SOCK_STREAM, 0);
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
    if ((sent_msg_size = send(socket_fd, (request) &msg_req_st, sizeof(st_request), 0)) < 0)
    {
        fprintf(stderr, RED("send : %s\n"), strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Closing the socket as the communication is done
    if (close(socket_fd) < 0) 
    {
        fprintf(stderr, RED("close : failed to close the socket!\n"));
        exit(EXIT_FAILURE);
    }

    return;
}

// ===================================== LINKED LIST FUNC ===========================================
linked_list_head create_linked_list_head() {
    linked_list_head linked_list = (linked_list_head) malloc(sizeof(linked_list_head_struct));
    linked_list->number_of_nodes = 0;
    linked_list->first = NULL;
    linked_list->last = NULL;
    return linked_list;
}

linked_list_node create_linked_list_node(char* path) {
    linked_list_node N = (linked_list_node) malloc(sizeof(linked_list_node_struct));
    N->next = NULL;
    N->path = (char*) calloc(MAX_PATH_LEN, sizeof(char));
    strcpy(N->path, path);
    return N;
}

void insert_in_linked_list(linked_list_head linked_list, char* path) {
    linked_list_node N = create_linked_list_node(path);
    if (linked_list->number_of_nodes == 0) {
        linked_list->first = N;
        linked_list->last = N;
        linked_list->number_of_nodes++;
    } else if (linked_list->number_of_nodes == 1) {
        linked_list->last = N;
        linked_list->first->next = N;
        linked_list->number_of_nodes++;
    } else {
        linked_list->last->next = N;
        linked_list->last = N;
        linked_list->number_of_nodes++;
    }
}

void free_linked_list(linked_list_head linked_list) {
    linked_list_node trav = linked_list->first;
    while (trav != NULL) {
        free(trav->path);
        linked_list_node temp = trav->next;
        free(trav);
        trav = temp;
    }
    free(linked_list);
}

char* remove_extension(char* file_name) {
    char* final = (char*) calloc(MAX_PATH_LEN, sizeof(char));
    int idx = 0;
    while (file_name[idx] != '\0' && file_name[idx] != '.') {
        final[idx] = file_name[idx];
        idx++;
    }
    final[idx] = '\0';
    return final;
}

void update_path(char* path, char* next_dir) {
    int start = strlen(path);
    path[start] = '/';
    for (int i = start + 1; i < start + 1 + strlen(next_dir); i++) {
        path[i] = next_dir[i - start - 1];
    }
    path[start + 1 + strlen(next_dir)] = '\0';
}

