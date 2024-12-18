#include <iostream>
#include <mutex>
#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <thread>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <unistd.h>

#define SERVER_ADDRESS  "172.17.21.164"     
#define PORT            8080 

/* Test sequences */
char buf_tx [BUFSIZ];
char buf_rx[BUFSIZ];                    
bool conn = true;
std::mutex lock_con;
std::mutex lock_usu;
char usuari[100 + BUFSIZ];
void escoltar(int sockfd){
    while (conn) {
        int bytes = recv(sockfd, buf_rx, 1024,0);
        if (bytes > 0){
            printf("%s \n", buf_rx);
        }
        if (bytes< 0 ){
            lock_con.lock();
            conn = false;
            lock_con.unlock();
        }
    }
}
void escriure(int sockfd){
    while (conn) {
        printf("> ");
  
        fgets(buf_tx, BUFSIZ, stdin);
        lock_usu.lock();
        strcat(usuari, buf_tx);
        
        lock_usu.unlock();
        write(sockfd, usuari, 1024);
        if (strcmp("/sortir", buf_tx)==0){
            lock_con.lock();
            conn = false;
            lock_con.unlock();
        }
    }
}

int main() 
{


    std::cout << "Introdueix el teu nom: ";
    lock_con.lock();
    std::cin >> usuari;
    strcat(usuari, ": ");
    lock_con.unlock();
    int sockfd; 
    struct sockaddr_in servaddr; 
    
    /* creació del socól */
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) 
    { 
        printf("CLIENT: Ha fallat la creació del socól..\n"); 
        return -1;  
    } 
    else
    {
        printf("CLIENT: Socól creat .\n"); 
    }
    
    
    memset(&servaddr, 0, sizeof(servaddr));

    /* Assinem la adreça del servidor */
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr( SERVER_ADDRESS ); 
    servaddr.sin_port = htons(PORT); 
  
    /* Intent de conexió al servidor*/
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) 
    { 
        printf("conexió amb el servidor fallada...\n");  
        return -1;
    } 
    
    printf("conectat al servidor..\n"); 
   


    std::thread escoltar_t = std::thread(escoltar, sockfd);
    std::thread escriure_t = std::thread(escriure, sockfd);
    escoltar_t.join();
    escriure_t.join();
    /* close the socket */
    close(sockfd); 
} 