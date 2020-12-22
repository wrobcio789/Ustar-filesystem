#include "inode.h"

static uint32_t ustar_type_to_posix_mode_type_bits[8] = {[0] = S_IFREG, [5] = S_IFDIR};

static struct file_operations ustar_dir_operations = {
    .owner = THIS_MODULE,
    .llseek = generic_file_llseek,
    .read = generic_read_dir,
      //.iterate = ustar_iterate,
};

static struct inode_operations ustar_dir_inode_operations = {
    .lookup = ustar_lookup
};

uint32_t ustar_calculate_size_in_blocks(loff_t size){
    return (size + USTAR_BLOCK_SIZE - 1)/USTAR_BLOCK_SIZE;
}

void ustar_inode_fill(struct inode* node, struct ustar_disk_inode* disk_node){
    uint32_t mode_type_bits = ustar_type_to_posix_mode_type_bits[disk_node->typeflag - '0'];

    node->i_mode = mode_type_bits | oct2bin(disk_node->mode);
    node->i_size = oct2bin(disk_node->size);
    node->i_blocks = ustar_calculate_size_in_blocks(node->i_size) + 1;
    node->i_atime.tv_sec = node->i_ctime.tv_sec = node-> i_mtime.tv_sec = oct2bin(disk_node->mtime);
    i_uid_write(node, (uid_t)(oct2bin(disk_node->uid)));
    i_gid_write(node, (gid_t)(oct2bin(disk_node->gid)));

    pr_debug("ustar filled inode using data:\n"
            "name= %s,\n"
            "uid= %s,\n"
            "gid= %s,\n"
            "size= %s,\n"
            "mtime= %s,\n"
            "chcecksum= %s,\n"
            "typeflag= %c,\n"
            "linkname= %s,\n"
            "magic= %s,\n"
            "version= %s,\n"
            "uname= %s,\n"
            "gname= %s,\n",
        disk_node->name,
        disk_node->uid,
        disk_node->gid,
        disk_node->size,
        disk_node->mtime,
        disk_node->checksum,
        disk_node->typeflag,
        disk_node->linkname,
        disk_node->magic,
        disk_node->version,
        disk_node->uname,
        disk_node->gname);
}

struct inode* ustar_inode_get(struct super_block* super_block, ino_t inode_number){
    struct buffer_head* read_block;
    struct ustar_disk_inode* current_disk_inode;

    struct inode* node = iget_locked(super_block, inode_number);
    if(node == NULL)
        return ERR_PTR(-ENOMEM);

    if((node->i_state & I_NEW) == 0)
        return node;

    read_block = sb_bread(super_block, inode_number); //inode_number == bloc_number
    if(read_block == NULL){
        pr_err("ustar cannot read block number %lu\n", inode_number);
        goto read_error;
    }

    current_disk_inode = (struct ustar_disk_inode*)read_block->b_data;
    ustar_inode_fill(node, current_disk_inode);
    if(S_ISDIR(node->i_mode)){
        node->i_op = &ustar_dir_inode_operations;
        node->i_fop = &ustar_dir_operations;
    }

    brelse(read_block);

    pr_debug("ustar created inode:\n"
            "ino= %lu\n"
            "mode= %u,\n"
            "size= %u,\n"
            "blocks= %u,\n"
            "mtime= %llu,\n"
            "uid=%u,\n"
            "gid=%u,\n"
            "is_file=%d,\n"
            "is_directory=%d,\n",
            node->i_ino,
            (unsigned int)node->i_mode,
            (unsigned int)node->i_size,
            (unsigned int)node->i_blocks,
            (unsigned long long)node->i_mtime.tv_sec,
            (unsigned int)i_uid_read(node),
            (unsigned int)i_gid_read(node),
            S_ISREG(node->i_mode),
            S_ISDIR(node->i_mode)
            );

    unlock_new_inode(node);
    return node;

read_error:
    pr_err("ustar cannot read inode number %d\n", (unsigned int)inode_number);
    iget_failed(node);

    return ERR_PTR(-EIO);
}

/*struct inode* ustar_inode_alloc(struct super_block* super_block){
    struct inode* inode;
    void* data = kvmalloc(sizeof(struct inode) + sizeof(struct ustar_inode_data), GFP_KERNEL);
    if(data == NULL){
        pr_err("ustar could not allocate new inode\n");
        return NULL;
    }
    inode = (struct inode*)data;
    inode_init_once(inode);
    inode->i_private = (struct ustar_inode_data*)(data + sizeof(struct inode));

    pr_debug("ustar inode allocated");
    return inode;
}*/

void ustar_destroy_inode(struct inode* inode){
    //kvfree(inode);
    pr_debug("ustar inode nr %u destroyed", (unsigned int)inode->i_ino);
}

int ustar_iterate(struct file* fileptr, struct dir_context* dir_ctx){
    uint32_t current_block_number = 0;
    uint32_t record_count = 0;
    struct buffer_head* read_block;
    struct ustar_disk_inode* current_disk_inode;
    struct inode* inode;
    struct super_block* sb;
    char ancestor_filename[USTAR_FILENAME_LENGTH];

    pr_debug("ustar iterate from pos %llu through dir named %s\n", dir_ctx->pos, fileptr->f_path.dentry->d_name.name);

    inode = fileptr->f_inode;
	sb = inode->i_sb;

    read_block = sb_bread(sb, inode->i_ino);
    current_disk_inode = (struct ustar_disk_inode*)(read_block->b_data);
    strcpy(ancestor_filename, current_disk_inode->name);
    brelse(read_block);

    pr_debug("ustar iterating, ancestor filename: %s", ancestor_filename);

    while((read_block = sb_bread(sb, current_block_number))){
        pr_debug("ustar iterating, read block number %u", current_block_number);
        current_disk_inode = (struct ustar_disk_inode*)read_block->b_data;
        current_block_number += 1 + ustar_calculate_size_in_blocks(oct2bin(current_disk_inode->size));

        if(ustar_direct_descendant_check(ancestor_filename, current_disk_inode->name)){
            pr_debug("ustar iterating, found child inode with filename %s",current_disk_inode->name);

            record_count++;
            if(dir_emit(dir_ctx, current_disk_inode->name, strlen(current_disk_inode->name), current_block_number, DT_UNKNOWN) == 0){
                brelse(read_block);
                return record_count;
            }
        }
        
        brelse(read_block);
    }


    pr_debug("ustar iterating, found %d matching records", record_count);
    return record_count;
}

int ustar_direct_descendant_check(const char* ancestor, const char* descendant){
    pr_debug("checking if %s is ancestor of %s", ancestor, descendant);

    while(*ancestor != '\0'){
        if(*ancestor != *descendant)
            return false;
        ancestor++;
        descendant++;
    }

    if(*descendant == '\0') //descendant == ancestor
        return false;  

    while(*descendant != '\0'){
        if(*descendant == '/'){
            if(*(descendant + 1) == '\0')   { //descendant is directory
                return true;
            }
            else
                return false; //descendant is not direct
        } 
        descendant++;
    }
    return true;
}

struct dentry* ustar_lookup(struct inode* dir, struct dentry* dentry, unsigned flags){
    ino_t inode_number;
    struct inode* inode;

    pr_debug("ustar lookup dir nr %lu, for name %*s", dir->i_ino, dentry->d_name.len, dentry->d_name.name);

    if (dentry->d_name.len >= USTAR_FILENAME_LENGTH){
        pr_debug("ustar cannot lookup entry %*s, name too long", dentry->d_name.len, dentry->d_name.name);
		return ERR_PTR(-ENAMETOOLONG);
    }

    inode_number =  ustar_find_inode_number_in_dir(dir, &dentry->d_name);
    if(inode_number == (ino_t)-1){
        pr_debug("ustar cannot find inode number in dir nr %lu, named %*s", dir->i_ino, dentry->d_name.len, dentry->d_name.name);
        return ERR_PTR(-EIO);
    }

    inode = ustar_inode_get(dir->i_sb, inode_number);
	if (IS_ERR(inode)) {
		pr_err("Cannot read inode %lu", inode_number);
		return ERR_PTR(PTR_ERR(inode));
	}

    pr_debug("found inode nr %lu with name %*s", inode->i_ino, dentry->d_name.len, dentry->d_name.name);

	d_add(dentry, inode);   
    return NULL;
}

ino_t ustar_find_inode_number_in_dir(struct inode* dir, struct qstr* name){
    struct super_block* sb = dir->i_sb;
    char fullname[USTAR_FILENAME_LENGTH];
    int fullname_index = 0, source_index = 0;
    struct ustar_disk_inode* disk_dir;
    struct buffer_head* read_block;

    read_block = sb_bread(sb, dir->i_ino);
    if(read_block == NULL){
        pr_err("ustar cannot read block nr. %lu. Inode not found.", dir->i_ino);
        return (ino_t)-1;
    }
    disk_dir = (struct ustar_disk_inode*)(read_block->b_data);

    while(disk_dir->name[source_index] != '\0' ){
        fullname[fullname_index] = disk_dir->name[source_index];
        fullname_index++;
        source_index++;
    }

    source_index = 0;
    while(fullname_index < USTAR_FILENAME_LENGTH - 1 && source_index < name->len ){
        fullname[fullname_index] = name->name[source_index];
        fullname_index++;
        source_index++;
    }
    fullname[fullname_index] = '\0';

    if(source_index < name->len){
        brelse(read_block);
        return (ino_t)-1;
    }

    brelse(read_block);
    return ustar_inode_number_by_name(sb, fullname);    
}

ino_t ustar_inode_number_by_name(struct super_block* sb, const char* name){
    loff_t current_block_number = 0;
    struct ustar_disk_inode* current_disk_inode;
    struct buffer_head* read_block;

    while((read_block = sb_bread(sb, current_block_number))){
        pr_debug("ustar name searching, read block number %llu", current_block_number);
        current_disk_inode = (struct ustar_disk_inode*)read_block->b_data;

        pr_debug("ustar lookup, comparing %s with %s", current_disk_inode->name, name);
        if(strcmp(current_disk_inode->name, name) == 0){
            pr_debug("ustar found inode named %s in bloc nr %llu", name, current_block_number);
            return current_block_number;
        }
        
        current_block_number += 1 + ustar_calculate_size_in_blocks(oct2bin(current_disk_inode->size));
        brelse(read_block);
    }

    pr_debug("ustar did not find inode named %s", name);
    return (ino_t)-1;
}