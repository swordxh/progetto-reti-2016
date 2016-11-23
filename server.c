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

    int var=0;

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) NULL, NULL);
        int n;
        if (newsockfd < 0) {
            printf("errore in accept");
            exit(1);
        }
        char buffer[maxlen];
        bzero(buffer,maxlen);
        int var=0;
        n = read(newsockfd,buffer,maxlen);
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
    int variable=0;
    snprintf(buffer, maxlen, "%s.txt", nome);
    int fd = open(buffer, O_CREAT | O_RDWR , 0666);
    if(fd < 0) exit(1);


    if (!(flock(fd, LOCK_EX | LOCK_NB))){

        if(read(fd, buffer, 1)==0){
            n = write(sock,"inserisci un numero\n",maxlen);
            if (n < 0) {
                printf("errore nella scrittura del socket");
                close(sock);
                flock(fd, LOCK_UN | LOCK_NB);
                close(fd);
                exit(1);
            }
            memset(buffer, 0, maxlen);
            n = read(sock,buffer,maxlen);

            if (n < 0) {
                printf("errore nella lettura del socket");
                close(sock);
                flock(fd, LOCK_UN | LOCK_NB);
                close(fd);
                exit(1);
            }
            buffer[n]='\0';
            printf("BUFFER %s\n",buffer);
            variable=atoi(buffer);
            printf("questo è l intero:%d",variable);
            n = write(fd, buffer, maxlen);
            n = write(sock, buffer, maxlen);
            if (n < 0) {
                printf("errore nella scrittura del socket");
                close(sock);
                flock(fd, LOCK_UN | LOCK_NB);
                close(fd);
                exit(1);
            }
        }
        else if (read(fd, buffer, maxlen)>=0){
            variable=atoi(buffer);
            n = write(sock,buffer,maxlen);
            if (n < 0) {
                printf("errore nella scrittura del socket");
                close(sock);
                flock(fd, LOCK_UN | LOCK_NB);
                close(fd);
                exit(1);
            }
        }
        while(1){
            memset(buffer, 0, maxlen);
            do{
                n = read(sock,buffer,maxlen);

                if (n < 0) {
                    printf("errore nella lettura del socket");
                    close(sock);
                    flock(fd, LOCK_UN | LOCK_NB);
                    close(fd);
                    exit(1);
                }

                if ((strcmp(buffer, "inc")!=0) && (strcmp(buffer, "dec")!=0)){
                    n = write(sock,"devi inserire dec o inc!",25);
                    if (n < 0) {
                        printf("errore nella scrittura del socket");
                        flock(fd, LOCK_UN | LOCK_NB);
                        close(sock);
                        close(fd);
                        exit(1);
                    }
                }


            }while ((strcmp(buffer, "inc")!=0) && (strcmp(buffer, "dec")!=0));

            if (strcmp(buffer, "dec")==0){
                variable=variable-1;
            }
            else {
                variable=variable+1;
            }
            snprintf(buffer, maxlen, "%d\0", variable);
            n=write(sock,buffer,maxlen);
            if (n < 0) {
                printf("errore nella scrittura del socket");
                flock(fd, LOCK_UN | LOCK_NB);
                close(sock);
                close(fd);
                exit(1);
            }

        }
        flock(fd, LOCK_UN | LOCK_NB);
        close(sock);
        close(fd);
        exit(0);
    }
    else{
        n = write(sock,"c'è già un utente attivo con quel ID!",39);
        if (n < 0) {
            printf("errore nella scrittura del socket");
        }
        close(fd);
        close(sock);
        exit(2);
    }
}
