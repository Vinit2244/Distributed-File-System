#include "headers.h"

void* copy_handler(request req,int client_id){

        char *source = (char *)malloc(sizeof(char) * MAX_DATA_LENGTH);
        char *desti = (char *)malloc(sizeof(char) * MAX_DATA_LENGTH);

        // printf("%s\n",req->data);
        char *token = strtok(req->data, "|");

        strcpy(source, token);
        token = strtok(NULL, "|");
        strcpy(desti, token);

        if(req->request_type == COPY_FOLDER){
            // printf("Invalid file to copy!\n");
            // request r = (request)malloc(sizeof(st_request));
            // r->request_type = FILE_NOT_FOUND;
            // strcpy(r->data, "File not found");
            // send(client_socket_arr[client_id], r, sizeof(st_request), 0);
            // client_socket_arr[client_id] = -1;  
            // close(client_socket_arr[client_id]);
            // return NULL;

            request r = (request)malloc(sizeof(st_request));
            r->request_type = COPY_FOLDER;
            strcpy(r->data, source);
            ss found_server = NULL;
            ss dest_server = NULL;
            pthread_mutex_lock(&server_lock);
            // printf("%d\n",server_count);
            for(int i=0;i<server_count;i++){
                if(search_path(ss_list[i]->root,source)==1){
                    
                    found_server=ss_list[i];
                    printf("found source at %d\n",i);
                    // printf("Found surce\n");
                    
                }
                if(search_path(ss_list[i]->root,desti)==1){
                    dest_server=ss_list[i];
                    // printf("Found dest\n");
                    printf("found dest at %d\n",i);
                }

                if(found_server!=NULL && dest_server!=NULL){
                    break;
                }

            }
            pthread_mutex_unlock(&server_lock);

            if(found_server==NULL || dest_server==NULL || found_server->status==0 || dest_server->status==0){
                r->request_type=FILE_NOT_FOUND;
                strcpy(r->data,"File not found");
                send(client_socket_arr[client_id], r, sizeof(st_request), 0);
                client_socket_arr[client_id] = -1;  
                close(client_socket_arr[client_id]);
                return NULL;
            }

            else{

                printf(YELLOW("Copying folder %s to %s\n"),found_server->port,dest_server->port);
                st_copy_folder* get_r=(st_copy_folder*)malloc(sizeof(st_copy_folder));
                int sock_fd=connect_to_port(found_server->port);
                send(sock_fd,r,sizeof(st_request),0);
                recv(sock_fd,get_r,sizeof(st_copy_folder),0);

 
                close(sock_fd);
                request r=(request)malloc(sizeof(st_request));
                printf("%d\n",get_r->num_paths);
                for(int i=0;i<get_r->num_paths;i++){
                    // printf("%s\n",get_r->paths[i]);
                    memset(r->data,0,MAX_DATA_LENGTH);
                    r->request_type = PASTE;

                    strcat(r->data,desti);
                    strcat(r->data,get_r->paths[i]);
                    // printf("%s\n",r->data);
                    int sock_fd1=connect_to_port(dest_server->port);
                    send(sock_fd1,r,sizeof(st_request),0);

                    close(sock_fd1);


            }
                

                // int sock_fd1=connect_to_port(dest_server->port);

                // if(r->request_type!=N_FILE_REQ){
                //     printf(RED("Error in copying folder with code : %d\n\n\n"),r->request_type);
                //     return NULL;
                // }

                // // printf("%s\n",r->data);
                // int total_packets = atoi(r->data);
                
                // int curr_packet = 0;

                // request r1 = (request)malloc(sizeof(st_request));
                // request r2 = (request)malloc(sizeof(st_request));

                // request all_requests[MAX_CONNECTIONS];
                // int req_ind=0;

                // while(curr_packet<total_packets){

                    
                //     char* path = (char*)malloc(sizeof(char)*MAX_DATA_LENGTH);

                //     int x=recv(sock_fd,r1,sizeof(st_request),0);
                    
                //     strcpy(path,r1->data);

                //     // printf(ORANGE("%s\n"),r1->data);
                //     r2->request_type=PASTE;
                //     snprintf(r2->data,sizeof(r2->data),"%s/%s",desti,path);
                //     printf("%s\n",r2->data);
                //     // send(sock_fd1,r2,sizeof(st_request),0);
                //     all_requests[req_ind++]=r2;
                //     curr_packet++;


                // }



            }





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
            get_r->request_type = COPY_FILE;
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