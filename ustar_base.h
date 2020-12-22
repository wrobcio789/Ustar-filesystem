#ifndef USTAR_H
#define USTAR_H
#include <linux/types.h>

#define USTAR_MAGIC_NUMBER 8583846582
#define USTAR_ROOT_INODE_NUMBER 0
#define USTAR_BLOCK_SIZE 512
#define USTAR_FILENAME_LENGTH 100

uint64_t oct2bin(char* string);

#endif //USTAR_H