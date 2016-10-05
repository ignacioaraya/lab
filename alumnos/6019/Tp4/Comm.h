#ifndef _COMM_H_
#define _COMM_H_

#define ERROR 42
#define LOG   44
#define FORBIDDEN 403
#define NOTFOUND  404
#define INTERNAL  500
#define NOFD 0

void comm(char *message, int type, int socketfd);
void logger(char *message);
void screenlog(char *message);
void filelog(char *message);

#endif
