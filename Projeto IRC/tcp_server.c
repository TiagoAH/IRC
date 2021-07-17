/*******************************************************************
 * SERVIDOR no porto 9000, à escuta de novos clientes.  Quando surjem  
 * novos clientes os dados por eles enviados são lidos e descarregados no ecran.
 *******************************************************************/
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/ipc.h> 
#include <sys/shm.h>
#include "semlib.h"
 #include <signal.h>

#define SERVER_PORT     8084
#define BUF_SIZE	1024
#define MAX_USERS 99
#define MAX_MESSAGES 999

typedef struct{
    char username[30];
    char password[9];
    int allowed;
} user_struct;

typedef struct{
    char From[30];
    char To[30];
    char Assunto[1024];
    char message[1024];
    int read;
    int id;
} message_struct;

int *user_counter;
int *message_counter;
int *id;
user_struct* users;
message_struct* messages;
int shmiduser_struct,shmidmessage_struct,shmiduser_counter,shmidmessage_counter,shmidid;



void process_client(int fd);
void erro(char *msg);
int config();
void init();
void remove_message(int index);
void remove_user(int index);
void update_files();
void sigint_handler();

int main() {
  init();
  config();
  int fd, client;
  struct sockaddr_in addr, client_addr;
  int client_addr_size;
  signal(SIGINT, sigint_handler);
  bzero((void *) &addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(SERVER_PORT);

  if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	erro("na funcao socket");
  if ( bind(fd,(struct sockaddr*)&addr,sizeof(addr)) < 0)
	erro("na funcao bind");
  if( listen(fd, 5) < 0) 
	erro("na funcao listen");
  
  while (1) {
    client_addr_size = sizeof(client_addr);
    client = accept(fd,(struct sockaddr *)&client_addr,(socklen_t *)&client_addr_size);
    if (client > 0) {
      if (fork() == 0) {
        close(fd);
        process_client(client);
        exit(0);
      }
    close(client);
    }
  }
  return 0;
}

void process_client(int client_fd)
{ 
  char logged_user[30];
  int logtypes[3]={0,99,123};
  int i=0;
  int y=0;
	int nread = 0;
	char buffer[BUF_SIZE];
  char string[BUF_SIZE];
  char aux_users[64][30];
  int aux=0;
  int aux2=0;
  int aux3=0;
  int counter_messages=0;
  char aux99[30];
  for(i=0;i<3;i++){
    printf("%d\n",logtypes[i]);
  }

Master:
	nread = read(client_fd, buffer, BUF_SIZE-1);	
	buffer[nread] = '\0';
	printf("Recebido:%s\n", buffer);
  if(strcmp(buffer,"0")==0)
    goto login;    
  else if(strcmp(buffer,"99")==0)
    goto fim;
  else if(strcmp(buffer,"1")==0)
    goto list_messages;
  else if(strcmp(buffer,"2")==0)
    goto list_users;
  else if(strcmp(buffer,"3")==0)
    goto write_message;
  else if(strcmp(buffer,"4")==0)
    goto list_read;
  else if(strcmp(buffer,"5")==0)
    goto delete_messages;
  else if(strcmp(buffer,"6")==0)
    goto change_pw;
  else if(strcmp(buffer,"7")==0)
    goto check_operator;
  else if(strcmp(buffer,"8")==0)
    goto remove_operator;
  else if(strcmp(buffer,"9")==0)
    goto list_unreadall;
  else if(strcmp(buffer,"10")==0)
    goto list_readall;
  else if(strcmp(buffer,"11")==0)
    goto delete_any_message;
  else if(strcmp(buffer,"12")==0)
    goto show_all_users;
  else if(strcmp(buffer,"13")==0)
    goto delete_user;
  else if(strcmp(buffer,"14")==0)
    goto create_user;
  else if(strcmp(buffer,"15")==0)
    goto change_permission;
  else
    goto fim;  



login:
  nread = read(client_fd, buffer, BUF_SIZE-1);  
  buffer[nread] = '\0';
  printf("User/Password:%s\n",buffer);
  for(i=0;i<*user_counter;i++){
    sprintf(string," ");
    strcpy(string,users[i].username);
    strcat(string,"/");
    strcat(string,users[i].password);
    if(strcmp(buffer,string)==0 && users[i].allowed==1){
      write(client_fd,&logtypes[0],sizeof(int));
      strcpy(logged_user,users[i].username);
      fflush(stdout);
      goto Master;
    }
    else if(strcmp(buffer,string)==0 && users[i].allowed==99){
      write(client_fd,&logtypes[1],sizeof(int));
      strcpy(logged_user,users[i].username);
      fflush(stdout);
      goto Master;
    }
    else if(strcmp(buffer,"ADMIN/ADMIN")==0){
       write(client_fd,&logtypes[1],sizeof(int));
       sprintf(logged_user,"ADMIN");
       fflush(stdout);
        goto Master;
    }
  }
  write(client_fd,&logtypes[2],sizeof(int));
  fflush(stdout);
  goto Master;



list_messages:
  aux2=0;
  counter_messages=0;
  for(i=0;i<*message_counter;i++){
    if(strcmp(messages[i].To,logged_user)==0 && messages[i].read==0){
      counter_messages++;
    }
  }
  write(client_fd,&counter_messages,sizeof(int));
  for(i=0;i<*message_counter;i++){
      if(strcmp(messages[i].To,logged_user)==0 && messages[i].read==0){
        strcpy(buffer,messages[i].From);
        write(client_fd,buffer,sizeof(buffer));
        write(client_fd,&messages[i].id,sizeof(int));
        strcpy(buffer,messages[i].Assunto);
        write(client_fd,buffer,sizeof(buffer));
        strcpy(buffer,messages[i].message);
        write(client_fd,buffer,sizeof(buffer));
        messages[i].read=1;
      }
  }
  goto Master;



write_message:
  aux2=0;
  nread = read(client_fd, buffer, BUF_SIZE-1);  
  buffer[nread] = '\0';
  printf("Numero de destinatarios %s\n",buffer);
  aux3=atoi(buffer);
  for(i=0;i<aux3;i++){
    printf("Esperando por nomes:\n");
    aux=0;
    nread = read(client_fd, buffer, BUF_SIZE-1);  
    buffer[nread] = '\0';
    printf("User tentanto enviar mensagem a:%s\n",buffer);
    for(y=0;y<*user_counter;y++){
      if(strcmp(buffer,users[y].username)==0 && users[y].allowed!=0){
        aux=1;
      }
    }
    if(aux==1){
      strcpy(aux_users[aux2],buffer);
      printf("Adicionado Destino no Aux:%s, com posicao%d\n",aux_users[aux2],aux2);
      messages[*message_counter].read=0;
      printf("Utilizador encontrado e nao bloqueado\n");
      write(client_fd,&aux,sizeof(int));
      aux2++;
    }
    else{
      printf("Utilizador nao encontrado ou bloquado\n");
      write(client_fd,&aux,sizeof(int));
    }
  }
  if(aux2==0)
    goto Master;
  else{
    printf("Esperando pelo assunto.\n");
    nread = read(client_fd, string, BUF_SIZE-1);  
    string[nread] = '\0';
    printf("Esperando pela mensagem.\n");
    nread = read(client_fd, buffer, BUF_SIZE-1);  
    buffer[nread] = '\0';
    printf("Recebi a mensagem com conteudo:%s",buffer);
    for(i=0;i<aux2;i++){
      strcpy(messages[*message_counter].From,logged_user);
      strcpy(messages[*message_counter].To,aux_users[i]);
      strcpy(messages[*message_counter].Assunto,string);
      strcpy(messages[*message_counter].message,buffer);
      messages[*message_counter].id=*id;
      (*message_counter)++;
      (*id)++;
    }
  }
  fflush(stdout);
  goto Master;


list_read:
  aux2=0;
  counter_messages=0;
  for(i=0;i<*message_counter;i++){
    if(strcmp(messages[i].To,logged_user)==0 && messages[i].read==1){
      counter_messages++;
    }
  }
  write(client_fd,&counter_messages,sizeof(int));
  for(i=0;i<*message_counter;i++){
      if(strcmp(messages[i].To,logged_user)==0 && messages[i].read==1){
        strcpy(buffer,messages[i].From);
        write(client_fd,buffer,sizeof(buffer));
        write(client_fd,&messages[i].id,sizeof(int));
        strcpy(buffer,messages[i].Assunto);
        write(client_fd,buffer,sizeof(buffer));
        strcpy(buffer,messages[i].message);
        write(client_fd,buffer,sizeof(buffer));
      }
  }
  goto Master;

delete_messages:
  nread = read(client_fd, buffer, BUF_SIZE-1);  
  buffer[nread] = '\0';
  printf("Tentando eliminar mensagem com id:%d\n",atoi(buffer));
  aux2=-1;
  for(i=0;i<*message_counter;i++){
      if(strcmp(messages[i].To,logged_user)==0 && atoi(buffer)==messages[i].id){
        aux2=i;
      } 
  }
  if(aux2==-1){
    printf("Nao apaguei nada\n");
    write(client_fd,&aux2,sizeof(int));
  }
  else{
    remove_message(aux2);
    printf("Devo ter apagado uma mensagem\n");
    write(client_fd,&aux2,sizeof(int));
  }
  goto Master;



list_users:
  printf("Listando users\n");
  strcpy(string,users[0].username);
  strcat(string,"\n");
  for(i=1;i<*user_counter;i++){
    if(users[i].allowed!=0){
      strcat(string,users[i].username);
      if(i!=*user_counter-1)
        strcat(string,"\n");
    }
  }
  write(client_fd,string,sizeof(string));
  printf("%s\n",string);
  fflush(stdout);
  goto Master;

change_pw:
  nread = read(client_fd, buffer, BUF_SIZE-1);  
  buffer[nread] = '\0';
  for(i=0;i<*user_counter;i++){
    if(strcmp(logged_user,users[i].username)==0){
      printf("Mudando a password do utilizador:%s\n",logged_user);
      strcpy(users[i].password,buffer);
      printf("%s\n",users[i].password);
    }
  }
  fflush(stdout);
  goto Master;

check_operator:
  printf("Listando todos os users e seus acessos");
  for(i=0;i<*user_counter;i++){
    printf("%s,%d\n",users[i].username,users[i].allowed );
  }
  aux=0;
  for(i=0;i<*user_counter;i++){
    if(users[i].allowed==99){
        aux=1;
       write(client_fd,&aux,sizeof(int));
       break;
    }
  }
    if(aux==0){
      write(client_fd,&logtypes[0],sizeof(int));
      for(i=0;i<*user_counter;i++){
        printf("%d\n",strcmp(users[i].username,logged_user));
        if(strcmp(users[i].username,logged_user)==0){
        users[i].allowed=99;
        printf("Users %s e um novo operador,%d\n",users[i].username,users[i].allowed);
        } 
      }
    }
  fflush(stdout);
  goto Master;

remove_operator:
  aux=0;
  for(i=0;i<*user_counter;i++){
    if(strcmp(users[i].username,logged_user)==0 && users[i].allowed==99){
      users[i].allowed=1;
      write(client_fd,&logtypes[0],sizeof(int));
      printf("Utilizador:%s deixou de ser operador\n",users[i].username);
    }
  }
  goto Master;

list_unreadall:
  aux2=0;
  counter_messages=0;
  for(i=0;i<*message_counter;i++){
    if(messages[i].read==0)
      counter_messages++;
  }
  write(client_fd,&counter_messages,sizeof(int));
  for(i=0;i<*message_counter;i++){
      if(messages[i].read==0){
        strcpy(buffer,messages[i].From);
        write(client_fd,buffer,sizeof(buffer));
        strcpy(buffer,messages[i].To);
        write(client_fd,buffer,sizeof(buffer));
        write(client_fd,&messages[i].id,sizeof(int));
        strcpy(buffer,messages[i].Assunto);
        write(client_fd,buffer,sizeof(buffer));
        strcpy(buffer,messages[i].message);
        write(client_fd,buffer,sizeof(buffer));
      }
  }
  goto Master;


list_readall:
  aux2=0;
  counter_messages=0;
  for(i=0;i<*message_counter;i++){
    if(messages[i].read==1)
      counter_messages++;
  }
  write(client_fd,&counter_messages,sizeof(int));
  for(i=0;i<*message_counter;i++){
      if(messages[i].read==1){
        strcpy(buffer,messages[i].From);
        write(client_fd,buffer,sizeof(buffer));
        strcpy(buffer,messages[i].To);
        write(client_fd,buffer,sizeof(buffer));
        write(client_fd,&messages[i].id,sizeof(int));
        strcpy(buffer,messages[i].Assunto);
        write(client_fd,buffer,sizeof(buffer));
        strcpy(buffer,messages[i].message);
        write(client_fd,buffer,sizeof(buffer));
      }
  }
  goto Master;


delete_any_message:
  nread = read(client_fd, buffer, BUF_SIZE-1);  
  buffer[nread] = '\0';
  printf("Tentando eliminar mensagem com id:%d\n",atoi(buffer));
  aux2=-1;
  for(i=0;i<*message_counter;i++){
      if(atoi(buffer)==messages[i].id){
        aux2=i;
      } 
  }
  if(aux2==-1){
    printf("Nao apaguei nada\n");
    write(client_fd,&aux2,sizeof(int));
  }
  else{
    remove_message(aux2);
    printf("Devo ter apagado uma mensagem\n");
    write(client_fd,&aux2,sizeof(int));
  }
  goto Master;


show_all_users:
  printf("Listando users\n");
  strcpy(string,users[0].username);
  strcat(string,"\n");
  for(i=1;i<*user_counter;i++){
      strcat(string,users[i].username);
      if(i!=*user_counter-1)
        strcat(string,"\n");
  }
  write(client_fd,string,sizeof(string));
  printf("%s\n",string);
  fflush(stdout);
  goto Master;

delete_user:
  nread = read(client_fd, buffer, BUF_SIZE-1);  
  buffer[nread] = '\0';
  printf("Tentando eliminar user:%s\n",buffer);
  aux2=-1;
  for(i=0;i<*user_counter;i++){
      if(strcmp(buffer,users[i].username)==0){
        aux2=i;
      } 
  }
  if(aux2==-1){
    printf("Nao apaguei ninguem\n");
    write(client_fd,&aux2,sizeof(int));
  }
  else{
    remove_user(aux2);
    printf("Devo ter apagado o user\n");
    write(client_fd,&aux2,sizeof(int));
  }
  goto Master;

create_user:
  nread = read(client_fd, buffer, BUF_SIZE-1);  
  buffer[nread] = '\0';
  strcpy(users[*user_counter].username,buffer);
  nread = read(client_fd, buffer, BUF_SIZE-1);  
  buffer[nread] = '\0';
  strcpy(users[*user_counter].password,buffer);
  users[*user_counter].allowed=1;
  (*user_counter)++;
  printf("Criei um user,Printado nova lista de users\n");
  for(i=0;i<*user_counter;i++){
    printf("%s,%s,%d\n",users[i].username,users[i].password,users[i].allowed);
  }
  goto Master;

change_permission:
  aux2=-1;
  nread = read(client_fd, buffer, BUF_SIZE-1);  
  buffer[nread] = '\0';
  for(i=0;i<*user_counter;i++){
    if(strcmp(buffer,users[i].username)==0){
      aux2=1;
      strcpy(aux99,users[i].username);
    }
  }
  write(client_fd,&aux2,sizeof(int));
  if(aux2==1){
    nread = read(client_fd, buffer, BUF_SIZE-1);  
    buffer[nread] = '\0';
    printf("Recebi do user o pedido para mudar o estado do user%s para%d\n",aux99,atoi(buffer));
    for(i=0;i<*user_counter;i++){
      if(strcmp(aux99,users[i].username)==0)
        users[i].allowed=atoi(buffer);
    }
    goto Master;   
  }
  else{
    goto Master;
  }  


fim:
  printf("User %s disconectou-se\n",logged_user);
}



void erro(char *msg)
{
	printf("Erro: %s\n", msg);
	exit(-1);
}


int config(){
    printf("Reading users file");
    int filemax;
    char line[1024];
    *user_counter=0;
    *message_counter=0;
    *id=0;
    FILE* fp;
    if( (fp = fopen("client.aut", "r")) == NULL){
        perror("Users file missing\n");
        exit(0);
    }
    fseek(fp,0,SEEK_END);
    filemax=ftell(fp);
    rewind(fp);
    while(ftell(fp)<filemax){
      fgets(line, sizeof(line)-1, fp);
      line[strlen(line)-1]='\0';
      strcpy(users[*user_counter].username,line);
      printf("Username: %s\n", users[*user_counter].username);
      fgets(line, sizeof(line)-1, fp);
      line[strlen(line)-1]='\0';
      strcpy(users[*user_counter].password,line);
      printf("Password: %s\n", users[*user_counter].password);
      fgets(line, sizeof(line)-1, fp);
      line[strlen(line)-1]='\0';
      users[*user_counter].allowed = atoi(line);
      printf("Allowed: %d\n", users[*user_counter].allowed);
      (*user_counter)++;
    }
  fclose(fp);
    if( (fp = fopen("messages.txt", "r")) == NULL){
        perror("Messages file missing\n");
        exit(0);
    }
    fseek(fp,0,SEEK_END);
    filemax=ftell(fp);
    rewind(fp);
    while(ftell(fp)<filemax){
      fgets(line, sizeof(line)-1, fp);
      line[strlen(line)-1]='\0';
      strcpy(messages[*message_counter].From,line);

      fgets(line, sizeof(line)-1, fp);
      line[strlen(line)-1]='\0';
      strcpy(messages[*message_counter].To,line);

      fgets(line, sizeof(line)-1, fp);
      line[strlen(line)-1]='\0';
      strcpy(messages[*message_counter].Assunto,line);

      fgets(line, sizeof(line)-1, fp);
      line[strlen(line)-1]='\0';
      strcpy(messages[*message_counter].message,line);

      fgets(line, sizeof(line)-1, fp);
      line[strlen(line)-1]='\0';
      messages[*message_counter].read=atoi(line);

      fgets(line, sizeof(line)-1, fp);
      line[strlen(line)-1]='\0';
      messages[*message_counter].id=atoi(line);

      (*message_counter)++;
      (*id)++;
    }
  fclose(fp);



    return 0;
}


void init(){
  shmiduser_struct = shmget(IPC_PRIVATE, MAX_USERS*sizeof(user_struct), IPC_CREAT|0777);
  users = (user_struct*) shmat(shmiduser_struct, NULL, 0);
  shmidmessage_struct = shmget(IPC_PRIVATE, MAX_MESSAGES*sizeof(message_struct), IPC_CREAT|0777);
  messages = (message_struct*) shmat(shmidmessage_struct, NULL, 0);
  shmiduser_counter = shmget(IPC_PRIVATE,sizeof(int), IPC_CREAT|0700);
  user_counter =(int*) shmat(shmiduser_counter, NULL, 0);
  shmidmessage_counter = shmget(IPC_PRIVATE,sizeof(int), IPC_CREAT|0700);
  message_counter =(int*) shmat(shmidmessage_counter, NULL, 0);
  shmidid = shmget(IPC_PRIVATE,sizeof(int), IPC_CREAT|0700);
  id =(int*) shmat(shmidid, NULL, 0);
}


void remove_message(int index){
  int i;
  for(i=index;i<*message_counter;i++)
    messages[i]=messages[i+1];
  (*message_counter)--;
}

void remove_user(int index){
  int i;
  for(i=index;i<*user_counter;i++)
    users[i]=users[i+1];
  (*user_counter)--;
}

void update_files(){
  printf("Updating files\n");
  int i=0;
  FILE* fc;
  if( (fc = fopen("client.aut", "w+")) == NULL){
      perror("Users file missing\n");
      exit(0);
  }
  for(i=0;i<*user_counter;i++){
    fprintf(fc,"%s\n",users[i].username);
    fprintf(fc,"%s\n",users[i].password);
    if(i==(*user_counter)-1)
      fprintf(fc,"%d",users[i].allowed);
    else
      fprintf(fc,"%d\n",users[i].allowed);
  }
  fclose(fc);
  FILE* fm;
  if( (fm = fopen("messages.txt", "w+")) == NULL){
      perror("message file missing\n");
      exit(0);
  }
  for(i=0;i<*message_counter;i++){
    fprintf(fm,"%s\n",messages[i].From);
    fprintf(fm,"%s\n",messages[i].To);
    fprintf(fm,"%s\n",messages[i].Assunto);
    fprintf(fm,"%s\n",messages[i].message);
    fprintf(fm,"%d\n",messages[i].read);
    if(i==(*message_counter)-1)
      fprintf(fm,"%d",messages[i].id);
    else
      fprintf(fm,"%d\n",messages[i].id);
  }
  fclose(fm);

}


void sigint_handler(){
    update_files();
    printf("Cleaning up\n");
    shmctl(shmiduser_struct, IPC_RMID, NULL);
    shmctl(shmidmessage_counter, IPC_RMID, NULL);
    shmctl(shmidid, IPC_RMID, NULL);
    shmctl(shmiduser_counter, IPC_RMID, NULL);
    shmctl(shmidmessage_struct, IPC_RMID, NULL);

    exit(0);
}
