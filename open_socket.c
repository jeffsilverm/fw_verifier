/* make_socket.c

This subroutine opens a IPv4 TCP port which is passed as an argument.

*/
#define MAIN    1   // for testing, compile a main program.
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
// #include <sys/socket.h>
#include <netinet/ip.h>


#define handle_error(msg) \
        do { perror(msg); exit(EXIT_FAILURE); } while (0)

int make_socket(int socket_number) {
    int listenfd, connfd;
    socklen_t len;
    struct sockaddr_in servaddr, cliaddr;

    listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenfd == -1)
        handle_error("server-side socket");

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(socket_number);
    servaddr.sin_addr = INADDR_ANY;


    if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1)
        handle_error("server side bind");

    printf("About to block on a listen call, port %d\n", socket_number);
    if (listen(listenfd, LISTEN_QUEUE) == -1)
        handle_error("server side listen");

    len = sizeof(cliaddr);
    connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &len);
    if (connfd == -1)
        handle_error("accept");
    printf("Connected to remote host, port %d remote host %d " ntohs(cliaddr.sin_port), inet_ntop(cliaddr.sin_addr) )

    exit(EXIT_SUCCESS);

}

#if (MAIN)
int main(int argc, char **argv) {

    print("Opening a socket on port 8080.  Use \nss -lpnt4\nto verify that it's open for listening")
    make_socket(8080);


}
#endif
