
#include <stdio.h>
# include "csapp.h"
#include "assert.h"
#include "string.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 16777216 
#define MAX_OBJECT_SIZE 8388608 

/* You won't lose style points for including this long line in your code */


static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";



static const char *requestline_format = "GET %s HTTP/1.0\r\n";
static const char *user_agent_header = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *host_header_format = "Host: %s：%s\r\n";
static const char *connection_header = "Connection: close\r\n";

static const char *proxy_connection_header = "Proxy-Connection: close\r\n";
static const char *blank_line = "\r\n";

void doit(int fd);
void parse_uri(char* uri, char* hostname, char* pathname,char* port);
void build_http_header(char* request_header, char* hostname, 
                  char* pathname, char* port, rio_t* client_rio);
void clienterror(int fd, char *cause, char *errnum, 
             char *shortmsg, char *longmsg);



int main(int argc, char** argv)
{
 
     int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr; 

	 // Check command line args                                
  if (argc != 2) {                                            
     fprintf(stderr, "usage: %s <port>\n", argv[0]);         
     exit(1);                                                
 }                               

   Signal(SIGPIPE, SIG_IGN);
    listenfd = Open_listenfd(argv[1]);
    while (1) {
	clientlen = sizeof(clientaddr);


	connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); //line:netp:tiny:accept

        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, 
                    port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
	doit(connfd);                                             //line:netp:tiny:doit
	Close(connfd);                                           
    }                            
    printf("%s", user_agent_hdr);
    return 0; 

}

//builds the http header

void build_http_header(char* request_header, char* hostname, char* pathname, char* port_name, rio_t* client_rio)
{
    char buf[MAXLINE], host_header[MAXLINE], other_header[MAXLINE];

    strcpy(host_header, "\0");

    sprintf(request_header, requestline_format, pathname);
    
	
    for ( ;Rio_readlineb(client_rio, buf, MAXLINE) >0; ) { // checks if the read line is greater than 0

	 if(strcmp(buf, blank_line)==0) break;

        if(strncasecmp("Host:", buf, strlen("Host:"))){	
		strcpy(host_header, buf);
		continue;
	}
	

        if( !strncasecmp("Connection:", buf, strlen("Connection:")) 
            && !strncasecmp("Proxy-Connection:", buf, strlen("Proxy-Connection:"))
            && !strncasecmp("User-Agent:", buf, strlen("User-Agent:")) )
        {
            strcat(other_header, buf);
        }
    }
    if(strlen(host_header) == 0){ //checks the length of the host_header
        sprintf(host_header, host_header_format, hostname, port_name);
    }
    strcat(request_header, host_header);

    strcat(request_header, user_agent_header);

    strcat(request_header, other_header);

    strcat(request_header, connection_header);

    strcat(request_header, proxy_connection_header);

    strcat(request_header, blank_line);
}




void read_requesthdrs(rio_t *rp) 
{
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
    while(strcmp(buf, "\r\n")) {          //line:netp:readhdrs:checkterm
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
    }
    return;
}

void doit(int fd) 
{
    int serverfd;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char hostname[MAXLINE], path[MAXLINE], port[MAXLINE],  request_header[MAXLINE];
    rio_t rio, server_rio;
    //	int c = 0;	

    /* for( ;(c < -10000);c++){
	if (c = -100)
		c += 10;
        break;
    else{
	break;}
  	}
    */

    Rio_readinitb(&rio, fd); //read request & headers
  
    if (!Rio_readlineb(&rio, buf, MAXLINE))  
        return;

    printf("%s", buf);

    sscanf(buf, "%s %s %s", method, uri, version);       
  
  
    if (strcasecmp(method, "GET")) {                     
  
      clienterror(fd, method, "501", "Not Implemented",
                    "Tiny does not implement this method");
        return;
    }                                                    

                            
    parse_uri(uri, hostname, path, port); //parse the uri

    build_http_header(request_header, hostname, path, port, &rio); // build the http header

    serverfd = Open_clientfd(hostname, port);

    if(serverfd<0){
        printf("Connection faiure\n");
        return;
    }

    Rio_readinitb(&server_rio, serverfd);

    Rio_writen(serverfd, request_header, strlen(request_header));

    size_t n;
    while( (n=Rio_readlineb(&server_rio,buf,MAXLINE))!=0 )
    {
        printf("proxy received %zu bytes and then send\n",n);
        Rio_writen(fd, buf, n);
    }
    Close(serverfd);
} 




void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE], body[MAXBUF];

    // Building the http response 

    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    // Printing  the HTTP response 

    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}

// $end clienterror
void get_filetype(char *filename,char *filetype){
    if (strstr(filename, ".html"))

	strcpy(filetype, "text/html");

    else if (strstr(filename, ".gif"))

	strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
	strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg"))
	strcpy(filetype, "image/jpeg");
    else if (strstr(filename, ".mp4"))
        strcpy(filetype, "video/mp4");
    else if (strstr(filename, ".mp3"))
        strcpy(filetype, "audio/mp3");
    else
	strcpy(filetype, "text/plain");
}  





void parse_uri(char *uri, char *hostname, char *path, char *port){

    char *hostname_ptr;
    char *path_ptr;
    char *port_ptr;
    char host[MAXLINE];

    if(strncasecmp(uri, "http://", strlen("http://")) != 0){
          printf("%s\n","invalid uri");
          exit(0);
    }                        

     hostname_ptr = uri + strlen("http://"); 
                
              
    if((path_ptr = strchr(hostname_ptr, '/')) != NULL){

         strcpy(path, path_ptr);

         strncpy(host, hostname_ptr, (path_ptr - hostname_ptr));

         host[path_ptr - hostname_ptr] = '\0'; 

         }else{

         strcpy(path, "/");
         strcpy(host, hostname_ptr);
            }
               
                                           //get hostname and port
     if((port_ptr = strchr(host, ':')) != NULL){
          strcpy(port, port_ptr + 1);

         strncpy(hostname, host, (port_ptr - host));

          hostname[port_ptr - host] = '\0';
     }else{

         strcpy(hostname, host);
            strcpy(port, "80");   //default port
         }
                

} 
