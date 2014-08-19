/**
ADD THE E-NUMBERS OF YOUR GROUP'S MEMBERS HERE.
**/
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <strings.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

/**
Some constants
MAXCLIENTS:	maximum number of clients that can connect at a time
MAXMSG:		maximum message length
PORTNO:		server port number
**/
#define MAXCLIENTS 8
#define MAXMSG 256
#define PORTNO 12345

/**
Data type to hold all information regarding a client connection
index: 	position in the clients array
sd: 	socket descriptor for this client
tid: 	ID of the thread handling this client
name: 	hostname/IP of the client 
**/
typedef struct {
	int index;
	int sd;
	pthread_t tid;
	char *name;
} client_t;

/**
Details of all clients currently connected to the server.
**/
static client_t *clients[MAXCLIENTS];

static volatile sig_atomic_t quit = 0;

/**
Send a received message to each client currently connected 
**/
void broadcast_msg(char *message,int size)
{
	int i;
	for(i=0;i<MAXCLIENTS;i++){
		if(clients[i]->index==i){
			
			write(clients[i]->sd,message,size);
			
		}
		

	}
}

/**
Thread function that handles an individual client
**/
void * handle_client (void *arg)
{
	char out_buf[MAXMSG];
	int n;
	int *connfd;
	connfd=(int *)arg ;
	while (!quit)
	{
	//read a message from this client 
		
		n = recv(*connfd,out_buf,MAXMSG,0);// information of the client by recvfrom function
		out_buf[n] = 0;
		//printf("Received:%s\n",out_buf);
		
	//if EOF quit, otherwise broadcast it using broadcast_msg()
		if (n==0)
			break;
		else
		    broadcast_msg(out_buf,n);
	}

	// Cleanup resources and free the client_t memory used by this client 
	perror("Client disconnected");
	close(*connfd);
	return NULL;
}

/**
Initialise the server socket and return it
**/
int setup_server(void)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	// Initialize socket structure 
	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(PORTNO);

	// Now bind the host address using bind() call
	bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
	
	

	return sockfd; 
}

/** 
Find the next unused slot in the clients array and return its index 
Returns -1 if MAXCLIENTS is exceeded
**/
int next_free(void)
{
	
	return -1;
}

/**
Signal handler to clean up on SIGINT (Ctrl-C)
**/
void cleanup (int signal)
{
	puts("\nCaught interrupt. Exiting...\n");
	quit = 1;
}

int main( int argc, char *argv[] )
{
	struct sockaddr_in cliaddr;
	int listenfd;
	socklen_t clilen;
	//Install signal handler for SIGINT
	signal(SIGINT, cleanup);
	//Initialise any synchronisation variables like mutexes, attributes and memory


	//allocate memory for clients array
	int c;
	for(c=0;c<MAXCLIENTS;c++)
		clients[c]= malloc(sizeof(client_t ));

	listenfd=setup_server();
	listen(listenfd,5);
	clilen=sizeof(cliaddr);
	
	int i=0;
	while(!quit)
	{

		//if max client break
		if(i==MAXCLIENTS){
			break;
		}
		//Accept an incoming connection 
		clients[i]->sd= accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
	
		
		//create a thread for new client 
		if ( pthread_create( &(clients[i]->tid), NULL, handle_client, &(clients[i]->sd)) )
		{
			printf("error creating thread.");
			abort();
		}
		
		 // add client index - to identify the client
		clients[i]->index =i;
	
		//Allocate and set up new client_t struct in clients array 
		printf("client %d\n",clients[i]->index);
		//Create a DETACHED thread to handle the new client until the quit is set
		pthread_detach((clients[i]->tid));


		
		
		i++;
	}

	puts("Shutting down client connections...\n");
	close(listenfd);
	return 0;
}
