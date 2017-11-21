/**
 *  \author ...Fábio Mendes nº72324
 *  \tester ...
 */

#include "direntries.h"
#include "direntries.bin.h"
#include "inode.h"
#include "inodeattr.h"
#include "datatypes.h"
#include "fileclusters.h"


#include "probing.h"
#include "exception.h"

#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include <stdio.h>

uint32_t PathVerification(uint32_t, char *);

uint32_t soTraversePath(char *path)
{
    soProbe(301, "soTraversePath(%s)\n", path);
    //return soTraversePathBin(path);
    if (strcmp("/",path) == 0)
    {
      return 0;
    }
    else
    {
      char * path2 = strdup(path); //duplicar path
      char * filh = strdup(basename(path2));
      char * pai = dirname(path2);
      uint32_t in1 = soTraversePath(pai); //corta o diretorio


      int ih = iOpen(in1); //chama ao inode handler =>pai
      int ih2 = iOpen(in1);

      SOInode *in = iGetPointer(ih);

      if(!iCheckAccess(ih,X_OK))
      {
        throw SOException(EACCES,__FUNCTION__);

      }
      if(!S_ISDIR(in->mode))
      {
        throw SOException(ENOTDIR,__FUNCTION__);

      }

        uint32_t ant_in1=in1; //vai tomar o mesmo dir do in1

       //Ficar com o inode pois vai ser alterado(o in1 vai para um novo dir) e tenho de o guardar

        in1 = soGetDirEntry(ih,filh);
        if(in1==NullReference)
        {
          throw SOException(ENOENT,__FUNCTION__);
        }

      iClose(ih);
      ih=iOpen(in1);
      in = iGetPointer(ih);


      if(S_ISLNK(in->mode))
      {
        uint32_t lnk=BlockSize*BlocksPerCluster;

      char tt[lnk];
      soReadFileCluster(ih,0,tt);
      in1=PathVerification(ant_in1,tt);
      }

      return in1;
    }


}

uint32_t PathVerification(uint32_t ant_in1, char *lnk)
{
  if (*lnk=='/')
  {
    return soTraversePath(lnk);
  }
  char *t =strtok(lnk,"/");
  uint32_t s_in=ant_in1;
  while(t!=NULL)
  {
    printf("\n%u\n\n%s\n",s_in,t);
    s_in=soGetDirEntry(iOpen(s_in),t);
    t=strtok(NULL,"/");

  }
  return s_in;

}
