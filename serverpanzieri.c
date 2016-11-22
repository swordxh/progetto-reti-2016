/* #include     "unp.h" */
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

#include	<time.h>

#define MAXLINE	100
#define LISTENQ  10

int
main(int argc, char **argv)
{
	int			listenfd, connfd;
	struct sockaddr_in	servaddr; //quindi anche il server è una socket
	char			buff[MAXLINE];
	time_t			ticks;

	listenfd = socket(AF_INET, SOCK_STREAM, 0); 

/*	bzero(&servaddr, sizeof(servaddr)); */
	memset(&servaddr,0 , sizeof(servaddr));/*... riempimento dei dati della struttura server ... */ //azzera serveraddress, vedi appunti su memset
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	/*
	-INADDR_ANY is used when you don't need to bind a socket to a specific IP.
	 When you use this value as the address when calling bind(), 
	 the socket accepts connections to all the IPs of the machine.

    -When sending, a socket bound with INADDR_ANY binds to the default IP address,
     which is that of the lowest-numbered interface.

	-When you wrote your simple FTP server in project 1, you probably bound 
	your listening socket to the special IP address INADDR_ANY. This allowed 
	your program to work without knowing the IP address of the machine it was 
	running on, or, in the case of a machine with multiple network interfaces, 
	it allowed your server to receive packets destined to any of the interfaces. 
	In reality, the semantics of INADDR_ANY are more complex and involved.
	In the simulator, INADDR_ANY has the following semantics: When receiving, a
	socket bound to this address receives packets from all interfaces. 
	For example, suppose that a host has interfaces 0, 1 and 2. If a UDP socket 
	on this host is bound using INADDR_ANY and udp port 8000, then the socket will 
	receive all packets for port 8000 that arrive on interfaces 0, 1, or 2. 
	If a second socket attempts to Bind to port 8000 on interface 1, the Bind will 
	fail since the first socket already ``owns'' that port/interface.
*/
	servaddr.sin_port= htons(60000);	/* daytime server */

	bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)); 
	/*collega al socket un indirizzo locale. Cioè assegna IP address e port# (numero porta) al socket (servaddr in questo caso).
	Applicazione può collegarsi ad una porta specifica del socket server oppure no.
	solitamente client si collega a port# assegnato dinamicamente (via bind).
	A questo punto quindi il server ha creato con la bind un'interfaccia con la 
	 quale il client può interfacciarsi (avendo quest'ultimo a disposizione indirizzo IP 
	del socket tramite il quale fare le richieste).
	
	*/

	listen(listenfd, LISTENQ);
	/*
    ordina al kernel di far passare il socket da stato iniziale CLOSED a stato
    accettare richieste di inizio connessione per quel socket, accodandole in code del kernel.
	LISTENQ: è il backlog, specifica max n. di richieste di inizio connessione kernel può 
	mantenere in attesa su quella socket: sia connessioni non ancora stabilite, 
	cioè che non hanno ancora raggiunto lo stato ESTABLISHED, sia connessioni 
	stabilite.
	Quando server riceve SYN da client, se il TCP verifica che c’è un socket per quella richiesta,  
    •  crea una nuova entry in una coda delle connessioni incomplete,  
    •  risponde con FIN + ACK secondo il 3-way hand-shake.  
    •  entry creata rimane in coda fino a terminazione del 3-way hand- shake (o del timeout) 
    •  entry spostata in coda di connessioni completate a terminazione del 3WH e connessione stabilita 
	*/
	for ( ; ; ) {

		connfd = accept(listenfd, (struct sockaddr *) NULL, NULL);
		/*accept() invocata solo da TCP server 
         ◊  coda delle connessioni completate ≠ ø 
         => la prima delle entry nella coda delle connessioni completate viene consegnata alla accept()  
         ◊  accept() ne restituisce l’indice come risultato,  
         •  i.e., restituisce nuovo socket che identifica nuova connessione 
         ◊  Coda delle connessioni completate = ø (i.e., vuota)  => accept() bloccante, server resta in attesa 

         connfd: connected socket 
         ◊  nuovo descrittore di socket  
         ◊  identifica connessione instaurata con un certo client secondo le regole del listening socket “socketfd” passato in input 
         ◊  verrà utilizzato per scambiare dati nella nuova connessione 

         2° scope di accept (impostato in questo caso a NULL): puntatore a struttura sockaddr_in dove accept() scrive 
         indirizzo IP e port# del client con cui è instaurata la connessione associata 
         a socketfd 
         3° scope di accept (impostato in questo caso a NULL):puntatore a dimensione della struttura nel secondo scope
*/ 
        	ticks = time(NULL);
        	snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
		printf("about to send time: %.24s\r\n", buff);
        	write(connfd, buff, strlen(buff));

		close(connfd);
		/*
		restituisce 0 se tutto OK, -1 in caso di errore.
		•  utilizzata per chiudere un socket e terminare una connessione TCP. 
		•  TCP continua ad utilizzare il socket trasmettendo i dati che eventualmente stanno nel suo buffer interno, fino a che non sono stati trasmessi tutti 
		•  In caso di errore (che impedisce questa trasmissione) successivo a close() applicazione non è informata e l’altro end system non riceverà alcuni dati 
		•  Opzione (SO_LINGER socket option) modifica comportamento della close: 
		◊  close restituisce controllo a chiamante solo dopo che tutti i dati nei buffer sono stati correttamente trasmessi e riscontrati 
		
Close su socket condiviso:
•  sockfd è condiviso da più processi (padre e figlio ottenuto da una fork)  
=> socket mantiene il conto di quanti sono i processi a cui appartiene.  
•  close(sockfd)  
1.  decrementa di una unità questo contatore 
2.  non innesca la sequenza FIN+ACK+FIN+ACK di terminazione della connessione fino a che tale contatore > 0 
•  perchè esiste ancora un processo che tiene aperta la connessione. 
•  Per innescare sequenza di terminazione anche se ci sono ancora processi per quella connessione si usa la funzione shutdown(). 
		*/
	}
/*  

struct sockaddr_in

/*
	sockaddr_in is the actual IPv4 address layout (it has .sin_port and .sin_addr). 
	A UNIX domain socket will have type sockaddr_un and will have different fields defined, 
	specific to the UNIX socket family.

	server è un socket!

	servaddr.sin_family: famiglia protocolli a cui appartiene server
	servaddr.sin_addr.s_addr: IP sulla quale si mette in ascolto il server, cioè host dalla quale si aspetta una comunicazione
	servaddr.sin_port: settare porta del server sulla quale dovrà avvenire connessione client/server

*/











