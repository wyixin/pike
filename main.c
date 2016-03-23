#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/select.h>
#include <signal.h>

#define PORT 8888
#define REQUESTMAX 1024*8
#define PIKE_IO_BUF_LEN 1024

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

    int i, retval;
    char http_status[] = "HTTP/1.0 200 OK\r\n";
    char header[] = "Server: Pike\r\nContent-Type: text/html\r\n\r\n";
    char body[] = "<html><head><body>From server</body></head></html>";
    fd_set readset, writeset;
    
    while(1) {
        FD_ZERO(&readset);
        
        FD_SET(server.fd, &readset);
        for(i=0; i<10; i++) {
            if(server.clients[i] != 0)
                FD_SET(server.clients[i], &readset);
        }
        
        retval = select(server.maxfd+1, &readset, NULL, NULL, NULL);
        printf("retval: %d\n", retval);
        
        if(retval <= 0)
            continue;
        
        if(FD_ISSET(server.fd, &readset)) {
            // handle accept
            int csfd;
            struct sockaddr_in sa;
            unsigned int saLen;
            saLen = sizeof(sa);

            if((csfd  = accept(server.fd, (struct sockaddr *) &sa, &saLen)) == -1) {
                printf("error on accept, sfd is %d, errno is %d \n", server.fd, errno);
                exit(1);
            }

            printf("handle request, client fd is: %d\n", csfd);
            
            setNonBlocking(csfd);
            addToClients(csfd);
        } else {
            // handle client msg
            for(i=0; i<10; i++) {
                int fd = server.clients[i];
                if(fd != 0 && FD_ISSET(fd, &readset)) {
                    char buf[PIKE_IO_BUF_LEN];
                    int conn_status;
    
                    if((conn_status = read(fd, buf, PIKE_IO_BUF_LEN)) == -1) {
                        printf("error on recv, errno is %d \n", errno);
                        exit(1);
                    }
                    printf("here is requese: %s\n", buf);
                    
                    sleep(10);
                    write(fd, "hello \n", 7);
                    //write(fd, body, strlen(body));
                    FD_CLR(fd, &readset);
                    close(fd);
                    server.clients[i] = 0;
                }
            }
        }
    }
}

void stopServer() {
    close(server.fd);
    printf("catch!\n");
    //free(&server);
    exit(0);
}

int main(){

    initServer();

    signal(SIGINT, stopServer);

    processingClientRequest();

    stopServer();
    
    return 0;
}
