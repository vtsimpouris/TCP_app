/* A simple server in the internet domain using TCP
   The port number is passed as an argument 
   
   Rensselaer Polytechnic Institute (RPI)
*/

#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>

void error(char *msg)
{
  perror(msg);
  exit(1);
}

struct arg_struct{
  int newsockfd;
  int linenumber;
  int servicenumber;
};
void *connectionHandler (void *arguments){
	struct arg_struct *args = arguments;	
	int linenumber = args->linenumber;
	int newsockfd = args->newsockfd;
	char buffer[256];
	int n,M,i;
	int buflen,tmp,datalen;
	
	n = read(newsockfd,(char*)&buflen,sizeof(buflen));
	buflen = ntohl(buflen);
	char addr[buflen];
	bzero(addr,buflen);
	n = read(newsockfd,addr,buflen);
	printf("Here is the senter address: %s\n",addr);
	n = read(newsockfd,&tmp,sizeof(int));
	M = ntohl(tmp);
	printf("Here is the number of messages: %d\n",M);
        datalen = 14;//sizeof(addr) doesnt work well;
        tmp = htonl(datalen);
        n = write(newsockfd,(char *)&tmp,sizeof(tmp));
	for(;;){
		sleep(1);
		if(linenumber == args->servicenumber){
		  for(i=0;i<M;i++){
		    bzero(buffer,256);
  		    n = read(newsockfd,buffer,255);
  		    if (n < 0) error("ERROR reading from socket");
  		    printf("Here is the message: %s\n",buffer);
		    n = write(newsockfd,buffer,255);
  		    if (n < 0) error("ERROR writing to socket");

		   
		   //datalen = 14;//sizeof(addr) doesnt work well;
  		   //tmp = htonl(datalen);
  		   //n = write(newsockfd,(char *)&tmp,sizeof(tmp));
  		   //if (n < 0) 
    		   //	error("ERROR writing to socket"); 
  		   n = write(newsockfd,addr,datalen);

		  }
		  close(newsockfd);
		  printf("i amd done with %dth customer\n", linenumber);		
		  args->servicenumber ++;
		  pthread_exit(0);
		}
	}
	pthread_exit(0);
	
}

int main(int argc, char *argv[])
{
  int sockfd, newsockfd, portno, clilen;
  char buffer[256];
  struct sockaddr_in serv_addr, cli_addr;
  struct arg_struct args;
  int n;
  pthread_t socket_thread;
  if (argc < 2) {
    fprintf(stderr,"ERROR, no port provided\n");
    exit(1);
  }
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) 
    error("ERROR opening socket");
  bzero((char *) &serv_addr, sizeof(serv_addr));
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  if (bind(sockfd, (struct sockaddr *) &serv_addr,
           sizeof(serv_addr)) < 0) 
    error("ERROR on binding");
  listen(sockfd,5);
  clilen = sizeof(cli_addr);
  args.linenumber = 0;
  args.servicenumber = 1;
  for(;;){
  	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
  	if (newsockfd < 0) 
    		error("ERROR on accept");
	args.newsockfd = newsockfd;
	args.linenumber ++;	
	pthread_create(&socket_thread,NULL,connectionHandler,(void *) &args);

  }
  return 0; 
}







