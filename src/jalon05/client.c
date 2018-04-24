#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "client.h"
#include <signal.h>
#include <sys/wait.h>

int main(int argc,char** argv)
{

  int s;
  struct sockaddr_in sock_host;
  char *addrserv=argv[1];
  char *port=argv[2];
  struct addrinfo* res;
  char server_reply[2000], msg[3000], response[3000];

  if (argc != 3)
    {
      fprintf(stderr,"usage: RE216_CLIENT hostname port\n");
      return 1;
    }

//get address info from the server
  get_addr_info(addrserv,port,&res);


  //get the socket
  s = do_socket(AF_INET,SOCK_STREAM,0);

  //connect to remote socket
  do_connect(s,res->ai_addr,sizeof(struct sockaddr_in));

  int ret;

  ret= fork();
  if(ret == -1){
    perror("failed process");
    exit(1);
  }

  else if (ret == 0) {

    do {
      signal(SIGINT, SIG_IGN);
      memset(response,0,sizeof(response));
      handle_client_message(s,response,3000,0);
    }while(1);
  } else {
    do{
      signal(SIGINT, SIG_IGN);
      if (response != 0)
        sleep(1.3);
      fgets(msg, 3000, stdin);
      if(send(s,msg,strlen(msg),0)<0)
        {
  	printf("Sending failed\n");
    wait(NULL);
  	return 1;
        }
    }while(strcmp(msg,"/quit\n") != 0);
    puts("You are disconnected");
    //kill(ret,SIGKILL);
  }
  close(s);
  return 0;
}
