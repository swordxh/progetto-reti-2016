#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include<pthread.h>
#include <sys/file.h>
#include <fcntl.h>
#define maxlen 256

void serverstart (int sock, char nome[]);

int main( int argc, char *argv[] ) {
    int sockfd, newsockfd;
    char buffer[maxlen];
    struct sockaddr_in serv_addr;
    int n, pid;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        printf("errore nel apertura socket ");
        exit(1);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));

    memset(&serv_addr,0 , sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(2016);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("errore in bind%s\n",strerror(errno));
        exit(1);
    }
    listen(sockfd,10);
    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) NULL, NULL);
        int n;
        if (newsockfd < 0) {
            perror("errore in accept");
            exit(1);
        }
        char buffer[maxlen];
        bzero(buffer,maxlen);
        n = read(newsockfd,buffer,maxlen-1);
        if (n>0){
            printf("%s è entrato\n", buffer);
            pid = fork();
            if (pid < 0) {
                perror("errore in fork");
                exit(1);
            }

            if (pid == 0) {

                close(sockfd);
                serverstart(newsockfd, buffer);
                exit(0);
            }
            else {
                close(newsockfd);
            }
        }
        else{
            printf("errore nella lettura del socket");
            close(newsockfd);
        }
    }

}

void serverstart (int sock, char nome[] ){
    int n;
    char buffer[maxlen];
    char buffer2[maxlen];
    bzero(buffer, maxlen);
    bzero(buffer2, maxlen);
    int variable=0;
    snprintf(buffer, maxlen, "%s.txt", nome);
    int fd = open(buffer, O_CREAT | O_RDWR);
    if(fd < 0) exit(1);
    if (!(flock(fd, LOCK_EX | LOCK_NB))){
        if(read(fd, buffer, 1)==0){
            n = read(sock,buffer2,maxlen);

            if (n < 0) {
                printf("errore nella lettura del socket");
                close(sock);
                flock(fd, LOCK_UN | LOCK_NB);
                exit(1);
            }
            printf("BUFFER %s\n",buffer2);
            variable=atoi(buffer2);
            n = write(fd, buffer2, maxlen);
            n = write(sock, buffer2, maxlen);
            if (n < 0) {
                printf("errore nella scrittura del socket");
                close(sock);
                flock(fd, LOCK_UN | LOCK_NB);
                exit(1);
            }
        }
        while(1){
            n = read(sock,buffer,maxlen);

            if (n < 0) {
                printf("errore nella lettura del socket");
                close(sock);
                flock(fd, LOCK_UN | LOCK_NB);
                exit(1);
            }

            printf("Here is the message: %s\n",buffer);
            n = write(sock,"I got your message",18);

            if (n < 0) {
                printf("errore nella scrittura del socket");
                flock(fd, LOCK_UN | LOCK_NB);
                close(sock);
                exit(1);
            }
        }
    }
    else{
        n = write(sock,"c'è già un utente attivo con quel ID!",37);
        if (n < 0) {
            printf("errore nella scrittura del socket");
        }
        close(sock);
    }
}
