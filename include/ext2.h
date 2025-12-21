#pragma once

#include <stdint.h>
#include <ata.h>
#include <terminal.h>

uint32_t find_block_group_from_inode(uint32_t inode);

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

void *memcpy_2(void *restrict dest, const void *restrict src, size_t n) {
    uint8_t *restrict pdest = (uint8_t *restrict)dest;
    const uint8_t *restrict psrc = (const uint8_t *restrict)src;
    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }
    return dest;
}

void read_inode(uint32_t inode_number) {
    // Find which block group contains this inode
    uint32_t block_group = find_block_group_from_inode(inode_number);
    
    // Calculate index within the block group's inode table
    uint32_t index_in_group = inode_number % sb.inodes_per_group;
    
    // Get the inode table starting block for this block group
    uint32_t inode_table_block = bgdt[block_group].inode_table;
    
    // Calculate which block within the inode table contains this inode
    uint32_t block_size_bytes = 1024 << sb.block_size;
    uint32_t inode_size = 128;
    uint32_t inodes_per_block = block_size_bytes / inode_size;
    uint32_t block_offset = index_in_group / inodes_per_block;
    uint32_t inode_offset_in_block = (index_in_group % inodes_per_block) * inode_size;
    
    // Calculate the actual block number
    uint32_t target_block = inode_table_block + block_offset;
    
    // Convert to sectors and read
    uint32_t sectors_per_block = block_size_bytes / 512;
    uint32_t sector = target_block * sectors_per_block;
    
    uint8_t buffer[1024];
    ata_read_sector(sector, buffer);
    ata_read_sector(sector + 1, buffer + 512);
    
    // Copy to global inode structure
    memcpy_2(&inode, buffer + inode_offset_in_block, sizeof(struct ext2_inode));
    
    // Display the inode details
    terminal_write("\n=== Inode ");
    terminal_write_dec(inode_number);
    terminal_write(" Details ===\n");
    
    terminal_write("Block Group: ");
    terminal_write_dec(block_group);
    terminal_write("\n");
    
    terminal_write("Type/Permissions: 0x");
    terminal_write_hex(inode.type_and_permissions);
    
    // Decode file type
    uint16_t file_type = (inode.type_and_permissions >> 12) & 0xF;
    terminal_write(" (");
    if (file_type == 0x1) terminal_write("FIFO");
    else if (file_type == 0x2) terminal_write("Character Device");
    else if (file_type == 0x4) terminal_write("Directory");
    else if (file_type == 0x6) terminal_write("Block Device");
    else if (file_type == 0x8) terminal_write("Regular File");
    else if (file_type == 0xA) terminal_write("Symbolic Link");
    else if (file_type == 0xC) terminal_write("Socket");
    else terminal_write("Unknown");
    terminal_write(")\n");
    
    terminal_write("Permissions: 0");
    terminal_write_dec((inode.type_and_permissions & 0x1FF));  // Lower 9 bits
    terminal_write("\n");
    
    terminal_write("UID: ");
    terminal_write_dec(inode.uid);
    terminal_write(", GID: ");
    terminal_write_dec(inode.gid);
    terminal_write("\n");
    
    terminal_write("Size: ");
    terminal_write_dec(inode.size_low);
    terminal_write(" bytes\n");
    
    terminal_write("Sectors: ");
    terminal_write_dec(inode.sectors_count);
    terminal_write("\n");
    
    terminal_write("Links: ");
    terminal_write_dec(inode.links_count);
    terminal_write("\n");
    
    terminal_write("Flags: 0x");
    terminal_write_hex(inode.flags);
    terminal_write("\n");
    
    terminal_write("\nDirect block pointers:\n");
    for (int i = 0; i < 12; i++) {
        if (inode.block[i] != 0) {
            terminal_write("  [");
            terminal_write_dec(i);
            terminal_write("]: ");
            terminal_write_dec(inode.block[i]);
            terminal_write("\n");
        }
    }
    
    if (inode.singly_indirect != 0) {
        terminal_write("Singly indirect: ");
        terminal_write_dec(inode.singly_indirect);
        terminal_write("\n");
    }
    
    if (inode.doubly_indirect != 0) {
        terminal_write("Doubly indirect: ");
        terminal_write_dec(inode.doubly_indirect);
        terminal_write("\n");
    }
    
    if (inode.triply_indirect != 0) {
        terminal_write("Triply indirect: ");
        terminal_write_dec(inode.triply_indirect);
        terminal_write("\n");
    }
    
    terminal_write("\n");
}

void create_file(uint32_t parent_inode, const char *filename){
    // find first free group
    // find inode in said group
    // find block in said group
    // update inode table and bitmap
    // create directory entry
    // add it to parent
}

void update_blockgroup_descriptor(uint32_t group_number, uint32_t delta_inodes, uint32_t delta_blocks){
    bgdt[group_number].free_blocks_count += delta_blocks;
    bgdt[group_number].free_inodes_count += delta_inodes;

    uint8_t buffer[1024];

    memcpy_2(buffer, bgdt, sizeof(bgdt));

    ata_write_sector(4, buffer);
    ata_write_sector(5, buffer + 512);
}

void update_block_bitmap(uint32_t group_number, uint32_t block_number, uint8_t new_value){

}

void update_inode_bitmap(uint32_t group_number, uint32_t inode_number, uint8_t new_value){

}

uint32_t find_first_free_group(void){
    return 0;
}

uint32_t find_free_block(uint32_t group_number){
    return 0;
}

uint32_t find_free_inode(uint32_t group_number){
    return 0;
}

void read_directory_entries(uint32_t inode_number) {
    // First, read the inode
    read_inode(inode_number);
    
    // Check if it's actually a directory
    uint16_t file_type = (inode.type_and_permissions >> 12) & 0xF;
    if (file_type != 0x4) {
        terminal_write("Error: Inode ");
        terminal_write_dec(inode_number);
        terminal_write(" is not a directory!\n");
        return;
    }
    
    terminal_write("\n=== Directory Entries for Inode ");
    terminal_write_dec(inode_number);
    terminal_write(" ===\n\n");
    
    uint32_t block_size_bytes = 1024 << sb.block_size;
    uint32_t sectors_per_block = block_size_bytes / 512;
    uint8_t block_buffer[1024];
    
    // Read through all direct blocks
    for (int block_idx = 0; block_idx < 12; block_idx++) {
        uint32_t block_num = inode.block[block_idx];
        
        if (block_num == 0) {
            break;  // No more blocks
        }
        
        // Read the block
        uint32_t sector = block_num * sectors_per_block;
        ata_read_sector(sector, block_buffer);
        ata_read_sector(sector + 1, block_buffer + 512);
        
        // Parse directory entries in this block
        uint32_t offset = 0;
        while (offset < block_size_bytes) {
            struct ext2_directory_entry *entry = (struct ext2_directory_entry *)(block_buffer + offset);
            
            // Check if we've reached the end
            if (entry->inode == 0 || entry->size == 0) {
                break;
            }
            
            // Display entry
            terminal_write("Inode: ");
            terminal_write_dec(entry->inode);
            terminal_write("\t");
            
            // Display type
            terminal_write("Type: ");
            if (entry->type == 1) terminal_write("File     ");
            else if (entry->type == 2) terminal_write("Directory");
            else if (entry->type == 3) terminal_write("CharDev  ");
            else if (entry->type == 4) terminal_write("BlockDev ");
            else if (entry->type == 5) terminal_write("FIFO     ");
            else if (entry->type == 6) terminal_write("Socket   ");
            else if (entry->type == 7) terminal_write("Symlink  ");
            else terminal_write("Unknown  ");
            
            terminal_write("\tName: ");
            
            // Display name (null-terminated or length-limited)
            for (int i = 0; i < entry->name_length && i < 255; i++) {
                terminal_putchar(entry->name[i]);
            }
            terminal_write("\n");
            
            // Move to next entry
            offset += entry->size;
            
            // Safety check
            if (entry->size < 8) {
                terminal_write("Warning: Invalid entry size, stopping\n");
                break;
            }
        }
    }
    
    // TODO: Handle singly/doubly/triply indirect blocks if needed
    if (inode.singly_indirect != 0) {
        terminal_write("\n(Note: This directory has indirect blocks - not yet implemented)\n");
    }
}

void update_superblock(uint32_t delta_inodes, uint32_t delta_blocks){

    sb.total_unallocated_blocks += delta_blocks;
    sb.total_unallocated_inodes += delta_inodes;

    uint8_t buffer[1024];

    memcpy_2(buffer, &sb, sizeof(struct ext2_superblock));

    ata_write_sector(2, buffer);
    ata_write_sector(3, buffer + 512);
}



void parse_blockgroup_descriptors(void) {
    uint8_t buffer[1024];

    ata_read_sector(4, buffer);
    ata_read_sector(5, buffer + 512);

    memcpy_2(bgdt, buffer, sizeof(bgdt));

    terminal_write("\n=== ext2 Block Group Descriptors ===\n");

    for (int i = 0; i < 0; i++) {
        terminal_write("\nGroup ");
        terminal_write_dec(i);
        terminal_write("\n");

        terminal_write(" Block Bitmap Block: ");
        terminal_write_dec(bgdt[i].block_usage_bitmap);
        terminal_write("\n");

        terminal_write(" Inode Bitmap Block: ");
        terminal_write_dec(bgdt[i].inode_usage_bitmap);
        terminal_write("\n");

        terminal_write(" Inode Table Block: ");
        terminal_write_dec(bgdt[i].inode_table);
        terminal_write("\n");

        terminal_write(" Free Blocks: ");
        terminal_write_dec(bgdt[i].free_blocks_count);
        terminal_write("\n");

        terminal_write(" Free Inodes: ");
        terminal_write_dec(bgdt[i].free_inodes_count);
        terminal_write("\n");

        terminal_write(" Directories: ");
        terminal_write_dec(bgdt[i].directories_count);
        terminal_write("\n");
    }
}


void parse_superblock(void) {
    uint8_t buffer[1024];
    ata_read_sector(2, buffer);
    ata_read_sector(3, buffer + 512);
    
    sb = *(struct ext2_superblock *)buffer;
    
    terminal_set_color(0x00FF00);
    terminal_write("✓ Valid ext2 filesystem detected!\n");
    terminal_set_color(0xFFFFFF);
    
    terminal_write("\n=== ext2 Superblock Information ===\n");
    
    terminal_write("Total Inodes: ");
    terminal_write_dec(sb.total_inodes);
    terminal_write("\n");
    
    terminal_write("Inodes per Group: ");
    terminal_write_dec(sb.inodes_per_group);
    terminal_write("\n");
    
    terminal_write("Free Inodes: ");
    terminal_write_dec(sb.total_unallocated_inodes);
    terminal_write("\n");
    
    terminal_write("Total Blocks: ");
    terminal_write_dec(sb.total_blocks);
    terminal_write("\n");
    
    terminal_write("Blocks per Group: ");
    terminal_write_dec(sb.blocks_per_group);
    terminal_write("\n");
    
    terminal_write("Free Blocks: ");
    terminal_write_dec(sb.total_unallocated_blocks);
    terminal_write("\n");
    
    // Calculate actual block size (1024 << block_size)
    uint32_t block_size_bytes = 1024 << sb.block_size;
    terminal_write("Block Size: ");
    terminal_write_dec(block_size_bytes);
    terminal_write(" bytes (log2: ");
    terminal_write_dec(sb.block_size);
    terminal_write(")\n");
    
    // Sectors per block
    uint32_t sectors_per_block = block_size_bytes / 512;
    terminal_write("Sectors per Block: ");
    terminal_write_dec(sectors_per_block);
    terminal_write("\n");

    // Calculate total size
    uint64_t total_bytes = (uint64_t)sb.total_blocks * block_size_bytes;
    uint64_t total_mb = total_bytes / (1024 * 1024);
    terminal_write("Total Size: ");
    terminal_write_dec(total_mb);
    terminal_write(" MB\n");
    
    // Calculate free space
    uint64_t free_bytes = (uint64_t)sb.total_unallocated_blocks * block_size_bytes;
    uint64_t free_mb = free_bytes / (1024 * 1024);
    terminal_write("Free Space: ");
    terminal_write_dec(free_mb);
    terminal_write(" MB\n");

    
    terminal_write("\n");

}


// we need a way to read/edit block bitmaps and inode bitmaps



// given an inode number, returns block group number (0-12)
uint32_t find_block_group_from_inode(uint32_t inode){
    return (inode - 1) / sb.inodes_per_group;
}
