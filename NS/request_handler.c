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
    else if (req->request_type == WRITE_REQ || req->request_type == READ_REQ || req->request_type == RETRIEVE_INFO){
        
        pthread_mutex_lock(&server_lock);
        int flag=0;
        request r = (request)malloc(sizeof(st_request));
        char* reference = (char*)malloc(sizeof(char)*MAX_DATA_LENGTH);
        for(int i=0;i<server_count;i++){
            
            pthread_mutex_lock(&ss_list[i]->lock);

            for(int i=0;i<ss_list[i]->path_count;i++){
                if(strcmp(ss_list[i]->paths[i],req->data)==0){
                    snprintf(reference,MAX_DATA_LENGTH,"%s|%s",ss_list[i]->ip,ss_list[i]->client_port);
                    flag=1;
                    break; 
                }
            }

            if(flag==1){
                break;
            }

            pthread_mutex_unlock(&ss_list[i]->lock);

        }
        pthread_mutex_unlock(&server_lock);

        if(flag==0){
            
            r->request_type=FILE_NOT_FOUND;
            strcpy(r->data,"File not found");
            send(client_socket_tcp, r, sizeof(st_request), 0);
        }
        else{
            
            //send to server
            r->request_type = RES;
            strcpy(r->data,reference);
            send(client_socket_tcp, r, sizeof(st_request), 0);
            

        }



    }
    else if (req->request_type == CREATE_REQ || req->request_type == DELETE_REQ){

        pthread_mutex_lock(&server_lock);
        int flag=0;
        request r = (request)malloc(sizeof(st_request));
        char* reference = (char*)malloc(sizeof(char)*MAX_DATA_LENGTH);
        char* path=(char*)malloc(sizeof(char)*MAX_DATA_LENGTH);
        ss found_server;
        for(int i=0;i<server_count;i++){
            
            pthread_mutex_lock(&ss_list[i]->lock);

            for(int i=0;i<ss_list[i]->path_count;i++){
                if(strcmp(ss_list[i]->paths[i],req->data)==0){
                    
                    snprintf(reference,MAX_DATA_LENGTH,"%s",ss_list[i]->port);
                    strcpy(path,ss_list[i]->paths[i]);
                    found_server=ss_list[i];
                    flag=1;
                    break;
                    
                }
            }

            if(flag==1){
                break;
            }

            pthread_mutex_unlock(&ss_list[i]->lock);

        }
        pthread_mutex_unlock(&server_lock);
            
            if(flag==0){
                
                r->request_type=FILE_NOT_FOUND;
                strcpy(r->data,"File not found");
                send(client_socket_tcp, r, sizeof(st_request), 0);
                
            }
            else{

                r->request_type = req->request_type;
                strcpy(r->data,path);
                send(found_server->client_socket, r, sizeof(st_request), 0);
                recv(found_server->client_socket, r, sizeof(st_request), 0);
                if(r->request_type==ACK){
                    r->request_type=ACK;
                    strcpy(r->data,"Operation succesful!\n");
                    send(client_socket_tcp, r, sizeof(st_request), 0);
                }
                else{
                    r->request_type=FILE_NOT_FOUND;
                    strcpy(r->data,"File not found");
                    send(client_socket_tcp, r, sizeof(st_request), 0);
                }
  
            }
    
        }
    else if (req->request_type == ACK){

        request r = (request)malloc(sizeof(st_request));
        r->request_type = ACK;
        strcpy(r->data,"Operation succesful!\n");
        send(client_socket_tcp, r, sizeof(st_request), 0);

    }
    else if (req->request_type == COPY_REQUEST){
         char* source = (char*)malloc(sizeof(char)*MAX_DATA_LENGTH);
         char* desti = (char*)malloc(sizeof(char)*MAX_DATA_LENGTH);

         char* token = strtok(req->data,"|");
         while(token!=NULL){
             strcpy(source,token);
             token = strtok(NULL,"|");
             strcpy(desti,token);
         }

         ss source_no,dest_no;
         int flag=0;

        pthread_mutex_lock(&server_lock);

            for(int i=0;i<server_count;i++){

                pthread_mutex_lock(&ss_list[i]->lock);

                for(int j=0;j<ss_list[i]->path_count;j++){
                    if(strcmp(ss_list[i]->paths[j],source)==0){
                        source_no=ss_list[i];
                        flag++;
                    }
                    if(strcmp(ss_list[i]->paths[j],desti)==0){
                        dest_no=ss_list[i];
                        flag++;
                    }
                    if(flag==2){
                        break;
                    }
                }
                if(flag==2){
                    break;
                }

                pthread_mutex_unlock(&ss_list[i]->lock);

        }
        pthread_mutex_unlock(&server_lock);

        if(flag<2){

            request r = (request)malloc(sizeof(st_request));
            r->request_type=FILE_NOT_FOUND;
            strcpy(r->data,"File not found");
            send(client_socket_tcp, r, sizeof(st_request), 0);
        }
        else{

            request get_r = (request)malloc(sizeof(st_request));
            get_r->request_type = COPY_FROM;
            strcpy(get_r->data,source);
            send(source_no->client_socket, get_r, sizeof(st_request), 0);
            request put_r = (request)malloc(sizeof(st_request));
            while(get_r->request_type != ACK){
                recv(source_no->client_socket, get_r, sizeof(st_request), 0);
                put_r->request_type=COPY_TO;
                strcpy(put_r->data,get_r->data);
                send(dest_no->client_socket,put_r,sizeof(st_request),0);

            }

            request r = (request)malloc(sizeof(st_request));
            r->request_type = ACK;
            strcpy(r->data,"Copying succesful!\n");
            send(client_socket_tcp, r, sizeof(st_request), 0);



        }

    }

    else if (req->request_type == ADD_PATHS){
        char * ss_id = (char*)malloc(sizeof(char)*MAX_DATA_LENGTH);
        char** path = (char**)malloc(sizeof(char*)*MAX_CONNECTIONS);
        for(int i=0;i<MAX_CONNECTIONS;i++){
            path[i]=(char*)malloc(sizeof(char)*MAX_DATA_LENGTH);
        }
        int ind=0;
        char* token = strtok(req->data,"|");
        while(token!=NULL){
            if(ind==0){
                strcpy(ss_id,token);
            }
            else{
                strcpy(path[ind-1],token);
            }
            ind++;
            token = strtok(NULL,"|");
        }

        ss found_server = ss_list[atoi(ss_id)];

        pthread_mutex_lock(&found_server->lock);
        for(int i=0;i<ind-1;i++){
            strcpy(found_server->paths[found_server->path_count+i],path[i]);
        }
        found_server->path_count = found_server->path_count + ind-1;
        pthread_mutex_unlock(&found_server->lock);



    }

    else if (req->request_type == DELETE_PATHS){
        char * ss_id = (char*)malloc(sizeof(char)*MAX_DATA_LENGTH);
        char** path = (char**)malloc(sizeof(char*)*MAX_CONNECTIONS);
        for(int i=0;i<MAX_CONNECTIONS;i++){
            path[i]=(char*)malloc(sizeof(char)*MAX_DATA_LENGTH);
        }
        int ind=0;
        char* token = strtok(req->data,"|");
        while(token!=NULL){
            if(ind==0){
                strcpy(ss_id,token);
            }
            else{
                strcpy(path[ind-1],token);
            }
            ind++;
            token = strtok(NULL,"|");
        }

        ss found_server = ss_list[atoi(ss_id)];
        pthread_mutex_lock(&found_server->lock);
        for(int i=0;i<ind-1;i++){
            for(int j=0;j<found_server->path_count;j++){
                if(strcmp(found_server->paths[j],path[i])==0){
                    for(int k=j;k<found_server->path_count-1;k++){
                        strcpy(found_server->paths[k],found_server->paths[k+1]);
                    }
                    found_server->path_count--;
                    break;
                }
            }
        }
        pthread_mutex_unlock(&found_server->lock);


    }
    // Yet to work on depending on type of requests
}