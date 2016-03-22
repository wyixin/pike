#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/select.h>

#define PORT 8888
#define REQUESTMAX 1024*8

struct pikeServer{
    int port;
    int fd;
    int pid;
    int clients[10];
    int maxfd;
} server;

void setNonBlocking(int fd) {
    fcntl(fd, F_SETFL, O_NONBLOCK);
}

int startTcpServer(int port) {
    int sfd;
    struct sockaddr_in my_addr, c_addr;
    
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons (port);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if((sfd = socket(PF_INET, SOCK_STREAM, 0)) == -1){
        printf("error on create socket");
        exit(1);
    }

    printf("socket ok \n");

    if(bind(sfd, (struct sockaddr *) &my_addr, sizeof(my_addr)) == -1) {
        printf("error on bind");
        exit(1);
    }

    printf("bind ok \n");

    if(listen(sfd, 1)) {
        printf("error on listen");
    }
    
    printf("start to accpet connect from client\n");
    
    return sfd;
}

void initServer(void) {
    int i;
    server.port = PORT;
    server.fd = startTcpServer(server.port);
    setNonBlocking(server.fd);
    for(i=0; i<10; i++) {
        server.clients[i] = 0;
    }
    server.maxfd = server.fd;
}

void addToClients(int fd) {
    int i;
    if(fd > server.maxfd)
        server.maxfd = fd;
    
    for(i=0; i<10 && server.clients[i] != 0; i++)
        ;
    server.clients[i] = fd;
}

void processingClientRequest() {

    int conn_status, csfd, i;
    struct sockaddr_in c_addr;
    char http_status[] = "HTTP/1.0 200 OK\r\n";
    char header[] = "Server: Pike\r\nContent-Type: text/html\r\n\r\n";
    char body[] = "<html><head><body>From server</body></head></html>";
    socklen_t c_addr_len;
    fd_set readset, writeset;
    
    while(1) {
        c_addr_len = sizeof(c_addr);

        FD_ZERO(&readset);
        
        FD_SET(server.fd, &readset);
        for(i=0; i<10; i++) {
            if(server.clients[i] != 0)
                FD_SET(server.clients[i], &readset);
        }
        
        if(select(server.maxfd+1, &readset, NULL, NULL, NULL) < 0) {
            printf("select");
            return;
        }

        if(FD_ISSET(server.fd, &readset)) {
            // handle accept
            if((csfd  = accept(server.fd, (struct sockaddr *) &c_addr, &c_addr_len)) == -1) {
                printf("error on accept, sfd is %d, errno is %d \n", server.fd, errno);
                exit(1);
            }
            setNonBlocking(csfd);
            addToClients(csfd);
        } else {
            // handle client msg
            for(i=0; i<10; i++) {
                if(server.clients[i] != 0 && FD_ISSET(server.clients[i], &readset)) {

                    char *c_b = malloc(sizeof(char) * REQUESTMAX);

                    if((conn_status = recv(server.clients[i], c_b, sizeof(c_b), 0)) == -1) {
                        printf("error on recv, errno is %d \n", errno);
                        exit(1);
                    }
                    printf("%s", c_b);
                    free(c_b);
                    
                    sleep(1);
                    send(server.clients[i], http_status, strlen(http_status), MSG_DONTWAIT);
                    send(server.clients[i], header, strlen(header), MSG_DONTWAIT);    
                    send(server.clients[i], body, strlen(body), MSG_DONTWAIT);
                    FD_CLR(server.clients[i], &readset);
                    close(server.clients[i]);
                    server.clients[i] = 0;
                }
            }
        }
    }
}

void stopServer() {
    free(&server);
}

int main(){

    initServer();

    processingClientRequest();

    stopServer();
    
    return 0;
}
