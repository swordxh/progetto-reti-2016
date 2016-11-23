/*
STRUTTURA FUNZIONAMENTO CLIENT:
-scrivi su buffer id cliente
for(;;){
-leggi da buffer finchè non è finito
-scrivi su shell client ciò che è nel buffer
-prendi da shell stringa che scrive client
-scrivila nel buffer
}
*/


/* #include	"unp.h" */
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

#define MAXLINE 99

int main(int argc, char **argv){
    
    int			sockfd, n;
    char			recvline[MAXLINE + 1];
    struct sockaddr_in	servaddr;
    char message[MAXLINE];
    
    
    if (argc != 3){
        /*	err_quit("usage: a.out <IPaddress>"); NON VA */
        printf("errore di lancio, usa: client tuoIP tuoID\n");
        exit(-1);
    }
    
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        /*	err_sys("socket error"); */
        printf("socket error\n");
        exit(-2);
    }
    
    /*	bzero(&servaddr, sizeof(servaddr)); */
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(2016);	/* daytime server */
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0){
        /*	err_quit("inet_pton error for %s", argv[1]); */
        printf("inet_pton error for %s\n", argv[1]);
        exit(-3);
    }
    
    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
        /*err_sys("connect error");*/
        printf("connect error\n");
        exit(-4);
    }
    
    //mando id al server
    
    if ((n=write(sockfd,argv[2],MAXLINE))<0){
        printf("write() error\n");
        exit(-6);
    }
    
    n=0;
    
    for(;;){ //per far si che una volta finito di incrementare ricomincia e rifa da capo finchè ctrl+c
        //stampa su shell client (leggendo dal buffer) quello che manda server
        
        if ((n = read(sockfd, recvline, MAXLINE)) > 0) {
            recvline[n] = 0;
            if (fputs(recvline, stdout) == EOF){
                printf("fputs() error\n");
                exit(-7);
            }
        }
        if (n < 0){
            
            printf("read() error\n");
            exit(-8);
        }
        
        n=0;
        
        
        /*procedimento di stampa su shell client di quello che server manda nel socket:
     -read legge dal socket e mette su recvline per un massimo di MAXLINE righe
     -fput fa la stampa su shell (stdout) di quello che c'è in recvline)
     */
        
        
        //client scrive ora su buffer con valore da inizializzare/decrementare o aumentare
        
        
        if (fgets(message, MAXLINE, stdin)==NULL) {
            printf("fgets() error\n");
            exit(-9);
        }
        
        while(message[n]!='\n' && (n<sizeof(message))){
            n++;
        }
        message[n]='\0';
        
        if ((n=write(sockfd,message,sizeof(message))<0)){
            printf("write() error\n");
            exit(-10);
        }
    }
    exit(0);
}
