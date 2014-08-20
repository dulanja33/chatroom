/**

ADD THE E-NUMBERS OF YOUR GROUP'S MEMBERS HERE.

**/

#include <stdlib.h>

#include <unistd.h>

#include <stdio.h>

#include <string.h>

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


pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER; 


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

void broadcast_msg(char *message,int sender)

{

	int i;

	for(i=0;i<MAXCLIENTS;i++){

		if( clients[i] != NULL  && clients[i]->index != sender){

			

			send(clients[i]->sd,message,strlen(message),0);

				

			

		

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

	int connfd;

	int  i= *(int *)arg ;

	

	pthread_mutex_lock(&mutex); 

	connfd=clients[i]->sd;

	pthread_mutex_unlock(&mutex);

	while (!quit)

	{


	

	//clear the message buffer

	 memset(out_buf, 0, MAXMSG);

	//read a message from this client 

		

	n = recv(connfd,out_buf,MAXMSG,0);// information of the client by recvfrom function

	out_buf[n]=0;	

		

		

	//if EOF quit, otherwise broadcast it using broadcast_msg()

		if (n<=0){

			break;

			}

		else{

		    //printf("Received:%s\n",out_buf);

		    pthread_mutex_lock(&mutex); 

		    broadcast_msg(out_buf,clients[i]->index);

		    pthread_mutex_unlock(&mutex); 

		}

	}


	// Cleanup resources and free the client_t memory used by this client 

	perror("Client disconnected");

	fflush(stdout);


	close(connfd);
	client_t *temp=clients[i];
	clients[i]=NULL;
	
	free(temp);

	

	pthread_exit(0);

	

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

	

	int i;

	for(i=0;i<MAXCLIENTS;i++){


		if( clients[i] == NULL){

		   // printf("next free client %d\n",i);	

		    return i;

			

		}

	}

	return -1;

}


/**

Signal handler to clean up on SIGINT (Ctrl-C)

**/

void cleanup (int signal)

{

	puts("\n Caught interrupt. Exiting...\n");

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


	pthread_attr_t attr; // Thread attribute

	

	pthread_attr_init(&attr); // Creating thread attributes

  	pthread_attr_setschedpolicy(&attr, SCHED_FIFO); // FIFO scheduling for threads 

  	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); // Don't want threads (particualrly main)

                                                               // waiting on each other

	


	listenfd=setup_server();

	listen(listenfd,5);

	clilen=sizeof(cliaddr);

	

	int i;

	while(!quit)

	{


		i=next_free();

		if(i == -1){

			printf("MAXCLIENTS is exceeded ....\n");

			break;

		}else{

			//allocate memory for clients array

			clients[i]= malloc(sizeof(client_t ));


			

			//Accept an incoming connection 

			if((clients[i]->sd= accept(listenfd, (struct sockaddr *) &cliaddr, &clilen)) > 1){
				
			}else{

				puts("No proper client connection established...\n");
				break;
				

			}

	

			 // add client index - to identify the client

			clients[i]->index =i;

			

			//create a thread for new client 

			if ( pthread_create(&(clients[i]->tid),&attr, handle_client, &(clients[i]->index) ) )

			{

				printf("error creating thread.");

				abort();

			}

		

		

	

			//Allocate and set up new client_t struct in clients array 

			printf("client %d\n",clients[i]->index);

			sleep(0); // Giving threads some CPU time

			

			int j;

			for(j=0;j<MAXCLIENTS ;j++){

				
				if(clients[j]!=NULL){
					printf("%d  %d\n",clients[j]->index,clients[j]->sd);
				}
					

				

			}


		

		}

		

	}


	puts("Shutting down client connections...\n");

	fflush(stdout);

	int j;
	

	for(j=0;j<MAXCLIENTS ;j++){

				
		if(clients[j]!=NULL){
			client_t *temp=clients[j];
			close(clients[j]->sd);
	
			clients[j]=NULL;
	
			free(temp);
		}
					

				

	}
			
	

	close(listenfd);

	return 0;

}

