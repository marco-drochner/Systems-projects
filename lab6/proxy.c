/*
 * Marco Drochner
 * marcodrochner
 */
#include "csapp.h" /* edited unix_error & app_error to not 'exit()' */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <limits.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define BUF_SIZE 256
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:56.0) Gecko/20100101 Firefox/56.0\r\n";
static char *canned_err = "HTTP/1.0 500 Project Not Done\r\n\r\n";
static const int canned_len = 33;

void *serve_client(void *args);
struct parts_of_first_line {
    char *host;  char *port;
};
struct thread_args {
    int clientfd;
};
int parseFirstLine(char *firstLine, struct parts_of_first_line *hostAndPort);
unsigned float_half(unsigned uf);



int main(int argc, char *argv[])
{
    printf("%d\n", float_half(atoi(argv[1])));

    return 0;
}

unsigned float_half(unsigned uf) {
  uf = 8388608;
  int signedUF = 0 | uf;
  int firstBitOfExp = (1<<23);
  unsigned unsignedRet;
  int expAndRightNeighbor;
  int nanOrInf = 255<<23;
  int exponent = uf & nanOrInf;
  if (exponent == nanOrInf)
    return uf;
  if (!exponent) { //denormalized
    expAndRightNeighbor = nanOrInf | (nanOrInf>>1);
    unsignedRet = 0|(((signedUF>>1) & (~expAndRightNeighbor)) | exponent);
    return unsignedRet;
  } else if(exponent == firstBitOfExp) {
    return (signedUF >> 1) & ~nanOrInf;
  } else {
    return uf - firstBitOfExp;
  }
}
