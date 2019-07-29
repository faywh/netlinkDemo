# netlinkDemo
a simple netlink communication demo between kernel and user space.

kernel_demo.c: create netlink socket and handle received msg from user space.

user_demo.c : create socket and send a msg to kernel space, waiting return msg.

running step:
```shell
# make
# sudo insmod kernel_demo.ko
```

```shell
# gcc -o user_demo user_demo.c
# ./user_demo
state_msg
waiting received!
Received msg: I am from kernel!
```

```shell
#dmesg
netlink_init: create socket ok.
Message received: Hello from client!
pid: 100
my_net_link: send message 'I am from kernel!'
my_net_link: self module exited
```
