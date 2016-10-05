#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Web.h"
#include "Comm.h"

#ifndef SIGCLD
#define SIGCLD SIGCHLD
#endif

int main(int argc, char **argv){
  
  int i, port, pid, listenfd, socketfd, hit;
  int daemon = -1;
  socklen_t length;
  static struct sockaddr_in cli_addr; // static o sea que ya va inicializado en 0
  static struct sockaddr_in serv_addr; // no hay necesidad de vaciar los structs

  struct {
    char *ext;
    char *filetype;
  } extensions [] = {   //mime.types
    {"txt", "text/plain"},
    {"pdf","application/pdf"},
    {"gif", "image/gif" },
    {"jpg", "image/jpg" },
    {"jpeg","image/jpeg"},
    {"png", "image/png" },
    {"ico", "image/ico" },
    {"zip", "image/zip" },
    {"gz",  "image/gz"  },
    {"tar", "image/tar" },
    {"htm", "text/html" },
    {"html","text/html" },
    {0,0} };

    if( argc < 3 ) {   //espero al menos 2 argumentos <puerto> y <directorio>
      printf("Uso: WebSrv [puerto] [Carpeta Raiz] -d\n\n"
      "\t Solamente servira a los clientes archivos del directorio raiz\n"
      "\t Y siempre que tengan las extensiones especificadas a continuacion\n\n");

      for(i=0;extensions[i].ext != 0;i++)  //recorro el struct de extensiones soportadas y las listo
      printf(" %s",extensions[i].ext);

      printf("\n\t Basado en nweb por Nigel Griffiths\n"  );
      exit(0);
    }

    if( !strncmp(argv[2],"/"   ,2 ) || !strncmp(argv[2],"/etc", 5 ) ||  //Compruebo que no se intente parar en una carpeta restringida
        !strncmp(argv[2],"/bin",5 ) || !strncmp(argv[2],"/lib", 5 ) ||
        !strncmp(argv[2],"/tmp",5 ) || !strncmp(argv[2],"/usr", 5 ) ||
        !strncmp(argv[2],"/dev",5 ) || !strncmp(argv[2],"/sbin",6) ){
      printf("Error, el directorio %s no es valido\n\nPrueba un subdirectorio de /home/\n",argv[2]);
      exit(3);
    }

    if(chdir(argv[2]) == -1){
      printf("Error, no puedo cambiar al directorio %s. \n\n Permission denied??\n",argv[2]);
      exit(4);
    }

    if(argc > 3){   // intentar comparar un argv inexistente no le gusta al sistema operativo!!!
      if(!strcmp(argv[3], "-d")){  //ejecución como daemon o como proceso regular
        daemon = 1;
      }
    }

    if(daemon == 1){
      if(fork() != 0){
        return 0; // Vuelvo a la terminal ok
      }
      comm("Modo DAEMON, el LOG estará disponible en WebSrv.log en la carpeta de ejecución.",LOG,NOFD);
      signal(SIGCLD, SIG_IGN); // ignoro los signals
      signal(SIGHUP, SIG_IGN);
      for(i=0;i<sysconf(_SC_OPEN_MAX);i++){   //sysconf(_SC_OPEN_MAX)  => maxima cantidad de archivos que un proceso puede tener abiertos en un momento
        close(i);    // cierra cualquier archivo (fd) incluyendo stdin, stdout y stderr
      }
      setpgrp();    // Separa del grupo de procesos, se convierte en un daemon
    }else{
      comm("Ejecución normal, el LOG se mostrara por pantalla y se guardará en el archivo WebSrv.log", LOG,NOFD);
    }

    comm("Iniciando WebSrv!!!",LOG,NOFD);

    if((listenfd = socket(AF_INET, SOCK_STREAM,0)) <0)  //Socket
    comm("socket()",ERROR,NOFD);

    port = atoi(argv[1]);  //convierto el string argv[1] (puerto) en int
    if(port < 0 || port >60000)
    comm("Puerto no valido. Probar 0->60000",ERROR,NOFD);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    if(bind(listenfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr)) <0)
    comm("bind()",ERROR,NOFD);

    if( listen(listenfd,64) <0)
    comm("listen()",ERROR,NOFD);

    for(hit=1; ;hit++) {
      length = sizeof(cli_addr);
      if((socketfd = accept(listenfd, (struct sockaddr *)&cli_addr, &length)) < 0)
      comm("accept()",ERROR,NOFD);

      if((pid = fork()) < 0) {                  //En caso de ser un demonio este es el 2do fork, si es en ejecución normal, el 1ro
        comm("fork()",ERROR,NOFD);
      }else {
        if(pid == 0) {   // HIJO
          close(listenfd);
          web(socketfd,hit);
        } else {   // padre
          close(socketfd);
        }
      }
    }
  }
