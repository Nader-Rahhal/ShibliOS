#pragma once

#include <stdint.h>

struct ext2_inode {
    uint16_t type_and_permissions;
    uint16_t uid;
    uint32_t size_low;
    uint32_t access_time;
    uint32_t creation_time;
    uint32_t modification_time;
    uint32_t deletion_time;
    uint16_t gid;
    uint16_t links_count;
    uint32_t sectors_count;
    uint32_t flags;
    uint32_t osd1;
    uint32_t block[12];
    uint32_t singly_indirect;
    uint32_t doubly_indirect;
    uint32_t triply_indirect;
    uint32_t generation_number;
    uint32_t extended_attribute_block;
    uint32_t size_high;
    uint32_t fragment_block;
    uint32_t osd2[3];
};
struct ext2_superblock {
    uint32_t total_inodes;
    uint32_t total_blocks;
    uint32_t total_reserved_superuser;
    uint32_t total_unallocated_blocks;
    uint32_t total_unallocated_inodes;
    uint32_t block_containing_superblock;
    uint32_t block_size;
    uint32_t fragment_size;
    uint32_t blocks_per_group;
    uint32_t fragments_per_group;
    uint32_t inodes_per_group;
    uint32_t last_mount_time;
    uint32_t last_write_time;
    uint16_t number_mounts_since_cc;
    uint16_t number_mounts_before_cc;
    uint16_t ext2_signature;
    uint16_t fs_state;
    uint16_t do_on_error;
    uint16_t version_minor;
    uint32_t last_cc_check;
    uint32_t time_btwn_cc;
    uint32_t os_id;
    uint32_t major_version;
    uint16_t uid_reserved_blocks;
    uint16_t gid_reserved_blocks;
};

struct ext2_group_descriptor {
    uint32_t block_usage_bitmap;
    uint32_t inode_usage_bitmap;
    uint32_t inode_table;
    uint16_t free_blocks_count;
    uint16_t free_inodes_count;
    uint16_t directories_count;
    uint8_t reserved[14];
};

struct ext2_directory_entry {
    uint32_t inode;
    uint16_t size;
    uint8_t name_length;
    uint8_t type;
    char name[255];
};

static struct ext2_superblock sb;
static struct ext2_group_descriptor bgdt[32];
static struct ext2_inode inode;

void *memcpy_2(void *restrict dest, const void *restrict src, size_t n);

void read_inode(uint32_t inode_number);

void create_file(uint32_t parent_inode, const char *filename);

void delete_file(uint32_t inode_number);

void write_file(uint32_t inode_number, const char* data);

void read_file(uint32_t inode_number, char* buffer, uint32_t max_size);

void print_file(uint32_t inode_number);

void add_directory_entry(uint32_t parent_inode, struct ext2_directory_entry *entry);

void edit_inode_table(uint32_t inode_number, struct ext2_inode *new_inode);

void update_blockgroup_descriptor(uint32_t group_number, uint32_t delta_inodes, uint32_t delta_blocks);

void update_block_bitmap(uint32_t group_number, uint32_t block_number, uint8_t new_value);

void update_inode_bitmap(uint32_t group_number, uint32_t inode_number, uint8_t new_value);

void read_inode_bitmap(uint32_t group_number);

uint32_t find_first_free_group(void);

uint32_t find_free_block(uint32_t group_number);

uint32_t find_free_inode(uint32_t group_number);

void read_directory_entries(uint32_t inode_number);

void update_superblock(uint32_t delta_inodes, uint32_t delta_blocks);

void parse_blockgroup_descriptors(void);

void parse_superblock(void);

uint32_t find_block_group_from_inode(uint32_t inode);