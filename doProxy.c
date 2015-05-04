#include "unp.h"
#include "myfunctions.h"

/* 完成子进程代理的全部功能 */
void do_Proxy(int connfd, int *login_fd)
{
    int servfd;
    struct sockaddr_in servaddr;
    struct hostent *hp;
    char send_buffer[BUFFSIZE],recv_buffer[BUFFSIZE], dName[MAXLINE], dn[MAXLINE],
	buf[BUFFSIZE];   /* 调试用，记得删掉 */
    char port[6], pt[6];
    int maxfd, flags;
    int conn_flags;   /* 浏览器请求是否结束标志 */
    int sendDone;   /* 是否发送过数据给服务器 */
    ssize_t n1,n2;
    fd_set rset,wset;
    char *sdptr, *rviptr, *rvoptr;

    servfd = Socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family  = AF_INET;

    flags = Fcntl(connfd,F_GETFL,0);
    Fcntl(connfd,F_SETFL,flags | O_NONBLOCK);
    flags = Fcntl(servfd,F_GETFL,0);
    Fcntl(servfd,F_SETFL,flags | O_NONBLOCK);
    
    /*initialize buffer pointers and set conn_flags to 0*/
    sdptr=send_buffer;
    rviptr=rvoptr=recv_buffer;
    conn_flags=0;   /* 浏览器请求是否结束，0表示未结束，1表示结束 */
    sendDone = 0;
    
    maxfd = max(servfd, connfd);
    
    for( ; ; ) {
	FD_ZERO(&rset);
	FD_ZERO(&wset);

	if(conn_flags ==0 && sdptr == send_buffer)
	    FD_SET(connfd,&rset);   /* 可以接收浏览器请求 */
	if(rviptr < &recv_buffer[BUFFSIZE])
	    FD_SET(servfd,&rset);   /* 可以接收服务器数据 */
	if(sdptr > send_buffer)
	    FD_SET(servfd,&wset);   /* 可以向服务器发送请求 */
	if(rviptr != rvoptr)
	    FD_SET(connfd,&wset);   /* 可以向浏览器发送数据 */

	Select(maxfd + 1, &rset, &wset, NULL, NULL);
	
	if (FD_ISSET(connfd,&rset) ) {             /*接收浏览器请求 */
	    
	    printf("read from browser start!\n");
	    
	    while ( (n1 = read(connfd, send_buffer, BUFFSIZE)) < 0) {
		if (errno == EINTR) /* 被中断的系统调用，接着读 */
		    continue;
		
		if (errno == EWOULDBLOCK || errno == EAGAIN)   /* 暂时没有数据可读 ,跳出循环，下次再读*/
		    break;   /* 有break改为continue */

		perror("read error from browser");    /* 否则，报错，结束 */
		return;

	    }

	    if (n1 == 0) {
		printf("EOF on browser\n");
  
		conn_flags = 1;
		if(sdptr == send_buffer)
		    FD_CLR(servfd, &wset);
		
	    } else if (n1 > 0){
		printf("read %d bytes from browser\n",n1);
  
		/* move sdptr forward by n1 */
		sdptr = send_buffer + n1;
		FD_SET(servfd, &wset);     
	    }
	}

	if (FD_ISSET(servfd, &wset) && ( (n1=sdptr-send_buffer) > 0)) {    /* 发送请求给服务器 */
    
	    printf("Check Request start!\n");
	    
	    if(checkRequest(connfd, send_buffer, dName, port, login_fd) < 0)
	    	return;    /* 用户还没有通过验证或者或者输入的是验证页面的地址或者请求域名在拒绝服务列表中，结束 */

	    if(strcmp(dName,dn) != 0 || strcmp(port, pt) != 0) {  /* 和上个请求的域名或端口不同，重新建立connect */
		
		servaddr.sin_port = htons(atol(port));

		if(inet_addr(dName) == INADDR_NONE) {
		    
		    printf("gethostbyname started!\n");
		    
		    if((hp = gethostbyname(dName )) == NULL) {
			printf("IP: %s",inet_ntoa(*(struct in_addr *)(hp->h_addr)));
			printf("gethostbyname() failed! %s: %s\n", dName, hstrerror(h_errno));
			return;
		    }
		    
		    printf("gethostbyname success!\n");
		    
		    servaddr.sin_addr.s_addr=*((unsigned long*)hp->h_addr);
		}
		else {
		    servaddr.sin_addr.s_addr= inet_addr(dName);
		}

		printf("connect start!\n");

		if(strlen(pt) != 0 || strlen(dn) != 0) {
		    FD_CLR(servfd, &rset);
		    FD_CLR(servfd, &wset);
		    
		    Close(servfd);

		    servfd = Socket(AF_INET, SOCK_STREAM, 0);
		    flags = Fcntl(servfd,F_GETFL,0);
		    Fcntl(servfd,F_SETFL,flags | O_NONBLOCK);

		    if(servfd > maxfd)
			maxfd = servfd;
		}
		    
		if(connect(servfd, (SA *)&servaddr, sizeof(servaddr)) < 0) {
		    if(errno != EINPROGRESS) {
			perror("connect error");
			return;
		    }
		}
		printf("Connect succeed!\n");

		memset(pt, '\0', sizeof(pt));
		memset(dn, '\0', sizeof(dn));
		strncpy(pt, port, strlen(port));
		strncpy(dn, dName, strlen(dName));
	    }
	    
	    printf("Write to server start!\n");
	    
	    while ( (n2 = write(servfd, send_buffer, n1)) < 0) {
		
		if (errno == EINTR ) /* 被中断的系统调用，接着写 */
		    continue;

		if ( errno == EWOULDBLOCK || errno == EAGAIN)   /* 暂时不可写 ,跳出循环，下次再写*/
		    break;            /* 由continue改为break */

		perror("write error to server");    /* 否则，报错，结束 */
		return;
	    }

	    if(n2 > 0){
		printf("wrote %d bytes to server\n",n2);
		sdptr = send_buffer;
		sendDone = 1;
	    }
	}
	
	if (  FD_ISSET(servfd,&rset) && sendDone == 1) {                 /*接收服务器数据  */

	    printf("read from server start!\n");
	    
	    while((n1 = read(servfd, rviptr, &recv_buffer[BUFFSIZE]-rviptr)) < 0) {
	    	if (errno == EINTR) /* 被中断的系统调用，接着读 */
	    	    continue;
		
	    	if (errno == EWOULDBLOCK || errno == EAGAIN)   /* 暂时没有数据可读 ,跳出循环，下次再读*/
	    	    break;   /* 有break改为continue */

	    	perror("read error from server");    /* 否则，报错，结束 */
	    	return;
	    }
 
	    if (n1== 0) {
	    	printf("EOF on server");
  
	    	if (conn_flags && sdptr == send_buffer) {
	    	    printf("server close normal!\n");
	    	    sendDone = 0;
	    	    return;   /* 浏览器请求结束且发送缓冲区中的数据已经发送，则结束 */
	    	}
	    	else {
	    	    printf("do_Proxy(): server terminated prematurely");
	    	    return;
	    	}
  
	    }
	    else if (n1 > 0){
	    	printf("read %d bytes from server\n",n1);
	    	rviptr += n1;
	    	FD_SET(connfd, &wset);
	    }
	    
	}
  
	if (FD_ISSET(connfd, &wset) && ( (n1 = rviptr-rvoptr) > 0)) {   /* 发送数据给浏览器*/
 
	    printf("Write to browser start!\n");
	    
	    while ( (n2 = write(connfd, rvoptr, n1)) < 0) {
		if (errno == EINTR) /* 被中断的系统调用，接着写 */
		    continue;
		
		if (errno == EWOULDBLOCK || errno == EAGAIN)   /* 暂时数据不可写 ,跳出循环，下次再写*/
		    break;   /* 由break改为continue */

		perror("write error to browser");    /* 否则，报错，结束 */
		return;
	    }
	    
	    if (n2 > 0){
		printf("wrote %d bytes to browser\n",n2);

		strncpy(buf, rvoptr, n1);
		printf("%s\n", buf);
		
		rvoptr += n2;           
		if (rvoptr == rviptr)
		    /* reset pointers */
		    rviptr = rvoptr = recv_buffer;
	    }
	}

	/* printf("\n-----------------------------------------------------------------\n"); */
    }
}
