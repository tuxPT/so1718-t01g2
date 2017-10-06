#include "mksofs.h"
#include "mksofs.bin.h"

#include "datatypes.h"
#include "rawdisk.h"
#include "exception.h"

#include <inttypes.h>
#include <errno.h>

#include <iostream>
#include <cmath>
using namespace std;

/* see mksofs.h for a description */

void computeStructure(uint32_t ntotal, uint32_t itotal,
            uint32_t * itsizep, uint32_t * rmsizep, uint32_t * ctotalp)
{
    bool original = false;
    if(original){
        computeStructureBin(ntotal, itotal, itsizep, rmsizep, ctotalp);
    }
    else{
        uint32_t tab_inodes, bitmap, clusters;
        if(itotal==0){//default number of inodes
            tab_inodes = ceil(float(ntotal/8)/float(InodesPerBlock));
        }
        else{
            tab_inodes = ceil(float(itotal)/float(InodesPerBlock));
        }
        uint32_t free_space = ntotal - 1 - tab_inodes;
        int i = 1;
        do{
            //*ctotalp pointer to mem where to store the number of clusters
            clusters= floor(float(free_space-i)/float(BlocksPerCluster));
            //*rmsizep pointer to mem where to store the size of cluster reference table in blocks
            bitmap = i;
            i++;
        }while(clusters > bitmap*BlockSize*8);
        //pointer to mem where to store the size of inode table in blocks
        free_space -= clusters * BlocksPerCluster + bitmap;
        cout << free_space;
        tab_inodes += free_space;
        *itsizep = tab_inodes;
        *rmsizep = bitmap;
        *ctotalp = clusters;
    }
}
