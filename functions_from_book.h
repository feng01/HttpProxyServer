#ifndef _functions_from_book
#define _functions_from_book

#include "unp.h"

void Pthread_mutexattr_init(pthread_mutexattr_t *);
void Pthread_mutexattr_setpshared(pthread_mutexattr_t *, int);
void Pthread_mutex_init(pthread_mutex_t *, pthread_mutexattr_t *);
void Pthread_mutex_lock(pthread_mutex_t *);
void Pthread_mutex_unlock(pthread_mutex_t *);

void my_lock_init();
void my_lock_wait();
void my_lock_release();

#endif 
