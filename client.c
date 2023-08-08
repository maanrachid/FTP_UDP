#include <sys/types.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <ctype.h>
#include <arpa/inet.h>
#define SIZE 5000                     /* Size of the message to be sent */
#define WINSIZE 500                   /* Size of the sliding window */
#define WAIT 90                /* waiting time for a packet */



typedef struct {
  int num;
  short occupy;
  short done;
  char message[SIZE+5];
}packet;

typedef struct{
  char nameoffile[30];
  char address[60];
  short port;
  int num;
}request;

void checkargument(FILE* stream,char *arg[],int numarg);
void checkanddecide(packet* p1);
int index1,requiredpacket;
packet window[WINSIZE];


int main(int argc,char *argv[]){
  int sock,length,temp,nb,n;
  struct sockaddr_in clientname,servername;
  struct hostent *hp,*gethostbyname();
  char hostname[100];
  char buf[SIZE*2];
  request r1;
  packet *p1;
  fd_set read_set;
  struct timeval wait;
 

  checkargument(stderr,argv,argc);

  sock=socket(AF_INET,SOCK_DGRAM,0);
  if (sock<0){
    perror("openning datagram socket");
    exit(1);
  }

  gethostname(hostname,sizeof(hostname));
  
  hp =gethostbyname(hostname);
  if (hp==0){
    fprintf(stderr,"%s: unknown host",hostname);
    exit(1);
  }

  bcopy(hp->h_addr,&clientname.sin_addr,hp->h_length);
  clientname.sin_family = AF_INET;
  
  if (bind(sock,(struct sockaddr *) &clientname,sizeof(clientname))) {
    perror("binding datagram socket");
    exit(1);
  }

  length =sizeof(clientname);
  if (getsockname(sock,(struct sockaddr *)&clientname,(unsigned int *)&length)){
    perror("getting socket name");
    exit(1);
  }



  
  hp =gethostbyname(argv[1]);
  if (hp==0){
    fprintf(stderr,"%s: unknown host",argv[1]);
    exit(1);
  }

  bcopy(hp->h_addr,&servername.sin_addr,hp->h_length);
  servername.sin_family = AF_INET;
  servername.sin_port = htons(atoi(argv[2]));

  length =sizeof(servername);
 

  index1=0;

  bzero(r1.nameoffile,sizeof(r1.nameoffile));
  sprintf(r1.nameoffile,"%s",argv[3]);
  sprintf(r1.address,"%s",hostname);
  r1.port=ntohs(clientname.sin_port);
  
  
  for(n=0;n<WINSIZE;n++){

    window[n].occupy=0;
    window[n].num=n;
  }
  
  requiredpacket=0;
  index1=0;
  do {
    temp = index1;
    for (n=0;n<WINSIZE;n++){
      if (window[(temp+n)%WINSIZE].occupy==0){
	r1.num =window[(temp+n)%WINSIZE].num;
	
	if (sendto(sock,(char *)&r1,sizeof(request),0,(struct sockaddr *)&servername,sizeof(servername))<0){
	  
	  wait.tv_sec=1;
	  wait.tv_usec=0;
	  FD_ZERO(&read_set);
	  FD_SET(sock,&read_set);
	  nb =select(FD_SETSIZE,&read_set,(fd_set *)0,(fd_set *)0,&wait);
	  if (FD_ISSET(sock,&read_set)){
	    bzero(buf,sizeof(buf));
	    if (read(sock,buf,sizeof(buf))>0){
	      p1=(packet *) buf;
	      checkanddecide(p1);
	    }
	  }
	}else {
	 
	  wait.tv_sec=0;
	  wait.tv_usec= WAIT;
	  FD_ZERO(&read_set);
	  FD_SET(sock,&read_set);
	  nb =select(FD_SETSIZE,&read_set,(fd_set *)0,(fd_set *)0,&wait);
	  if (FD_ISSET(sock,&read_set)){
	    bzero(buf,sizeof(buf));
	    if (read(sock,buf,sizeof(buf))>0){
	      p1=(packet *) buf;
	      checkanddecide(p1);
	    }
	  }
	}
      }
    }
    
    
  }while(1);
  close(sock);

  return 0;
}  
  



void checkargument(FILE* stream,char *arg[],int numarg){
  int n;

  if (numarg != 4){
    fprintf(stream,"wrong number of argument\n");
    exit(1);
  }

  for(n=0;n<strlen(arg[2]);n++){
    if (!isdigit(arg[2][n])){
      fprintf(stream,"port should be a number\n");
      exit(1);
    }
  }


}

void checkanddecide(packet* p1){
  
  int temp,n;
  if (p1->num==requiredpacket){
    printf("%s",p1->message);
    if  (p1->done==1)
      exit(0);
    window[index1].occupy=0;
    window[index1].num=requiredpacket+WINSIZE;

    requiredpacket++;
    index1=(index1+1)%WINSIZE;
    
    while(window[index1].occupy){
      printf("%s",window[index1].message);
      if  (window[index1].done==1){
	exit(0);
      }
      window[index1].occupy=0;
      window[index1].num=requiredpacket+WINSIZE;

      requiredpacket++;
      index1=(index1+1)%WINSIZE;


    }



  } else if (p1->num > requiredpacket && 
	     (p1->num < (requiredpacket +WINSIZE)) && strlen(p1->message) > 0 ){

   
    temp = p1->num-requiredpacket;

    strcpy(window[(index1+temp)%WINSIZE].message,p1->message);
    for(n=strlen(p1->message);n<SIZE;n++)
      window[(index1+temp)%WINSIZE].message[n]='\0';

    window[(index1+temp)%WINSIZE].num=p1->num;
    window[(index1+temp)%WINSIZE].occupy=1;



  }
}
