/**
 *  \author ...
 *  \tester ...
 */

#include "freelists.h"
#include "freelists.bin.h"
#include "datatypes.h"
#include "sbdealer.h"
#include "probing.h"
#include "exception.h"

#include <errno.h>

/*
 * Even if some of them are not functionally necessary,
 * the following dictates must be obyed by the implementation:
 * - if crefs is equal to zero, 
 *      first transfer as much as possible to head cache
 * - 
 */
void soDeplete(void)
{
    soProbe(541, "soDeplete()\n");
    #ifdef __original__
    soDepleteBin();
    #else
    SOSuperBlock* sb;
    sb=sbGetPointer();
    SORefBlock srb;
    uint32_t count =-8;
    uint32_t iB,iBy;
    for(uint32_t i=0; i<sb->icache.idx;i+=1)
    {
    	if(sb->icache.ref[i] == NullReference)
    	{
    		continue;
    	}
    	if((sb->rmstart +(sb->icache.ref[i]/ReferencesPerBitmapBlock))!=count)
    	{
    		count = sb->rmstart + sb->icache.ref[i]/ReferencesPerBitmapBlock;
    		soReadRawBlock(count,&srb);
    	}

    	iBy= sb->icache.ref[i];
    	iBy=iBy%8;
    	iB = (sb->icache.ref[i]/ReferencesPerBitmapBlock)>>3;
    	srb.map[iB]= (0x80>>iBy);

    	if(iB < srb.idx)
    	{
    		srb.idx = iB;
    	}
    	srb.cnt+=1;
    	soWriteRawBlock(sb->rmstart +sb->icache.ref[i]/ReferencesPerBitmapBlock ,&srb);
    	sb->icache.ref[i]= NullReference;

    }
    sb->icache.idx=0;
    sbSave();
    #endif
    }
