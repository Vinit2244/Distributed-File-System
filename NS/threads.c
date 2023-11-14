#include "headers.h"

pthread_mutex_t server_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t send_buffer_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t send_signal = PTHREAD_COND_INITIALIZER;
pthread_mutex_t status_lock = PTHREAD_MUTEX_INITIALIZER;

int server_socket_tcp, client_socket_tcp;
struct sockaddr_in server_addr_tcp, client_addr_tcp;
socklen_t client_addr_len_tcp = sizeof(client_addr_tcp);

server_status connections[100];
int connection_count=0;


void *receive_handler()
{
    printf(YELLOW("-------------------------TCP handler started-------------------------------\n\n\n"));

    server_socket_tcp = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_tcp == -1)
    {
        perror(RED("Socket creation failed"));
        
    }

    server_addr_tcp.sin_family = AF_INET;
    server_addr_tcp.sin_port = htons(NS_PORT);
    server_addr_tcp.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket_tcp, (struct sockaddr *)&server_addr_tcp, sizeof(server_addr_tcp)) == -1)
    {
        perror(RED("Binding to port failed"));
        
    }

    if (listen(server_socket_tcp, MAX_CONNECTIONS) == -1)
    {
        perror("Listening on port failed");
        
    }

    while (1)
    {
        client_socket_tcp = accept(server_socket_tcp, (struct sockaddr *)&client_addr_len_tcp, &client_addr_len_tcp);
        if (client_socket_tcp == -1)
        {
            perror("Accepting connection failed");
        }
        request req = (request)malloc(sizeof(st_request));
        int x=recv(client_socket_tcp,req,sizeof(st_request),0);
        
        process(req);  //thread
        
        free(req);
        
        close(client_socket_tcp);
        
    }

    close(server_socket_tcp);

    return NULL;
}

void *server_handler(void *p)
{

    ss pack = (ss)p;
    
    pack->server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (pack->server_socket == -1)
    {
        perror("Socket creation failed");
    }
    memset(&pack->server_addr, '\0', sizeof(pack->server_addr));
    pack->server_addr.sin_family = AF_INET;
    pack->server_addr.sin_port = htons(atoi(pack->port));
    pack->server_addr.sin_addr.s_addr = INADDR_ANY;


    // if(send(pack->server_socket,r,sizeof(st_request),0)<0){
    //     perror("Send error");
    // }
    
    // recv(pack->server_socket,r,sizeof(st_request),0);
    int x=connect(pack->server_socket, (struct sockaddr *)&pack->server_addr, sizeof(pack->server_addr));
    if(x==0)printf(GREEN("Connected to server succesfully!\n\n\n"));
    else perror("Connection error");
    while (1)
    {

        request r=(request)malloc(sizeof(st_request));
        r->request_type=PING;
        strcpy(r->data,"");
        int x=send(pack->server_socket,r,sizeof(st_request),0);
    
        if(x<0)perror("Send error");
        
        while(r->request_type!=ACK){
            int x=recv(pack->server_socket,r,sizeof(st_request),0);
            // printf("x: %d\n",x);
            if(x==0)
            {
            printf(RED("Server %s disconnected!\n\n\n"),pack->port);
            return NULL;

            }
            // printf("%d\n",r->request_type);
            pthread_mutex_lock(&status_lock);
            for(int i=0;i<connection_count;i++){
                if(strcmp(pack->port,connections[i].port)==0 && connections[i].status==1){
                    connections[i].status=0;
                    break;
                }
            }
            pthread_mutex_unlock(&status_lock);
            // return NULL;
            // break;
        }
        
       
        free(r);
        time_t current=time(NULL);
        while((int)(difftime(time(NULL),current))<=5){

        }
        //yet to write
    }
    // close(pack->client_socket);
    close(pack->server_socket);
    return NULL;
}

void *backup_thread(){

    //this thread will check if an SS is backed up or not regularly

    while(1){

        pthread_mutex_lock(&server_lock);

        for(int i=0;i<server_count;i++){

            if(ss_list[i]->is_backedup==0 && ss_list[i]->status==1){
                // printf(ORANGE("Searching backup for server %s\n\n\n"),ss_list[i]->port);
                //search for two free online servers
                int flag=0;
                int id1=-1,id2=-1;
                for(int j=0;j<server_count;j++){
                    if(strcmp(ss_list[j]->port,ss_list[i]->port)!=0 && ss_list[j]->is_backedup==0){
                        if(flag==0){
                            id1=j;
                            flag=1;
                        }
                        else{
                            id2=j;
                            break;
                        }
                    }
                }

                //copy paths in these two servers
                if(id1!=-1 && id2!=-1){

                printf(ORANGE("Backing up server %s in servers %s %s\n\n\n"),ss_list[i]->port,ss_list[id1]->port,ss_list[id2]->port);
                ss_list[id1]->is_backedup=1;
                ss_list[id2]->is_backedup=1;
                ss_list[id1]->has_backup=1;

                for(int j=0;j<ss_list[i]->path_count;j++){


                    strcpy(ss_list[id1]->backup_paths[j],ss_list[i]->paths[j]);
                    strcpy(ss_list[id2]->backup_paths[j],ss_list[i]->paths[j]);

                    //check if the current path is to a file , if yes make a copy request 
                    if(strstr(ss_list[i]->paths[j],".txt")!=NULL){
                        printf("path: %s\n",ss_list[i]->paths[j]);
                        request r=(request)malloc(sizeof(st_request));
                        request put_r=(request)malloc(sizeof(st_request));
                        r->request_type=COPY;
                        strcpy(r->data,ss_list[i]->paths[j]);
                        send(ss_list[i]->server_socket,r,sizeof(st_request),0);
                        while(r->request_type!=ACK){
                            
                            recv(ss_list[i]->server_socket,r,sizeof(st_request),0);
                            if(r->request_type==ACK)break;
                            // printf("hi\n");
                            // printf("%d\n",r->request_type);
                            put_r->request_type = PASTE;
                            char** token = (char**)malloc(sizeof(char*)*2);
                            for(int i=0;i<2;i++)
                            {
                                token[i]=(char*)malloc(sizeof(char)*MAX_DATA_LENGTH);
                            }
                            int tkn_cnt=0;
                            char* tkn=strtok(r->data,"|");
                            while(tkn!=NULL)
                            {
                                strcpy(token[tkn_cnt],tkn);
                                tkn_cnt++;
                                tkn=strtok(NULL,"|");
                            }
                            char* desti = (char*)malloc(sizeof(char)*MAX_DATA_LENGTH);
                            strcpy(desti,"");

                            char** tokens = (char**)malloc(sizeof(char*)*10);
                            for(int i=0;i<10;i++)
                            {
                                tokens[i]=(char*)malloc(sizeof(char)*MAX_DATA_LENGTH);
                            }

                            int tkn_cnt1=0;
                            char* tkn1=strtok(token[0],"/");
                            while(tkn1!=NULL)
                            {
                                strcpy(tokens[tkn_cnt1],tkn1);
                                tkn_cnt1++;
                                tkn1=strtok(NULL,"/");
                            }

                            // for(int k=0;k<tkn_cnt1-1;k++){
                            //     strcat(desti,tokens[k]);
                            //     strcat(desti,"/");
                            // }

                            strcat(desti,"backup_");
                            strcat(desti,tokens[tkn_cnt1-1]);
                            
                            sprintf(put_r->data,"%s|%s",desti,token[1]);
                            send(ss_list[id1]->server_socket,put_r,sizeof(st_request),0);

                            
                            // recv(ss_list[id2]->server_socket,r,sizeof(st_request),0);
                        }


                        r->request_type=COPY;
                        strcpy(r->data,ss_list[i]->paths[j]);
                        send(ss_list[i]->server_socket,r,sizeof(st_request),0);
                        while(r->request_type!=ACK){

                            recv(ss_list[i]->server_socket,r,sizeof(st_request),0);
                            if(r->request_type==ACK)break;
                            put_r->request_type = PASTE;
                            char** token = (char**)malloc(sizeof(char*)*2);
                            for(int i=0;i<2;i++)
                            {
                                token[i]=(char*)malloc(sizeof(char)*MAX_DATA_LENGTH);
                            }
                            int tkn_cnt=0;
                            char* tkn=strtok(r->data,"|");
                            while(tkn!=NULL)
                            {
                                strcpy(token[tkn_cnt],tkn);
                                tkn_cnt++;
                                tkn=strtok(NULL,"|");
                            }
                            char* desti = (char*)malloc(sizeof(char)*MAX_DATA_LENGTH);
                            strcpy(desti,"");

                            char** tokens = (char**)malloc(sizeof(char*)*10);
                            for(int i=0;i<10;i++)
                            {
                                tokens[i]=(char*)malloc(sizeof(char)*MAX_DATA_LENGTH);
                            }

                            int tkn_cnt1=0;
                            char* tkn1=strtok(token[0],"/");
                            while(tkn1!=NULL)
                            {
                                strcpy(tokens[tkn_cnt1],tkn1);
                                tkn_cnt1++;
                                tkn1=strtok(NULL,"/");
                            }

                            // for(int k=0;k<tkn_cnt1-1;k++){
                            //     strcat(desti,tokens[k]);
                            //     strcat(desti,"/");
                            // }

                            strcat(desti,"backup_");
                            strcat(desti,tokens[tkn_cnt1-1]);

                           

                            sprintf(put_r->data,"%s|%s",desti,token[1]);
                            send(ss_list[id2]->server_socket,put_r,sizeof(st_request),0);
                            // recv(ss_list[id2]->server_socket,r,sizeof(st_request),0);
                        }

                        


                        free(r);
                    }

                }
                }

            }

        }

        pthread_mutex_unlock(&server_lock);

    }

}