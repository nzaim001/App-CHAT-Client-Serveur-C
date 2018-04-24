#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "colors.h"



int do_socket(int domain, int type, int protocole){
  int sockfd;
  int yes =1;

  sockfd = socket(domain,type,protocole);
  if(sockfd == -1){
    printf("Socket error");
    exit(EXIT_FAILURE);
    //autoriser la relance immmédiate
  }
  printf("Socket created\n");
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        perror("ERROR setting socket options");

  return sockfd;
}

void get_addr_info(const char* address, const char* port, struct addrinfo** res){

  int status;
  struct addrinfo hints, p;
  memset(&hints,0,sizeof(hints));
  hints.ai_family=AF_INET;
  hints.ai_socktype=SOCK_STREAM;
  status = getaddrinfo(address,port,&hints,res);

}

void do_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    int res = connect(sockfd, addr, addrlen);
    if (res != 0) {
      perror("Connection failed !");
      exit( EXIT_FAILURE );
    }
    char tab1[44] = RED "[Server]" RESET " : You are connected\n";
    char alert_msg[71] = RED "[Server] :" GREEN "please logon with /nick < your pseudo >\n" RESET;
    puts(tab1);
    puts(alert_msg);
}



ssize_t handle_client_message(int s ,char *response,size_t len,  int flags){
  if( recv(s , response , len , flags) < 0)
    {
      printf("Reception failed\n");
    }

  puts(response);
}
