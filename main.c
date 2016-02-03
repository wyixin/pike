#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define PORT 8888
#define REQUESTMAX 5000

int
main(){
    int sfd, csfd, conn_status;
    struct sockaddr_in my_addr, c_addr;

    char http_status[] = "HTTP/1.0 200 OK\r\n";
    char header[] = "Server: Pike\r\nContent-Type: text/html\r\n\r\n";
    char body[] = "<html><head><body>From server</body></head></html>";
    socklen_t c_addr_len;
    
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons (PORT);
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
    while(1) {
        char *c_b = malloc(sizeof(char) * REQUESTMAX);
        c_addr_len = sizeof(c_addr);
        if((csfd  = accept(sfd, (struct sockaddr *) &c_addr, &c_addr_len)) == -1) {
            printf("error on accept, sfd is %d, errno is %d \n", sfd, errno);
            exit(1);
        }

        if((conn_status = recv(csfd, c_b, sizeof(c_b), 0)) == -1) {
            printf("error on recv, errno is %d \n", errno);
            exit(1);
        }
        printf("%s", c_b);

        send(csfd, http_status, strlen(http_status), MSG_DONTWAIT);
        send(csfd, header, strlen(header), MSG_DONTWAIT);    
        send(csfd, body, strlen(body), MSG_DONTWAIT);
        close(csfd);
        free(c_b);
    }
    close(sfd);
    return 0;
}
