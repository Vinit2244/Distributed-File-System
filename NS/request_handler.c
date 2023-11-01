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

    

    // Yet to work on depending on type of requests
}