#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <errno.h>

#define MAX_PAYLOAD 1024
#define NETLINK_TEST 25

int main(int argc, char* argv[])
{
    int state;  // 作为recvmsg()的返回值
    struct sockaddr_nl src_addr, dest_addr;
    struct nlmsghdr *nlh = NULL;
    struct iovec iov;
    struct msghdr msg;
    int sock_fd, retval;  // 作为创建socket和bind打的返回值
    int state_msg = 0;  // 作为send_msg的返回值
    
    // create a socket
    sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_TEST);
    if(sock_fd == -1) {
        printf("error getting socket: %s", strerror(errno));
        return -1;
    }

    // prepare bind，设置源地址信息
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = 100; // 设置源端口号
    src_addr.nl_groups = 0;

    // bind, 将本地套接字和源地址绑定
    retval = bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));
    if(retval < 0) {
        printf("bind failed: %s", strerror(errno));
        close(sock_fd);
        return -1;
    }

    //to prepare create message，初始化msghdr
    nlh = (struct nlmsghdr*)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    if(!nlh) {
        printf("malloc nlmsghdr error!\n");
        close(sock_fd);
        return -1;
    }

    // 设置目的地址信息
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0; //设置目的端口号
    dest_addr.nl_groups = 0;

    // 填充netlink报头，即nlmsghdr结构体信息
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = 100; // 设置源端口
    nlh->nlmsg_flags = 0;
    strcpy(NLMSG_DATA(nlh), "Hello from client!");  // 设置消息体

    iov.iov_base = (void*)nlh;  // 将缓冲区向量iovec与消息进行绑定，指向消息头
    iov.iov_len = NLMSG_SPACE(MAX_PAYLOAD);

    // create message，填充msghdr结构体
    memset(&msg, 0, sizeof(msg));
    msg.msg_name = (void*)&dest_addr;  // msg_name是指向目的地址结构体void*指针
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;  // 这里和toturiol的不太一样因为msg的结构体定义不同了,测试结果失败，没有返回内核的信息
    msg.msg_iovlen = 1;                             // msg.msg_iov = &iov; msg.msg_iovlen = 1;
    // 这里的iov没有

    // send message，调用sendmsg向内核发送消息
    printf("state_msg\n");
    state_msg = sendmsg(sock_fd, &msg, 0);
    if( state_msg == -1) {
        printf("get error sendmsg = %s\n", strerror(errno));
    }
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));

    // receive msg，调用recvmsg接收内核消息
    printf("waiting received!\n");
    state = recvmsg(sock_fd, &msg, 0);
    if(state < 0) {
        printf("state < 1");
    }
    printf("Received message:%s\n", (char*)NLMSG_DATA(nlh));

    close(sock_fd);
    return 0;


}