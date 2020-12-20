#include <linux/module.h>
#include <linux/init.h> 
#include "ustar.h"
#include "super.h"

struct dentry* ustar_mount(struct file_system_type *fs_type, int flags, char const *dev, void *data){
    struct dentry* entry = mount_bdev(fs_type, flags, dev, data, ustar_fill_super_block);

    if(IS_ERR(entry))
        pr_err("ustar mounting failed\n");
    else
        pr_debug("ustar mounted\n");
    return entry;
}

static struct file_system_type ustar_fs_type = {
    .owner = THIS_MODULE,
    .name = "ustar",
    .mount = ustar_mount,
    .kill_sb = kill_block_super,
    .fs_flags = FS_REQUIRES_DEV,
};

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
