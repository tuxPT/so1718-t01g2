/**
 *  \brief Definition of the inode data type.
 *
 *  \author Artur Pereira - 2008-2009, 2016-2017
 *  \author Miguel Oliveira e Silva - 2009, 2017
 *  \author António Rui Borges - 2010-2015
 */

#ifndef __SOFS17_INODE__
#define __SOFS17_INODE__

#include <stdint.h>

/** \brief flag signaling inode is free (it uses the sticky bit) */
#define INODE_FREE (0001000)

/** \brief direct block references in the inode */
#define N_DIRECT 6

/** \brief Definition of the inode data type. */
struct SOInode {
    /** \brief inode mode: it stores the file type and permissions.
     *  (see man 2 stat).
     */
	uint16_t mode;
    /** \brief link count: number of hard links (directory entries) associated to the inode */
	uint16_t lnkcnt;
    /** \brief user ID of the file owner */
	uint32_t owner;
    /** \brief group ID of the file owner */
	uint32_t group;
    /** \brief file size in bytes: */
	uint32_t size;
    /** \brief cluster count: total number of clusters used by the file */
	uint32_t clucnt;

    /** \brief time of last access to file information */ 
	uint32_t atime;

    /* \brief usage depends on inode state */
	union {
	    /** \brief time of last change to file information */
	    uint32_t mtime;
		/** \brief next free inode (only used when inode is free) */
		uint32_t next;
	};

    /* \brief usage depends on inode state */
	union {
	    /** \brief time of last change to inode information */
	    uint32_t ctime;
		/** \brief prev free inode (only used when inode is free) */
		uint32_t prev;
	};

   /** \brief direct references to the clusters that comprise the file information content */
	uint32_t d[N_DIRECT];
   /** \brief reference to clusters that extend the d array */
	uint32_t i1;
   /** \brief reference to a cluster that extends the i1 array */
	uint32_t i2;
};

#endif				/* __SOFS17_INODE__ */
