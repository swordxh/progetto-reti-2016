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
#define maxlen 256

typedef struct lista{
    char name[maxlen];
    struct lista *next;
}nodo;

void serverstart (int sock, char nome[], nodo **head,volatile long* tst);
int isthere(char nome[], nodo** head);
void deletename(char nome[], nodo **head);

int main( int argc, char *argv[] ) {
    volatile long testandset=0;
    nodo *tmphead=NULL;
    nodo ** head=&tmphead;
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
        strcpy(buffer, "\0");
        n = read(newsockfd,buffer,maxlen-1);
        n = write(newsockfd,"test",5);
        if (n>0){
            if (!(isthere (buffer, head))){
                printf("%s è entrato\n", buffer);
                pid = fork();
                if (pid < 0) {
                    perror("errore in fork");
                    exit(1);
                }

                if (pid == 0) {

                    close(sockfd);
                    serverstart(newsockfd, buffer, head, &testandset);
                    exit(0);
                }
                else {
                    close(newsockfd);
                }
            }
            else{
                n = write(newsockfd,"c'è già un utente attivo con quel ID!",37);
                if (n < 0) {
                    printf("errore nella scrittura del socket");
                }
                close(newsockfd);
            }
        }
        else {close(newsockfd);
        }

    }
}

void serverstart (int sock, char nome[], nodo **head,volatile long* tst ){
    int n;
    int firsthit=0;
    char buffer[maxlen];
    bzero(buffer,maxlen);
    while(1){
        n = read(sock,buffer,maxlen-1);

        if (n < 0) {
            printf("errore nella lettura del socket");
            while (__sync_lock_test_and_set(tst,1));
            deletename(nome, head);
            printf("%s è uscito\n", nome);
            __sync_lock_release(tst);
            exit(1);
        }

        printf("Here is the message: %s\n",buffer);
        n = write(sock,"I got your message",18);
        while (__sync_lock_test_and_set(tst,1));
        deletename(nome, head);
        printf("%s è uscito\n", nome);
        __sync_lock_release(tst);

        if (n < 0) {
            printf("errore nella scrittura del socket");
            while (__sync_lock_test_and_set(tst,1));
            deletename(nome, head);
            printf("%s è uscito\n", nome);
            __sync_lock_release(tst);
            exit(1);
        }
    }
}

int isthere (char nome[], nodo **head){
    if ((*head)==NULL){
        (*head)=malloc(sizeof(nodo));
        (*head)->next=NULL;
        strcpy((*head)->name,nome);
        return(0);
    }
    else {
        nodo* tmp=*head;
        nodo* prev=NULL;
        while (tmp!=NULL){
            if ((strcmp(tmp->name, nome)==0)){
                return(1);
            }
            prev=tmp;
            tmp=tmp->next;
        }
        //qui ci va solo se non ha trovato il nome
        prev=malloc(sizeof(nodo));
        prev->next=NULL;
        strcpy(prev->name,nome);
        return(0);
    }
}

void deletename(char nome[], nodo **head){
    if ((*head)!=NULL){
        if ((strcmp((*head)->name, nome)==0)){
            free(*head);
            (*head)=NULL;
        }
        else{

            nodo* tmp=*head;
            nodo* prev=tmp;
            int found=0;
            while(tmp!=NULL){
                if ((strcmp(tmp->name, nome)==0)){
                    found=1;
                    break;
                }
                prev=tmp;
                tmp=tmp->next;
            }
            if (found==0) return;
            nodo* tmp2=tmp->next;
            free(tmp);
            prev->next=tmp2;
        }
    }
    return;
}
