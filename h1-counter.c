/*
 * Warren Grugett, Nathan Ueberschaer
 * EECE 446 Introduction to Computer Networks
 * Fall 2026
 */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif

/* This code is based on an updated version of the sample code from "Computer
 * Networks: A Systems Approach," 5th Edition by Larry L. Peterson and Bruce S.
 * Davis. Some code comes from man pages, mostly getaddrinfo(3).
 */
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/*
 * Lookup a host IP address and connect to it using service. Arguments match the
 * first two arguments to getaddrinfo(3).
 *
 * Returns a connected socket descriptor or -1 on error. Caller is responsible
 * for closing the returned socket.
 */
int lookup_and_connect(const char *host, const char *service);
int sendall(int s, char *buf, int len);
int recvall(int s, char *buf, int len);

int main(int argc, char *argv[]) {
  const char *host = "www.ecst.csuchico.edu";
  const char *port = "80";
  char buf[1001] = "GET /~kkredo/file.html HTTP/1.0\r\n\r\n";
  int s;
  int tagCount = 0;
  int byteCount = 0;
  int n;
  int chunkSize;

  if (argc < 2) {
    printf("usage: h1-counter chunk_size\n");
    exit(1);
  }
  chunkSize = atoi(argv[1]);

  /* Lookup IP and connect to server */
  if ((s = lookup_and_connect(host, port)) < 0)
    exit(1);

  if (sendall(s, buf, strlen(buf)) < strlen(buf))
    exit(1);

  while (1) {
    if ((n = recvall(s, buf, chunkSize)) == 0) {
      printf("Number of <h1> tags: %d\n", tagCount);
      printf("Number of bytes: %d\n", byteCount);
      break;
    }
    if (n == -1) {
      printf("Error while receiving data");
      break;
    }

    byteCount += n;
    char *tmp = buf;
    memset(tmp + n, '\0', 1);
    while (*tmp != '\0' && (tmp = strstr(tmp, "<h1>"))) {
      tagCount++;
      tmp++;
    }
  }

  close(s);

  return 0;
}

// Modified from Beej's Guide to Network Programming 
int sendall(int s, char *buf, int len) {
  int total = 0;        // how many bytes we've sent
  int bytesleft = len;  // how many we have left to send
  int n;

  while (total < len) {
    n = send(s, buf + total, bytesleft, 0);
    if (n == -1) {
      break;
    }
    total += n;
    bytesleft -= n;
  }

  return total;
}

// Modified from Beej's Guide to Network Programming
int recvall(int s, char *buf, int len) {
  int total = 0;       // how many bytes we've read
  int bytesleft = len; // how many we have left to read
  int n;

  while (total < len) {
    n = recv(s, buf + total, bytesleft, 0);
    if (n < 1)
      break;
    total += n;
    bytesleft -= n;
  }
  return total;
}

int lookup_and_connect(const char *host, const char *service) {
  struct addrinfo hints;
  struct addrinfo *rp, *result;
  int s;

  /* Translate host name into peer's IP address */
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;

  if ((s = getaddrinfo(host, service, &hints, &result)) != 0) {
    fprintf(stderr, "stream-talk-client: getaddrinfo: %s\n", gai_strerror(s));
    return -1;
  }

  /* Iterate through the address list and try to connect */
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    if ((s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1) {
      continue;
    }

    if (connect(s, rp->ai_addr, rp->ai_addrlen) != -1) {
      break;
    }

    close(s);
  }
  if (rp == NULL) {
    perror("stream-talk-client: connect");
    return -1;
  }
  freeaddrinfo(result);

  return s;
}
