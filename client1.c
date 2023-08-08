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
#define SIZE 10
#define WINSIZE 5




typedef struct {
  int num;
  short occupy;
  short done;
  char message[SIZE];
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
  char buf[2000];
  request r1;
  packet *p1;
  fd_set read_set;
  struct timeval wait;


  //  checkargument(stderr,argv,argc);

  sock=socket(AF_INET,SOCK_DGRAM,0);
  if (sock<0){
    perror("openning datagram socket");
    exit(1);
  }

  gethostname(hostname,sizeof(hostname));
  
  hp =gethostbyname(hostname);
  if (hp==0){
    fprintf(stderr,"%s: unknown host",hostname);
    exit(2);
  }

  bcopy(hp->h_addr,&clientname.sin_addr,hp->h_length);
  clientname.sin_family = AF_INET;
  clientname.sin_port = htons(atoi("2000"));
  if (bind(sock,&clientname,sizeof(clientname))) {
    perror("binding datagram socket");
    exit(1);
  }

  length =sizeof(clientname);
  if (getsockname(sock,&clientname,&length)){
    perror("getting socket name");
    exit(1);
  }

  //  printf("Socket has port #%d\n",ntohs(clientname.sin_port));

  
  hp =gethostbyname(argv[1]);
  if (hp==0){
    fprintf(stderr,"%s: unknown host",argv[1]);
    exit(2);
  }

  bcopy(hp->h_addr,&servername.sin_addr,hp->h_length);
  servername.sin_family = AF_INET;
  servername.sin_port = htons(atoi(argv[2]));

  length =sizeof(servername);
 
  //printf("Socket has port #%d\n",ntohs(servername.sin_port));
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
	
	if (sendto(sock,(char *)&r1,sizeof(request),0,&servername,sizeof(servername))<0){
	  perror("receiving datagram packet");
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
	}
      }
      wait.tv_sec=0;
      wait.tv_usec=200000;
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
    
  }while(1);
  close(sock);
  return 0;
}  
  



void checkargument(FILE* stream,char *arg[],int numarg){
  int n;
  if (numarg!=4){
    fprintf(stream,"wrong number of argument\n");
    exit (2);
  }

  for(n=0;n<strlen(arg[2]);n++){
    if (!isdigit(arg[2][n])){
      fprintf(stream,"port should be a number\n");
      exit (2);
    }
  }


}

void checkanddecide(packet* p1){
  
  int temp;
  if (p1->num==requiredpacket){
    printf("%s",p1->message);
    if  (p1->done==1)
      exit(4);
    window[index1].occupy=0;
    window[index1].num=requiredpacket+WINSIZE;
    bzero( window[index1].message,SIZE);
    requiredpacket++;
    index1=(index1+1)%WINSIZE;
    while(window[index1].occupy){
      printf("%s",window[index1].message);
      if  (window[index1].done==1)
	exit(4);
      window[index1].occupy=0;
      window[index1].num=requiredpacket+WINSIZE;
      bzero(window[index1].message,SIZE);
      requiredpacket++;
      index1=(index1+1)%WINSIZE;

    }


  } else if (p1->num > requiredpacket && 
	     (p1->num < (requiredpacket +WINSIZE))){

    temp = p1->num-requiredpacket;
    bzero(window[(index1+temp)%WINSIZE].message,
	  SIZE);
    strcpy(window[(index1+temp)%WINSIZE].message,p1->message);
    //    sprintf(window[(index1+temp)%WINSIZE].message,"%s",p1->message);
    window[(index1+temp)%WINSIZE].num=p1->num;
    window[(index1+temp)%WINSIZE].occupy=1;

  }
}
