#include "super.h"

static struct super_operations ustar_super_operations = {
    .put_super = ustar_put_super_block,
    .destroy_inode = ustar_destroy_inode,
    //.alloc_inode = ustar_inode_alloc,
};

int ustar_fill_super_block(struct super_block* superblock, void* data, int silent){
    struct inode *root;

    superblock->s_magic = USTAR_SUPERBLOCK_MAGIC_NUMBER;
    sb_set_blocksize(superblock, USTAR_BLOCK_SIZE);
    superblock->s_maxbytes = USTAR_MAX_FILE_SIZE;
    superblock->s_op = &ustar_super_operations;
    superblock->s_flags |= SB_RDONLY;

    root = ustar_inode_get(superblock, USTAR_ROOT_INODE_NUMBER);
    if(IS_ERR(root))
        return PTR_ERR(root);

    superblock->s_root = d_make_root(root);
    if(superblock->s_root == NULL){
        pr_err("ustar root directory could not be created\n");
        return -ENOMEM;
    }

    pr_debug("ustar super block filled\n");
    return 0;
}

void ustar_put_super_block(struct super_block *superblock){
    pr_debug("ustar super block destroyed\n");
}