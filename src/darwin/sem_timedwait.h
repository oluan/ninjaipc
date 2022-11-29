#ifndef SEM_TIMEDWAIT_H
#define SEM_TIMEDWAIT_H

#include <semaphore.h>

int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout);

#endif
