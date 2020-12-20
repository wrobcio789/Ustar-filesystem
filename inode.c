#include "inode.h"

static uint32_t ustar_type_to_posix_mode_type_bits[8] = {[0] = S_IFREG, [5] = S_IFDIR};

uint32_t ustar_calculate_size_in_blocks(loff_t size){
    return (size + USTAR_BLOCK_SIZE - 1)/USTAR_BLOCK_SIZE;
}

void ustar_inode_fill(struct inode* node, struct ustar_disk_inode* disk_node){
    uint32_t mode_type_bits = ustar_type_to_posix_mode_type_bits[disk_node->typeflag - '0'];

    node->i_mode = mode_type_bits | oct2bin(disk_node->mode);
    node->i_size = oct2bin(disk_node->size);
    node->i_blocks = ustar_calculate_size_in_blocks(node->i_size);
    node->i_atime.tv_sec = node->i_ctime.tv_sec = node-> i_mtime.tv_sec = oct2bin(disk_node->mtime);
    i_uid_write(node, (uid_t)(oct2bin(disk_node->uid)));
    i_gid_write(node, (gid_t)(oct2bin(disk_node->gid)));
}

struct inode* ustar_inode_get(struct super_block* super_block, ino_t inode_number){
    uint32_t current_inode_number = 0;
    uint32_t current_block_number = 0;
    struct buffer_head* read_block;
    struct ustar_disk_inode* current_disk_inode;

    struct inode* node = iget_locked(super_block, inode_number);
    if(node == NULL)
        return ERR_PTR(-ENOMEM);

    if((node->i_state & I_NEW) == 0)
        return node;

    while(current_inode_number < inode_number){
        read_block = sb_bread(super_block, current_block_number);
        if(read_block == NULL){
            pr_err("ustar cannot read block number %d\n", current_block_number);
            goto read_error;
        }

        current_disk_inode = (struct ustar_disk_inode*)read_block->b_data;
        current_block_number += 1 + ustar_calculate_size_in_blocks(oct2bin(current_disk_inode->size));
        current_inode_number++;

        brelse(read_block);
    }

    read_block = sb_bread(super_block, current_block_number);
    if(read_block == NULL){
        pr_err("ustar cannot read block number %d\n", current_block_number);
        goto read_error;
    }

    current_disk_inode = (struct ustar_disk_inode*)read_block->b_data;
    ustar_inode_fill(node, current_disk_inode);
    brelse(read_block);

    pr_debug("ustar created inode:\n"
            "mode= %u\n,"
            "size= %u\n,"
            "blocks= %u\n,"
            "mtime= %llu\n,"
            "uid=%o\n,"
            "gid=%o\n",
            (unsigned int)node->i_mode,
            (unsigned int)node->i_size,
            (unsigned int)node->i_blocks,
            (unsigned long long)node->i_mtime.tv_sec,
            (unsigned int)i_uid_read(node),
            (unsigned int)i_gid_read(node)
            );

    unlock_new_inode(node);
    return node;

read_error:
    pr_err("ustar cannot read inode number %d\n", (unsigned int)inode_number);
    iget_failed(node);

    return ERR_PTR(-EIO);
}