#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "utils.h"
#include "process.h"
#include <sys/types.h>

static int id = 0; // ensures new SHM segment for each shmAlloc

void* shmAlloc(int size)
{
   /* TODO: change this function to your needs */
   char* fullpath = realpath("simulation-process", NULL);
   key_t key = ftok(fullpath, ++id); // same key if same arguments
   check_error(key);
   /* creation of the SHM segment */
   int shm_id = pshmget(key, size, IPC_CREAT | 0660);
   /* mapping of SHM segment to the address space of the caller process */
   void* shm_addr = (void*) pshmat(shm_id, NULL, 0);
   /* ensures deletion of the SHM segment in case of fail/exit */
   pshmctl(shm_id, IPC_RMID, NULL);
   return shm_addr;
}

char* stringClone(char* str)
{
   assert (str != NULL);

   char* res = (char*)shmAlloc(strlen(str)+1);
   strcpy(res, str);

   return res;
}

static int numDigits(int num)
{
   int res = 1;
   int n = num/10;
   while(n != 0)
   {
      res++;
      n /= 10;
   }
   return res;
}

char* intToString(char* res, int num, int len)
{
   int d = numDigits(num);
   if (len > d)
      d = len;
   if (res == NULL)
      res = (char*)memAlloc(d+1);
   sprintf(res, "%*d", d, num);
   return res;
}

char* percentageToString(char* res, int perc)
{
   assert (perc >= 0 && perc <= 100);

   if (res == NULL)
      res = (char*)memAlloc(5);

   sprintf(res, "%3d%%", perc);

   return res;
}

