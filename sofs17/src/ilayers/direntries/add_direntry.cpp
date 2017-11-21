/**
 *  \author Leonardo Miguel Oliveira Costa 80162
 *  \tester Leonardo Miguel Oliveira Costa 80162
 */

#include "direntries.h"
#include "direntries.bin.h"

#include "probing.h"
#include "exception.h"

#include <errno.h>

//imports do aluno
#include <czdealer.h>
#include <itdealer.h>
#include <fileclusters.h>
#include "fileclusters.bin.h"

/* assumes that:
 * - pih is a valid inode handler of a directory where the user has write access
 * - name can already exist or not (that should be tested)
 * - cin is a valid inodo number
 */
void soAddDirEntry(int pih, const char *name, uint32_t cin)
{
    soProbe(352, "soAddDirEntry(%d, %s, %u)\n", pih, name, cin);
    //define __original__
    #ifdef __original__
    soAddDirEntryBin(pih, name, cin);
    #else
    if(name[0] == '\0')
    {
        throw SOException(EINVAL, __FUNCTION__);
    }
    else if(strlen(name) > SOFS17_MAX_NAME)
    {
        throw SOException(ENAMETOOLONG, __FUNCTION__);
    }
    SOInode* pinode = iGetPointer(pih);
    uint32_t i = 0;
    // onde e armazenado o cluster atual lido
    SODirEntry* entries2read = (SODirEntry*) malloc(DirentriesPerCluster * sizeof(SODirEntry));
    uint32_t cluster2read; // cluster a ler
    bool ffentry = false; // indica se existe expaço para armazenar uma SODirEntry
    uint32_t cluster2write; // cluster a escrever
    SODirEntry* entries2write; // array de SODirEntries a gravar
    uint32_t first_nullref = NullReference; // variavel que vai conter a primeira posicao livre
    while(i < pinode->size/ClusterSize)
    {
        soGetFileCluster(pih, i);
        if(cluster2read == NullReference)
        {
            if(first_nullref == NullReference)
            {
                first_nullref = i;
            }
            i++;
            continue;
        }
        soReadFileCluster(pih, i, entries2read);
        uint32_t entry = 0;
        while(entry < DirentriesPerCluster)
        {
            if(strcmp(entries2read[entry].name, name) == 0)
            {
                i = pinode->size;
                break;
            }
            else if(ffentry == false && entries2read[entry].name[0] == '\0')
            {
                cluster2write = cluster2read;
                entries2write = (SODirEntry*) malloc(DirentriesPerCluster * sizeof(SODirEntry));
                memcpy(entries2write, entries2read, DirentriesPerCluster * sizeof(SODirEntry));
                memcpy(entries2write[entry].name,name,SOFS17_MAX_NAME+1);
                entries2write[entry].in = cin;
                ffentry = true;
            }
            entry++;
        }
        i++;
    }
    if(i != pinode->size)
    {
        if(ffentry == true)
        {
            soWriteFileCluster(pih, cluster2write, entries2write);
        }
        else
        {
            entries2write = (SODirEntry*) calloc(DirentriesPerCluster, sizeof(SODirEntry));
            memcpy(entries2write[0].name, name, SOFS17_MAX_NAME+1);
            cluster2write = soAllocFileCluster(pih,first_nullref);
            soWriteFileCluster(pih,cluster2write, entries2write);
        }
    }
    #endif
}
