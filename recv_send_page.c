#include "unp.h"
#include "myfunctions.h"

/* 登录或注销时，发送或者接收页面数据*/
void rv_sd_page(int connfd, char *send_buffer, int *login_fd)
{
    int n, flags;
    FILE *ht,*fp;
    char loginState[2], *p;
    char Path[MAXLINE],path[MAXLINE],param[MAXLINE],type[4];
    char userPasswd[50];
    char text_content[BUFFSIZE] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";

    memset(loginState, '\0', sizeof(loginState));
    memset(Path, '\0', sizeof(Path));
    memset(type, '\0', sizeof(type));
    memset(userPasswd, '\0', sizeof(userPasswd));
    
    analyseUrl(send_buffer, path, param);

    strcpy(Path, "./config/html");
    strcat(Path, path);     /* 将目录切换到./config/html/下 */

    printf("read login state start\n");
    Read(login_fd[0], loginState, 1);
    printf("read login state success: %s\n", loginState);
    if(strcmp(loginState,"1") != 0) {  /* 没有验证 */

	if(strcmp(Path, "./config/html/") == 0) {

	    ht = fopen("./config/html/login.html","r");
	    Write(login_fd[1], "0", 1);
	}
	else {
	    if(strcmp(Path, "./config/html/loginResult.html") == 0 && strlen(param) != 0) {    /* 登录 */
		if((p = strstr(param, "&repassword")) != NULL) {   /* 是从注册页面跳转过来的 */
		    strncpy(userPasswd, param, strlen(param)-strlen(p));
		    userPasswd[strlen(userPasswd)] = '\n';  /* 在最后加一个换行 */
		    printf("userPasswd:%s",userPasswd);
		    fp = fopen("./config/userList","a+");
		    fwrite(userPasswd, strlen(userPasswd), 1, fp);
		    Write(login_fd[1], "1", 1);    /* 登录成功 */
		    fclose(fp);

		    ht = fopen("./config/html/loginResult.html", "r");
		}
		else {    /* 从登录页面跳转过来 */
		    flags = 0;
		    fp = fopen("./config/userList","r");
		    while(fgets(userPasswd,50,fp) != NULL) {
			userPasswd[strlen(userPasswd)-1] = '\0';
			if(strcmp(userPasswd, param) == 0) {
			    Write(login_fd[1], "1", 1);    /* 登录成功 */
			    flags = 1;
			    fclose(fp);
			    ht = fopen("./config/html/loginResult.html", "r");
			    break;
			}
		    }

		    if( flags == 0) {    /* 登录失败 */
			Write(login_fd[1], "0", 1);
			fclose(fp);
			/* 发送错误页面 */
			ht = fopen("./config/html/loginError.html", "r");
		    }
		}
		    
	    }
	    else {
		
		/* 判断文件是否存在 */
		if((p = strchr(path, '.')) != NULL)
		    strncpy(type, p, strlen(p));   /* 得到文件类型 */
		
		if(access(Path, R_OK) == 0) {  /* 如果文件存在，且可读 */
		    if(strcmp(type, ".html") == 0) {   
			ht = fopen(Path, "r");
		    }

		    if(strcmp(type, ".jpg") == 0) {
			strcpy(text_content, "HTTP/1.1 200 OK\r\nContent-Type: image/jpg\r\n\r\n");
			ht = fopen(Path, "rb");
		    }
		}
		else {   /* 文件不存在，未登录状态返回 */
		    Write(login_fd[1], "0", 1);
		    return;
		}
		Write(login_fd[1], "0", 1);
	    }
	}
    }
    else {        /* 用户已登录 */
	if(strcmp(Path, "./config/html/") == 0 && strcmp(param, "logout=true") == 0) {   /* 注销 */
	    Write(login_fd[1], "0", 1);
	    ht = fopen("./config/html/login.html","r");
	}
	else if (strcmp(Path, "./config/html/") == 0){
	    Write(login_fd[1], "1", 1);
	    ht = fopen("./config/html/loginResult.html", "r");
	}
	else {
	    
	    Write(login_fd[1], "1", 1);

	    if((p = strchr(path, '.')) != NULL)
		    strncpy(type, p, strlen(p));   /* 得到文件类型 */
	    
	    if(access(Path, R_OK) == 0) {  /* 如果文件存在，且可读 */
		if(strcmp(type, ".html") == 0) {   
		    ht = fopen(Path, "r");
		}
		
		if(strcmp(type, ".jpg") == 0) {
		    strcpy(text_content, "HTTP/1.1 200 OK\r\nContent-Type: image/jpg\r\n\r\n");
		    ht = fopen(Path, "rb");
		}
	    }
	}
    }
    
    Write(connfd, text_content, strlen(text_content));   /* 先将报文头发给浏览器 */
    while((n = Read(fileno(ht), text_content, BUFFSIZE)) != 0)
	Write(connfd, text_content, n);
    
    fclose(ht);
    return;
}
