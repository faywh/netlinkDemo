#include <linux/init.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/time.h>
#include <linux/types.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/string.h>

#define NETLINK_TEST 25  // 需要在linux/netlink.h里面添加 #define NETLINK_TEST 25
#define MAX_MSGSIZE 1024

struct sock *nl_sk = NULL;

void send_msg(char *message, int pid)
{
    struct sk_buff *skb;
    struct nlmsghdr *nlh;
    int len = NLMSG_SPACE(MAX_MSGSIZE);  //返回不小于NLMSG(len)的且字节对齐的最小数值。

    if(!message || !nl_sk) { return; }
    printk(KERN_INFO "pid: %d\n", pid);
    // 创建skb结构
    skb = alloc_skb(len, GFP_KERNEL);  // 分配大小为len的套接字缓存，标志为GFP_KERNEL，可能会睡眠（GFP_ATOMIC从不睡眠）
                                       // 成功返回skb的指针
    if(!skb) {
        printk(KERN_ERR "send_msg: alloc_skb error\n");
        return;
    }
    // 设置消息头和控制字段
    nlh = nlmsg_put(skb, 0, 0, 0, MAX_MSGSIZE, 0);
    NETLINK_CB(skb).portid = 0;
    NETLINK_CB(skb).dst_group = 0;
    strcpy(NLMSG_DATA(nlh), message);
    printk(KERN_INFO "my_net_link: send message '%s'.\n", (char*)NLMSG_DATA(nlh));
    // 发送数据
    netlink_unicast(nl_sk, skb, pid, MSG_DONTWAIT);
}

void recv_nlmsg(struct sk_buff* skb)
{
    int pid;
    struct nlmsghdr *nlh = nlmsg_hdr(skb);   // 通过lmsg_hdr()函数获取到netlink报头

    if(nlh->nlmsg_len < NLMSG_HDRLEN || skb->len < nlh->nlmsg_len) return;

    printk(KERN_INFO "Message received:%s\n", (char*)NLMSG_DATA(nlh));  // NLMSG_DATA获取netlink的pyload并打印
    pid = nlh->nlmsg_pid;
    send_msg("I am from kernel!", pid);
}

struct netlink_kernel_cfg nl_kernel_cfg = {
    .groups = 0,
    .flags = 0,
    .input = recv_nlmsg,
    .cb_mutex = NULL,
    .bind = NULL,
    .compare = NULL,
};

// 创建netlink socket
int netlink_init(void)
{
    nl_sk = netlink_kernel_create(&init_net, NETLINK_TEST, &nl_kernel_cfg);
    if(!nl_sk) {
        printk(KERN_ERR "my_net_link: create netlink socket error.\n");
        return 1;
    }
    printk(KERN_INFO "netlink_init: create socket ok.\n");
    return 0;
}

static void netlink_exit(void)
{
    if(nl_sk != NULL) {
        sock_release(nl_sk->sk_socket);  // 卸载模块时，调用soxk_release释放套接字
    }
    printk(KERN_INFO "my_net_link: self module exited\n");
}

module_init(netlink_init);
module_exit(netlink_exit);
MODULE_AUTHOR("fay");
MODULE_LICENSE("GPL");