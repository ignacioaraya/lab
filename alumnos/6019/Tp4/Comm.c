#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include "Comm.h"

void comm(char *message, int type, int socket_fd){

  char out[sizeof(message) + 20] = "";

  switch (type) {
    case ERROR:
    strcpy(out, "ERROR:\n");
    strcat(out, message);
    logger(out);
    break;
    case LOG:
    logger(message);
    break;
    case FORBIDDEN:
    write(socket_fd, "HTTP/1.1 403 Forbidden\nContent-Length: 185\nConnection: close\nContent-Type: text/html\n\n<html><head>\n<title>403 Forbidden</title>\n</head><body>\n<h1>Forbidden</h1>\nThe requested URL, file type or operation is not allowed on this webserver.\n</body></html>\n",271);
    strcpy(out, "FORBIDDEN:\n");
    strcat(out, message);
    logger(out);
    break;
    case NOTFOUND:
    write(socket_fd, "HTTP/1.1 404 Not Found\nContent-Length: 136\nConnection: close\nContent-Type: text/html\n\n<html><head>\n<title>404 Not Found</title>\n</head><body>\n<h1>Not Found</h1>\nThe requested URL was not found on this server.\n</body></html>\n",224);
    strcpy(out, "NOT FOUND:\n");
    strcat(out, message);
    logger(out);
    break;
    case INTERNAL:
    write(socket_fd, "HTTP/1.1 500 Internal Server Error\nContent-Length: 216\nConnection: close\nContent-Type: text/html\n\n<html><head>\n<title>500 Internal Server Error</title>\n</head><body>\n<h1>Internal Server Error</h1>\nThe server encountered an internal error or misconfiguration and was unable to complete your request.\n</body></html>\n",314);
    strcpy(out, "INTERNAL ERROR:\n");
    strcat(out, message);
    logger(out);
  }
  if(type == ERROR || type == NOTFOUND || type == FORBIDDEN || type == INTERNAL)
  exit(3);
}

void logger(char *message){
  if(isatty(1)){    //pruebo si stdout sigue conectado a la terminal y no lo cerré al demonizar el programa
    screenlog(message);
  }else{                 //Si ejecuta como programa normal, logger por pantalla. Si es un daemon, sólo logger por archivo.
    filelog(message);
  }
}

void screenlog(char *message){
  printf("\n%s\n", message);
  filelog(message);
}

void filelog(char *message){
  int fd;
  if((fd = open("WebSrv.log", O_CREAT| O_WRONLY | O_APPEND,0644)) >= 0) {
    write(fd,message,strlen(message));
    write(fd,"\n",1);
    close(fd);
  }
}
