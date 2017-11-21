/**
 *  \author ...Fábio Mendes nº72324
 *  \tester ...
 */

#include "fileclusters.h"
#include "fileclusters.bin.h"

#include "probing.h"
#include "exception.h"
#include "inode.h"
#include "direntry.h"
#include "rawdisk.h"
#include "superblock.h"
#include "itdealer.h"
#include "datatypes.h"
#include "czdealer.h"



#include <errno.h>
#include <stdint.h>

#if 0


#endif

/* ********************************************************* */
static uint32_t soGetIndirectFileCluster(SOInode * ip, uint32_t fcn);
static uint32_t soGetDoubleIndirectFileCluster(SOInode * ip, uint32_t fcn);


uint32_t soGetFileCluster(int ih, uint32_t fcn)
{
    soProbe(403, "soGetFileCluster(%d, %u)\n", ih, fcn);
  //  return soGetFileClusterBin(ih, fcn);

  SOInode * ip = iGetPointer(ih);

    if(fcn<N_DIRECT)
    {
      return ip->d[fcn];
    }
    else if(fcn>=N_DIRECT && fcn<N_DIRECT + BlockSize)
    {
      return soGetIndirectFileCluster(ip,fcn-N_DIRECT);

    }
    else if(fcn >=N_DIRECT + BlockSize)
    {
      return soGetDoubleIndirectFileCluster(ip,fcn-N_DIRECT-BlockSize);

    }

}
static uint32_t soGetIndirectFileCluster(SOInode * ip, uint32_t fcn)
{
    uint32_t temp[BlockSize];
    if(ip->i1 == NullReference)
    {
      return NullReference;
    }
    else
    {
      soReadCluster(ip->i1,temp);

      return temp[fcn];
    }

}
static uint32_t soGetDoubleIndirectFileCluster(SOInode * ip, uint32_t fcn)
{
uint32_t temp1[BlockSize];
uint32_t temp2[BlockSize];


  if(ip->i2 ==NullReference)
  {
  return NullReference;
  }
  else
  {
  soReadCluster(ip->i2,temp1);
  soReadCluster(temp1[fcn/BlockSize],temp2);
  return temp2[fcn%BlockSize];
  }
}

//#if 0




/* ********************************************************* */

/* only a hint to decompose the solution */
//
//{
  //  soProbe(403, "soGetIndirectFileCluster(%p, %u, %p)\n", ip, afcn, cnp);

    //throw SOException(ENOSYS, __FUNCTION__);
//}

/* ********************************************************* */

/* only a hint to decompose the solution */
//
//{
    //soProbe(403, "soGetDoubleIndirectFileCluster(%p, %u, %p)\n", ip, afcn, cnp);

    //throw SOException(ENOSYS, __FUNCTION__);
//}
