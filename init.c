#include <linux/module.h>
#include <linux/init.h> 
#include <linux/kernel.h>
#include <linux/fs.h>

#define USTAR_MAGIC_NUMBER 8583846582
#define USTAR_ROOT_INODE_NUMBER 0

struct dentry* ustar_mount(struct file_system_type *type, int flags, char const *dev, void *data);

int ustar_fill_super_block(struct super_block* superblock, void* data, int silent);

void ustar_put_super_block(struct super_block *superblock);

static struct file_system_type ustar_fs_type = {
    .owner = THIS_MODULE,
    .name = "ustar",
    .mount = ustar_mount,
    .kill_sb = kill_block_super,
    .fs_flags = FS_REQUIRES_DEV,
};

static struct super_operations ustar_super_operations = {
    .put_super = ustar_put_super_block,
};

/*static struct inode_operations ustar_inode_operations = {

};*/

int __init ustar_init(void){
    int status = register_filesystem(&ustar_fs_type);
    if(status == 0)
        pr_debug("ustar filesystem registered\n");
    else
        pr_err("ustar filesystem registration failed\n");

    return status;
}

void __exit ustar_exit(void){
    int status = unregister_filesystem(&ustar_fs_type);
    if(status == 0)
        pr_debug("ustar filesystem unregistered\n");
    else
        pr_err("ustar filesystem unregistration failed\n");
}

module_init(ustar_init);
module_exit(ustar_exit);

MODULE_VERSION("0.0.1");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Maciej Wroblewski");

struct dentry* ustar_mount(struct file_system_type *fs_type, int flags, char const *dev, void *data){
    struct dentry* entry = mount_bdev(fs_type, flags, dev, data, ustar_fill_super_block);

    if(IS_ERR(entry))
        pr_err("ustar mounting failed\n");
    else
        pr_debug("ustar mounted\n");
    return entry;
}

int ustar_fill_super_block(struct super_block* superblock, void* data, int silent){
    struct inode *root;

    superblock->s_magic = USTAR_MAGIC_NUMBER;
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