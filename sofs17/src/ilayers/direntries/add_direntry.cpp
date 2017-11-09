/**
 *  \author ...
 *  \tester ...
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
    SOInode* pinode = iGetPointer(pih);
    uint32_t i = 0;
    // onde e armazenado o cluster atual lido
    SODirEntry* entries2read = (SODirEntry*) malloc(DirentriesPerCluster * sizeof(SODirEntry));
    uint32_t* references; // array de referencias para clusters contendo SODirEntries
    uint32_t* references2; // array de referencias para clusters do tipo i1
    uint32_t cluster2read; // cluster a ler
    bool ffentry = false; // indica se existe expaço para armazenar uma SODirEntry
    uint32_t cluster2write; // cluster a escrever
    SODirEntry* entries2write; // array de SODirEntries a gravar
    uint32_t first_nullref = NullReference; // variavel que vai conter a primeira posicao livre
    // if i1 exists
    if(pinode->i1 != NullReference)
    {
        references = (uint32_t*) malloc(ReferencesPerCluster * sizeof(uint32_t));
        soReadCluster(pinode->i1, references);
        // if i2 exists
        if(pinode->i2 != NullReference)
        {
            references2 = (uint32_t*) malloc(ReferencesPerCluster * sizeof(uint32_t));
            soReadCluster(pinode->i2, references2);
        }
    }
    while(i < pinode->clucnt)
    {
        if(i < N_DIRECT && pinode->d[i] != NullReference)
        {
            cluster2read = pinode->d[i];
            if(cluster2read != NullReference) soReadCluster(cluster2read, entries2read);
        }
        else if(i >= N_DIRECT && i< ReferencesPerCluster)
        {
            cluster2read = references[i-N_DIRECT];
            if(cluster2read != NullReference) soReadCluster(cluster2read, entries2read);
        }
        else if(i >= ReferencesPerCluster && i < ReferencesPerCluster * ReferencesPerCluster)
        {
            cluster2read = references2[(i-N_DIRECT-ReferencesPerCluster) / ReferencesPerCluster];
            if(cluster2read != NullReference)
            {
                soReadCluster(cluster2read, references);
                cluster2read = references[(i-N_DIRECT-ReferencesPerCluster) % ReferencesPerCluster];
                if(cluster2read != NullReference) soReadCluster(cluster2read, entries2read);
            }
        }
        if(cluster2read == NullReference)
        {
            if(first_nullref == NullReference)
            {
                first_nullref = i;
            }
            i++;
            continue;
        }
        uint32_t entry = 0;
        while(entry < DirentriesPerCluster)
        {
            if(strcmp(entries2read[entry].name, name) == 0)
            {
                i = pinode->clucnt;
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
    if(i != pinode->clucnt + 1)
    {
        if(ffentry == true)
        {
            soWriteCluster(cluster2write, entries2write);
        }
        else
        {
            entries2write = (SODirEntry*) calloc(DirentriesPerCluster, sizeof(SODirEntry));
            memcpy(entries2write[0].name, name, SOFS17_MAX_NAME+1);
            cluster2write = soAllocFileCluster(pih,first_nullref);
            soWriteCluster(cluster2write, entries2write);
        }
    }
    #endif
}
