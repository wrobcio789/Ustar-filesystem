#ifndef SUPER_H
#define SUPER_H
#include <linux/fs.h>
#include "ustar_base.h"
#include "inode.h"

int ustar_fill_super_block(struct super_block* superblock, void* data, int silent);

void ustar_put_super_block(struct super_block *superblock);

#endif //SUPER_H