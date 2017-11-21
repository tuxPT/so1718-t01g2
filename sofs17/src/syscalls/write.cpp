/*
 * \author Fábio Mendes nº72324
 */

#include "syscalls.h"
#include "syscalls.bin.h"
#include "direntries.h"
#include "direntries.bin.h"
#include "inode.h"
#include "inodeattr.h"
#include "datatypes.h"
#include "fileclusters.h"
#include "czdealer.h"

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <time.h>
#include <utime.h>
#include <libgen.h>
#include <string.h>

#include "probing.h"
#include "exception.h"

/*
 *  \brief Write data into an open regular file.
 *
 *  It tries to emulate <em>write</em> system call.
 *
 *  \param path path to the file
 *  \param buff pointer to the buffer where data to be written is stored
 *  \param count number of bytes to be written
 *  \param pos starting [byte] position in the file data continuum where data is to be written into
 *
 *  \return the number of bytes written, on success;
 *      -errno in case of error,
 *      being errno the system error that better represents the cause of failure
 */
int soWrite(const char *path, void *buff, uint32_t count, int32_t pos)
{
    soProbe(230, "soWrite(\"%s\", %p, %u, %u)\n", path, buff, count, pos);

  //  int ret;

    try
    {
      uint32_t Byperclus=ClusterSize;
      uint32_t clsmxsize=soGetMaxFileSize();

      if(pos+count>clsmxsize)
      {
        throw SOException(EFBIG,__FUNCTION__);
      }
        char name[SOFS17_MAX_NAME + 1]; // MAXIMO que o meu ficheiro permite
        char* name1=&name[0];


        strcpy(name1,path);

        uint32_t inode;
        inode = soTraversePath(name1);
        int ih = iOpen(inode);
        SOInode *in = iGetPointer(ih);
          int x=0; //acesso à posicao onde vai ser escrito o ficheiro

        if(!iCheckAccess(ih,X_OK))
        {
          throw SOException(EACCES,__FUNCTION__);
        }

        else
        {
          in->size = pos+count; //dar o mesmo tamanho

          uint32_t n_pos=pos%Byperclus;
          uint32_t nn_pos=pos/Byperclus;

          char n_tam[count];
         char n_tam2[Byperclus];
          memcpy(n_tam,buff,count);

          soReadFileCluster(ih,nn_pos,&n_tam2);

          int i=0;

          while(i<count)
          {
        //    x=n_pos+i;
            if(n_pos+i<Byperclus)
            {
              n_tam2[n_pos+i]=n_tam[i];
            }
            else
            {
              soWriteFileCluster(ih,nn_pos,&n_tam2);
              nn_pos++;

              soReadFileCluster(ih,nn_pos,&n_tam2);
              n_pos=n_pos- Byperclus;
              n_tam[n_pos+i]=n_tam[i];
            }
            x= i+1;
            i++;
          }

          soWriteFileCluster(ih,nn_pos,n_tam);
        }
        iClose(ih);
        return x;
      }

        /* replace next line with your code */
        //ret = soWriteBin(path, buff, count, pos);
    catch(SOException & err)
    {
        return -err.en;
    }

  //  return ret;
}
