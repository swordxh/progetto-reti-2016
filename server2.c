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
#include <sys/file.h>
#include <fcntl.h>
#define maxlen 99

void serverstart (int sock, char nome[]);

int main( int argc, char *argv[] ) {
    int sockfd, newsockfd;
    char buffer[maxlen];
    struct sockaddr_in serv_addr;
    int n, pid;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        printf("errore nel apertura socket\n");
        exit(1);
    }

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
            printf("errore in accept\n");
            close(sockfd);
            close(newsockfd);
            exit(2);
        }
        char buffer[maxlen];
        bzero(buffer,maxlen);
        var=0;
        n = read(newsockfd,buffer,maxlen);
        if (n>0){
            printf("%s è entrato\n", buffer);
            pid = fork();
            if (pid < 0) {
                perror("errore in fork\n");
                exit(3);
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
            printf("errore nella lettura del socket\n");
            close(newsockfd);
        }
    }

}


void serverstart (int sock, char nome[] ){
    int n;
    char buffer[maxlen];
    char buffer2[maxlen];
    int variable=0;
    snprintf(buffer, maxlen, "%s.txt", nome);
    int fd = open(buffer, O_CREAT | O_RDWR , 0666);
    if(fd < 0){
        printf("errore nel apertura del file %s\n",buffer);
        close(sock);
        exit(4);
    }
    memset(buffer,0 , sizeof(buffer));

    if (!(flock(fd, LOCK_EX | LOCK_NB))){

        if(n=read(fd, buffer, 1)==0){
            do{
                n = write(sock,"inserisci un numero\n",maxlen);
                if (n < 0) {
                    printf("errore nella scrittura del socket\n");
                    close(sock);
                    flock(fd, LOCK_UN | LOCK_NB);
                    close(fd);
                    exit(5);
                }
                memset(buffer,0 , sizeof(buffer));
                n = read(sock,buffer,maxlen);

                if (n < 0) {
                    printf("errore nella lettura del socket\n");
                    close(sock);
                    flock(fd, LOCK_UN | LOCK_NB);
                    close(fd);
                    exit(6);
                }

            }while (sscanf(buffer, "%d", &variable) == 0);

            lseek(fd, 0, SEEK_SET);
            n = write(fd, buffer, maxlen);
            snprintf(buffer2, maxlen, "Hai inizializzato il numero: %s, inserisci un incremento\n", buffer);
            n = write(sock, buffer2, maxlen);
            if (n < 0) {
                printf("errore nella scrittura del socket\n");
                close(sock);
                flock(fd, LOCK_UN | LOCK_NB);
                close(fd);
                exit(7);
            }
        }
        else if (n>=0){
            lseek(fd, 0, SEEK_SET);
            n=read(fd, buffer, maxlen);
            if (n < 0) {
                printf("errore nella lettura del file\n");
                close(sock);
                flock(fd, LOCK_UN | LOCK_NB);
                close(fd);
                exit(8);
            }
	    if (sscanf(buffer, "%d", &variable)==0){
	    	printf("il file è corrotto/n");
                close(sock);
                flock(fd, LOCK_UN | LOCK_NB);
                close(fd);
                exit(8);
	    }

            //variable=atoi(buffer);
            lseek(fd, 0, SEEK_SET);
            snprintf(buffer2, maxlen, "Nella vecchia sessione il numero inizializzato era: %s, inserisci un incremento\n", buffer);
            n = write(sock,buffer2,maxlen);
            if (n < 0) {
                printf("errore nella scrittura del socket\n");
                close(sock);
                flock(fd, LOCK_UN | LOCK_NB);
                close(fd);
                exit(9);
            }
        }
        while(1){
	    int n2=0;
            int variable2=0;
            do{
                memset(buffer,0 , sizeof(buffer));
                n = read(sock,buffer,maxlen);

                if (n < 0) {
                    printf("errore nella lettura del socket\n");
                    close(sock);
                    flock(fd, LOCK_UN | LOCK_NB);
                    close(fd);
                    exit(10);
                }

                if ((n2=sscanf(buffer, "%d", &variable2))==0){
                    n = write(sock,"devi inserire un numero!\n",26);
                    if (n < 0) {
                        printf("errore nella scrittura del socket\n");
                        flock(fd, LOCK_UN | LOCK_NB);
                        close(sock);
                        close(fd);
                        exit(11);
                    }
                }
	    }while (n2==0);

            variable=variable+variable2;
            memset(buffer,0 , sizeof(buffer));
            snprintf(buffer, maxlen, "%d\n", variable);
            lseek(fd, 0, SEEK_SET);
            n = write(fd, buffer, maxlen);
            n=write(sock,buffer,maxlen);
            if (n < 0) {
                printf("errore nella scrittura del socket\n");
                flock(fd, LOCK_UN | LOCK_NB);
                close(sock);
                close(fd);
                exit(12);
            }

        }
        flock(fd, LOCK_UN | LOCK_NB);
        close(sock);
        close(fd);
        exit(0);
    }
    else{
        n = write(sock,"c'è già un utente attivo con quel ID!\n",40);
        if (n < 0) {
            printf("errore nella scrittura del socket\n");
        }
        close(fd);
        close(sock);
        exit(13);
    }
}
