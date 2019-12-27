#include <stdio.h>	
#include <stdlib.h>	
#include <string.h>	
#include <unistd.h>	
#include <errno.h>	
#include <netdb.h>	
#include <sys/types.h>	
#include <sys/socket.h>	
#include <netinet/in.h>	
#include <arpa/inet.h>	

typedef struct _gui_msg_t {	
    unsigned int magic;	
    unsigned int op;	
    char param[256];	
} gui_msg_t;	

int main(int argc, char *argv[])	
{	
    char *ipaddr = NULL;	
    unsigned int port = 0;	
    unsigned int op = 0;	
    char *param = NULL;	
    int sock = -1;	
    struct sockaddr_in server_addr;	
    gui_msg_t mymsg = {0};	
    int send_size = 0;	

    if (argc != 5) {	
        printf("Usage: ./udp_server <IP> <port> <op> <param>\n");	
        return 0;	
    }	
    ipaddr = argv[1];	
    port = atoi(argv[2]);	
    op = atoi(argv[3]);	
    param = argv[4];	

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {	
        printf("***Error: create udp socket failed");	
        return -1;	
    }	
    memset(&server_addr, 0x00, sizeof(server_addr));	
    server_addr.sin_family = AF_INET;	
    server_addr.sin_port = htons(port);	
    inet_pton(AF_INET, ipaddr, &server_addr.sin_addr);	

//mymsg.magic = 0xabcdbdac;	
    mymsg.magic = 0x12345678;	
    mymsg.op = op;	
    snprintf(mymsg.param, 256, "%s", param);	
    send_size = sendto(sock, &mymsg, sizeof(mymsg), 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));	
    if (send_size < 0) {	
        printf("***Error: sendto failed");	
    } else {	
        printf("Sent to (%s:%d) %d bytes: 0x%x %d \"%s\"\n", ipaddr, port, send_size, mymsg.magic, mymsg.op, mymsg.param);	
    }	

    close(sock);	

    return 0;	
			}
