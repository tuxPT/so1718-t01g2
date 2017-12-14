#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "process.h"

// process

pid_t pfork(void)
{
   pid_t res = fork();
   check_error(res);
   return res;
}

pid_t pwait(int *status)
{
   pid_t res = wait(status);
   check_error(res);
   return res;
}

pid_t pwaitpid(pid_t pid, int *status, int options)
{
   pid_t res = waitpid(pid, status, options);
   check_error(res);
   return res;
}

// System V - shared memory

int pshmget(key_t key, size_t size, int shmflg)
{
   assert(size > 0);

   int res = shmget(key, size, shmflg);
   check_error(res);
   return res;
}

int pshmctl(int shmid, int cmd, struct shmid_ds *buf)
{
   int res = shmctl(shmid, cmd, buf);
   check_error(res);
   return res;
}

void *pshmat(int shmid, const void *shmaddr, int shmflg)
{
   void *res = shmat(shmid, shmaddr, shmflg);
   pcheck_error(res);
   return res;
}

void pshmdt(const void *shmaddr)
{
   int st = shmdt(shmaddr);
   check_error(st);
}

// System V - semaphores

int psemget(key_t key, int nsems, int semflg)
{
   assert(nsems > 0);

   int res = semget(key, nsems, semflg);
   check_error(res);
   return res;
}

int psemctl(int semid, int semnum, int cmd, ...)
{
   va_list vargs;
   va_start(vargs, cmd);
   int res = semctl(semid, semnum, cmd, vargs);
   check_error(res);
   va_end(vargs);
   return res;
}

void psemop(int semid, struct sembuf *sops, size_t nsops)
{
   int st = semop(semid, sops, nsops);
   check_error(st);
}

// POSIX semaphores

// named:
sem_t *psem_open(const char *name, int oflag, ...)
{
   va_list vargs;
   va_start(vargs, oflag);
   sem_t* res = sem_open(name, oflag, vargs);
   psemcheck_error(res);
   va_end(vargs);
   return res;
}

void psem_close(sem_t *sem)
{
   assert (sem != NULL);

   int st = sem_close(sem);
   check_error(st);
}

void psem_unlink(const char *name)
{
   int st = sem_unlink(name);
   check_error(st);
}

// unnamed:

void psem_init(sem_t *sem, int pshared, unsigned int value)
{
   assert (sem != NULL);

   int st = sem_init(sem, pshared, value);
   check_error(st);
}

void psem_destroy(sem_t *sem)
{
   assert (sem != NULL);

   int st = sem_destroy(sem);
   check_error(st);
}

//int sem_getvalue(sem_t *sem, int *sval); // race-condition!

// named and unnamed:


void psem_wait(sem_t *sem)
{
   assert (sem != NULL);

   int st = sem_wait(sem);
   check_error(st);
}

//  return true on success
int psem_trywait(sem_t *sem)
{
   assert (sem != NULL);

   int res = 1; // success

   int st = sem_trywait(sem);
   if (st == -1 && errno == EAGAIN)
      res = 0;
   else
      check_error(st);
   return res;
}

//  return true on success, false if timeout has expired
int psem_timedwait(sem_t *sem, const struct timespec *abs_timeout)
{
   assert (sem != NULL);

   int res = 1; // success

   int st = sem_timedwait(sem, abs_timeout);
   if (st == -1 && errno == ETIMEDOUT)
      res = 0;
   else
      check_error(st);
   return res;
}

void psem_post(sem_t *sem)
{
   assert (sem != NULL);

   int st = sem_post(sem);
   check_error(st);
}

