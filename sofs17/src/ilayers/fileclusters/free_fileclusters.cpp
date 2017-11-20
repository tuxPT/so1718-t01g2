/**
 *  \author Rodrigo Cunha e Silva
 *  \tester Rodrigo Cunha e Silva
 */

#include "fileclusters.h"
#include "fileclusters.bin.h"
#include "freelists.h"

#include "probing.h"
#include "exception.h"
#include "datatypes.h"

#include "itdealer.h"
#include "sbdealer.h"
#include "czdealer.h"

#include <errno.h>
#include <stdint.h>

#include <iostream>

#if 1
static void soFreeIndirectFileClusters(uint32_t i1, SOInode * ip, uint32_t ffcn, int * total);
static void soFreeDoubleIndirectFileClusters(SOInode * ip, uint32_t ffcn, int * total);
#endif

using namespace std;

/* ********************************************************* */
/// Note: I think there's an edge case I couldn't figure out and therefore isn't tested
/// Se alguém tiver erro aqui ^ que avise!
void soFreeFileClusters(int ih, uint32_t ffcn)
{
    soProbe(402, "soFreeFileClusters(%d, %u)\n", ih, ffcn);

    // Get super block pointer
    SOSuperBlock * sbp = sbGetPointer();  

    // Get node pointer
	SOInode * node = iGetPointer(ih);

    // Get total number of cluster occupied
    int total = node->clucnt;

    unsigned int i;

    // printf("DEBUG: ffcn: %d Total : %d\n", ffcn, total);

    // If only .d is occupied
    if (ffcn < N_DIRECT)
    {
        for (i = 1; i <= N_DIRECT; i++)
        {
            if (node->d[i-1] != NullReference)
            {
                // printf("DEBUG: FOR: %d\n", i);
                soFreeCluster(i);
                node->d[i-1] = NullReference;
                total--;
            }
        }

        // Stop if no more clusters to clear
        if (total > 0)
        {
            // Process i1
            soFreeIndirectFileClusters(node->i1, node, 0, &total);

            // Stop if no more clusters to clear
            if (total > 0)
            {
                // Process i2
                soFreeDoubleIndirectFileClusters(node, 0, &total);   
            }
        }
    }
    else if (ffcn > 5 && ffcn < 518)
    {
        // printf("DEBUG (INDIRECT): %d \n", (int)(ffcn - N_DIRECT));
        soFreeIndirectFileClusters(node->i1, node, ffcn - N_DIRECT, &total);
        soFreeDoubleIndirectFileClusters(node, 0, &total);
    }
    else
    {
        // printf("DEBUG (INDIRECT): %d \n", (int)(ffcn - N_DIRECT - ReferencesPerBlock));
        soFreeDoubleIndirectFileClusters(node, ffcn - N_DIRECT - ReferencesPerBlock, &total);
    }

    // Update cluster count
    node->clucnt = total;

    // printf("DEBUG: AFTER TOTAL: %d\n", total);

    iSave(ih);
    iClose(ih);
    sbSave();
}

#if 1
/* ********************************************************* */

/* only a hint to decompose the solution */
static void soFreeIndirectFileClusters(uint32_t i1, SOInode * ip, uint32_t ffcn, int * total)
{
    soProbe(402, "soFreeIndirectFileClusters(%p, %u)\n", ip, ffcn);

    if (i1 == NullReference)
    {
        // printf("DEBUG: I1 IS NULL!!!\n");
        return;
    }

    // Alloc size for references
    uint32_t * refs = (uint32_t*) malloc(ReferencesPerBlock * sizeof(uint32_t));

    // Read cluster i1 (may be i1 or a reference from i2)
    soReadCluster(i1, refs);

    unsigned int i;

    // Iterate cluster references
    for (i = ffcn; i < ReferencesPerBlock; i++)
    {
        if (refs[i] != NullReference)
        {            
            // Decrement cluster count and free cluster
            // printf("DEBUG: INDIRECT: CLUSTER %d cleared!\n", refs[i]);
            soFreeCluster(refs[i]);
            refs[i] = NullReference;
            (*total)--;
            
        }
    }

    free(refs);

    // Free own cluster
    soFreeCluster(i1);
    ip->i1 = NullReference;
    (*total)--;
}

/* ********************************************************* */

/* only a hint to decompose the solution */
static void soFreeDoubleIndirectFileClusters(SOInode * ip, uint32_t ffcn, int * total)
{    
    soProbe(402, "soFreeDoubleIndirectFileClusters(%p, %u)\n", ip, ffcn);
    // printf("DEBUG: DOUBLE arrived\n");

    if (ip->i2 == NullReference)
    {
        return;
    }

    uint32_t * refs = (uint32_t*) malloc(ReferencesPerBlock * sizeof(uint32_t));
    soReadCluster(ip->i2, refs);

    unsigned int i;

    for (i = ffcn; i < ReferencesPerBlock; i++)
    {
        if (refs[i] != NullReference)
        {
            uint32_t c_refs = refs[i];
            // printf("DEBUG: DOUBLE: c_refs: %d ffcn: %d \n", c_refs, ffcn);
            soFreeIndirectFileClusters(c_refs, ip, 0, total); // initial location should be verified
        }
    }

    free(refs);

    // Free own cluster
    soFreeCluster(ip->i2);
    ip->i2 = NullReference;
    (*total)--;
}

#endif

/* ********************************************************* */
