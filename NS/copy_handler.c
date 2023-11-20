#include "headers.h"

void* copy_handler(request req,int client_id){

        char *source = (char *)malloc(sizeof(char) * MAX_DATA_LENGTH);
        char *desti = (char *)malloc(sizeof(char) * MAX_DATA_LENGTH);


        char *token = strtok(req->data, "|");

        strcpy(source, token);
        token = strtok(NULL, "|");
        strcpy(desti, token);

        if(strstr(source,".txt")==NULL){
            printf("Invalid file to copy!\n");
            request r = (request)malloc(sizeof(st_request));
            r->request_type = FILE_NOT_FOUND;
            strcpy(r->data, "File not found");
            send(client_socket_arr[client_id], r, sizeof(st_request), 0);
            client_socket_arr[client_id] = -1;  
            close(client_socket_arr[client_id]);
            return NULL;
        }
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
            
            request r = (request)malloc(sizeof(st_request));
            r->request_type = FILE_NOT_FOUND;
            strcpy(r->data, "File not found");
            send(client_socket_arr[client_id], r, sizeof(st_request), 0);
        }
        else
        {

            request get_r = (request)malloc(sizeof(st_request));
            get_r->request_type = COPY;
            strcpy(get_r->data, source);


            int s_fd = connect_to_port(source_no->port);

            send(s_fd, get_r, sizeof(st_request), 0);
            recv(s_fd, get_r, sizeof(st_request), 0);

            if(get_r->request_type!=21){
                printf(RED("Error in copying file with code : %d\n\n\n"),get_r->request_type);
                return NULL;
            }

            close(s_fd);

            char *token = strtok(get_r->data, "|");
            char *token1 = strtok(NULL, "|");

            char **paths = (char **)malloc(sizeof(char *) * MAX_CONNECTIONS);
            for (int i = 0; i < MAX_CONNECTIONS; i++)
            {
                paths[i] = (char *)malloc(sizeof(char) * MAX_DATA_LENGTH);
            }
            char *token_path = strtok(token, "/");
            int ind = 0;
            while (token_path != NULL)
            {
                strcpy(paths[ind], token_path);
                ind++;
                token_path = strtok(NULL, "/");
            }
            char *new_dest = (char *)malloc(sizeof(char) * MAX_DATA_LENGTH);
            snprintf(new_dest, MAX_DATA_LENGTH, "%s/%s", desti, paths[ind - 1]);
            

            get_r->request_type = PASTE;
            snprintf(get_r->data, MAX_DATA_LENGTH, "%s|%s", new_dest, token);
            // printf("%s\n", get_r->data);
            int s_fd1 = connect_to_port(dest_no->port);

            send(s_fd1, get_r, sizeof(st_request), 0);
            close(s_fd1);

            if (dest_no->is_backedup == 1)
            {

                get_r->request_type = BACKUP_PASTE;

                int s_fd1=connect_to_port(dest_no->backup_port[0]);


                send(s_fd1, get_r, sizeof(st_request), 0);
                close(s_fd1);

                get_r->request_type = BACKUP_PASTE;
                int s_fd2=connect_to_port(dest_no->backup_port[1]);

                close(s_fd2);
            }

            request r = (request)malloc(sizeof(st_request));
            r->request_type = ACK;
            strcpy(r->data, "Copying succesful!\n");
            printf(BLUE("Copying succesful!\n\n\n"));
            send(client_socket_arr[client_id], r, sizeof(st_request), 0);
        }

}