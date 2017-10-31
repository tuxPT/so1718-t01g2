/**
 *  \author Leonardo Costa
 *  \tester ...
 */

#include "freelists.h"
#include "freelists.bin.h"

#include "probing.h"
#include "exception.h"

#include <errno.h>

//imports do aluno
#include "sbdealer.h"
#include "datatypes.h"
#include "rawdisk.h"

/*
 * Even if some of them are not functionally necessary,
 * the following dictates must be obeyed by the implementation:
 * - do nothing if retrieval cache is not empty;
 * -
 * - after every reference is transferred,
 *      the previous location should be filled with NullReference
 */
void soReplenish(void)
{
//#define __original__
    soProbe(542, "soReplenish()\n");
#ifdef __original__
    soReplenishBin();
#else
    //sb é um ponteiro para o superbloco
    SOSuperBlock* sb = sbGetPointer();

    //não faz nada se a retrieval cache não está vazia
    if(sb->rcache.idx < REFERENCE_CACHE_SIZE) return;

    //refblock é um ponteiro para referencias do bitmap
    SORefBlock* refblock = (SORefBlock*) malloc(sizeof(SORefBlock));
    soReadRawBlock(sb->rmstart+(sb->rmidx/ReferenceBytesPerBitmapBlock),refblock);

    //index no array de referencias refblock->map
    uint32_t index = (sb->rmidx)%ReferenceBytesPerBitmapBlock;

    //insertion cache para retrieval cache
    if(refblock->map[index] == 0)
    {
        sb->rcache.idx = REFERENCE_CACHE_SIZE - sb->icache.idx;
        memcpy(&(sb->rcache.ref[sb->rcache.idx]),&(sb->icache.ref[0]), sb->icache.idx * sizeof(uint32_t));
        memset(&(sb->icache.ref[0]),NullReference,sb->icache.idx * sizeof(uint32_t));
        sb->icache.idx = 0;
    }
    //bitmap para retrieval cache
    else
    {
        uint32_t i = 0;
        for(; i < REFERENCE_CACHE_SIZE && index < ReferenceBytesPerBitmapBlock; index++)
        {
            uint8_t byte = refblock->map[index];
            uint8_t counter = 0;
            for(uint8_t mask = 0x80; i < REFERENCE_CACHE_SIZE && mask > 0; mask>>=1, counter++)
            {
                //quando encontra um bit a 1 calcula o numero do cluster e move-o para a retrieval cache
                if((refblock->map[index] & mask) == mask)
                {
                    sb->rcache.ref[i] = (index * 8) + counter;
                    i++;
                    byte = byte & ~mask;
                }
            }
            refblock->map[index] = byte;
        }
        //atualiza a bitmap table
        soWriteRawBlock(sb->rmstart+(sb->rmidx/ReferenceBytesPerBitmapBlock),refblock);

        //proximo bloco a ser lido é atualizado
        if(sb->rcache.ref[index - 1] != 0) sb->rmidx = index - 1;
        else sb->rmidx = index;

        //faz o shift caso nao esteja preenchida a retrieval cache e atualiza o ponteiro da rcache
        memmove(&(sb->rcache.ref[0]),&(sb->rcache.ref[REFERENCE_CACHE_SIZE - i]), i * sizeof(uint32_t));
        sb->rcache.idx = REFERENCE_CACHE_SIZE - i;
    }
    //salva para o disco
    sbSave();
#endif
}
