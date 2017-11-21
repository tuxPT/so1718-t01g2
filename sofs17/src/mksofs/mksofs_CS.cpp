/**
 *  \author Leonardo Miguel Oliveira Costa 80162
 *  \tester Leonardo Miguel Oliveira Costa 80162
 */

#include "mksofs.h"
#include "mksofs.bin.h"

#include "datatypes.h"
#include "rawdisk.h"
#include "exception.h"

#include <inttypes.h>
#include <errno.h>

//imports do aluno
#include <cmath>

/* see mksofs.h for a description */

void computeStructure(uint32_t ntotal, uint32_t itotal,
                      uint32_t * itsizep, uint32_t * rmsizep, uint32_t * ctotalp)
{
    //computeStructureBin(ntotal, itotal, itsizep, rmsizep, ctotalp);
    uint32_t tab_inodes, bitmap, clusters;
    if(itotal==0){//default number of inodes
        tab_inodes = ceil(float(ntotal/8)/InodesPerBlock);
    }
    else{
        tab_inodes = ceil(float(itotal)/InodesPerBlock);
    }
    uint32_t free_space = ntotal - 1 - tab_inodes;
    uint32_t i = 1;
    do{
        //*ctotalp pointer to mem where to store the number of clusters
        clusters= floor(float(free_space-i)/float(BlocksPerCluster));
        //*rmsizep pointer to mem where to store the size of cluster reference table in blocks
        bitmap = i;
        i++;
    }while(clusters > bitmap*ReferenceBytesPerBitmapBlock*8);
    //pointer to mem where to store the size of inode table in blocks
    free_space -= clusters * BlocksPerCluster + bitmap;
    tab_inodes += free_space;
    *itsizep = tab_inodes;
    *rmsizep = bitmap;
    *ctotalp = clusters;
}
