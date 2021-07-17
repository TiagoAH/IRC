#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

void erro(char *msg);

int main(int argc, char *argv[]) {
  int i=0;
  char buffer[1024];
  char endServer[100];
  int fd;
  struct sockaddr_in addr;
  struct hostent *hostPtr;
  int logi,tryagain=1;
  int aux2;
  int aux3=0;
  char aux[20];
  char user[20];
  char send[30];
  char choice[3];

  if (argc != 3) {
    printf("cliente <host> <port>\n");
    exit(-1);
  }

  strcpy(endServer, argv[1]);
  if ((hostPtr = gethostbyname(endServer)) == 0) {
    printf("Couldn't get host address.\n");
    exit(-1);
  }

  bzero((void *) &addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;
  addr.sin_port = htons((short) atoi(argv[2]));

  if((fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	erro("socket");
  if( connect(fd,(struct sockaddr *)&addr,sizeof (addr)) < 0)
	erro("Connect");
login:
    sprintf(choice,"0");
    write(fd,choice,strlen(choice));
      do{ 
        printf("Login: ");
        scanf("%s",user);
        if(strlen(user)>20)
          printf("Username nao valido, demasiado longo\n");
      }while(strlen(user)>20);
      strcpy(send,user);
      do{
        printf("Password: ");
        scanf("%s",aux);
        if(strlen(aux)>9)
          printf("Password nao valida, demasiado longa\n");
      }while(strlen(aux)>9);
      strcat(send,"/");
      strcat(send,aux);
      write(fd,send,strlen(send));
      read(fd,&logi,sizeof(int));
      printf("%d\n",logi);
      if(logi==0){
        goto menuUser;
      }
      else if(logi==99){
        goto menuOperator;
      }
      else{
        printf("User nao existente/password errada/User bloqueado\n");
        printf("Tentar de novo? 0-S 1-N\n");
        scanf("%d",&tryagain);
        if(tryagain==1)
          goto fim;
        else if(tryagain==0)
          goto login;
        else{
          goto fim;
        }
      }

menuUser:
  printf("Menu\n 1-Listar mensagens nao lidas\n 2-Listar Utilizadores\n 3-Enviar mensagem\n 4-Listar mensagens lidas\n 5-Apagar mensagen\n 6-Alterar Password\n 7-Obter privilegios\n 99-Quit\n");
  printf("Opcao:");
  scanf("%s",choice);
  if(strcmp(choice,"99")==0){
    goto fim;
  }
  else if(strcmp(choice,"1")==0){
    printf("Listando todas as mensagens nao lidas\n");
    write(fd,choice,strlen(choice));
    read(fd,&aux2,sizeof(int));
    printf("Numero de mensagens%d\n",aux2);
    for(i=0;i<aux2;i++){
      read(fd,buffer,sizeof(buffer));
      printf("Mensagem de:%s\n",buffer);
      read(fd,&aux3,sizeof(int));
      printf("Id da mensagem:%d\n",aux3);
      read(fd,buffer,sizeof(buffer));
      printf("Assunto da mensagem:%s\n",buffer);
      read(fd,buffer,sizeof(buffer));
      printf("Conteudo da mensagem:%s\n",buffer);
      printf("*********************************************************************************\n");
    }
    goto menuUser;
  }
  else if(strcmp(choice,"2")==0){
   write(fd,choice,strlen(choice));
   read(fd,buffer,sizeof(buffer));
   printf("%s\n",buffer);
    goto menuUser;
  }
  else if(strcmp(choice,"3")==0){
    aux3=0;
    write(fd,choice,strlen(choice));
    do{
        printf("Insira a quantas pessoas prentede enviar:\n");
        scanf("%s",choice);
        if(atoi(choice)>64 || atoi(choice)==0)
          printf("Tem que ser pelo menos para 1 ou para menos de 64\n");
    }while(atoi(choice)==0 || atoi(choice)>64);
    write(fd,choice,strlen(choice));
    for(i=0;i<atoi(choice);i++){
      do{
        printf("Insira o nome do destinatario:\n");
        scanf("%s",user);
        if(strlen(user)>20 || strlen(user)==0)
          printf("O destinatario e demasiado curto ou longo\n");
      }while(strlen(user)>20 || strlen(user)==0);
      write(fd,user,strlen(user));
      read(fd,&aux2,sizeof(int));
      if(aux2==0)
        printf("Utilizador nao encontrado ou bloqueado\n");
      else{
        printf("Utilizador encontrado e nao bloqueado\n");
        aux3++;
      }
    }
    if(aux3==0)
      goto menuUser;
    printf("Insira o Assunto da mensagem com max de 1024 caracteres\n");
    scanf("%s",buffer);//TROCAR SCANF PARA PODER LER ESPACOS
    write(fd,buffer,strlen(buffer));
    printf("Insira a mensagem com max de 1024 caracteres\n");
    scanf("%s",buffer);//TROCAR SCANF PARA PODER LER ESPACOS

    write(fd,buffer,strlen(buffer));
    printf("Mensagem enviada\n");
    goto menuUser;
  }
  else if(strcmp(choice,"5")==0){
    write(fd,choice,strlen(choice));
    do{
        printf("Insira o id da mensagem:\n");
        scanf("%s",choice);
        if(strlen(choice)>3)
          printf("ID entre 0 a 999\n");
    }while(strlen(choice)>3);
    write(fd,choice,strlen(choice));
    read(fd,&tryagain,sizeof(int));
    if(tryagain!=-1)
      printf("Mensagem apagada\n");
    else
      printf("Nao existe mensagem sua com este Id tente de novo\n");
    goto menuUser;
  }
  else if(strcmp(choice,"4")==0){
    printf("Listando todas as mensagens ja lidas\n");
    write(fd,choice,strlen(choice));
    read(fd,&aux2,sizeof(int));
    printf("Numero de mensagens ja lidas:%d\n",aux2);
    for(i=0;i<aux2;i++){
      read(fd,buffer,sizeof(buffer));
      printf("Mensagem de:%s\n",buffer);
      read(fd,&aux3,sizeof(int));
      printf("Id da mensagem:%d\n",aux3);
      read(fd,buffer,sizeof(buffer));
      printf("Assunto da mensagem:%s\n",buffer);
      read(fd,buffer,sizeof(buffer));
      printf("Conteudo da mensagem:%s\n",buffer);
      printf("*********************************************************************************\n");
    }
    goto menuUser;
  }
  else if(strcmp(choice,"6")==0){
    write(fd,choice,strlen(choice));
      do{
        printf("Insira nova password:\n");
        scanf("%s",aux);
        if(strlen(aux)>9)
          printf("Password nao valida, demasiado longa\n");
      }while(strlen(aux)>9);
    write(fd,aux,strlen(aux));
    printf("Password mudada\n");
    goto menuUser;
  }
  else if(strcmp(choice,"7")==0){
    write(fd,choice,strlen(choice));
    read(fd,&aux2,sizeof(int));
    if(aux2==0){
      printf("Privilegios de operador concedidos\n");
      goto menuOperator;
    }
    else{
      printf("Ja existe um operador\n");
      goto menuUser;
    }
  }
  else{
    printf("Nao existe esta opcao tente de novo\n");
    goto menuUser;
  }



menuOperator:
  printf("Menu\n 1-Listar mensagens nao lidas\n 2-Listar Utilizadores\n 3-Enviar mensagem\n 4-Listar mensagens lidas\n 5-Apagar mensagen\n 6-Alterar Password\n 8-Deixar de ser operador\n 9-Listar todas a mensagens nao lidas de todos os users(Nao Altera estado para lido)\n 10-Listar todas mensagens lidas de todos os users\n 11-Apagar mensagem com certo ID\n 12-Mostrar todos os utilizadores\n 13-Apagar utilizador\n 14-Criar Utilizador\n 15-Mudar permissoes de um utilizador\n 99-Quit\n");
  printf("Opcao:");
  scanf("%s",choice);
  if(strcmp(choice,"99")==0){
    goto fim;
  }
  else if(strcmp(choice,"1")==0){
    printf("Listando todas as mensagens\n");
    write(fd,choice,strlen(choice));
    read(fd,&aux2,sizeof(int));
    printf("Numero de mensagens%d\n",aux2);
    for(i=0;i<aux2;i++){
      read(fd,buffer,sizeof(buffer));
      printf("Mensagem de:%s\n",buffer);
      read(fd,&aux3,sizeof(int));
      printf("Id da mensagem:%d\n",aux3);
      read(fd,buffer,sizeof(buffer));
      printf("Assunto da mensagem:%s\n",buffer);
      read(fd,buffer,sizeof(buffer));
      printf("Conteudo da mensagem:%s\n",buffer);
      printf("*********************************************************************************\n");
    }
    goto menuOperator;
  }
  else if(strcmp(choice,"2")==0){
   write(fd,choice,strlen(choice));
   read(fd,buffer,sizeof(buffer));
   printf("%s\n",buffer);
    goto menuOperator;
  }
  else if(strcmp(choice,"3")==0){
    aux3=0;
    write(fd,choice,strlen(choice));
    do{
        printf("Insira a quantas pessoas prentede enviar:\n");
        scanf("%s",choice);
        if(atoi(choice)>64 || atoi(choice)==0)
          printf("Tem que ser pelo menos para 1 ou para menos de 64\n");
    }while(atoi(choice)==0 || atoi(choice)>64);
    write(fd,choice,strlen(choice));
    for(i=0;i<atoi(choice);i++){
      do{
        printf("Insira o nome do destinatario:\n");
        scanf("%s",user);
        if(strlen(user)>20 || strlen(user)==0)
          printf("O destinatario e demasiado curto ou longo\n");
      }while(strlen(user)>20 || strlen(user)==0);
      write(fd,user,strlen(user));
      read(fd,&aux2,sizeof(int));
      if(aux2==0)
        printf("Utilizador nao encontrado ou bloqueado\n");
      else{
        printf("Utilizador encontrado e nao bloqueado\n");
        aux3++;
      }
    }
    if(aux3==0)
      goto menuOperator;
    printf("Insira o Assunto da mensagem com max de 1024 caracteres\n");
    scanf("%s",buffer);//TROCAR SCANF PARA PODER LER ESPACOS
    write(fd,buffer,strlen(buffer));
    printf("Insira a mensagem com max de 1024 caracteres\n");
    scanf("%s",buffer);//TROCAR SCANF PARA PODER LER ESPACOS

    write(fd,buffer,strlen(buffer));
    printf("Mensagem enviada\n");
    goto menuOperator;
  }
  else if(strcmp(choice,"5")==0){
    write(fd,choice,strlen(choice));
    do{
        printf("Insira o id da mensagem:\n");
        scanf("%s",choice);
        if(strlen(choice)>3)
          printf("ID entre 0 a 999\n");
    }while(strlen(choice)>3);
    write(fd,choice,strlen(choice));
    read(fd,&tryagain,sizeof(int));
    if(tryagain!=-1)
      printf("Mensagem apagada\n");
    else
      printf("Nao existe mensagem sua com este Id tente de novo\n");
    goto menuOperator;
  }
  else if(strcmp(choice,"4")==0){
    printf("Listando todas as mensagens ja lidas\n");
    write(fd,choice,strlen(choice));
    read(fd,&aux2,sizeof(int));
    printf("Numero de mensagens ja lidas:%d\n",aux2);
    for(i=0;i<aux2;i++){
      read(fd,buffer,sizeof(buffer));
      printf("Mensagem de:%s\n",buffer);
      read(fd,&aux3,sizeof(int));
      printf("Id da mensagem:%d\n",aux3);
      read(fd,buffer,sizeof(buffer));
      printf("Assunto da mensagem:%s\n",buffer);
      read(fd,buffer,sizeof(buffer));
      printf("Conteudo da mensagem:%s\n",buffer);
      printf("*********************************************************************************\n");
    }
    goto menuOperator;
  }
  else if(strcmp(choice,"6")==0){
    write(fd,choice,strlen(choice));
      do{
        printf("Insira nova password:\n");
        scanf("%s",aux);
        if(strlen(aux)>9)
          printf("Password nao valida, demasiado longa\n");
      }while(strlen(aux)>9);
    write(fd,aux,strlen(aux));
    printf("Password mudada\n");
    goto menuOperator;
  }
  else if(strcmp(choice,"8")==0){
    write(fd,choice,strlen(choice));
    read(fd,&aux2,sizeof(int));
    if(aux2==0){
      printf("Voce deixou de ser operador\n");
      goto menuUser;
    }
    else{
      printf("Ocorreu um erro\n");
      goto menuOperator;
    }
  }
  else if(strcmp(choice,"9")==0){
    printf("Listando todas as mensagens nao lidas de todos os users\n");
    write(fd,choice,strlen(choice));
    read(fd,&aux2,sizeof(int));
    printf("Numero de mensagens%d\n",aux2);
    for(i=0;i<aux2;i++){
      read(fd,buffer,sizeof(buffer));
      printf("Mensagem de:%s\n",buffer);
      read(fd,buffer,sizeof(buffer));
      printf("Para:%s\n",buffer);
      read(fd,&aux3,sizeof(int));
      printf("Id da mensagem:%d\n",aux3);
      read(fd,buffer,sizeof(buffer));
      printf("Assunto da mensagem:%s\n",buffer);
      read(fd,buffer,sizeof(buffer));
      printf("Conteudo da mensagem:%s\n",buffer);
      printf("*********************************************************************************\n");
    }
    goto menuOperator;
  }
  else if(strcmp(choice,"10")==0){
    printf("Listando todas as mensagens ja lidas de todos os users\n");
    write(fd,choice,strlen(choice));
    read(fd,&aux2,sizeof(int));
    printf("Numero de mensagens%d\n",aux2);
    for(i=0;i<aux2;i++){
      read(fd,buffer,sizeof(buffer));
      printf("Mensagem de:%s\n",buffer);
      read(fd,buffer,sizeof(buffer));
      printf("Para:%s\n",buffer);
      read(fd,&aux3,sizeof(int));
      printf("Id da mensagem:%d\n",aux3);
      read(fd,buffer,sizeof(buffer));
      printf("Assunto da mensagem:%s\n",buffer);
      read(fd,buffer,sizeof(buffer));
      printf("Conteudo da mensagem:%s\n",buffer);
      printf("*********************************************************************************\n");
    }
    goto menuOperator;
  }
  else if(strcmp(choice,"11")==0){
    write(fd,choice,strlen(choice));
    do{
        printf("Insira o id da mensagem:\n");
        scanf("%s",choice);
        if(strlen(choice)>3)
          printf("ID entre 0 a 999\n");
    }while(strlen(choice)>3);
    write(fd,choice,strlen(choice));
    read(fd,&tryagain,sizeof(int));
    if(tryagain!=-1)
      printf("Mensagem apagada\n");
    else
      printf("Nao existe mensagem com este Id tente de novo\n");
    goto menuOperator;
  }

  else if(strcmp(choice,"12")==0){
   write(fd,choice,strlen(choice));
   read(fd,buffer,sizeof(buffer));
   printf("%s\n",buffer);
    goto menuOperator;
  }

  else if(strcmp(choice,"13")==0){
   write(fd,choice,strlen(choice));
   printf("Insira o nome do utilizador\n");
   scanf("%s",buffer);
   write(fd,buffer,strlen(buffer));
   read(fd,&tryagain,sizeof(int));
    if(tryagain!=-1)
      printf("User apagado\n");
    else
      printf("User nao existe\n");
    goto menuOperator;
  }
  else if(strcmp(choice,"14")==0){
    write(fd,choice,strlen(choice));
    do{
        printf("Insira o username:\n");
        scanf("%s",send);
        if(strlen(choice)>30)
          printf("Username demasiado longo\n");
    }while(strlen(choice)>30);
    write(fd,send,strlen(send));
    do{
        printf("Insira a password:\n");
        scanf("%s",send);
        if(strlen(choice)>9)
          printf("Password demasiado longa\n");
    }while(strlen(choice)>9);
    write(fd,send,strlen(send));
    printf("User criado\n");
    goto menuOperator;
  }

  else if(strcmp(choice,"15")==0){
    write(fd,choice,strlen(choice));
    do{
        printf("Insira o username:\n");
        scanf("%s",send);
        if(strlen(choice)>30)
          printf("Username demasiado longo\n");
    }while(strlen(choice)>30);
    write(fd,send,strlen(send));
    read(fd,&tryagain,sizeof(int));
    if(tryagain==1){
      printf("Insira novo estado do utilizador 1-Acesso User, 0-Acesso bloqueado\n");
      scanf("%s",aux);
      write(fd,aux,strlen(aux));
      printf("Estado do utilizador alterado\n");
      goto menuOperator;
    }
    else{
      printf("Utilizador nao existe\n");
      goto menuOperator;
    }
  }

  else{
    printf("Nao existe esta opcao tente de novo\n");
    goto menuOperator;
  }



fim:
    sprintf(choice,"99");
    write(fd,choice,strlen(choice));
    close(fd);
    exit(0);
}

void erro(char *msg)
{
	printf("Erro: %s\n", msg);
	exit(-1);
}
