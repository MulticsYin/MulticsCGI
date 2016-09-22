/*
 * fork()调用后，父进程打开的文件描述符在子进程中依然保持打开。
 * 传递一个文件描述符并不是传递一个文件描述符的值，而是在接收进程中创建一个
 * 新的文件描述符，并且该文件描述符和发送进程中被传递的文件描述符指向内核中
 * 相同的表项
 * 该文件利用UNIX域socket在进程间传递特殊的辅助数据，以实现文件描述符的传递
 * 先在子进程中打开一个文件描述符，然后将它传递给父进程，父进程通过读取该文
 * 件描述符来获得文件的内容
 * */
#include <sys/socket.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

static const int CONTROL_LEN = CMSG_LEN( sizeof(int) );
/*
 * 发送文件描述符。
 * fd参数是用来传递信息的cocket，fd_to_send参数是待发送的文件描述符。
 * */
void send_fd( int fd, int fd_to_send )
{
    struct iovec iov[1];
    struct msghdr msg;
    char buf[0];

    iov[0].iov_base = buf;
    iov[0].iov_len = 1;
    msg.msg_name    = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov     = iov;
    msg.msg_iovlen = 1;

    cmsghdr cm;
    cm.cmsg_len = CONTROL_LEN;
    cm.cmsg_level = SOL_SOCKET;
    cm.cmsg_type = SCM_RIGHTS;
    *(int *)CMSG_DATA( &cm ) = fd_to_send;
    msg.msg_control = &cm;      //设置辅助数据
    msg.msg_controllen = CONTROL_LEN;

    sendmsg( fd, &msg, 0 );
}
//接收文件描述符
int recv_fd( int fd )
{
    struct iovec iov[1];
    struct msghdr msg;
    char buf[0];

    iov[0].iov_base = buf;
    iov[0].iov_len = 1;
    msg.msg_name    = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov     = iov;
    msg.msg_iovlen = 1;

    cmsghdr cm;
    msg.msg_control = &cm;
    msg.msg_controllen = CONTROL_LEN;

    recvmsg( fd, &msg, 0 );

    int fd_to_read = *(int *)CMSG_DATA( &cm );
    return fd_to_read;
}

int main()
{
    int pipefd[2];
    int fd_to_pass = 0;
    //创建父子进程间的管道，文件描述符pipefd[0]和pipefd[1]都是UNIX域socket。
    int ret = socketpair( PF_UNIX, SOCK_DGRAM, 0, pipefd );
    assert( ret != -1 );

    pid_t pid = fork();
    assert( pid >= 0 );

    if ( pid == 0 )
    {
        close( pipefd[0] );
        fd_to_pass = open( "test.txt", O_RDWR, 0666 );
        /*
         * 子进程通过管道将文件描述符发送到父进程
         * 如果打开文件失败，则子进程将标准输入文件描述符发送到父进程。
         */
        send_fd( pipefd[1], ( fd_to_pass > 0 ) ? fd_to_pass : 0 );
        close( fd_to_pass );
        exit( 0 );
    }

    close( pipefd[1] );
    //父进程从管道接受文件描述符
    fd_to_pass = recv_fd( pipefd[0] );
    char buf[1024];
    memset( buf, '\0', 1024 );
    //读取文件描述符，以验证其有效性
    read( fd_to_pass, buf, 1024 );
    printf( "I got fd %d and data %s\n", fd_to_pass, buf );
    close( fd_to_pass );
}
