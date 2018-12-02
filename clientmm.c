/* A simple client program to communicate with the corresponding
   server executable.
   The IP and TCP port are passed as arguments.
   
   Rensselaer Polytechnic Institute (RPI)
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/time.h> 

void error(char *msg)
{
  perror(msg);
  exit(0);
}

int main(int argc, char *argv[])
{
  int sockfd, portno, n, M,i,buflen;

  struct sockaddr_in serv_addr;
  struct hostent *server;

  char buffer[256];
  char messagebuffer[256];
  if (argc < 3) {
    fprintf(stderr,"usage %s hostname port\n", argv[0]);
    exit(0);
  }
  portno = atoi(argv[2]);
  M = atoi(argv[3]);
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) 
    error("ERROR opening socket");
  server = gethostbyname(argv[1]);
  if (server == NULL) {
    fprintf(stderr,"ERROR, no such host\n");
    exit(0);
  }
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, 
        (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
  serv_addr.sin_port = htons(portno);
  if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
    error("ERROR connecting");
  printf("Please enter the message: ");
  bzero(buffer,256);
  bzero(messagebuffer,256);
  fgets(messagebuffer,255,stdin);
  // get my own address

  struct ifaddrs *ifap, *ifa;
  struct sockaddr_in *sa;
  char *addr;
  getifaddrs (&ifap);
  for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
       if (ifa->ifa_addr->sa_family==AF_INET) {
           sa = (struct sockaddr_in *) ifa->ifa_addr;
           addr = inet_ntoa(sa->sin_addr);
       }
  }
  freeifaddrs(ifap);
  //first send data-length then data
  int datalen = 14;//sizeof(addr) doesnt work well;
  int tmp = htonl(datalen);
  n = write(sockfd,(char *)&tmp,sizeof(tmp));
  if (n < 0) 
    error("ERROR writing to socket"); 
  n = write(sockfd,addr,datalen);
  if (n < 0) 
    error("ERROR writing to socket");
  // write number of messages
  tmp = htonl(M);
  n = write(sockfd,&tmp,sizeof(int));
  if (n < 0) 
    error("ERROR writing to socket");
  n = read(sockfd,(char*)&buflen,sizeof(buflen));
  buflen = ntohl(buflen);
  char address[buflen];
  bzero(address,buflen);
  //write message
  int msgrecvd = 0;
  struct timeval t1, t2;
  gettimeofday(&t1, NULL);
  for(i = 0;i< M;i++){
  	n = write(sockfd,messagebuffer,strlen(messagebuffer));
  	if (n < 0) 
    	  error("ERROR writing to socket");
  	bzero(buffer,256);
  	n = read(sockfd,buffer,255);
  	if (n < 0) 
    	  error("ERROR reading from socket");
  	printf("You have a message: %s\n",buffer);

	
	//n = read(sockfd,(char*)&buflen,sizeof(buflen));
	//buflen = ntohl(buflen);
	//char addr[buflen];
	bzero(address,buflen);
	n = read(sockfd,address,datalen);
	printf("Send by: %s\n\n",address);


	msgrecvd ++;
  }
  gettimeofday(&t2, NULL);
  double mseconds = (t2.tv_sec - t1.tv_sec)* 1000.0;
  mseconds += (t2.tv_usec - t1.tv_usec) / 1000.0;
  printf("messages received = %d \n", msgrecvd);
  printf("messages lost = %d \n", M - msgrecvd);
  printf("time to receive messages = %f \n", mseconds/1000);
  printf("time / number of messages = %f / %d = %f \n", mseconds/1000, M, mseconds/(1000*M));
  return 0;
}
