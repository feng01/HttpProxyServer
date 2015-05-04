#include "unp.h"
#include "myfunctions.h"

/* 得到文件路径和参数 */
void analyseUrl(char *send_buffer, char *path, char *param)
{
    char *p;
    char url[MAXLINE],host[MAXLINE];

    memset(path, '\0', sizeof(path));
    memset(param, '\0', sizeof(param));
    
    sscanf(send_buffer, "%*[^/]//%*[^/]%[^ ]", path);

    if((p = strstr(send_buffer, "username=")) != NULL || (p = strstr(send_buffer, "logout=")) != NULL)
        sscanf(p, "%s", param);
    else
        strcpy(param, "");

    printf("path: %s\n", path);
    printf("param: %s\n", param);
}

/* 获取域名和端口号*/
int getDomainName(char *send_buffer, char *dName, char *port)
{
    char host[MAXLINE];
    char cmd[10];

    memset(host, '\0', sizeof(host));
    memset(dName, '\0', sizeof(dName));
    memset(port, '\0', sizeof(port));
    
    sscanf(send_buffer, "%[^ ]", cmd);

    if(strcmp(cmd, "POST") == 0 || strcmp(cmd, "GET") == 0)
    {   
        sscanf(send_buffer, "%*[^/]//%[^/]", host);
    }   
    else if(strcmp(cmd, "CONNECT") == 0)
    {   
        sscanf(send_buffer, "%*[^ ] %[^ ]", host);
    }   
    else   /* 其它请求格式没有写 */
        return -1; 

    if(strchr(host, ':') != NULL) 
    {   
        sscanf(host, "%[^:]", dName);
        sscanf(host, "%*[^:]:%s", port);
    }   
    else
    {   
        strcpy(dName, host);
        strcpy(port, "80");
    }

    return 0;
}
