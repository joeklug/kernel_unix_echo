#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/socket.h>
#include <linux/net.h>
#include <linux/un.h>
#include <net/sock.h>
#include <net/af_unix.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/string.h>

#define PATH "/tmp/blahsock"
#define MIN(a,b) ((a)<(b) ? (a):(b))
#define BUFSZ 0x1000
static struct task_struct *svr_thread;

void handle_message(struct socket *s){
    struct msghdr mhdr;
    struct msghdr mhdr2;
    struct kvec vec;
    struct kvec vec2;
    int len;
    char *buf;
    buf = kmalloc(BUFSZ, GFP_KERNEL);

    vec.iov_base = buf;
    vec.iov_len = BUFSZ;
    memset(&mhdr, 0, sizeof(mhdr));
    memset(&mhdr2, 0, sizeof(mhdr2));

    len = kernel_recvmsg(
        s, 
        &mhdr, 
        &vec,
        1,
        BUFSZ,
        mhdr.msg_flags
        );
    if (len < 0){
        printk(KERN_INFO "ERROR in %s, 0x%08x\n", __func__, len);
        goto CLEAN;
    }
    if (len == 0){
        printk(KERN_INFO "Received empty packet\n");
        goto CLEAN;
    }
    printk(KERN_INFO "Got data: '%s'\n", buf);
    vec2.iov_base = buf;
    vec2.iov_len = len;
    len = kernel_sendmsg(s, &mhdr2, &vec2, 1, len);
    CLEAN:
    kfree(buf);
}

int server_main(void *data){
    struct sockaddr_storage staddr;
    struct sockaddr_un *saddr;
    struct socket *svr;
    struct socket *cli;
    int err;

    saddr = (struct sockaddr_un*)&staddr;
    memset(&staddr, 0, sizeof(staddr));
    err = sock_create(
        AF_UNIX,
        SOCK_STREAM,
        0,
        &svr
    );
    if (err < 0){
        printk(KERN_INFO "Failed to create socket\n");
        return err;
    }
    saddr->sun_family = AF_UNIX;
    strcpy(saddr->sun_path, PATH);
    err = kernel_bind(svr, (struct sockaddr*)saddr, sizeof(*saddr));
    if (err < 0){
        printk(KERN_INFO "Failed to bind");
        goto END;
    }
    kernel_listen(svr, 5);
    while(!kthread_should_stop()){
        err = kernel_accept(svr, &cli, O_NONBLOCK);
        if (err == 0){
            printk(KERN_INFO "Client connected\n");
            handle_message(cli);
            sock_release(cli);
        }
        schedule();
    }
    err = 0;
    END:
    sock_release(svr);
    return err;
}

static int __init echo_init(void){
    int ret = 0;
    char name[] = "svr_thread";
    svr_thread = kthread_create(server_main, NULL, name);
    if(svr_thread){
        wake_up_process(svr_thread);
    }
    return ret;
}

static void __exit echo_exit(void){
    int ret;
    ret = kthread_stop(svr_thread);
    if(!ret){
        printk(KERN_INFO "Stopped the thread\n");
    }
}
module_init(echo_init);
module_exit(echo_exit);
MODULE_LICENSE("GPL");
