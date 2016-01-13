#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>

#define PORT 8888

int
main(){
    int sfd, csfd;
    struct sockaddr_in my_addr, c_addr;
    char buffer[] = "From server";
    char c_b[1000];
    
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
        c_addr_len = sizeof(c_addr);    
        if((csfd  = accept(sfd, (struct sockaddr *) &c_addr, &c_addr_len)) == -1) {
            printf("error on accept, sfd is %d, errno is %d \n", sfd, errno);
            exit(1);
        }

        //        recv(csfd, c_b, sizeof(c_b), MSG_DONTWAIT);
        //        printf("%s", c_b);
        
        send(csfd, buffer, sizeof(buffer), MSG_WAITALL);
    }
    
    close(sfd);
    close(csfd);
    
    return 0;
}
