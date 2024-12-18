#include <algorithm>
#include <iostream>
#include <ostream>
#include <ratio>
#include <unistd.h>  

#include <netdb.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <arpa/inet.h>

#include <errno.h>
#include <stdio.h> 
#include <string.h>

#include <thread>
#include <mutex>
#include <utility>
#include <vector>

//definim la adreça del servidor
#define SERV_PORT       8080           
#define SERV_HOST_ADDR "172.17.21.164"
//logitud dels misatges     

//conexions maximes               
#define BACKLOG 10                
std::vector<int> clientes;
std::mutex lockclients;
bool more_clients = true;
bool bescriure = true;
char msg[BUFSIZ];

void escrire(int sock){
    while (1) {
        int bytes;
        std::cout << "escrivint desde : " << sock << std::endl;
        bytes = recv(sock,msg, 1024, 0);
        std::cout << msg << std::endl;
        if (bytes > 0){
            lockclients.lock();
            for (int c2 : clientes){
                send(c2, msg, 1024, 0);
            } 
            lockclients.unlock();
        }
        if (bytes < 0){
            lockclients.lock();
            clientes.erase(std::find(clientes.begin(),clientes.end(),sock));
            lockclients.unlock();
            close(sock);
            break;
        }
    }
}

void apcept_Client(int  sockfd , sockaddr_in client, unsigned int len){
    std::vector<std::thread *> fils;
    while(more_clients){
        int socket = accept(sockfd, (struct sockaddr *)&client, &len);
        std::cout << "nova conexió" << std::endl;
        lockclients.lock();
        clientes.push_back(socket);
        more_clients = clientes.size() !=0 ;
        lockclients.unlock();
        std::thread * t = new std::thread(escrire,socket);
        fils.push_back(t);
    
    }
    for (auto t : fils){
        t->join();
    }
    for (auto &t : fils) {
        delete t;
    }
    std::cout << "Fil de aceptar clients tancat" << std::endl;
}

int main(int argc, char* argv[])          
{ 

    //descriptors dels socóls
    int sockfd, connfd ; 
    unsigned int len;
    //estructures per definir les adreces dels socóls     
    struct sockaddr_in servaddr, client; 
     
    int  len_rx, len_tx = 0;    
    //missages                 
    char buff_tx[BUFSIZ] = "Hola des del servidor!";
    char buff_rx[BUFSIZ];   
    
     
    //creem el socól del servidor amb els protocols ipv4 i tcp
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) 
    { 
        fprintf(stderr, "[servidor-error]:. %d: %s \n", errno, strerror( errno ));
        return -1;
    } 
    else
    {
        printf("[servidor]: Socól creat.\n"); 
    }
    

    memset(&servaddr, 0, sizeof(servaddr));
  

    servaddr.sin_family      = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR); 
    servaddr.sin_port        = htons(SERV_PORT); 

    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) 
    { 
        fprintf(stderr, "[servidor-error]: %d: %s \n", errno, strerror( errno ));
        return -1;
    } 
    else
    {
        printf("[servidor]: Socól ellaçat \n");
    }

    if ((listen(sockfd, BACKLOG)) != 0) 
    { 
        fprintf(stderr, "[servidor-error]:. %d: %s \n", errno, strerror( errno ));
        return -1;
    } 
    else
    {
        printf("[servidor]: Escoltant en el port %d \n\n", ntohs(servaddr.sin_port) ); 
    }
    
    len = sizeof(client); 
  
    std::thread escotar = std::thread(apcept_Client,sockfd, client, len);

  


    escotar.join();

       
} 