/**
 *  \author ...
 *  \tester ...
 */

#include "freelists.h"
#include "freelists.bin.h"

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

    soOpenSBDealer();
    SOSuperBlock *sb=sbGetPointer();
    assert(cn<sb->ctotal);
    sb->cfree++;
    sb->icache.ref[sb->icache.idx]=cn;
    sb->icache.idx++;
    sbSave();

}
