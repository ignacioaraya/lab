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

void web(int fd, int hit){

  int j, file_fd, buflen;
  long i, ret, len;
  char * fstr;
  static char buffer[BUFSIZE+1]; // static = inicializado vacío

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

    ret =read(fd,buffer,BUFSIZE);   //leer el request completo
    if(ret == 0 || ret == -1) {  //Si tengo un error cierro
      comm("No se pudo leer el request del cliente",FORBIDDEN,fd);
    }

    if(ret > 0 && ret < BUFSIZE)
    buffer[ret]=0;
    else buffer[0]=0;

    for(i=0;i<ret;i++)  // quito \r \n
    if(buffer[i] == '\r' || buffer[i] == '\n')
    buffer[i]='*';

    comm("REQUEST:",LOG,NOFD);
    comm(buffer,LOG,NOFD);

    if( strncmp(buffer,"GET ",4) && strncmp(buffer,"get ",4) ) {
      comm("Request inconrrecto, solo esta soportado GET",FORBIDDEN,fd);
    }

    for(i=4;i<BUFSIZE;i++) {
      if(buffer[i] == ' ') { //buffer = "GET URL [...]"
      buffer[i] = 0;
      break;
    }
  }

  for(j=0;j<i-1;j++)   // pruebo que no intente subir a la carpeta anterior
  if(buffer[j] == '.' && buffer[j+1] == '.') {
    comm("Prohibido solicitar volver a la carpeta anterior. Operacion (..)",FORBIDDEN,fd);
  }

  if( !strncmp(&buffer[0],"GET /\0",6) || !strncmp(&buffer[0],"get /\0",6) ) // si no se solicita ningun archivo, enviar index.html
  strcpy(buffer,"GET /index.html");

  buflen=strlen(buffer);
  fstr = (char *)0;

  for(i=0;extensions[i].ext != 0;i++) {  // comprobar si el archivo solicitado tiene un tipo soportado
    len = strlen(extensions[i].ext);
    if( !strncmp(&buffer[buflen-len], extensions[i].ext, len)) {
      fstr =extensions[i].filetype;
      break;
    }
  }

  if(fstr == 0){
    comm("Tipo de archivo solicitado no soportado",FORBIDDEN,fd);
  }

  if(( file_fd = open(&buffer[5],O_RDONLY)) == -1) {  // abro el archivo solicitado
    comm("Archivo no encontrado o no se pudo abrrir",NOTFOUND,fd);
  }

  comm("RESPONSE: ",LOG,NOFD);
  comm(&buffer[5],LOG,NOFD);
  len = (long)lseek(file_fd, (off_t)0, SEEK_END); // voy al fin del archivo para averiguar el tamaño
  lseek(file_fd, (off_t)0, SEEK_SET); // rewind
  sprintf(buffer,"HTTP/1.1 200 OK\nServer: WebSrv/%d.0\nContent-Length: %ld\nConnection: close\nContent-Type: %s\n\n", 1, len, fstr);
  write(fd,buffer,strlen(buffer));

  while (  (ret = read(file_fd, buffer, BUFSIZE)) > 0 ) { // envio el archivo en partes de 8kb
    (void)write(fd,buffer,ret);
  }

  sleep(1);  // permite que el socket termine antes de cerrarlo
  close(fd);
  exit(1);
}
