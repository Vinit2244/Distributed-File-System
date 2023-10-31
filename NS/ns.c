#include "headers.h"

ss ss_list[100];        //List of all storage servers
int server_count = 0;     //Number of storage servers
packet send_buffer[100];    //Buffer to store packets to be sent
int send_count=0;          //Number of packets in buffer
int sockfd_udp;             //UDP socket to check for new initialisations
struct sockaddr_in server_addr_udp, client_addr_udp;    //UDP server and client addresses
socklen_t addr_size_udp;   //Size of UDP address


//Helper function to split string into tokens (n tokens)
char** processstring(char data[],int n){

    char** tokens = (char**)malloc(sizeof(char*)*n);
    for(int i=0;i<n;i++){
        tokens[i] = (char*)malloc(sizeof(char)*MAX_DATA_LENGTH);
    }

    int i=0;
    char* token = strtok(data,"|");
    int j=0;
    while(token!=NULL && j<n){
        j++;
        strcpy(tokens[i],token);
        token = strtok(NULL,"|");
        i++;
    }

    return tokens;

}

//Code to initialise nfs
void init_nfs(){

    //nothing as of now but any global pointers declared will be malloced here

    return;

}

//Client requests handled here
void client_handler(char data[]){
    
    //yet to work on based on client request format
    
    char** tokens = (char**)malloc(sizeof(char*)*3);
    tokens = processstring(data,3);
    char* path=(char*)malloc(sizeof(char)*MAX_DATA_LENGTH);
    strcpy(path,tokens[0]);
    


}

//Code to add a new storage server in naming server list
void init_storage(char data[]){
    
    //tokenise the string and create a new server object with extracted attributes

    char** tokens = (char**)malloc(sizeof(char*)*5);
    tokens = processstring(data,5);
    ss new_ss = (ss)malloc(sizeof(ss_info));
   
    strcpy(new_ss->ip,tokens[0]);
    strcpy(new_ss->port,tokens[2]);
    strcpy(new_ss->client_port,tokens[1]);

    new_ss->path_count = atoi(tokens[3]);
    char** paths = (char**)malloc(sizeof(char*)*new_ss->path_count);
    paths=processstring(tokens[4],new_ss->path_count);

    for(int i=0;i<new_ss->path_count;i++){
        strcpy(new_ss->paths[i],paths[i]);
    }


    //Locking CS and entering server storage list then updating the list
    pthread_mutex_lock(&server_lock);
    ss_list[server_count] = new_ss;
    server_count++;
    pthread_mutex_unlock(&server_lock);



    //send Registration ACK to the SS
    packet p = (packet)malloc(sizeof(send_packet));
    p->send_to=0;
    strcpy(p->port,tokens[2]);
    request r =(request)malloc(sizeof(st_request));
    r->request_type=REGISTRATION_ACK; 
    p->r=r;
    p->status=0;
    int x=sendto(sockfd_udp, p->r, sizeof(st_request), 0, (struct sockaddr*)&client_addr_udp, sizeof(client_addr_udp));
    

    return;
    

}

//Code to process the request according to request type
void process(request req){
    
    if(req->request_type == REGISTRATION_REQUEST){
        
        init_storage(req->data);  //Code to add a new storage server in naming server list
    }
    else if(req->request_type == REQ){
        //create a new thread for each client request here for multi client handling

        client_handler(req->data);  //Client requests handled here
    
    }

    //Yet to work on depending on type of requests

    

}


int main(){

    init_nfs(); //initialises ns server

    //declaring thread variables
    pthread_t send_thread;
    pthread_t receive_thread;
    pthread_t udp_thread;
    
    //UDP socket to check for new connections
    //TCP socket to check for new requests

    //constructing threads for listening to UDP and TCP sockets
    pthread_create(&udp_thread,NULL,&udp_handler,NULL);
    pthread_create(&send_thread,NULL,&send_handler,NULL);
    pthread_create(&receive_thread,NULL,&receive_handler,NULL);


    //joining threads
    pthread_join(send_thread,NULL);
    pthread_join(receive_thread,NULL);
    pthread_join(udp_thread,NULL);

    return 0;
    
}
