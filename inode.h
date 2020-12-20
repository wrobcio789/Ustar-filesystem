#ifndef INODE_H
#define INODE_H
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include "ustar_base.h"

struct ustar_disk_inode{
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
};

uint32_t ustar_calculate_size_in_blocks(loff_t size);

void ustar_inode_fill(struct inode* node, struct ustar_disk_inode* disk_node);

struct inode* ustar_inode_get(struct super_block* super_block, ino_t inode_number);

void ustar_destroy_inode(struct inode* node);

#endif //INODE_H