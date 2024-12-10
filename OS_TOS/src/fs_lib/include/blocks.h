#pragma once

#include <cstdint>

#define INODE_DIRECT_ADDRESSABLE 7
#define INODE_TABLE_SIZE 121
#define INODE_SIZE sizeof(inode)
#define CLUSTER_RECORD_SIZE sizeof(uint32_t)

#pragma pack(push, 1)
typedef struct superblock {
	uint16_t s_magic;                        // Magic number 0x534f
	uint16_t s_bsize;                        // Disk block size (512, 1024, 2048, 4096)
	uint32_t s_fsize;                        // Disk size (in disk blocks)
	uint32_t s_isize;                        // inode array size (in disk blocks)
	uint32_t s_asize;                        // cluster table size (in disk blocks)
	uint32_t s_tinode;                       // free inode count
	uint32_t s_tfree;                        // free file block count
	uint32_t s_ttinode;                      // free inode count in superblock table
	uint32_t s_inode[INODE_TABLE_SIZE];      // List of free inode indexes
} superblock;

typedef struct inode {
	uint16_t i_attr;                         // File permissions & attributes
	uint16_t i_uid;                          // Owner user id
	uint16_t i_gid;                          // Owner group id
	uint16_t i_links;                        // Number of links to this inode
	uint32_t i_ctime;                        // File creation time (unix timestamp)
	uint32_t i_mtime;                        // File modification time (unix timestamp)
	uint32_t i_size;                         // File size (in bytes)
	uint32_t i_fsize;                        // File size (in blocks)
	uint32_t i_blocks[10];                   // Table of file block addresses
} inode;
#pragma pack(pop)