#include "unp.h"
#include "myfunctions.h"

/* 检查用户是否通过验证，是否在拒绝服务列表中。成功返回0,且将域名或IP地址放在dName中，端口放在port中;失败返回-1 */
int checkRequest(int connfd, char *send_buffer, char *dName, char *port, int *login_fd)
{
    char loginState[2];   /* 登录状态 */
    
    /* 读到了客户的GET请求 */
    printf("%s\n",send_buffer);

    memset(loginState, '\0', sizeof(loginState));
    
    if (getDomainName(send_buffer, dName, port) < 0)   /* 从GET请求中获取域名或IP地址保存在dName中,端口保存在port中 */
	return -1;

    printf("addr: %s\n", dName);
    printf("port: %s\n", port);

    if(strcmp(dName, "127.0.0.1") == 0 ) {
	
	rv_sd_page(connfd, send_buffer, login_fd);

	return -1;
    }
    else {
	printf("read login state start\n");
	Read(login_fd[0], loginState, 1);
	printf("read login state success: %s\n", loginState);
	if(strcmp(loginState,"1") != 0) {     /* 还没有验证 */
	    Write(login_fd[1], "0", 1);
	    return -1;
	}
	Write(login_fd[1], "1", 1);     /* 已经验证 */
    }

    /* 检查是否在自己配置的拒绝服务列表中 */
    if(checkDomainName(dName) < 0) {
    	printf("the domain name is in the refuseList,connection refused!\n");
    	return -1;
    }

    return 0;   /* 成功 */
}

/* 检查Url是否在refuseList列表中，在其中则返回-1,否则返回0 */
int checkDomainName(char *dName)
{
    FILE *f; 
    char reList[1024];   /* 存放读取的拒绝地址 */

    memset(reList, '\0', sizeof(reList));

    if((f = fopen("./config/refuseList","r")) == NULL)
        perror("open refuseList failed!");

    while(fgets(reList,1024,f) != NULL){   
	reList[strlen(reList)-1] = '\0';   /* 去掉最后的换行符 */

	if(strcmp(dName,reList) == 0) {
	    fclose(f);
	    return -1;
	}
    }
    fclose(f);
    return 0;
}
