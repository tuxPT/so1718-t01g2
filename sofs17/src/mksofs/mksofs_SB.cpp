#include "mksofs.h"
#include "mksofs.bin.h"

#include "datatypes.h"
#include "rawdisk.h"
#include "exception.h"

#include <errno.h>
#include <math.h>

/* see mksofs.h for a description */
void fillInSuperBlock(const char *name, uint32_t ntotal, uint32_t itsize, uint32_t rmsize)
{
  //fillInSuperBlockBin(name, ntotal, itsize, rmsize);
  SOSuperBlock *sb,tmp;
  sb=&tmp;
  sb->magic = 0xffff;
  sb->version = VERSION_NUMBER;
  strcpy(sb->name,name);
  sb->mntstat = 1;
  sb->mntcnt = 0;
  sb->ntotal =ntotal;

  sb->itstart=1;
  sb->itsize= itsize;
  sb->itotal=itsize*InodesPerBlock;
  sb->ifree=sb->itotal-1;
  sb->ihead=1;

  sb->rmstart=1 + sb->itsize;
  sb->rmsize=rmsize;
  sb->rmidx=0;

  sb->czstart=1+itsize+rmsize;
  sb->ctotal=floor((ntotal-itsize-rmsize-1)/BlocksPerCluster);
  sb->cfree=sb->ctotal-1;

  for(uint8_t i=0;i<REFERENCE_CACHE_SIZE;i++)
  {
    sb->rcache.ref[i]=NullReference;
    sb->icache.ref[i]=NullReference;
  }
  sb->rcache.idx=REFERENCE_CACHE_SIZE;
  sb->icache.idx=0;
  soWriteRawBlock(0,sb);
}
