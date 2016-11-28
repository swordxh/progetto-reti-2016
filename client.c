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

#define maxlen 99

int main(int argc, char **argv){
    
    int sockfd, n; //identificatore socket
    char recvline[maxlen+1]; //stringa buffer del socket
    struct sockaddr_in	servaddr; //creo server per famiglia indirizzi IPV4 (AF_INET)
    char message[maxlen]; //stringa scrittura su shell client
    
    
    if (argc != 3){
        printf("errore di lancio, usa: client tuoIP tuoID\n");
        exit(-1);
    }
    
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){ //creo socket TCP: famiglia indirizzi AF_INET con socket di tipo stream (SOCK_STREAM)
        printf("errore creazione socket con socket()\n");
        exit(-2);
    }
    
    //INIZIALIZZAZIONE CAMPI STRUTTURA SOCKET
    
    memset(&servaddr, 0, sizeof(servaddr)); //azzero la struttura del socket server: setto a 0 un numero pari a sizeof(servaddr) di byte a partire da &serveraddr
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(2016); //imposto porta# sulla quale avviene comunicazione server-client a 2016 e converto questo numero nel formato binario della rete
    
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0){ //converto stringa caratteri in argv[1] (indirizzo I client) in indirizzo di rete della famiglia AF_INET e lo copio in &servaddr.sin_addr
        printf("errore funzione inet_pton() per l'indirizzo IP %s\n", argv[1]);
        close(sockfd);
        exit(-3);
    }
    
    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){ //stabilisco connessione tra client TCP e server TCP
        printf("errore di connessione tramite connect()\n");
        close(sockfd);
        exit(-4);
    }
    
    if ((n = write(sockfd,argv[2],maxlen)) < 0 ){//mando id client al server
        printf("errore scrittura buffer tramite write()\n");
        close(sockfd);
        exit(-5);
    }
    
    for(;;){
        
        if ((n = read(sockfd, recvline, maxlen)) > 0) { //leggo buffer scritto da server e lo metto nella stringa recvline
            recvline[n] = 0;
            if (fputs(recvline, stdout) == EOF){ //scrivo recvline su shell client
                printf("errore fputs()\n");
                close(sockfd);
                exit(-6);
            }
        }
        if (n < 0){
            printf("errore read()\n");
            close(sockfd);
            exit(-7);
        }
        
        if (fgets(message, maxlen, stdin)==NULL) { //leggo da shell ciò che scrive il client
            printf("errore fgets()\n");
            close(sockfd);
            exit(-8);
        }
        
        n=0;
        
        while(message[n]!='\n' && (n<sizeof(message))){ //imposto il carattere di fine stringa di message
            n++;
        }
        message[n]='\0';
        
        if ((n=write(sockfd,message,sizeof(message))<0)){ //mando al server la stringa message
            printf("errore write()\n");
            close(sockfd);
            exit(-9);
        }
    }
    exit(0);
}
