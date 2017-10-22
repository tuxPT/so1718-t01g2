/**
 *  \author ...
 *  \tester ...
 */

#include "freelists.h"
#include "datatypes.h"
#include "freelists.bin.h"

#include "probing.h"
#include "exception.h"
#include "sbdealer.h"

#include <errno.h>

/*
 * Dictates to be obeyed by the implementation:
 * - error ENOSPC should be thrown if there is no free clusters
 * - after the reference is removed,
 *      its location should be filled with NullReference
 */
uint32_t soAllocCluster()
{
  bool original =true;

    if(original)
    {
        soProbe(501, "soAllocCluster()\n");
    return soAllocClusterBin();
    }
    else
    {

    soOpenSBDealer();
    SOSuperBlock *sb=sbGetPointer();
    uint32_t clusterNum;

    if(sb->cfree==0) throw SOException(ENOSPC,"No free clusters");
    if(sb->rcache.idx>=REFERENCE_CACHE_SIZE) soReplenish();


    clusterNum=sb->rcache.ref[sb->rcache.idx];
    sb->rcache.ref[sb->rcache.idx]=NullReference;
    sb->rcache.idx++;
    sb->cfree=(uint32_t)(sb->cfree-1);
    sbSave();
    return clusterNum;
  }
}
