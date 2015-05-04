#include "unp.h"
#include "functions_from_book.h"
#include "myfunctions.h"

int main(int argc, char **argv)
{
    int listenfd, i;
    FILE *fp;
    pid_t selfpid;   /* 父进程用来存放自己的pid */
    const int on = 1;
    int login_fd[2];   /* 用来记录登录状态的管道 */
    struct sockaddr_in servaddr;

    daemon(1, 1);  /* 变为守护进程 */
    
    if(pipe(login_fd) == 0)
	Write(login_fd[1], "0", 1);    /* 初始向管道中写入0,表示用户未登录 */
    else {
	perror("pipe failed");
	return -1;
    }
    
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family  = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    /* Inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr); */
    servaddr.sin_port = htons(CLIENT_PORT);

    Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    Bind(listenfd, (SA *)&servaddr, sizeof(servaddr));
    Listen(listenfd, LISTENQ);

    pids = Calloc( NCHILDREN, sizeof(pid_t));

    fp = fopen("./config/parentPid", "w");
    selfpid = getpid();
    fprintf(fp , "%d", selfpid );    /* 将父进程的pid保存起来 */
    fclose(fp);
    
    my_lock_init();
    
    for (i = 0; i < NCHILDREN; i++)
	pids[i] = child_make(listenfd, login_fd);	/* parent returns */

    Signal(SIGTERM, sig_term);
    
    for ( ; ; )
	pause();	/* everything done by children */
    
    return 0;
}

pid_t child_make(int listenfd, int *login_fd)
{
    pid_t pid;
    int	connfd;
    socklen_t clilen;
    struct sockaddr_in cliaddr;
    
    if ( (pid = Fork()) > 0)
	return(pid);		/* parent */

    /* child execute,never returns */
    printf("child %ld starting\n", (long) getpid());
    for ( ; ; ) {
	clilen = sizeof(cliaddr);

	my_lock_wait();
	connfd = Accept(listenfd, (SA *)&cliaddr, &clilen);
	printf("\nchild %ld accepted!\n", (long)getpid());
	my_lock_release();
	printf("\nchild %ld release!\n", (long)getpid());
	
	do_Proxy(connfd, login_fd);	/* process the request */

	Close(connfd);
    }

    printf("child %ld exit!\n", (long) getpid());
}

/* 信号处理函数 */
void sig_term(int signo)
{
    int i;

    for(i=0; i < NCHILDREN; i++)
	kill(pids[i],SIGTERM);
    
    while( wait(NULL) > 0);

    exit(0);
}
