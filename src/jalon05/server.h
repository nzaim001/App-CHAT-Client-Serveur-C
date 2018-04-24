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


#define MAX_Connexion 20
struct channel{

  char *name;
  int sock_client;// du proproetaire
  int nb;
  int empty;
  int user[MAX_Connexion];
};

struct Info_client{
  int member;
  int sock_client;
  int empty;
  int port;
  char *ip_addr;
  char *nickname;
  char *time_connexion;
};


void quit_channel(int id, char *msg,struct Info_client **all_client,struct channel **ch,int how_many);
void talk_channel(int id, char *msg, struct Info_client **all_client, struct channel **ch,int how_many);
void share_file(int id, char * msg,struct Info_client **all_client,int how_many);
void fill_channel(int id_ch,struct channel list_channel[],int fd,int k);
void join(int id, char *msg,struct Info_client **all_client,struct channel **ch,int how_many);
void channel(int id, char *msg,struct Info_client **all_client,struct channel **ch,int how_many);
int same(char *chaine1,char *chaine2);
void unicast(int id, char *msg, struct Info_client **all_client,int how_many_const);
void broadcast(int id, char *msg, struct Info_client **all_client,int how_many_const);
int is_empty(char *message);
int whois(int id,int how_many, char* msg, struct Info_client **all_client);
int name_exist(int id,char *msg, struct Info_client **all_client,int how_many);
void who(int id,struct Info_client **all_client,struct channel **ch,int how_many);
int nick(int id,char *msg,struct Info_client **all_client);
int modify_nick(int id,char *msg,struct Info_client **all_client);
int test_nickname(int id, struct Info_client **all_client);
void clear_client(int id1,struct Info_client **all_client,struct channel **ch);
void fill_data(struct Info_client **all_client,struct channel **ch,int id,int sock,char *ip_addr,int port, char *time_connexion);
void get_connexion_time(char *chaine);

///////////////////////////////////////////////////////////////////////////
void channel(int id, char *msg,struct Info_client **all_client,struct channel **ch,int how_many){
  int i,k,l;
  int exist_ch=0;
  char *alert = malloc(500);
  char *Chname = malloc(50);
  memset(Chname,'\0',50);
  memset(alert,'\0',500);
  if(ch[id]->empty == 1){// the ch cooresponding to client id is already created
    send(all_client[id]->sock_client,RED"[Server] : "RESET"You have already a channel!",49,0);
    return;
  }
  if(all_client[id]->member == 0){ // client would create a channel and he isn t a member in other channel
    for( i = 0; i < strlen(msg)-1; i++){//rechieve the name of channel
      if(msg[16+i] != ' '){
        Chname[i] = msg[16+i];
      }
      else{
        break;
      }
    }
    for(i=0;i<how_many;i++){ // test if channel already exist
      if(strcmp(ch[i]->name,Chname)==0){
        exist_ch = 1;
        break;
      }
    }
    if(exist_ch ==0){ //the channel doesn't exist youpi
      if(ch[id]->empty == 0){ //test if ch corresponding to client id , is vide
        strcpy(ch[id]->name,Chname); // copy the name of channel
        ch[id]->empty=1; // male it up to now, full
        ch[id]->nb ++; //increment number of client exist in channel id
        ch[id]->sock_client =  all_client[id]->sock_client;// identify channel by sock client
        all_client[id]->member = 1;
        for(k=0;k<MAX_Connexion;k++){
          if(ch[id]->user[k] == 0){ // full the array by member
              ch[id]->user[k]= all_client[id]->sock_client; // ajouter le propritaire as the first member in channel
              break;
          }
        }
        if(all_client[id]->member == 1){
          strcpy(alert,RED "[Server] : "RESET"You have created channel ");
          strcat(alert,Chname);
          send(all_client[id]->sock_client,alert,strlen(alert),0);// inform the cliet by his new and only channell
        }
      }
    }
    else{// channel name already exist !! try again hahaha
      send(all_client[id]->sock_client,RED"[Server] : "RESET" This channel already exist, try other name!",66,0);
    }

  }
  else if(all_client[id]->member != 0 && ch[id]->empty == 0 ){ //member != 0
    // client would create a channel and he is a  member in other channel ,
    strcpy(alert,RED "[Server] : "RESET"please disconnect from the channel : ");
    for(k = 0; k<how_many; k++){
      for (l = 0; l < MAX_Connexion; l++) {
        if(ch[k]->user[l] == all_client[id]->sock_client){
          strcat(alert,ch[k]->name);
          send(all_client[id]->sock_client,alert,strlen(alert),0);
          break;
        }
      }
      break;
    }
    return;
  }
}

////////////////////////////////////////////////////////////////////////////

void unicast(int id, char *msg, struct Info_client **all_client,int how_many_const){
  int i = 0;
  int stat = 0;
  char *sending = malloc(3000);
  char *user = malloc(1000);
  char *tab1 = malloc(50);
  memset(user,'\0',1000);
  memset(tab1,'\0',50);
  memset(sending,'\0',3000);
// rechieve name of dest
  for( i = 0; i < strlen(msg)-1; i++){
    if(msg[5+i] != ' '){
      tab1[i] = msg[5+i];
    }
    else{
      break;
    }
  }
  for(i = 0;i<how_many_const;i++)
  {
    if(strncmp(all_client[i]->nickname,tab1,strlen(tab1)-1) == 0){
      stat = 1;
      strcpy(sending, msg+5+strlen(tab1));
      strcpy(user,all_client[id]->nickname);
      send(all_client[i]->sock_client,user,strlen(user),0);
      send(all_client[i]->sock_client, sending, strlen(sending),0);
      break; // the destination is founded
    }
  }
  if(stat == 0){
    send(all_client[id]->sock_client,RED "[Server] : " RESET "the user doesn't exist,Sorry\n",55,0);
  }
}

////////////////////////////////////////////////////////////////////////////

int same(char *chaine1,char *chaine2){ // return 1 if they are similar
int i;
if(strlen(chaine1) == strlen(chaine2))
{
  for(i=0;i<strlen(chaine1);i++){
    if(chaine1[i] != chaine2[i]) return 0;
  }
  return 1;
}
return 0;
}

////////////////////////////////////////////////////////////////////////////

void talk_channel(int id, char *msg, struct Info_client **all_client, struct channel **ch,int how_many){
  int i = 0, k, l;
  int ret, id_ch;
  char *channel_name = malloc(50);
  char *user_talking = malloc(3000);
  memset(channel_name,'\0',50);
  memset(user_talking,'\0',3000);
  strcpy(user_talking,all_client[id]->nickname);
  strcat(user_talking," -- channel : ");
  if(strncmp(msg,"/quit channel",strlen("/quit channel")) == 0){
    quit_channel(id,msg,all_client,ch,how_many);
    memset(msg,'\0',strlen(msg));
  }
  else{
    for(k = 0;k<how_many;k++){
      if(ch[k]->sock_client != 0){
        for(l = 0;l<MAX_Connexion; l++){
          if(ch[k]->user[l] == all_client[id]->sock_client){ // know channel includes client talking
            id_ch = k;
            strcat(user_talking,ch[id_ch]->name);
            strcat(user_talking," --> ");
            strcat(user_talking,msg);
            for(i=0;i<MAX_Connexion;i++){
              if(ch[id_ch]->user[i] != 0 && ch[id_ch]->user[i] != all_client[id]->sock_client){
                send(ch[id_ch]->user[i], user_talking, strlen(user_talking),0);
                memset(user_talking,'\0',3000);
                send(ch[id_ch]->user[i],"\n",2,0);
                memset(user_talking,'\0',3000);
                memset(msg,'\0',strlen(msg));
              }
            }
            break;
          }
        }
      }
    }

  }
}

////////////////////////////////////////////////////////////////////////////

void broadcast(int id, char *msg, struct Info_client **all_client,int how_many_const){
  int i = 0;
  char *sending = malloc(3000);
  char *user = malloc(1000);
  char *chaine = malloc(3000);
  memset(chaine,'\0',3000);
  memset(user,'\0',1000);
  memset(sending,'\0',3000);
  strcpy(sending, msg+8);
  strcpy(user,all_client[id]->nickname);
  for(i=0;i<how_many_const;i++){
    if(all_client[i]->sock_client != all_client[id]->sock_client && all_client[i]->sock_client != 0){
      send(all_client[i]->sock_client,user,strlen(user),0);
      send(all_client[i]->sock_client, sending, strlen(sending),0);
      send(all_client[i]->sock_client,"\n",2,0);
    }
    else continue;
  }
}

////////////////////////////////////////////////////////////////////////////

int is_empty(char *message){ // Retourne 0 si le message est vide, 1 sinon
	int i, ret = 0;
	for(i = 0; i < strlen(message) ; i++){
		if(!(message[i] == '\0' || message[i] == '\n' || message[i] == ' ')){
		  ret = 1;
		}
	}
	return ret;
}

////////////////////////////////////////////////////////////////////////////

int name_exist(int id,char *msg, struct Info_client **all_client,int how_many){// return1: doesn't exist before
int i,debut;
char *wait_name = malloc(200);
memset(wait_name,'\0',200);
if(strncmp(msg,"/nick ",6) == 0){
  debut=6;
  for(i=6;i<strlen(msg);i++){
    if (msg[i]==' '){
      debut=debut+1;
    }
  }
}
else{// would change nickname
  debut=12;
  for(i=12;i<strlen(msg);i++){
    if (msg[i]==' '){
      debut=debut+1;
    }
  }

}
if(is_empty(msg+debut)==0){
  send(all_client[id]->sock_client,RED"[Server] : "RESET"ERROR nickname, It is empty!",50,0);
  return 0;
}
strncpy(wait_name,msg+debut,50);
for(i=0;i<how_many;i++){
  if(strcmp(all_client[i]->nickname,wait_name) == 0){
    send(all_client[id]->sock_client,"nickname already exist! try a new nickname!",43,0);
    return 0;
  }
}

return 1;
}

////////////////////////////////////////////////////////////////////////////

int whois(int id,int how_many, char* msg, struct Info_client **all_client){
  int i;
  int state = 0;
  char *declared_client = malloc(500);
  char *answer = malloc(1000);
  char *port_chaine = malloc(500);
  memset(declared_client,'\0',500);
  memset(answer,'\0',1000);
  memset(port_chaine,'\0',500);
  int k,debut=7;
  for(k = 7;k<strlen(msg);k++){
    if (msg[k]==' '){
      debut=debut+1;
    }
  }
  strcpy(declared_client, msg+debut);
  strcpy(answer, RED "[Server] : " RESET); // answer = " [server] : "
  for (i= 0; i < how_many; i++) {
    if(strcmp(all_client[i]->nickname, declared_client) == 0){
      strcat(answer,declared_client);
      strcat(answer," connected since ");
      strcat(answer,all_client[i]->time_connexion);
      strcat(answer," with IP address ");
      strcat(answer,all_client[i]->ip_addr);
      sprintf(port_chaine, "%d", all_client[i]->port);
      strcat(answer, " and port number ");
      strcat(answer,port_chaine);
      //send(id,answer,strlen(answer),0);
      state = 1;
    }
  }
  if(state == 1){
    send(id,answer,strlen(answer),0);
  }
  else{
    send(id,RED"[Server] : "RESET"Sorry, this client doesn't exist",54,0);
  }
  return state;
}

////////////////////////////////////////////////////////////////////////////

void who(int id,struct Info_client **all_client,struct channel **ch,int how_many_const){
  int i=0;
  int k,l,m,n;
  char users[300];
  char *Chname = malloc(50);
  memset(users,'\0',300);
  memset(Chname,'\0',50);
  strcpy(users,RED "[Server] :" RESET " Online users are :\n");
  send(all_client[id]->sock_client,users,strlen(users),0);
  if(all_client[id]->member == 0){ //client who is talking is not a member in any channel
    for(i=0;i<how_many_const;i++){
      if(all_client[i]->empty != 0 ){
        if(all_client[i]->member == 0){
          // client i is also not a member in other channel and he has a nickname
          memset(users,'\0',300);
          strcat(users,"\n-----------");
          strcat(users,all_client[i]->nickname);
          send(all_client[id]->sock_client,users,strlen(users),0);
        }
      }
    }
  }
  else{ //who is talking is a member
    for(k = 0;k<how_many_const;k++){// parcourir les channel
      if(ch[k]->sock_client != 0){ // this channel is full
        for(l = 0;l<MAX_Connexion;l++){ // parcourir la liste des users pour chaque channel
          if(ch[k]->user[l] ==  all_client[id]->sock_client){ //trouver la channel k qui contient le client id
            for(m = 0;m<MAX_Connexion;m++){
              if(ch[k]->user[m] != 0){ //parcourir les users inclus dans la channel de client k qui contient client id
                for(n=0;n<MAX_Connexion;n++){
                  if(ch[k]->user[m] == all_client[n]->sock_client){
                    memset(users,'\0',300);
                    strcat(users,"\n-----------");  // to know te name of user
                    strcat(users,all_client[n]->nickname);
                    send(all_client[id]->sock_client,users,strlen(users),0);
                    break;
                  }
                }
              }
            }
            break;
          }
        }
        //break;
      }
    }
  }

}

////////////////////////////////////////////////////////////////////////////

int modify_nick(int id,char *msg,struct Info_client **all_client){ //add a nickname to client
  char *Welcome = malloc(200);
  char *last = malloc(200);
  memset(Welcome,'\0',200);
  memset(last,'\0',200);
  strcpy(last,all_client[id]->nickname);
  memset(all_client[id]->nickname,'\0',strlen(all_client[id]->nickname));
  strcpy(Welcome,RED "[Server]" RESET " : You have changed your nickname to : ");
  int i,debut=12;
  for(i=12;i<strlen(msg);i++){
    if (msg[i]==' '){
      debut=debut+1;
    }
  }
  if(is_empty(msg+debut)==0){
    send(all_client[id]->sock_client,RED"[Server] : "RESET"ERROR nickname, It is empty!",50,0);
    return 0;
  }
  strncpy(all_client[id]->nickname, msg+debut,50);
  strcat(Welcome, all_client[id]->nickname);
  printf(BLUE"Info : " RESET "The client %s is named up to now " CYAN ": %s \n" RESET, last, all_client[id]->nickname);
  send(all_client[id]->sock_client,Welcome,strlen(Welcome),0);
  all_client[id]->empty = 1;
  return 1;
}

////////////////////////////////////////////////////////////////////////////

int nick(int id,char *msg,struct Info_client **all_client){ //add a nickname to client
  char *Welcome = malloc(200);
  memset(Welcome,'\0',200);
  strcpy(Welcome,RED "[Server]" RESET " : Welcome to the chat ");
  int i,debut=6;
  for(i=6;i<strlen(msg);i++){
    if (msg[i]==' '){
      debut=debut+1;
    }
  }
  strncpy(all_client[id]->nickname, msg+debut,50);
  strcat(Welcome, all_client[id]->nickname);
  printf(BLUE"Info : " RESET "The client[%d] is named " CYAN ": %s \n" RESET, all_client[id]->sock_client, all_client[id]->nickname);
  send(all_client[id]->sock_client,Welcome,strlen(Welcome),0);
  all_client[id]->empty = 1;
  return 1;
}

////////////////////////////////////////////////////////////////////////////

int test_nickname(int id, struct Info_client **all_client){// return 1 if nickname exist; 0 else
  if(all_client[id]->empty != 0)
  {
    return 1;
  }
  else{
    return 0 ;
  }
}

////////////////////////////////////////////////////////////////////////////

void clear_client(int id1,struct Info_client **all_client,struct channel **ch){
  int i,j;
  memset(ch[id1]->name,'\0',strlen(ch[id1]->name));
  free(ch[id1]->name);
  ch[id1]->sock_client = 0;
  ch[id1]->empty = 0;
  ch[id1]->nb = 0;
  for(i=0;i<MAX_Connexion;i++){
    ch[id1]->user[i] = 0;
  }

  all_client[id1]->sock_client = -1;
  all_client[id1]->empty = 0;
  all_client[id1]->member = 0;
  all_client[id1]->port = 0;
  ////
  memset(all_client[id1]->ip_addr,'\0',strlen(all_client[id1]->ip_addr));
  memset(all_client[id1]->nickname,'\0',strlen(all_client[id1]->nickname));
  memset(all_client[id1]->time_connexion,'\0',strlen(all_client[id1]->time_connexion));
  /////
  free(all_client[id1]->ip_addr);
  free(all_client[id1]->nickname);
  free(all_client[id1]->time_connexion);

}

////////////////////////////////////////////////////////////////////////////

void fill_data(struct Info_client **all_client,struct channel **ch,int id,int sock,char *ip_addr,int port, char *time_connexion){
int i,aa;
  struct Info_client *client = malloc(sizeof(struct Info_client));
  struct channel *salon = malloc(sizeof(struct channel));
  salon->name = malloc(50);
  salon->sock_client = 0;
  salon->empty = 0;
  salon->nb = 0;
  for(i=0;i<MAX_Connexion;i++){
    salon->user[i] = 0;
  }
  ch[id] = salon;

  client->member =  0;
  client->empty =  0;
  client->sock_client = sock;
  client->port = port;
  client->ip_addr = malloc(50);
  client->nickname = malloc(50);
  client->time_connexion = malloc(50);
  strcpy(client->ip_addr,ip_addr);
  strcpy(client->time_connexion,time_connexion);
  all_client[id] = client;
}
////////////////////////////////////////////////////////////////////////////

void init_channel(struct channel channel){
	memset(channel.name,'\0',sizeof(channel.name));
	memset(channel.user,'\0',sizeof(channel.user));
}

////////////////////////////////////////////////////////////////////////////

void init_channel_name(struct channel channel){
	memset(channel.name,'\0',sizeof(channel.name));
}

////////////////////////////////////////////////////////////////////////////

void init_channel_user(struct channel channel){
	memset(channel.user,'\0',sizeof(channel.user));
}

////////////////////////////////////////////////////////////////////////////

void fill_channel(int id_ch,struct channel list_channel[],int fd,int k){
	list_channel[id_ch].user[k] = fd;
}

////////////////////////////////////////////////////////////////////////////

void remove_sock(int id_ch,struct channel list_channel[],int k){
	list_channel[id_ch].user[k] = 0;
}
////////////////////////////////////////////////////////////////////////////

void get_connexion_time(char *chaine){
  time_t now = time (0);
  strftime (chaine, 100, "%Y-%m-%d@%H:%M", localtime (&now));
}

////////////////////////////////////////////////////////////////////////////

void join(int id, char *msg,struct Info_client **all_client,struct channel **ch,int how_many){
  int i,k,yes = 0;
  int id_ch = -1;
  int exist = 0;
  char *alert = malloc(100);
  char *alert2 = malloc(100);
  char *Chname = malloc(50);
  memset(Chname,'\0',50);
  memset(alert,'\0',100);
  memset(alert2,'\0',100);
  strcpy(alert2,RED"[SERVER] : "RESET"You had Joined the channel ");
  //user already member in other channel
  if(all_client[id]->member == 1){
    send(all_client[id]->sock_client,RED"[SERVER] : "RESET"You can't join a group of channel\n",57,0);
    return;
  }
  //rechieve name of channel to join
  for( i = 0; i < strlen(msg)-1; i++){
    if(msg[14+i] != ' '){
      Chname[i] = msg[14+i];
    }
    else{
      break;
    }
  }
  strcat(alert2, Chname);
  //////////////////////////////////////
  for( i = 0; i < how_many; i++){
    if(strcmp(ch[i]->name,Chname) == 0){
      yes = 1; //channell exist
      id_ch =i;// rechieve its identify
      for(k = 0;k<how_many;k++){
        if(all_client[id]->member != 0 && ch[id_ch]->user[k] == all_client[id]->sock_client){ //tester si le demandeur est deja membre ou nn
          exist = 1;
          break;
        }
      }
      //break;
    }
  }
  if(yes==0){
    send(all_client[id]->sock_client,RED"[SERVER] : "RESET"Channel doesn't exist",43,0);
    //return;
  }
  else if(exist == 1){
    send(all_client[id]->sock_client,RED"[SERVER] : "RESET"You already exist in this channel",55,0);
    //return;
  }
  // member is not a member in any channel and channel name exist
  else{
      for(i=0;i<how_many;i++){
        if(ch[id_ch]->user[i] == 0){
          ch[id_ch]->user[i] = all_client[id]->sock_client;
          ch[id_ch]->nb++;
          all_client[id]->member = 1;
          send(all_client[id]->sock_client,alert2,strlen(alert2),0);
          break;
        }
      }
  }
}

////////////////////////////////////////////////////////////////////////////

void quit_channel(int id, char *msg,struct Info_client **all_client,struct channel **ch,int how_many){
  int i,k,j;
  char *msg_quit = malloc(100);
  memset(msg_quit,'\0',100);
      for(i = 0;i<how_many;i++){
        if(ch[i]->nb != 0){
          for(j = 0;j<MAX_Connexion;j++){
            if(ch[i]->user[j] == all_client[id]->sock_client){
              ch[i]->user[j] = 0;
              ch[i]->nb--;
              all_client[id]->member = 0;
              strcpy(msg_quit,RED"[Server] : "RESET"You are disconnected from the channel ");
              strcat(msg_quit, ch[i]->name);
              send(all_client[id]->sock_client,msg_quit,strlen(msg_quit),0);
              if (ch[i]->nb == 0) {
                memset(ch[i]->name,'\0',strlen(ch[i]->name));
                ch[i]->sock_client = 0;
                for(k = 0;k<MAX_Connexion;k++){
                  ch[i]->user[k] = 0;
                  ch[i]->empty = 0;
                }
              }
              break;
            }
          }
          break;
        }
      }
}
/////////////////////////////////////////////////////////////////////////////////////

void share_file(int id, char * msg,struct Info_client **all_client,int how_many){
  int i,k,j;
  int exist_user=0;
  int sock_dest;
  char *alert = malloc(500);
  char * chemin = malloc(500);
  char *dest = malloc(50);
  const char *delim= "/";
  char *token = malloc(100);
  //char * tab_token[strlen(chemin)];
  char *file_name = malloc(100);
  char client_response[100];
  memset(client_response,'\0',100);
  memset(file_name,'\0',100);
  memset(token,'\0',100);
  memset(chemin,'\0',500);
  memset(dest,'\0',50);
  memset(alert,'\0',500);
  // rechieve name of dest
  for( i = 0; i < strlen(msg)-1; i++){
    if(msg[6+i] != ' '){
      dest[i] = msg[6+i];
    }
    else{
      break;
    }
  }
  int len = 6+strlen(dest)+1;
  //rechieve path
  for( i = 0; i < strlen(msg)-1; i++){
    if(msg[len+i] == '"'){
      for( j = len+i+1; j < strlen(msg)-1; j++){
        if(msg[j] != '"'){
          chemin[j-len-i-1] = msg[j];
        }
        else{
          break;
        }
      }
      break;
    }
  }
  char path[strlen(chemin)];
  char name_rcv[500];
  strcpy(path,chemin);
  for(i =0;i<how_many;i++){
    if(strncmp(all_client[i]->nickname,dest,strlen(dest)-1) == 0){
      exist_user = 1;
      sock_dest = all_client[i]->sock_client;
      break;
    }
  }
  if(exist_user == 1){
    // get the first token
    token = strtok(chemin, delim);
    // walk through other tokens
    while( token != NULL ) {
      file_name = token;
      token = strtok(NULL, delim);
    }
    strcpy(alert,all_client[id]->nickname);
    strcat(alert," wants you to accept the transfer of the file named ");
    char str[100];
    sprintf(str, "-%s-. Do you accept? [Y/n]",file_name);
    strcat(alert,str);
    send(sock_dest,alert,strlen(alert),0);
    memset(alert,'\0',strlen(alert));
    memset(str,'\0',strlen(str));
    for(;;){
      int recv_bytes = recv(sock_dest,client_response,100,0);
      if(strcmp(client_response,"Y\n")==0 || strcmp(client_response,"y\n")==0){
        memset(client_response,'\0',strlen(client_response));
        strcpy(str,"Do you want Save or Open the file?\n-----Enter Save to have a copy of file\n-----Or,Tap Open to view the file\n");
        send(sock_dest,str,strlen(str),0);
        memset(str,'\0',strlen(str));
        for(;;){
          recv(sock_dest,client_response,100,0);// recieve if client tap Save or open
          if(strcmp(client_response,"Save\n")==0){
            memset(alert,'\0',strlen(alert));
            strcpy(alert,"Please give a <name.txt> to save the file\n");
            send(sock_dest,alert,strlen(alert),0);
            send(sock_dest,BLUE"Name of recieved file is : "RESET,38,0);
            recv_bytes = recv(sock_dest,client_response,100,0);// get the new path
            strcpy(name_rcv,client_response);
            //printf("path rcv --%s--\n",name_rcv );
            FILE *fichier = NULL;
            char chaine[512]; //
            fichier = fopen(path, "r");
            if(fichier == NULL){
              printf("fopen ERROR\n");
              exit(1);
            }
            mkdir(dest,0777);

            char *chemiiiin = malloc(1000);
            memset(chemiiiin,'\0',1000);
            char *adresse = malloc(500);
            memset(adresse,'\0',500);
            sprintf(adresse,"%s/%s",dest,name_rcv);
            char *currentpath = malloc(500);
            memset(currentpath,'\0',500);
            getcwd(currentpath, 300);
            //getwd(currentpath);
            sprintf(chemiiiin,"%s/%s/%s",currentpath,dest,name_rcv);
            FILE *file_rcv = NULL;
            //file_rcv = fopen(name_rcvcv,"w");
            file_rcv = fopen(adresse,"w");
            if(file_rcv == NULL){
              printf("fopen file recv ERROR\n");
              exit(1);
            }
            while(fgets(chaine,512,fichier)){
              fseek(file_rcv, 0, SEEK_END);// se deplacer toujours à la fin du ficheir pour ajouter les nouvelle lignes
              //printf("%s\n",chaine);
              fputs(chaine,file_rcv); // ecrire la ligne "chaine" à la fin du fichier de descripteur file_rcv
            }
            //printf("%s\n",path);
            //printf("%s\n",name_rcvcv);
            fclose(fichier);
            fclose(file_rcv);
            memset(alert,'\0',strlen(alert));
            strcpy(alert,dest);
            strcat(alert," accepted file transfert.\n");
            send(all_client[id]->sock_client,alert,strlen(alert),0);
            memset(alert,'\0',strlen(alert));
            sprintf(alert,"%s saved in %s\n",file_name,chemiiiin);
            send(sock_dest,alert,strlen(alert),0);
            memset(alert,'\0',strlen(alert));
            sprintf(alert,"%s received file %s\n",dest,file_name);
            send(all_client[id]->sock_client,alert,strlen(alert),0);
            memset(alert,'\0',strlen(alert));
            break;
          }
          else if(strcmp(client_response,"Open\n")==0){
            memset(alert,'\0',strlen(alert));
            FILE *fichier = NULL;
            char chaine[512]; //
            fichier = fopen(path, "r");
            if(fichier == NULL){
              printf("fopen ERROR\n");
              exit(1);
            }
            memset(alert,'\0',strlen(alert));
            sprintf(alert,"///////////////////////////////////// %s opened: /////////////////////////////////////\n",file_name);
            send(sock_dest,alert,strlen(alert),0);
            memset(alert,'\0',strlen(alert));
            while(fgets(chaine,512,fichier)){
              send(sock_dest,chaine,strlen(chaine),0);
            }
            sprintf(alert,"///////////////////////////////////// %s Ended /////////////////////////////////////\n",file_name);
            send(sock_dest,alert,strlen(alert),0);
            fclose(fichier);
            memset(alert,'\0',strlen(alert));
            strcpy(alert,dest);
            strcat(alert," accepted file transfert.\n");
            send(all_client[id]->sock_client,alert,strlen(alert),0);
            memset(alert,'\0',strlen(alert));
            sprintf(alert,"%s received file %s\n",dest,file_name);
            send(all_client[id]->sock_client,alert,strlen(alert),0);
            memset(alert,'\0',strlen(alert));
            break;
          }
          else{
            strcpy(alert,RED"Please Save or Open the file \n"RESET);
            send(sock_dest,alert,strlen(alert),0);
            memset(alert,'\0',strlen(alert));
            memset(client_response,'\0',strlen(client_response));
          }
        }
        break;
      }
      else if(strcmp(client_response,"N\n")==0 || strcmp(client_response,"n\n")==0){
        strcpy(alert,dest);
        strcat(alert," cancelled file transfer");
        send(all_client[id]->sock_client,alert,strlen(alert),0);
        memset(alert,'\0',strlen(alert));
        memset(client_response,'\0',strlen(client_response));
        break;
      }
      else{
        strcpy(alert,RED"Please send a response to this request \n"RESET);
        send(sock_dest,alert,strlen(alert),0);
        memset(alert,'\0',strlen(alert));
        memset(client_response,'\0',strlen(client_response));
      }
    }
  }
  else{
    send(all_client[id]->sock_client,RED"[Server] : "RESET"User doesn't EXIST\n",42,0);
  }
}

///////////////////////////////////////////////////////////////////////////////


void error(const char *msg)
{
    perror(msg);
    exit(1);
}



//do_socket()
int do_socket(int domain, int type, int protocole){
  int sockfd;
  int yes =1;

  sockfd = socket(domain,type,protocole);
  if(sockfd == -1){
    printf("Socket error");
    exit(EXIT_FAILURE);
  }
  printf("Socket created\n");
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        perror("ERROR setting socket options");

  return sockfd;
}

void init_serv_addr(const char* port, struct sockaddr_in *serv_addr) {
  int portno;
  //clean the serv_add structure
  memset(serv_addr, 0, sizeof(serv_addr) );
  //cast the port from a string to an int
  portno= atoi(port);
  //internet family protocol
  serv_addr->sin_family= AF_INET;
  //we bind to any ip form the host
  serv_addr->sin_addr.s_addr=INADDR_ANY ;
  //we bind on the tcp port specified
  serv_addr->sin_port=htons(portno) ;
}

void do_bind(int socket,struct sockaddr_in addr_in ){
  if( bind(socket,(struct sockaddr *)&addr_in , sizeof(addr_in)) < 0)
    {
      //print the error message
      perror("Bind failed !");
      exit( EXIT_FAILURE );
    }
  printf("Bind succeded !\n");
}
int do_accept(int socket,struct sockaddr *addr, socklen_t * addrlen){

  int new_sock=accept(socket,addr,addrlen);
  if(new_sock==-1){
    perror("Failed !");
    exit( EXIT_FAILURE );
  }
  printf("Connection accepted\n");
  return new_sock;
}

void do_listen(int socket,int nbclient){

  if(listen(socket,nbclient) ==-1 )
    {
      perror("listen");
      exit( EXIT_FAILURE );
    }
  printf("Waiting...\n");
}
