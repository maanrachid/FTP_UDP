#include <sys/types.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>
#define SIZE 5000

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

int main(int argc,char *argv[]){
  int sock,length,f1,n,z;
  struct sockaddr_in name;
  char buf[500],buf2[SIZE];
  request *r1=NULL;
  packet p1;
  int l;
  struct hostent *gethostbyname();



 
  if (argc!=2){
    fprintf(stderr,"wrong namber of argument\n");
    exit(0);
  }

  for(n=0;n<strlen(argv[1]);n++){
    if (!isdigit(argv[1][n])){
      fprintf(stderr,"port should be a number \n");
      exit(0);
    }
  }

  sock=socket(AF_INET,SOCK_DGRAM,0);
  if (sock<0){
    perror("openning datagram socket");
    exit(1);
  }
  
  name.sin_family = AF_INET;
  name.sin_addr.s_addr = INADDR_ANY;
  name.sin_port = htons(atoi(argv[1]));
  if (bind(sock, (struct sockaddr *) &name,sizeof(name))) {
    perror("binding datagram socket");
    exit(1);
  }

  length =sizeof(name);
  if (getsockname(sock,(struct sockaddr *) &name,(unsigned int *) &length)){
    perror("getting socket name");
    exit(1);
  }

  printf("Socket has port #%d\n",ntohs(name.sin_port));
  do {
    bzero(buf,sizeof(buf));
    l= sizeof(name);
    if ((z=recvfrom(sock,buf,sizeof(buf),0,(struct sockaddr *)&name
		,(unsigned int *) &l))<0)/* do nothing */;
    else if (z>0){ 
      r1=(request *)buf; 
      if (strstr(r1->nameoffile,"/")!=NULL)/* do nothing */;
      else {
	if ((f1= open(r1->nameoffile , O_RDONLY , 0777))<0)
	  /*do nothing*/;
	else{ /*open successfully */
	  bzero(buf2,sizeof(buf2));
	  if (lseek(f1,r1->num * SIZE,SEEK_SET)<0)
	    /*do nothing*/;
	  else if ((z=read(f1,buf2,sizeof(buf2)))<0)
	    /*do nothing*/;
	  else {/*
	    hp =gethostbyname(r1->address);
	    if (hp==0){
	    do nothing; 
	    }else{
	    
	    bcopy(hp->h_addr r1->address,&name.sin_addr,hp->h_length);
	      name.sin_family = AF_INET;
	      name.sin_port = htons(r1->port);
	    */  
	      p1.num = r1->num;
	      if (z<SIZE){
		p1.done=1;
	      
	     } else
		p1.done=0;
	      bzero(p1.message,sizeof(p1.message));
	      sprintf(p1.message,"%s",buf2);
	      if (sendto(sock,(char *)&p1,sizeof(p1),0,(struct sockaddr *) &name,sizeof(name))<0)
		/*do nothing*/;
	    
		
      
	     
	  }
	  close(f1);
	}
      }
    }
  }while(1);
  close(sock);
  return 0;
  
}
