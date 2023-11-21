#include "headers.h"

// Code to process the request according to request type

void *process(void *arg)
{
    proc n = (proc)arg;
    int client_id = n->client_id;
    request req = (request)malloc(sizeof(st_request));
    req->request_type = n->request_type;
    strcpy(req->data, n->data);

    if (req->request_type == REGISTRATION_REQUEST)
    {

        init_storage(req->data);
        client_socket_arr[client_id] = -1;
        close(client_socket_arr[client_id]); // Code to add a new storage server in naming server list
        return NULL;
    }

    else if (req->request_type == WRITE_REQ || req->request_type == READ_REQ || req->request_type == RETRIEVE_INFO || req->request_type == APPEND_REQ)
    {

        printf(BLUE("New request received from client number! %d\n\n\n"), client_id);
        basic_ops(req, client_id);
        if (client_socket_arr[client_id] > 0)
        {
            client_socket_arr[client_id] = -1;
            close(client_socket_arr[client_id]);
        }
        return NULL;
    }
    else if (req->request_type == DELETE_FOLDER || req->request_type == DELETE_FILE)
    {
        handle_delete(req, client_id);
        printf(BLUE("New delete request from client %d\n\n\n"), client_id);
        if (client_socket_arr[client_id] > 0)
        {
            client_socket_arr[client_id] = -1;
            close(client_socket_arr[client_id]);
        }
        return NULL;
    }
    else if (req->request_type == CREATE_FOLDER || req->request_type == CREATE_FILE)
    {
        printf(BLUE("New create request from client %d\n\n\n"), client_id);
        handle_create(req, client_id);
        client_socket_arr[client_id] = -1;
        close(client_socket_arr[client_id]);
    }

    else if (req->request_type == COPY_FILE || req->request_type == COPY_FOLDER)
    {
        printf("Copy request received from client %d\n\n\n", client_id);
        copy_handler(req, client_id);
        if (client_socket_arr[client_id] > 0)
        {
            client_socket_arr[client_id] = -1;
            close(client_socket_arr[client_id]);
        }
        return NULL;
    }

    else if (req->request_type == ADD_PATHS)
    {
        // printf("Adding paths\n");
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
        int count = 0;
        // ss found_server = ss_list[atoi(ss_id) - 1];
        ss found_server;
        for (int i = 0; i < server_count; i++)
        {

            if (ss_list[i]->ssid == atoi(ss_id))
            {
                found_server = ss_list[i];
                break;
            }
        }

        pthread_mutex_lock(&found_server->lock);
        for (int i = 0; i < ind - 1; i++)
        {
            strcpy(found_server->paths[found_server->path_count + i], path[i]);
            if (search_path(found_server->root, path[i]) == 0)
            {

                if (insert_path(found_server->root, path[i]) == 1)
                {
                    count++;
                }
            }
        }

        found_server->added = 1;
        found_server->path_count = found_server->path_count + ind - 1;
        found_server->synced = 0;
        pthread_mutex_unlock(&found_server->lock);
        if (count > 0)
            printf(BLUE("Added %d new files/directories from server  %s\n\n\n"), count, found_server->port);

        client_socket_arr[client_id] = -1;
        close(client_socket_arr[client_id]);

        return NULL;
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
        int count = 0;
        ss found_server = ss_list[atoi(ss_id) - 1];
        pthread_mutex_lock(&found_server->lock);
        for (int i = 0; i < tkn_cnt - 1; i++)
        {

            if (search_path(found_server->root, path[i]) == 1)
            {
                if (delete_path(found_server->root, path[i]) == 1)
                {
                    count++;
                }
                found_server->path_count--;
            }
        }
        pthread_mutex_unlock(&found_server->lock);
        if (count > 0)
            printf(BLUE("Deleted %d files/directories from server number %d\n\n\n"), count, atoi(ss_id));
        client_socket_arr[client_id] = -1;
        close(client_socket_arr[client_id]);
        return NULL;
    }
    // Yet to work on depending on type of requests
    else if (req->request_type == LIST)

    {

        printf(BLUE("List request received from client %d\n\n\n"), client_id);
        char *list = (char *)malloc(sizeof(char) * MAX_DATA_LENGTH);
        strcpy(list, "");
        for (int i = 0; i < server_count; i++)
        {
            // pthread_mutex_lock(&ss_list[i]->lock);
            if (ss_list[i]->status == 1)
            {
                for (int j = 0; j < ss_list[i]->path_count; j++)
                {
                    strcat(list, ss_list[i]->paths[j]);
                    strcat(list, "|");
                }
            }
            // pthread_mutex_unlock(&ss_list[i]->lock);
        }
        request r = (request)malloc(sizeof(st_request));
        r->request_type = RES;
        strcpy(r->data, list);
        send(client_socket_arr[client_id], r, sizeof(st_request), 0);

        return NULL;
    }

    else if (req->request_type == WRITE_APPEND_COMP)
    {

        delete_path_lock(req->data);
        printf(GREEN("Given path is deleted from locked paths : %s\n"), req->data);
    }

    else if (req->request_type == CONSISTENT_WRITE)
    {

        // pthread_mutex_lock(&server_lock);

        char *token = strtok(req->data, "|");
        char *token1 = strtok(NULL, "|");
        ss found_server;
        for (int i = 0; i < server_count; i++)
        {

            if (search_path(ss_list[i]->root, token) == 1)
            {
                found_server = ss_list[i];
                break;
            }
        }
        // pthread_mutex_unlock(&server_lock);
        // printf("hi\n");
        if (found_server->is_backedup == 1)
        {
            // printf("hi\n");
            request r = (request)malloc(sizeof(st_request));
            r->request_type = BACKUP_WRITE_REQ;

            snprintf(r->data, sizeof(r->data), "%s|%s", token, token1);

            int sock_one = connect_to_port(found_server->backup_port[0]);

            close(sock_one);

            int sock_two = connect_to_port(found_server->backup_port[1]);

            close(sock_two);
        }
        client_socket_arr[client_id] = -1;
        close(client_socket_arr[client_id]);
        return NULL;
        
    }

    client_socket_arr[client_id] = -1;
    close(client_socket_arr[client_id]);
    return NULL;
}