#include "headers.h"

// Code to process the request according to request type
void process(request req)
{

    if (req->request_type == REGISTRATION_REQUEST)
    {

        init_storage(req->data); // Code to add a new storage server in naming server list
    }
    else if (req->request_type == REQ)
    {
        // create a new thread for each client request here for multi client handling

        client_handler(req->data); // Client requests handled here
    }
    else if (req->request_type == WRITE_REQ || req->request_type == READ_REQ || req->request_type == RETRIEVE_INFO || req->request_type == APPEND_REQ)
    {

        // printf("%d\n",server_count);
        // printf("%s\n",req->data);
        pthread_mutex_lock(&server_lock);
        int flag = 0;
        int id = 0;
        request r = (request)malloc(sizeof(st_request));
        char *reference = (char *)malloc(sizeof(char) * MAX_DATA_LENGTH);
        for (int i = 0; i < server_count; i++)
        {
            pthread_mutex_lock(&ss_list[i]->lock);
            if (search_path(ss_list[i]->root, req->data) == 1)
            {
                snprintf(reference, MAX_DATA_LENGTH, "%s|%s", ss_list[i]->ip, ss_list[i]->client_port);
                flag = 1;
                id = i;
                pthread_mutex_unlock(&ss_list[i]->lock);
                break;
            }
            pthread_mutex_unlock(&ss_list[i]->lock);
        }
        pthread_mutex_unlock(&server_lock);

        if (flag == 0)
        {

            r->request_type = FILE_NOT_FOUND;
            strcpy(r->data, "File not found");
            printf(RED("No files found , informing client\n\n\n"));
            send(client_socket_tcp, r, sizeof(st_request), 0);
        }
        else
        {

            // send to server
            if (ss_list[id]->status == 1)
            {
                r->request_type = RES;
                strcpy(r->data, reference);
                send(client_socket_tcp, r, sizeof(st_request), 0);
            }

            else
            {

                // server offline look for backup
                if (req->request_type == READ_REQ && ss_list[id]->is_backedup == 1)
                {
                    // allow
                    r->request_type = BACKUP_READ_REQ;
                    snprintf(r->data, MAX_DATA_LENGTH, "%s|%s", ss_list[id]->ip, ss_list[id]->backup_port[0]);
                    send(client_socket_tcp,r,sizeof(st_request),0);
                }
                else
                {

                    r->request_type = FILE_NOT_FOUND;
                    strcpy(r->data, "File not found");
                    printf(RED("No files found , informing client\n\n\n"));
                    send(client_socket_tcp, r, sizeof(st_request), 0);
                }
            }
        }
    }
    else if (req->request_type == DELETE_FOLDER || req->request_type == DELETE_FILE)
    {

        // printf("delete req received\n");
        pthread_mutex_lock(&server_lock);
        int flag = 0;
        request r = (request)malloc(sizeof(st_request));
        memset(r->data, 0, MAX_DATA_LENGTH);
        char *reference = (char *)calloc(MAX_DATA_LENGTH, sizeof(char));
        char *path = (char *)calloc(MAX_DATA_LENGTH, sizeof(char));
        ss found_server;
        for (int i = 0; i < server_count; i++)
        {

            pthread_mutex_lock(&ss_list[i]->lock);
            if (search_path(ss_list[i]->root, req->data) == 1)
            {
                snprintf(reference, MAX_DATA_LENGTH, "%s|%s", ss_list[i]->ip, ss_list[i]->client_port);
                flag = 1;
                found_server = ss_list[i];
                pthread_mutex_unlock(&ss_list[i]->lock);
                break;
            }
            pthread_mutex_unlock(&ss_list[i]->lock);
        }
        pthread_mutex_unlock(&server_lock);

        if (flag == 0)
        {
            r->request_type = FILE_NOT_FOUND;
            strcpy(r->data, "File/Directory not found");
            send(client_socket_tcp, r, sizeof(st_request), 0);
        }
        else
        {

            if (found_server->status == 1)
            {
                r->request_type = req->request_type;
                strcpy(r->data, req->data);

                struct sockaddr_in address;
                memset(&address, 0, sizeof(address));
                int s_fd = socket(PF_INET, SOCK_STREAM, 0);
                address.sin_port = htons(atoi(found_server->port));
                address.sin_family = AF_INET;
                inet_pton(AF_INET, found_server->ip, &address.sin_addr.s_addr);
                connect(s_fd, (struct sockaddr *)&address, sizeof(address));
                send(s_fd, r, sizeof(st_request), 0);
                recv(s_fd, r, sizeof(st_request), 0);


                if(found_server->is_backedup==1){

                    if (req->request_type == DELETE_FOLDER)
                    {
                        r->request_type = BACKUP_DELETE_FOLDER;
                    }
                    else
                    {
                        r->request_type = BACKUP_DELETE_FILE;
                    }
                    strcpy(r->data, req->data);
                    struct sockaddr_in address;
                    memset(&address, 0, sizeof(address));
                    int s_fd = socket(PF_INET, SOCK_STREAM, 0);
                    address.sin_port = htons(atoi(found_server->backup_port[0]));
                    address.sin_family = AF_INET;
                    inet_pton(AF_INET, found_server->ip, &address.sin_addr.s_addr);
                    connect(s_fd, (struct sockaddr *)&address, sizeof(address));
                    send(s_fd, r, sizeof(st_request), 0);
                    recv(s_fd, r, sizeof(st_request), 0);

                    close(s_fd);
                    if (req->request_type == DELETE_FOLDER)
                    {
                        r->request_type = BACKUP_DELETE_FOLDER;
                    }
                    else
                    {
                        r->request_type = BACKUP_DELETE_FILE;
                    }
                    strcpy(r->data, req->data);
                    s_fd = socket(PF_INET, SOCK_STREAM, 0);
                    address.sin_port = htons(atoi(found_server->backup_port[1]));
                    address.sin_family = AF_INET;
                    inet_pton(AF_INET, found_server->ip, &address.sin_addr.s_addr);
                    connect(s_fd, (struct sockaddr *)&address, sizeof(address));
                    send(s_fd, r, sizeof(st_request), 0);
                    recv(s_fd, r, sizeof(st_request), 0);

                    close(s_fd);
                    

                }






                if (r->request_type == ACK)
                {
                    r->request_type = ACK;
                    strcpy(r->data, "Operation succesful!\n");
                    send(client_socket_tcp, r, sizeof(st_request), 0);
                }
                else
                {
                    r->request_type = FILE_NOT_FOUND;
                    strcpy(r->data, "File not found");
                    send(client_socket_tcp, r, sizeof(st_request), 0);
                }
                close(s_fd);
            }
            else
            {
                r->request_type = FILE_NOT_FOUND;
                strcpy(r->data, "File not found");
                send(client_socket_tcp, r, sizeof(st_request), 0);
            }
        }
    }
    else if (req->request_type == CREATE_FOLDER || req->request_type == CREATE_FILE)
    {
        int flag = 0;
        request r = (request)malloc(sizeof(st_request));
        memset(r->data, 0, MAX_DATA_LENGTH);
        char *reference = (char *)calloc(MAX_DATA_LENGTH, sizeof(char));
        char *path = (char *)calloc(MAX_DATA_LENGTH, sizeof(char));

        char *search_path_string = strtok(req->data, "|");
        char *name = strtok(NULL, "|");
        // printf("%s %s\n",search_path_string,name);
        ss found_server;
        pthread_mutex_lock(&server_lock);
        for (int i = 0; i < server_count; i++)
        {

            pthread_mutex_lock(&ss_list[i]->lock);
            if (search_path(ss_list[i]->root, search_path_string) == 1)
            {
                snprintf(reference, MAX_DATA_LENGTH, "%s|%s", ss_list[i]->ip, ss_list[i]->client_port);
                flag = 1;
                found_server = ss_list[i];
                pthread_mutex_unlock(&ss_list[i]->lock);
                break;
            }
            pthread_mutex_unlock(&ss_list[i]->lock);
        }
        pthread_mutex_unlock(&server_lock);
        

        if (flag == 0 || found_server->status == 0)
        {
            r->request_type = FILE_NOT_FOUND;
            strcpy(r->data, "File/Directory not found");
            send(client_socket_tcp, r, sizeof(st_request), 0);
        }
        else
        {
            r->request_type = req->request_type;
            snprintf(r->data, sizeof(r->data), "%s/%s", search_path_string, name);

            struct sockaddr_in address;
            memset(&address, 0, sizeof(address));
            int s_fd = socket(PF_INET, SOCK_STREAM, 0);
            address.sin_port = htons(atoi(found_server->port));
            address.sin_family = AF_INET;
            inet_pton(AF_INET, found_server->ip, &address.sin_addr.s_addr);
            connect(s_fd, (struct sockaddr *)&address, sizeof(address));
            send(s_fd, r, sizeof(st_request), 0);
            recv(s_fd, r, sizeof(st_request), 0);

            if (found_server->is_backedup == 1)
            {

                if (req->request_type == CREATE_FOLDER)
                {
                    r->request_type = BACKUP_CREATE_FOLDER;
                }
                else
                {
                    r->request_type = BACKUP_CREATE_FILE;
                }
                snprintf(r->data, sizeof(r->data), "%s/%s", search_path_string, name);
                struct sockaddr_in address;
                memset(&address, 0, sizeof(address));
                int s_fd = socket(PF_INET, SOCK_STREAM, 0);
                address.sin_port = htons(atoi(found_server->backup_port[0]));
                address.sin_family = AF_INET;
                inet_pton(AF_INET, found_server->ip, &address.sin_addr.s_addr);
                connect(s_fd, (struct sockaddr *)&address, sizeof(address));
                send(s_fd, r, sizeof(st_request), 0);
                recv(s_fd, r, sizeof(st_request), 0);

                close(s_fd);
                if (req->request_type == CREATE_FOLDER)
                {
                    r->request_type = BACKUP_CREATE_FOLDER;
                }
                else
                {
                    r->request_type = BACKUP_CREATE_FILE;
                }
                snprintf(r->data, sizeof(r->data), "%s/%s", search_path_string, name);
                s_fd = socket(PF_INET, SOCK_STREAM, 0);
                address.sin_port = htons(atoi(found_server->backup_port[1]));
                address.sin_family = AF_INET;
                inet_pton(AF_INET, found_server->ip, &address.sin_addr.s_addr);
                connect(s_fd, (struct sockaddr *)&address, sizeof(address));
                send(s_fd, r, sizeof(st_request), 0);
                recv(s_fd, r, sizeof(st_request), 0);

                close(s_fd);
            }

            if (r->request_type == ACK)
            {
                r->request_type = ACK;
                strcpy(r->data, "Operation succesful!\n");
                send(client_socket_tcp, r, sizeof(st_request), 0);
            }
            else
            {
                r->request_type = FILE_NOT_FOUND;
                strcpy(r->data, "File not found");
                send(client_socket_tcp, r, sizeof(st_request), 0);
            }
            close(s_fd);
        }
    }
    else if (req->request_type == ACK)
    {
        request r = (request)malloc(sizeof(st_request));
        r->request_type = ACK;
        strcpy(r->data, "Operation succesful!\n");
        send(client_socket_tcp, r, sizeof(st_request), 0);
    }
    else if (req->request_type == COPY_REQUEST)
    {

        char *source = (char *)malloc(sizeof(char) * MAX_DATA_LENGTH);
        char *desti = (char *)malloc(sizeof(char) * MAX_DATA_LENGTH);

        char *token = strtok(req->data, "|");

        strcpy(source, token);
        token = strtok(NULL, "|");
        strcpy(desti, token);

        ss source_no, dest_no;
        int flag = 0;

        pthread_mutex_lock(&server_lock);

        for (int i = 0; i < server_count; i++)
        {

            pthread_mutex_lock(&ss_list[i]->lock);

            

            if (search_path(ss_list[i]->root, source) == 1)
            {
                source_no = ss_list[i];
                flag++;
            }
            if (search_path(ss_list[i]->root, desti) == 1)
            {
                dest_no = ss_list[i];
                flag++;
            }

            if (flag == 2)
            {
                pthread_mutex_unlock(&ss_list[i]->lock);
                break;
            }
            pthread_mutex_unlock(&ss_list[i]->lock);
        }
        pthread_mutex_unlock(&server_lock);
        if (flag < 2 || source_no->status == 0 || dest_no->status == 0)
        {
            // printf("3\n");
            request r = (request)malloc(sizeof(st_request));
            r->request_type = FILE_NOT_FOUND;
            strcpy(r->data, "File not found");
            send(client_socket_tcp, r, sizeof(st_request), 0);
        }
        else
        {

            request get_r = (request)malloc(sizeof(st_request));
            get_r->request_type = COPY;
            strcpy(get_r->data, source);

            struct sockaddr_in address;
            memset(&address, 0, sizeof(address));
            int s_fd = socket(PF_INET, SOCK_STREAM, 0);
            address.sin_port = htons(atoi(source_no->port));
            address.sin_family = AF_INET;
            inet_pton(AF_INET, source_no->ip, &address.sin_addr.s_addr);
            connect(s_fd, (struct sockaddr *)&address, sizeof(address));
            send(s_fd, get_r, sizeof(st_request), 0);
            recv(s_fd, get_r, sizeof(st_request), 0);
            close(s_fd);

            char *token = strtok(get_r->data, "|");
            char *token1 = strtok(NULL, "|");

            char** paths=(char**)malloc(sizeof(char*)*MAX_CONNECTIONS);
            for(int i=0;i<MAX_CONNECTIONS;i++){
                paths[i]=(char*)malloc(sizeof(char)*MAX_DATA_LENGTH);
            }
            char* token_path=strtok(token,"/");
            int ind=0;
            while(token_path!=NULL){
                strcpy(paths[ind],token_path);
                ind++;
                token_path=strtok(NULL,"/");
            }
            char* new_dest=(char*)malloc(sizeof(char)*MAX_DATA_LENGTH);
            snprintf(new_dest, MAX_DATA_LENGTH, "%s/%s", desti, paths[ind-1]);
            printf("%s\n",desti);

            get_r->request_type = PASTE;
            snprintf(get_r->data, MAX_DATA_LENGTH, "%s|%s", new_dest, token);
            printf("%s\n",get_r->data);
            struct sockaddr_in address1;
            memset(&address1, 0, sizeof(address1));
            int s_fd1 = socket(PF_INET, SOCK_STREAM, 0);
            address1.sin_port = htons(atoi(dest_no->port));
            address1.sin_family = AF_INET;
            inet_pton(AF_INET, dest_no->ip, &address1.sin_addr.s_addr);
            connect(s_fd1, (struct sockaddr *)&address1, sizeof(address1));
            send(s_fd1, get_r, sizeof(st_request), 0);
            close(s_fd1);

            if(dest_no->is_backedup==1){

                get_r->request_type = BACKUP_PASTE;
                struct sockaddr_in address1;
                memset(&address1, 0, sizeof(address1));
                int s_fd1 = socket(PF_INET, SOCK_STREAM, 0);
                address1.sin_port = htons(atoi(dest_no->backup_port[0]));
                address1.sin_family = AF_INET;
                inet_pton(AF_INET, dest_no->ip, &address1.sin_addr.s_addr);
                connect(s_fd1, (struct sockaddr *)&address1, sizeof(address1));
                send(s_fd1, get_r, sizeof(st_request), 0);
                close(s_fd1);

                get_r->request_type = BACKUP_PASTE;
                struct sockaddr_in address2;
                memset(&address2, 0, sizeof(address2));
                int s_fd2 = socket(PF_INET, SOCK_STREAM, 0);
                address2.sin_port = htons(atoi(dest_no->backup_port[1]));
                address2.sin_family = AF_INET;
                inet_pton(AF_INET, dest_no->ip, &address2.sin_addr.s_addr);
                connect(s_fd2, (struct sockaddr *)&address2, sizeof(address2));
                send(s_fd2, get_r, sizeof(st_request), 0);
                close(s_fd2);
            }

        

           

            request r = (request)malloc(sizeof(st_request));
            r->request_type = ACK;
            strcpy(r->data, "Copying succesful!\n");
            printf(BLUE("Copying succesful!\n\n\n"));
            send(client_socket_tcp, r, sizeof(st_request), 0);
        }
    }

    else if (req->request_type == ADD_PATHS)
    {

        char *ss_id = (char *)malloc(sizeof(char) * MAX_DATA_LENGTH);
        char **path = (char **)malloc(sizeof(char *) * MAX_CONNECTIONS);
        for (int i = 0; i < MAX_CONNECTIONS; i++)
        {
            path[i] = (char *)malloc(sizeof(char) * MAX_DATA_LENGTH);
        }
        int ind = 0;
        char *token = strtok(req->data, "|");
        while (token != NULL)
        {
            if (ind == 0)
            {
                strcpy(ss_id, token);
            }
            else
            {
                strcpy(path[ind - 1], token);
            }
            ind++;
            token = strtok(NULL, "|");
        }
        int count=0;
        ss found_server = ss_list[atoi(ss_id) - 1];
        pthread_mutex_lock(&found_server->lock);
        for (int i = 0; i < ind - 1; i++)
        {
            strcpy(found_server->paths[found_server->path_count + i], path[i]);
            if(search_path(found_server->root,path[i])==0){
            
            if(insert_path(found_server->root, path[i])==1){
                count++;
                
            }
            
            }
        }

        found_server->added = 1;
        found_server->path_count = found_server->path_count + ind - 1;
        found_server->synced=0;
        pthread_mutex_unlock(&found_server->lock);
        printf(BLUE("Added %d new files/directories from server  %s\n\n\n"), count,found_server->port);
        // print_paths(found_server->root);
    }

    else if (req->request_type == DELETE_PATHS)
    {
        char *ss_id = (char *)malloc(sizeof(char) * MAX_DATA_LENGTH);
        char **path = (char **)malloc(sizeof(char *) * MAX_CONNECTIONS);
        for (int i = 0; i < MAX_CONNECTIONS; i++)
        {
            path[i] = (char *)malloc(sizeof(char) * MAX_DATA_LENGTH);
        }
        int tkn_cnt = 0;
        char *token = strtok(req->data, "|");
        while (token != NULL)
        {
            if (tkn_cnt == 0)
            {
                strcpy(ss_id, token);
            }
            else
            {
                strcpy(path[tkn_cnt - 1], token);
            }
            tkn_cnt++;
            token = strtok(NULL, "|");
        }

        ss found_server = ss_list[atoi(ss_id) - 1];
        pthread_mutex_lock(&found_server->lock);
        for (int i = 0; i < tkn_cnt - 1; i++)
        {
            // for (int j = 0; j < found_server->path_count; j++)
            // {
            //     if (strcmp(found_server->paths[j], path[i]) == 0)
            //     {
            //         for (int k = j; k < found_server->path_count - 1; k++)
            //         {
            //             strcpy(found_server->paths[k], found_server->paths[k + 1]);
            //         }
            //         found_server->path_count--;
            //         break;
            //     }
            // }

            if (search_path(found_server->root, path[i]) == 1)
            {
                delete_path(found_server->root, path[i]);
                found_server->path_count--;
            }
        }
        pthread_mutex_unlock(&found_server->lock);
        printf(BLUE("Deleted files/directories from server number %d\n\n\n"), atoi(ss_id));
    }
    // Yet to work on depending on type of requests
    else if (req->request_type == LIST)
    {

        char *list = (char *)malloc(sizeof(char) * MAX_DATA_LENGTH);
        strcpy(list, "");
        for (int i = 0; i < server_count; i++)
        {
            pthread_mutex_lock(&ss_list[i]->lock);
            for (int j = 0; j < ss_list[i]->path_count; j++)
            {
                strcat(list, ss_list[i]->paths[j]);
                strcat(list, "|");
            }
            pthread_mutex_unlock(&ss_list[i]->lock);
        }
        request r = (request)malloc(sizeof(st_request));
        r->request_type = RES;
        strcpy(r->data, list);
        send(client_socket_tcp, r, sizeof(st_request), 0);
    }
}