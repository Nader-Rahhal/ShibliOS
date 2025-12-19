#include <stdint.h>

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

void parse_superblock(){

}