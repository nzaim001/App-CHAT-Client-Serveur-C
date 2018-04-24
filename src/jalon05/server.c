#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <limits.h>
#include "colors.h"
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "server.h"


int main(int argc, char** argv)
{
  struct sockaddr_in saddr_in ;
  int sock,kk, id ,size, sock_new;
  int sock_income[MAX_Connexion];
  int nfds, how_many_const =0,size1,i,indice =0,aa, how_many = 0, active_files;
  char client_msg[3000];
  int size_addr = sizeof(struct sockaddr_in);
  char *failed_message = "Sorry, try later!!";
  struct Info_client *all_client[MAX_Connexion+1];
  struct channel *ch[MAX_Connexion+1];
  fd_set workingfds,readfds;
  char *tab_time = malloc(100);
  char *clients = malloc(300);
  char *chaine1 = malloc(100);

  if (argc != 2)
  {
    fprintf(stderr, "usage: RE216_SERVER port\n");
    return 1;
  }
  //create the socket, check for validity!
  sock = do_socket(AF_INET , SOCK_STREAM , 0);
  //init the serv_add structure
  init_serv_addr(argv[1],&saddr_in);
  //perform the binding
  //we bind on the tcp port specified
  //do_bind()
  do_bind(sock,saddr_in);
  //specify the socket to be a server socket and listen for at most 20 concurrent client
  //listen()
  do_listen(sock, MAX_Connexion);
  FD_ZERO(&readfds);
  FD_SET(sock, &readfds);
  nfds = sock +1;
  for(aa = 0; aa< MAX_Connexion ;aa++) sock_income[aa] = 0;//initialize the array of socket
  for (;;)
  {
    memcpy(&workingfds,&readfds,sizeof(readfds)); // copy the readfds into workingfds and make all modification in workingfds
    active_files = select(nfds,&workingfds,NULL,NULL, NULL);
    if(active_files < 0) perror("Select failed");
    for( i = 0; i < nfds && active_files > 0; i++)  // parcourir tout les descripteurs fichiers prêts
    {
      if(FD_ISSET(i,&workingfds)) // tester son existence
      {
        active_files-=1;
        if(i==sock) // ici, faite pour le traitement des nouvelles connexions ( sock  = socket of server)
        {
          //accept connection from client
          sock_new = do_accept(sock,(struct sockaddr *)&saddr_in , (socklen_t*)&size_addr);
          how_many += 1; // increment the number of clients connected
          how_many_const = how_many; // this variable it constant all time
          if(how_many >= MAX_Connexion) // this loop, to test if the maximun of clients is full or not,
          {
            send(sock_new,failed_message,strlen(failed_message),0);
            how_many -= 1;
            close(sock_new);// close the sock of new client how hasnt a place
            break;
          }
          if(sock_new >= nfds) nfds = sock_new +1;
          kk=0; // it s all time initialized by 0, to check if a client has disconencted i can detect this place is empty
          while(sock_income[kk] != 0) kk++; // kk mention the place taken by client in array of client
          sock_income[kk] = sock_new;
          printf("You have a new connection from client[Number : %d, socket : %d]\n",kk,sock_new);
          id = kk;
          FD_SET(sock_new,&readfds);// add the sock new to readfds
          memset(tab_time,'\0',100);
          get_connexion_time(tab_time); // get time of first conenxion of client
          fill_data(all_client,ch,id,sock_income[id],inet_ntoa(saddr_in.sin_addr),saddr_in.sin_port,tab_time);
          //fflush(stdout);
        }
        else
        { // in this case, we have an old client who is connected and would talk
          size=recv(i,client_msg,3000,0);  // receive the message taped by clientn and save it on buffer client_msg
          int jj,id1;
          for(jj=0;jj<MAX_Connexion;jj++)
          {  // this loop, will finf the id (identifier) of client and his position in array of client
            // and this id1 will simplify to check all deatils of client in position id1
            if(all_client[jj]->sock_client == i)
            {
              id1 = jj;
              break;
            }
          }
          // error of receiving data from client of socket == i
          if(size<0)
          {
            perror("Reception failed ! \n");
          }
          if(size > 0)
          { // here, we received successfully data from client of position id1
            if(strncmp(client_msg,"/nick ",6)==0 && all_client[id1]->empty == 0 && name_exist(id1,client_msg,all_client,how_many))
            {// client give a nickname, and test if he hadn't already, and check if it isn"t a nickname of others client
              if(nick(id1,client_msg,all_client) == 1) // save the nickname of client
              {
                memset(client_msg,0, strlen(client_msg));
              }
            }
            else
            {
              if (all_client[id1]->empty == 0) {
                // client still hasn't a nickname, don't give him permission to talk until he has a nickname
                send(all_client[id1]->sock_client,RED "[Server] :" RESET " please logon with /nick <pseudo>\n",56,0);
                memset(client_msg,'\0', strlen(client_msg));
              }
              else// the client has a nickname, alright I'll treat his message
              {
                if (strcmp(client_msg,"/quit\n")==0)
               { // cient would disconnect
                 // I inform the server that client has disconencted
                 printf("client" BLUE" %s" RESET " has disconnected\n",all_client[id1]->nickname);
                 how_many -= 1; // I decrease the number of connected client
                 sock_income[id1] = 0; // I initialize it socet, to free it for other new client
                 //init client to zero
                 FD_CLR(all_client[id1]->sock_client, &workingfds); // clear the client from my fd_set of workingfds
                 clear_client(id1,all_client,ch); // clear all his details
                 memset(client_msg,'\0', strlen(client_msg));
              }
                else if (strncmp(client_msg,"/nick ",6)==0) { // here client had already a nickname, there is an other command to change the nickname
                  send(all_client[id1]->sock_client,RED"[Server] : "RESET"You have already your nickname Enter"YELLOW " /ModifyNick <pseudo>\n"RESET,96,0);
                  memset(client_msg,'\0', strlen(client_msg));
                }
                else if (strncmp(client_msg,"/create channel_",16)==0) {// client would create a channel
                  channel(id1, client_msg,all_client,ch,how_many_const);
                  memset(client_msg,'\0', strlen(client_msg));
                }
                else if (strncmp(client_msg,"\n",8)==0) {//client enter a empty message, I don't accept
                  send(all_client[id1]->sock_client,RED"[Server] : "RESET"Enter a valid message\n",45,0);
                  memset(client_msg,'\0', strlen(client_msg));
                }
                else if (strncmp(client_msg,"/send ",6)==0) { // client would send a file to a client
                  share_file(id1,client_msg,all_client,how_many_const);
                  memset(client_msg,'\0', strlen(client_msg));
                }
                else if (strncmp(client_msg,"/join ",strlen("/join "))==0) {//client would join a channel
                  join(id1,client_msg,all_client,ch,how_many_const);
                  memset(client_msg,'\0', strlen(client_msg));
                }
                else if (strncmp(client_msg,"/msgall ",8)==0){ // client would send a message to all connected client
                  broadcast(id1, client_msg, all_client,how_many_const);
                  //memset(client_msg,0, strlen(client_msg));
                  break;
                }
                else if (strncmp(client_msg,"/msg ",5)==0){ // client would send a private message to a specific client
                  unicast(id1, client_msg, all_client,how_many_const);
                  memset(client_msg,0, strlen(client_msg));
                  break;
                }
                else if (strcmp(client_msg,"/who\n")==0){ // client would know the connected client
                  who(id1,all_client,ch,how_many_const);
                  memset(client_msg,0, strlen(client_msg));
                  break;
                }
                else if (strncmp(client_msg,"/whois ",7)==0){ // client would rechieve details of a client
                   if(whois(all_client[id1]->sock_client,how_many_const,client_msg,all_client)){
                     memset(client_msg,0, strlen(client_msg));
                     break;
                   }
                   else{
                     memset(client_msg,0, strlen(client_msg));
                   }
                 } // client would change his nickname
                else if (strncmp(client_msg,"/ModifyNick ",12)==0 && all_client[id1]->empty == 1 && name_exist(id1,client_msg,all_client,how_many)){
                  if(modify_nick(id1,client_msg,all_client) == 1){
                    memset(client_msg,0, strlen(client_msg));
                    break;
                  }
                  else{
                    memset(client_msg,0, strlen(client_msg));
                  }
                }
                else { // here client would talk, ther is 2 case; client is a member in a channel so he exhange message with members of channel
                  if(all_client[id1]->member != 0 ){
                    talk_channel(id1, client_msg,all_client, ch, how_many_const);
                  }
                  else{ // the client isn' t a member in any channel, and would talk to server
                    if(strcmp(client_msg,"/nick\n")!=0 && strcmp(client_msg,"/whois\n")!=0 && strcmp(client_msg,"/ModifyNick\n")!=0){
                      char *reply = malloc(30);
                      memset(reply,0, strlen(reply));
                      strcat(reply,RED"[Server] : "RESET);
                      send(all_client[id1]->sock_client,reply,strlen(reply),0);
                      write(all_client[id1]->sock_client, client_msg , strlen(client_msg));
                      printf(YELLOW"%s "RESET"sent the following message : %s\n",all_client[id1]->nickname, client_msg);
                      memset(client_msg,'\0', strlen(client_msg));
                      memset(reply,0, strlen(reply));
                    }
                    else { //client send the specific command of application, I inform him by the function of these commands
                      if(strcmp(client_msg,"/ModifyNick\n")==0) send(all_client[id1]->sock_client,"Commande is reserved to change your nickname!\n",47,0);
                      if(strcmp(client_msg,"/nick\n")==0) send(all_client[id1]->sock_client,"Commande is reserved to create a nickname!\n",44,0);
                      if(strcmp(client_msg,"/whois\n")==0)  send(all_client[id1]->sock_client,"Commande is reserved to rechieve information about a client! you must add client's nickname\n",93,0);
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  //clean up server socket
  close(sock);
  return 0;
}
