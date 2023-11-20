#include "headers.h"

//This file handles creation and deletion of files in the storage servers and also ensures redundancy (consistency with backup)


void* handle_create(request req,int client_id){


    int flag = 0;
        request r = (request)malloc(sizeof(st_request));
        memset(r->data, 0, MAX_DATA_LENGTH);
        char *reference = (char *)calloc(MAX_DATA_LENGTH, sizeof(char));
        char *path = (char *)calloc(MAX_DATA_LENGTH, sizeof(char));

        char *search_path_string = strtok(req->data, "|");
        char *name = strtok(NULL, "|");
        
        
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
            send(client_socket_arr[client_id], r, sizeof(st_request), 0);
        }
        else
        {
            r->request_type = req->request_type;
            snprintf(r->data, sizeof(r->data), "%s/%s", search_path_string, name);

            int s_fd=connect_to_port(found_server->port);
            send(s_fd, r, sizeof(st_request), 0);
            recv(s_fd, r, sizeof(st_request), 0);
            close(s_fd);
            
            if(r->request_type!=ACK){
                printf(RED("Error in creating file/folder with code : %d\n\n\n"),r->request_type);
                return NULL;
            }

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
                int s_fd=connect_to_port(found_server->backup_port[0]);
                send(s_fd, r, sizeof(st_request), 0);
                

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
                s_fd = connect_to_port(found_server->backup_port[1]);
                
                send(s_fd, r, sizeof(st_request), 0);
                

                close(s_fd);
            }

            r->request_type = ACK;
            strcpy(r->data, "Operation succesful!\n");
            send(client_socket_arr[client_id], r, sizeof(st_request), 0);

            // close(s_fd);
        }

}

void* handle_delete(request req,int client_id){


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
            send(client_socket_arr[client_id], r, sizeof(st_request), 0);
        }
        else
        {

            if (found_server->status == 1)
            {
                r->request_type = req->request_type;
                strcpy(r->data, req->data);

                int s_fd=connect_to_port(found_server->port);
                send(s_fd, r, sizeof(st_request), 0);
                recv(s_fd, r, sizeof(st_request), 0);
                close(s_fd);
                if(r->request_type!=ACK){
                printf(RED("Error in creating file/folder with code : %d\n\n\n"),r->request_type);
                return NULL;
            }

                if (found_server->is_backedup == 1)
                {

                    if (req->request_type == DELETE_FOLDER)
                    {
                        r->request_type = BACKUP_DELETE_FOLDER;
                    }
                    else
                    {
                        r->request_type = BACKUP_DELETE_FILE;
                    }
                    strcpy(r->data, req->data);
                    s_fd=connect_to_port(found_server->backup_port[0]);
                    send(s_fd, r, sizeof(st_request), 0);
                    // recv(s_fd, r, sizeof(st_request), 0);

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
                    s_fd=connect_to_port(found_server->backup_port[1]);
                    send(s_fd, r, sizeof(st_request), 0);
                    // recv(s_fd, r, sizeof(st_request), 0);

                    close(s_fd);
                }

                r->request_type = ACK;
                strcpy(r->data, "Operation succesful!\n");
                send(client_socket_arr[client_id], r, sizeof(st_request), 0);

                close(s_fd);
            }
            else
            {
                r->request_type = FILE_NOT_FOUND;
                strcpy(r->data, "File not found");
                send(client_socket_arr[client_id], r, sizeof(st_request), 0);
            }
        }
}