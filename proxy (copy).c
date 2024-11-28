// TODO!!!! Canned response-------!!!!!_!_!_!_!_!!!______   !!!!  !! ! !??? !? !?!????!! !!!! ! ___!
//          If u get header give theirs. After writing client headers, 
#include "csapp.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define BUF_SIZE 256
void *serve_client(void *args);
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:56.0) Gecko/20100101 Firefox/56.0\r\n";
struct parts_of_first_line {
    char *host;  char *port;
};
struct thread_args {
    int clientfd;
};
int parseFirstLine(char *firstLine, struct parts_of_first_line *hostAndPort);


int main(int argc, char *argv[])
{
    char *portToListen = argv[1];
    int lfd = open_listenfd(portToListen);
    if (lfd == -1) {
        printf("bad port: %s\n", portToListen);
        return -1;
    }

    for (;;) {
        // used during parsing
        
        int clientfd = Accept(lfd, NULL, NULL);
        if (clientfd < 0) {
            app_error("-->int clientfd = Accept(lfd, NULL, NULL);");
            break;
        }
        pthread_t thread;
        struct thread_args *arg = malloc(sizeof(*arg));
        arg->clientfd = clientfd;
        int rv = pthread_create(
            &thread,
            NULL,
            serve_client,
            arg);
        if (rv < 0) {
            perror("pthread_create");
            continue;
        }
        
    }
    close(lfd);
    return 0;
}

void *serve_client(void *args)
{
    int clientfd = (int) 
            ((struct thread_args*) args)->clientfd;
    free(args);
    struct parts_of_first_line h_p;
    h_p.host = NULL; h_p.port = NULL; //to know if they need freeing
    char buf[BUF_SIZE];
    rio_t rioFD;
    rio_readinitb(&rioFD, clientfd);
    int bytesRead;

    while ((bytesRead = rio_readlineb(&rioFD, buf, BUF_SIZE)) != 0) {
        if (bytesRead <  0) {
            unix_error("Read error");
            goto closed; //???error
        }

        // copy parsed line into buf (also the arg containing original line)
        // and set h_p.host and h_p.port
        if (0 != parseFirstLine(buf, &h_p)) {
            app_error("Parse error");
            goto try_again;
        }
        int con_fd = open_clientfd(h_p.host, h_p.port);
        if (con_fd < 0) {
            unix_error("connecting to server failed");
            goto try_again; //???error
        }

       /*---write first line---*/
        // buf is the parsed line
        int bytWr;
        bytWr = rio_writen(con_fd, buf, strlen(buf));
        if (bytWr != strlen(buf)) {
            unix_error("first line write to server failed");
            goto closed; //???error
        }

       /*---write own headers---*/
        // char hostHeader[strlen(buf)+6 + 1]; ???
        char *hostHeader = buf;
        strcpy(hostHeader, "Host: ");
        strcat(strcat(strcat(strcat(hostHeader, 
            h_p.host), ":"),h_p.port), "\r\n");
        // // hold off writing host header until checking if client provided one
        // int clientProvidedAHostHeader = 0;
        bytWr = rio_writen(con_fd, hostHeader, strlen(hostHeader));
        if (bytWr != strlen(buf)) {
            unix_error("first line write to server failed");
            goto closed;; //???error
        }
        strcpy(buf, "Connection: close\r\n");
        bytWr = rio_writen(con_fd, buf, strlen(buf));
        if (bytWr != strlen(buf)) {
            unix_error("own header write to server failed");
            goto closed;; //???error
        }
        strcpy(buf, "Proxy-Connection: close\r\n");
        bytWr = rio_writen(con_fd, buf, strlen(buf));
        if (bytWr != strlen(buf)) {
            unix_error("own header write to server failed");
            goto closed;; //???error
        }
        strcpy(buf, user_agent_hdr);
        bytWr = rio_writen(con_fd, buf, strlen(buf));
        if (bytWr != strlen(buf)) {
            unix_error("own header write to server failed");
            goto try_again; //???error
        }

       /*---write the client's headers---*/
        while ((bytesRead = rio_readlineb(&rioFD, buf, BUF_SIZE)) != 0) {
            /***/if (0 == strncmp(buf, "Host", 5))
                continue;
                // clientProvidedAHostHeader = 1;
            else if (0 == strncmp(buf, "Proxy-Connection", 16))
                continue;
            else if (0 == strncmp(buf, "Connection", 10))
                continue;
            else if (0 == strncmp(buf, "User-Agent", 10))
                continue;
            bytWr = rio_writen(con_fd, buf, strlen(buf));
            if (bytWr != strlen(buf)) {
                unix_error("clients headers failed to write");
                continue;//???error
            }
            if (0 == strcmp(buf, "\r\n")) break;
        }
        if (bytesRead == 0) {
            app_error("Closed connection before finishing request");
            goto closed;
        }

       /*---forward server response---*/
        rio_t rio_fd_server;
        rio_readinitb(&rio_fd_server, con_fd);
        /*headers*/
        while ((bytesRead = rio_readlineb(&rio_fd_server, buf, BUF_SIZE)) != 0) {
            if (bytesRead != rio_writen(clientfd, buf, bytesRead)) {
                unix_error("Didn't write everything back to client");
                goto closed;
            }

            if (0 == strcmp("\r\n", buf))
                break;
        }
        /*body*/
        while ((bytesRead = rio_readnb(&rio_fd_server, buf, BUF_SIZE)) != 0) {
            if (bytesRead != rio_writen(clientfd, buf, bytesRead)) {
                unix_error("Didn't write everything back to client");
                goto closed;
            }
        }

        continue;
        try_again:
        // malloc-ed during parsing of first line
        if (h_p.host != NULL) {
            free(h_p.host);   free(h_p.port); 
            h_p.host = NULL;  h_p.port = NULL;
            //  set null ^ to know if they ^ need freeing
        }
        continue;
        closed:
        if (h_p.host != NULL) {
            free(h_p.host);   free(h_p.port);
        }
        break;
    }

    close(clientfd);
    return NULL;
}


/**
 * returns -1 on error, else 0
 * first line of client's request to proxy is in firstLine.
 * firstLine will also contain the first line to write.
 * Sets host and port accordingly
 */
int parseFirstLine(char *firstLine, struct parts_of_first_line *hostAndPort)
{
    char *port = malloc(sizeof(char) * 32);
    strcpy(port, "80"); // default

    char buffer[BUF_SIZE];

   /*---copy method---*/
    int i = 0;
    for(; firstLine[i] != ' '; i++){
        if (i > 10) {
            app_error("method too long, reaction not implemented yet");
            return -1; //???error
        }
        buffer[i] = firstLine[i];
    }
    buffer[i] = '\0';
    char method[strlen(buffer) + 1];
    strcpy(method, buffer); //check for errors????

   /*---copy host---*/
    if (strchr(firstLine, ':') == NULL) {
        app_error("has no ':' i.e. no http(s)://");
        return -1; //error
    }
    char *hostAndOn = strchr(firstLine, ':') + (3*sizeof(char)); // skip to host chars
    for(i = 0; 1; i++){
        // the ^ up there is because the condition is big and messy.
        // Conceptually substitute it with this if statement:
        if (/*not a letter*/  ((hostAndOn[i] > 122) || (hostAndOn[i] < 97))
            /*or a '.'    */  && hostAndOn[i] != '.')
            break;
        buffer[i] = hostAndOn[i];
    }
    buffer[i] = '\0';
    char *host = malloc((strlen(buffer) + 1) * sizeof(char));
    strcpy(host, buffer);

   /*---update port if given---*/
    if (hostAndOn[i] == ':') {
        // then set port, done manually to 
        // avoid scanf inefficiently converting
        // to int then back to string
        int bufI = 0;
        i++; // skip ':' to get to number
        for (;;) {
            // if not a digit
            if ((hostAndOn[i] < 48) || (57 < hostAndOn[i])) {
                if (bufI == 0) {
                    app_error("no number following : for port");
                    return -1; //error
                }
                break;
            } else {
                buffer[bufI] = hostAndOn[i];
            }
            i++;
            bufI++;
        }
        buffer[bufI] = '\0';
        strcpy(port, buffer);
        // ???if not a number OR a space
    }


   /*---copy path---*/
    int bufI = 0;
    if (hostAndOn[i] == ' ') {
        buffer[0] = '/';
        bufI++;
    } else if (hostAndOn[i] == '/') { 
        while (hostAndOn[i] != ' ') {
            if (hostAndOn[i] == '\n') return -1; //error
            buffer[bufI] = hostAndOn[i];
            i++;
            bufI++;
        }
    } else {
        app_error(hostAndOn);
        app_error("random unmeaningful symbol after the host that has no port");
    }
    buffer[bufI] = '\0';
    char path[strlen(buffer) + 1];
    buffer[bufI] = '\0';
    path[0] = '\0';
    strcpy(path, buffer);

   /*---construct first line to write---*/
    buffer[0] = '\0';
    strcat(strcat(strcat(strcat(strcat(buffer,
        method),
        " "), 
        path), 
        " "), 
        "HTTP/1.0\r\n");
    
    strcpy(firstLine, buffer);
    hostAndPort->host = host;
    hostAndPort->port = port;
    return 0;
}