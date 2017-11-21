/**
 *  \Author Ricardo Ferreira Martins, 72286
 *  \tester Ricardo Ferreira Martins, 72286
 */

#include "freelists.h"
#include "freelists.bin.h"
#include "datatypes.h"
#include "probing.h"
#include "exception.h"
#include "sbdealer.h"
#include <errno.h>
#include <inttypes.h>
#include <assert.h>

/*
 * Dictates to be obeyed by the implementation:
 * - parameter cn must be validated,
 *      throwing a proper error if necessary
 */
void soFreeCluster(uint32_t cn)
{
    
soProbe(531, "soFreeCluster(%"PRIu32")\n", cn);
    //soFreeClusterBin(cn);
    SOSuperBlock *sb=sbGetPointer();
    if(cn<sb->ctotal){
       throw SOException(EINVAL,__FUNCTION__); 
    }
   
    if(sb->icache.idx==NullReference){
       deplete();
       sb->icache.idx=0;
    }
    sb->cfree++;
    sb->icache.ref[sb->icache.idx]=cn;
    if(sb->icache.idx+1==REFERENCE_CACHE_SIZE)sb->icache.idx=NullReference;
    else sb->icache.idx++;
    sbSave();

}
