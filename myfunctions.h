#ifndef _myfunctions
#define _myfunctions

#include "unp.h"

#define	CLIENT_PORT 9999
#define NCHILDREN 10

static pid_t *pids;

/* static char userPasswd[50];    */
/* static long dataflow; */

void sig_term(int );
pid_t child_make(int ,int *);
void do_Proxy(int ,int *);
int getDomainName(char *, char *, char *);
int checkDomainName(char *);
int checkRequest(int, char *, char *,char *, int *);
void rv_sd_page(int, char *, int *);   /* 接收或发送验证页面 */
void analyseUrl(char *, char *, char *);

#endif
