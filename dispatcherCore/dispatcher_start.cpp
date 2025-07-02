#include <iostream>
#include <pthread.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include "../common/comm_type.h"
#include "../common/dmsgOp.h"

#define DISPATCH_RECV_Q_MAX_MSG_SIZE 2048

extern void dispatcherDbDisplay();
extern dmsg_t *dispatcherProcessPublisherMsg(dmsg_t *msg, uint32_t bytes_read);
extern dmsg_t *dispatcherProcessSubscriberMsg(dmsg_t *msg, uint32_t bytes_read);

static void dispatcherReply(int sock_fd, dmsg_t *reply_msg, struct sockaddr_in *client_addr) {
    size_t msg_size_to_send = sizeof(*reply_msg) + reply_msg->tlvBufferSize;
    int rc = sendto(sock_fd, (char*)reply_msg, msg_size_to_send, 0, (struct sockaddr*)client_addr, sizeof(struct sockaddr));
    if(rc < 0) {
        std::cout << "Dispatcher Error: Feedback reply to publisher failed\n";
    }
}

static void *dispatcherRecvMsgListen(void *arg) {
    int ret;
    fd_set readfds;
    int sock_fd, addr_len, opt = 1;
    subMsgType_t sub_msg_code;
    dmsg_t *reply_msg;

    static char buffer[DISPATCH_RECV_Q_MAX_MSG_SIZE];
    struct sockaddr_in server_addr, client_addr;

    if((sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        std::cout << "Dispatcher Error: Listener socket creation failed\n";
        exit(1);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DISPATCHER_UDP_PORT);
    server_addr.sin_addr.s_addr = htonl(DISPATCHER_IP_ADDR);
    addr_len = sizeof(struct sockaddr);

    if(bind(sock_fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) == -1){
        std::cout << "Dispatcher Error: Socket bind failed\n";
        close(sock_fd);
        exit(1);
    }
    std::cout << "Dispatcher: Listening for requests...\n";
    while(1) {
        FD_ZERO(&readfds);
        FD_SET(sock_fd, &readfds);
        select(sock_fd + 1, &readfds, NULL, NULL, NULL);
        ssize_t bytes_read = recvfrom(sock_fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, (socklen_t*)&addr_len);
        buffer[bytes_read] = '\0';
        dmsg_t *msg = (dmsg_t*)buffer;

        switch(msg->msgType) {
            case PUB_TO_DISPATCH:
                if(msg->id.publisherId) {
                    std::cout << "Dispatcher: Received message from publisher ID: " << msg->id.publisherId << std::endl;
                }
                else {
                    std::cout << "Dispatcher: Received message from new Publisher\n";
                }
                dmsgDebugPrint(msg);
                reply_msg = dispatcherProcessPublisherMsg(msg, bytes_read);
                if(reply_msg) { 
                    dispatcherReply(sock_fd, reply_msg, &client_addr);
                    free(reply_msg);
                }
            break;
            case SUB_TO_DISPATCH:
                if(msg->id.subscriberId) {
                    std::cout << "Dispatcher: Received message from subscriber ID: " << msg->id.subscriberId << std::endl;
                }
                else {
                    std::cout << "Dispatcher: Received message from new Subscriber\n";
                }
                dmsgDebugPrint(msg);
                reply_msg = dispatcherProcessSubscriberMsg(msg, bytes_read);
                if(reply_msg) { 
                    dispatcherReply(sock_fd, reply_msg, &client_addr);
                    free(reply_msg);
                }
            break;
            default:;
        }
    }
}

static void dispatcherForkListenerThreads() {
    static pthread_t udp_listener_thread;
    pthread_create(&udp_listener_thread, NULL, dispatcherRecvMsgListen, NULL);
}


void dispatcherMain() {
    dispatcherForkListenerThreads();
    //dispatcherForkDistributionThreads();
}