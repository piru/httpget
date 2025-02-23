/*
 *      HTTPGET - Ver. 0.2
 * Get a file using HTTP.
 * Compile with: gcc httpget.c -o httpget
 *           or: cc httpget.c -o httpget
 * Usage: httpget <host> <port> <remote_file> <local_file>
 *      Example: httpget 143.54.10.6 80 /info_logo.gif teste.gif
 * TODO: implement URLs.
 *
 * Rafael Sagula - sagula@inf.ufrgs.br
 *                 http://www.inf.ufrgs.br/~sagula
 *
 * Bibliography: "UNIX Network Programming" - Richard Stevens (rstevens@noao.edu)
 *               "Advanced Programming in UNIX Environment" - Richard Stevens (the same)
 *
 *      Daniel Stenberg - Daniel.Stenberg@sth.frontec.se
 *        Adjusted it slightly to accept named hosts on the command line. We
 *        wouldn't wanna use IP numbers for the rest of our lifes, would we?
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <winsock.h>
#include <io.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#endif
#include <fcntl.h>

#ifndef _WIN32
#include <netdb.h>
#endif

#define bzero(x,y) memset(x,0,y)

#define BUFSIZE 2048

int readline(int fd,char *ptr,int maxlen);
int send_get(int fd, char *file);
void skip_header(int fd);

/* Stolen from Dancer source code: */
#ifndef INADDR_NONE
#define INADDR_NONE -1
#endif
struct hostent *GetHost(char *hostname)
{
  struct hostent *h = NULL;
  unsigned long in;

  if ( (in=inet_addr(hostname)) != INADDR_NONE ) {
    if ( (h=gethostbyaddr((char *)&in, sizeof(in), AF_INET)) == NULL )
      fprintf(stderr, "gethostbyaddr(2) failed for %s", hostname);
  } else if ( (h=gethostbyname(hostname)) == NULL ) {
    fprintf(stderr, "gethostbyname(2) failed for %s", hostname);
  }
  return (h);
}

/*int main()*/
int main(argc,argv)
    int argc;
    char *argv[];
{
  struct hostent *hp=NULL;
  int sockfd, addrlen, clilen, nread, port, fileout;
  struct sockaddr_in *my_addr, serv_addr, cli_addr;
  char buf[BUFSIZE];
  char host_addr[100];
/*  char *argv[5];
  int argc=0;

	argv[1]="143.54.10.6";
	argv[2]="80";
	argv[3]="/info_logo.gif";
	argv[4]="teste.gif";
	argc=5;*/

  if (argc<5) {
    printf("HTTPGET - Rafael Sagula/Daniel Stenberg - 1996\n");
    printf("usage: httpget <host> <port> <remote_file> <local_file>\n");
    exit(-1);
  }
  
//  alarm(60*30); /* 30 minutes */

  fileout = open(argv[4], O_WRONLY|O_CREAT|O_TRUNC, 0644);
	if (fileout>=0) {
		if((hp = GetHost(argv[1]))) {
			sockfd = socket(AF_INET, SOCK_STREAM, 0);
		
			bzero((char *) &serv_addr, sizeof(serv_addr));
			serv_addr.sin_family = hp->h_addrtype;
			memcpy((char *)&(serv_addr.sin_addr), hp->h_addr, hp->h_length);
			serv_addr.sin_port = htons(atoi(argv[2]));
		  
			if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) >= 0) {
				printf("Connected!\n");
			
				send_get(sockfd, argv[3]);
			
				skip_header(sockfd); 
			
				for (;;) {
					nread = read(sockfd, buf, BUFSIZE);
					if (nread)
						write(fileout, buf, nread);
					if (nread < BUFSIZE)
						break;
					printf("#");
					fflush(stdout);
				}
				printf("\nDone.\n");
			} else
				printf("Can't connect to server. \n");
			close(sockfd);
	  } else
			fprintf(stderr, "Couldn't get host, exiting");
		close(fileout);
  } else
		printf("Can't open output file: %s\n",argv[4]);
  return 0;
}


int readline(int fd,char *ptr,int maxlen)
{
  int n, rc;
  char c;
        
  for (n = 1; n < maxlen; n++) {
    if ( (rc = read(fd, &c, 1)) == 1) {
      if (c == '\n')
				break;
      *ptr++ = c;
    } else if (rc == 0) {
			if (n == 1)
				return(0);
			else
				break;
		} else
			return(-1);
	}
	*ptr = 0;
  return(n);
} 

int send_get(int fd, char *file)
{
  char s[256];

  sprintf(s, "GET %s HTTP/1.0\n\n", file);
  return(write(fd, s, strlen(s)));
}

void skip_header(int fd)
{
  char s[256];
  int nr = 3;

  while ((nr>2)) {
    nr=readline(fd, s, 256);
  }
}
