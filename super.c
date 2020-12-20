#include "super.h"

static struct super_operations ustar_super_operations = {
    .put_super = ustar_put_super_block,
};

int ustar_fill_super_block(struct super_block* superblock, void* data, int silent){
    struct inode *root;

    superblock->s_magic = USTAR_MAGIC_NUMBER;
    superblock->s_blocksize = USTAR_BLOCK_SIZE;
    superblock->s_op = &ustar_super_operations;

    root = new_inode(superblock);
    if(!root){
        pr_err("root inode allocation failed\n");
        return -ENOMEM;
    }

    root->i_ino = USTAR_ROOT_INODE_NUMBER;
    root->i_sb = superblock;
    root->i_atime = root->i_mtime = root->i_ctime = current_time(root);
    inode_init_owner(root, NULL, S_IFDIR);

    superblock->s_root = d_make_root(root);
    if(!superblock->s_root){
        pr_err("root directory could not be created\n");
        return -ENOMEM;
    }

    pr_debug("ustar super block filled\n");
    return 0;
}

void ustar_put_super_block(struct super_block *superblock){
    pr_debug("ustar super block destroyed\n");
}