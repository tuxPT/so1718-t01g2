/**
 *  \author Ariel António da Silva Bastos, 72204
 *  \tester ...
 */
#include "fileclusters.h"
#include "fileclusters.bin.h"

#include "probing.h"
#include "exception.h"
#include "datatypes.h"
#include "czdealer.h"

#include <errno.h>
#include <inttypes.h>

void soReadFileCluster(int ih, uint32_t fcn, void *buf)
{
    soProbe(404, "soReadFileCluster(%d, %u, %p)\n", ih, fcn, buf);
    #ifdef __original__
    soReadFileClusterBin(ih,fcb,*buf);
    #else
    //cluster number
    uint32_t cn;
    //obter o cluster number do file cluster number passado como argumento
    cn=soGetFileCluster(ih,fcn);

    //se o cluster estiver vazio a funcao alloc irá associar o cn ao fcn dado(provavelmente alloc errado)
    if(cn == NullReference)
    {
     	memset(buf,'\0',ClusterSize);
 	}

    soReadCluster(cn,buf);
    #endif
}
