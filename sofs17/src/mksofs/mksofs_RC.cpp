//autor: Fábio Mendes nº72324

#include "mksofs.h"
#include "mksofs.bin.h"

#include "datatypes.h"
#include "rawdisk.h"
#include "exception.h"

#include <errno.h>

/* see mksofs.h for a description */
void resetClusters(uint32_t cstart, uint32_t ctotal)
{

  //uint8_t tam[BlockSize];

  //uint8_t *zeros,tmp;
 uint8_t *zeros =(uint8_t*)calloc (512,sizeof(uint8_t));
//  zeros=&tmp;

////  memset(zeros,0,BlockSize);
//for(uint32_t j=0;j<BlockSize;j++)
//{
  //zeros[j]=0;
//}

  for(uint32_t i=cstart*BlocksPerCluster; i<ctotal*BlocksPerCluster;i++)
  {

      soWriteRawBlock(i,zeros);
  }


    //resetClustersBin(cstart, ctotal);
}
