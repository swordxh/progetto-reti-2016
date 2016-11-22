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

int
main(int argc, char **argv)
{
    int sockfd; //usato per valore di ritorno socket, per indicare se è avvenuto errore se <0
    int n;
	char			recvline[MAXLINE + 1];
	struct sockaddr_in	servaddr; //quindi anche il server è una socket
	/*
	sockaddr_in is the actual IPv4 address layout (it has .sin_port and .sin_addr). 
	A UNIX domain socket will have type sockaddr_un and will have different fields defined, 
	specific to the UNIX socket family.
	*/

	if (argc != 2){
	/*	err_quit("usage: a.out <IPaddress>"); NON VA */
		printf("usage: a.out <IPaddress>\n");
		exit(-1);
	}

	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
	/*	err_sys("socket error"); */
		printf("socket error\n");
		exit(-2);
	}

/*	bzero(&servaddr, sizeof(servaddr)); */
	memset(&servaddr, 0, sizeof(servaddr)); //azzera serveraddress, vedi appunti su memset
	servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons(2016);	/* daytime server ,host to network short”
	converte i caratteri che specificano port# (in questo caso porta 60000) 
	nel formato binario di rete */
	if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0){
		/*inet_pton converte argv[1], cioè IP in input da shell, 
		in IP della famiglia AF_INET in questo caso, e mette il risultato
		della conversione in serveraddr.sin_addr
		It returns 1 if the address was valid for the specified
		address family, or 0 if the address was not parseable in the specified
		address family, or -1 if some system error occurred
		*/
	/*	err_quit("inet_pton error for %s", argv[1]); */
		printf("inet_pton error for %s\n", argv[1]);
		exit(-3);
	}

	if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){ //usata dal client TCP per stabilire la connessione con un server TCP 
		/*
		Client non specifica proprio indirizzo <IP, port#> 
		OS assegna port# TCP libero (lato client) e indirizzo IP di interfaccia di rete usata da client.
		esempi: ethernet (802.3), wireless (802.11), ognuno di questi avrà un indirizzo diverso
		*/


		/*err_sys("connect error");*/
		printf("connect error\n");
		exit(-4); 
	}

	while ( (n = read(sockfd, recvline, MAXLINE)) > 0) { 
		recvline[n] = 0;	/* null terminate */
		if (fputs(recvline, stdout) == EOF){
		/* 	err_sys("fputs error"); */
			printf("fputs error\n");
			exit(-5);
		}
	}

/*In generale, non si può assumere che la risposta di
un server sia restituita da singola read perciò: 
quando si legge da una socket TCP è necessario codificare read in un loop che 
termina o quando read restituisce 0 o un errore (i.e. un valore < 0)*/

	if (n < 0){
	/*	err_sys("read error");*/
		printf("read error\n");
		exit(-6);
	}

	exit(0);
}

