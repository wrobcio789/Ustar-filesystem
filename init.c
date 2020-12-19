#include <linux/module.h>
#include <linux/init.h> 

int __init ustar_init(void){
    pr_debug("USTAR module loaded\n");
    return 0;
}

void __exit ustar_exit(void){
    pr_debug("USTAR module unloaded\n");
}

module_init(ustar_init);
module_exit(ustar_exit);

MODULE_VERSION("0.0.1");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Maciej Wroblewski");