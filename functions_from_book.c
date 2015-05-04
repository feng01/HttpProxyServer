#include "functions_from_book.h"
#include <sys/mman.h>

static pthread_mutex_t	*mptr1;	/* actual mutex will be in shared memory */

void
my_lock_init()
{
    int		fd;
    pthread_mutexattr_t	mattr;

    fd = Open("/dev/zero", O_RDWR, 0);

    mptr1 = Mmap(0, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE,
		MAP_SHARED, fd, 0);
    Close(fd);

    Pthread_mutexattr_init(&mattr);
    Pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    Pthread_mutex_init(mptr1, &mattr);
}

void
my_lock_wait()
{
    Pthread_mutex_lock(mptr1);
}

void
my_lock_release()
{
    Pthread_mutex_unlock(mptr1);
}
