#ifndef INODE_H
#define INODE_H
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <linux/string.h>
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

struct ustar_inode_data{
    loff_t block_number;
};

uint32_t ustar_calculate_size_in_blocks(loff_t size);

void ustar_inode_fill(struct inode* node, struct ustar_disk_inode* disk_node);

struct inode* ustar_inode_get(struct super_block* super_block, ino_t inode_number);

void ustar_destroy_inode(struct inode* node);

int ustar_iterate(struct file* fileptr, struct dir_context* dir_ctx);

struct dentry* ustar_lookup(struct inode* dir, struct dentry* dentry, unsigned flags);

ssize_t ustar_read(struct file * filp, char __user * buf, size_t len, loff_t * ppos);

ino_t ustar_inode_number_by_name(struct super_block* sb, const char* name);

ino_t ustar_find_inode_number_in_dir(struct inode* dir, struct qstr* name);


bool string_starts_with(const char* string, const char* prefix);

bool ustar_are_paths_equal(const char* a, const char* b);

void ustar_extract_filename(char* fullpath, char* filename);

int ustar_is_direct_descendant(const char* ancestor, const char* descendant);

struct buffer_head* ustar_read_present_block(struct super_block* sb, sector_t block_number);

#endif //INODE_H