#Makefile written by Chengfeng


#Which compiler
CC = gcc

#Option for development
CFLAGS = -g

#Local Libraries
MYLIB = libunp.a


HttpProxyServer: main.o analyseurl.o doProxy.o checkrequest.o functions_from_book.o recv_send_page.o $(MYLIB)
	$(CC) -o HttpProxyServer main.o analyseurl.o doProxy.o checkrequest.o functions_from_book.o recv_send_page.o $(MYLIB) -L/usr/lib64 -lpthread

main.o: main.c unp.h config.h functions_from_book.h myfunctions.h

analyseurl.o: analyseurl.c unp.h config.h myfunctions.h

doProxy.o: doProxy.c unp.h config.h myfunctions.h

checkrequest.o: checkrequest.c unp.h config.h myfunctions.h

functions_from_book.o: functions_from_book.c functions_from_book.h unp.h config.h

recv_send_page.o: recv_send_page.c unp.h config.h myfunctions.h
