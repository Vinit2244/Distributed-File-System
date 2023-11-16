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
int ack=0;

int connect_ss(char* port,struct sockaddr_in* server_addr){

    int sock=-1;
    struct sockaddr_in s=*server_addr;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("Socket creation failed");
    }
    memset(&s, '\0', sizeof(s));
    s.sin_family = AF_INET;
    s.sin_port = htons(atoi(port));
    s.sin_addr.s_addr = INADDR_ANY;
    server_addr=&s;
    return sock;

}

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

    request r=(request)malloc(sizeof(st_request));
    
    
    while (1)


    {

        // printf("%s\n",pack->port);
        struct sockaddr_in server_addr;
        int sock;

        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1)
        {
            perror("Socket creation failed");
        }
        memset(&server_addr, '\0', sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(atoi(pack->port));
        server_addr.sin_addr.s_addr = INADDR_ANY;

        connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

        
        if(sock==-1){
            printf(RED("Server %s disconnected!\n\n\n"),pack->port);
            pack->status=0;
            return NULL;
        }
        // printf(GREEN("Connected to server %s succesfully!\n\n\n"),pack->port);
        

        pack->status=1;
        r->request_type=PING;
        strcpy(r->data,"");
        
        int x=send(sock,r,sizeof(st_request),MSG_NOSIGNAL);
        if(x<0){
            pack->status=0;
             printf(RED("Server %s disconnected!\n\n\n"),pack->port);
             return NULL;
        }
        recv(sock,r,sizeof(st_request),0);
        

        if(r->request_type==PING){
            // printf("x: %d\n",x);
            
            printf(RED("Server %s disconnected!\n\n\n"),pack->port);
            // printf("%d\n",r->request_type);
            pthread_mutex_lock(&server_lock);
            for(int i=0;i<server_count;i++){
                if(strcmp(ss_list[i]->port,pack->port)==0){
                    ss_list[i]->status=0;
                }
            }
            pthread_mutex_unlock(&server_lock);
            return NULL;
            
            
        }
        
       
        sleep(5);
    
        
    }
    // close(pack->client_socket);
    return NULL;
}

void *backup_thread(){

    //this thread will check if an SS is backed up or not regularly
    // printf("hi\n");
    while(1){

        pthread_mutex_lock(&server_lock);

        for(int i=0;i<server_count;i++){

            if(ss_list[i]->is_backedup==0 && ss_list[i]->status==1 && ss_list[i]->added==1){
                
                int flag=0;
                int id1=-1,id2=-1;
                for(int j=0;j<server_count;j++){
                    if(strcmp(ss_list[j]->port,ss_list[i]->port)!=0 && ss_list[j]->has_backup==0 && i!=j){
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
                if(id1!=-1 && id2!=-1 && ss_list[id1]->status==1 && ss_list[id2]->status==1 && id1!=i && id2!=i){

                printf(ORANGE("Backing up server %s in servers %s %s\n\n\n"),ss_list[i]->port,ss_list[id1]->port,ss_list[id2]->port);
                ss_list[i]->is_backedup=1;
                ss_list[id2]->has_backup=1;
                ss_list[id1]->has_backup=1;
                    strcpy(ss_list[id1]->backup_port,ss_list[i]->port);
                    strcpy(ss_list[id2]->backup_port,ss_list[i]->port);

                ss_list[id1]->backup_path_count=ss_list[i]->path_count;
                ss_list[id2]->backup_path_count=ss_list[i]->path_count;
                for(int j=0;j<ss_list[i]->path_count;j++){


                    strcpy(ss_list[id1]->backup_paths[j],ss_list[i]->paths[j]);
                    strcpy(ss_list[id2]->backup_paths[j],ss_list[i]->paths[j]);
                     
                    if(strstr(ss_list[i]->paths[j],".txt")!=NULL){
                            // printf("path: %s\n",ss_list[i]->paths[j]);
                            request r=(request)malloc(sizeof(st_request));
                            request put_r=(request)malloc(sizeof(st_request));
                            struct sockaddr_in addr;
                            // int sock=connect_ss(ss_list[i]->port,&addr);
                            int sock = socket(AF_INET, SOCK_STREAM, 0);
                            if (sock == -1)
                            {
                                perror("Socket creation failed");
                            }
                            memset(&addr, '\0', sizeof(addr));
                            addr.sin_family = AF_INET;
                            addr.sin_port = htons(atoi(ss_list[i]->port));
                            addr.sin_addr.s_addr = INADDR_ANY;

                            int y=-1;

                            while(y!=0){
                            y=connect(sock, (struct sockaddr *)&addr, sizeof(addr));
                            }
                            
                            r->request_type=COPY;
                            strcpy(r->data,ss_list[i]->paths[j]);
                            
                            int x=send(sock,r,sizeof(st_request),0);
                            recv(sock,r,sizeof(st_request),0);

                            close(sock);

                            
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

                            
                            for(int i=0;i<tkn_cnt1-1;i++)
                            {
                                strcat(desti,tokens[i]);
                                strcat(desti,"/");
                            }

                            strcat(desti,"backup_");
                            strcat(desti,tokens[tkn_cnt1-1]);
                            
                            sprintf(put_r->data,"%s|%s",desti,token[1]);
                            
                            struct sockaddr_in addr1;   
                            
                            int sock_one = socket(AF_INET, SOCK_STREAM, 0);
                            if (sock_one == -1)
                            {
                                perror("Socket creation failed");
                            }
                            memset(&addr1, '\0', sizeof(addr1));
                            addr1.sin_family = AF_INET;
                            addr1.sin_port = htons(atoi(ss_list[id1]->port));
                            addr1.sin_addr.s_addr = INADDR_ANY;

                            y=-1;

                            while(y!=0){
                            y=connect(sock_one, (struct sockaddr *)&addr1, sizeof(addr1));
                            }
                            send(sock_one,put_r,sizeof(st_request),0);
                            close(sock_one);

                            struct sockaddr_in addr2;   
                            
                            int sock_two = socket(AF_INET, SOCK_STREAM, 0);
                            if (sock_two == -1)
                            {
                                perror("Socket creation failed");
                            }
                            memset(&addr2, '\0', sizeof(addr2));
                            addr2.sin_family = AF_INET;
                            addr2.sin_port = htons(atoi(ss_list[id2]->port));
                            addr2.sin_addr.s_addr = INADDR_ANY;

                            y=-1;
                            // printf("hi\n");
                            while(y!=0){
                            y=connect(sock_two, (struct sockaddr *)&addr2, sizeof(addr2));
                            }

                            
                            x=send(sock_two,put_r,sizeof(st_request),0);
                            close(sock_two);

                    }

                }
                }

            }
            else if(ss_list[i]->status==1){
                // // printf(ORANGE("Checking new files for %s\n\n\n"),ss_list[i]->port);
                // int id=-1;
                // for(int j=0;j<server_count;j++){
                //     if(strcmp(ss_list[i]->port,ss_list[j]->backup_port)==0){
                //         id=j;
                //         break;
                //     }
                // }

                // if(id!=-1){
                // int m_flag=0;
                // for(int j=0;j<ss_list[i]->path_count;j++){
                //     int flag=0;
                //     for(int k=0;k<ss_list[id]->backup_path_count;k++){
                //         if(strcmp(ss_list[i]->paths[j],ss_list[id]->backup_paths[k])==0 && (strstr(ss_list[i]->paths[j],".txt")!=NULL)){
                //             // printf("%s\n",ss_list[i]->paths[j]);
                            
                //             flag=1;
                //             break;
                //         }
                //     }
                //     if(flag==0  && (strstr(ss_list[i]->paths[j],".txt")!=NULL)){
                //             m_flag=1;
                //             break;
                        
                //     }

                // }

                // if(m_flag==1 || (ss_list[i]->path_count!=ss_list[id]->backup_path_count)){
                //     // printf("%d\n",m_flag);
                //     printf(ORANGE("Server %s is not backed up properly\n\n\n"),ss_list[i]->port);
                //     // ss_list[i]->is_backedup=0;
                    
                // }

                // }
                

            }
        }

        pthread_mutex_unlock(&server_lock);

    }

}