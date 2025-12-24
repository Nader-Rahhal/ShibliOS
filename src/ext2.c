#include <stdint.h>

#include "ata.h"
#include "terminal.h"
#include "ext2.h"
#include "memory.h"

void read_inode(uint32_t inode_number) {

    uint32_t block_group = find_block_group_from_inode(inode_number);
    

    uint32_t index_in_group = inode_number % sb.inodes_per_group;
    
    uint32_t inode_table_block = bgdt[block_group].inode_table;
    
    uint32_t block_size_bytes = 1024 << sb.block_size;
    uint32_t inode_size = 128;
    uint32_t inodes_per_block = block_size_bytes / inode_size;
    uint32_t block_offset = index_in_group / inodes_per_block;
    uint32_t inode_offset_in_block = (index_in_group % inodes_per_block) * inode_size;
    
    uint32_t target_block = inode_table_block + block_offset;
    
    uint32_t sectors_per_block = block_size_bytes / 512;
    uint32_t sector = target_block * sectors_per_block;
    
    uint8_t buffer[1024];
    read(sector, 1024, buffer);
    
    memcpy(&inode, buffer + inode_offset_in_block, sizeof(struct ext2_inode));

    /*
    
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
    */
}

void create_file(uint32_t parent_inode, const char *filename) {
    uint32_t group_number = find_block_group_from_inode(parent_inode);
    
    uint32_t free_inode = find_free_inode(group_number);
    
    update_inode_bitmap(group_number, free_inode, 1);
    
    struct ext2_inode new_inode = {0};
    new_inode.type_and_permissions = 0x81A4; // Regular file with permissions 0644 (rw-r--r--)
    new_inode.uid = 0;
    new_inode.gid = 0;
    new_inode.size_low = 0;          // Empty file
    new_inode.sectors_count = 0;     // No sectors allocated
    new_inode.links_count = 1;
    new_inode.flags = 0;

    edit_inode_table(free_inode, &new_inode);
    
    struct ext2_directory_entry new_entry = {0};
    new_entry.inode = free_inode;
    uint8_t name_len = 0;
    while (filename[name_len] != '\0' && name_len < 255) {
        name_len++;
    }
    new_entry.name_length = name_len;
    new_entry.type = 1;

    memcpy(new_entry.name, filename, name_len);
    
    new_entry.size = 0;
    
    add_directory_entry(parent_inode, &new_entry);

}

void delete_file(uint32_t inode_number) {
    read_inode(inode_number);
    
    uint32_t block_group = find_block_group_from_inode(inode_number);
    
    update_inode_bitmap(block_group, inode_number, 0);
    
    struct ext2_inode empty_inode = {0};
    edit_inode_table(inode_number, &empty_inode);
}

void write_file(uint32_t inode_number, const char* data) {
    uint32_t data_len = 0;
    while (data[data_len] != '\0') {
        data_len++;
    }
    
    if (data_len == 0) {
        return;
    }
    
    read_inode(inode_number);

    uint16_t file_type = (inode.type_and_permissions >> 12) & 0xF;
    if (file_type != 0x8) {
        terminal_write("Error: Inode is not a regular file!\n");
        return;
    }
    
    uint32_t block_size_bytes = 1024 << sb.block_size;
    uint32_t sectors_per_block = block_size_bytes / 512;
    uint32_t blocks_needed = (data_len + block_size_bytes - 1) / block_size_bytes;
    
    if (blocks_needed > 12) {
        terminal_write("Error: File too large (indirect blocks not implemented)\n");
        return;
    }
    
    uint32_t group_number = find_block_group_from_inode(inode_number);
    uint32_t data_offset = 0;
    
    for (uint32_t block_idx = 0; block_idx < blocks_needed; block_idx++) {
        uint32_t block_num;
        
        if (inode.block[block_idx] == 0) {
            block_num = find_free_block(group_number);
            update_block_bitmap(group_number, block_num, 1);
            inode.block[block_idx] = block_num;
        } else {
            block_num = inode.block[block_idx];
        }
        
        uint8_t block_buffer[1024];
        
        uint32_t bytes_to_write = data_len - data_offset;
        if (bytes_to_write > block_size_bytes) {
            bytes_to_write = block_size_bytes;
        }
        
        memcpy(block_buffer, data + data_offset, bytes_to_write);
        
        for (uint32_t i = bytes_to_write; i < block_size_bytes; i++) {
            block_buffer[i] = 0;
        }
        
        uint32_t sector = block_num * sectors_per_block;
        write(sector, 1024, block_buffer);
        
        data_offset += bytes_to_write;
    }
    
    inode.size_low = data_len;
    inode.sectors_count = blocks_needed * sectors_per_block;
    
    edit_inode_table(inode_number, &inode);
    
    terminal_write("Wrote ");
    terminal_write_dec(data_len);
    terminal_write(" bytes to inode ");
    terminal_write_dec(inode_number);
    terminal_write("\n");
}

void read_file(uint32_t inode_number, char* buffer, uint32_t max_size) {

    read_inode(inode_number);
    
    uint16_t file_type = (inode.type_and_permissions >> 12) & 0xF;
    if (file_type != 0x8) {
        terminal_write("Error: Inode is not a regular file!\n");
        return;
    }
    
    if (inode.size_low == 0) {
        terminal_write("File is empty\n");
        if (buffer != 0) {
            buffer[0] = '\0';
        }
        return;
    }
    
    uint32_t block_size_bytes = 1024 << sb.block_size;
    uint32_t sectors_per_block = block_size_bytes / 512;
    uint8_t block_buffer[1024];
    
    uint32_t bytes_read = 0;
    uint32_t bytes_to_read = inode.size_low;
    
    if (buffer != 0 && bytes_to_read > max_size) {
        bytes_to_read = max_size;
    }
    
    for (int block_idx = 0; block_idx < 12; block_idx++) {
        uint32_t block_num = inode.block[block_idx];
        
        if (block_num == 0) {
            break;
        }
        
        uint32_t sector = block_num * sectors_per_block;
        read(sector, 1024, block_buffer);
        
        uint32_t bytes_in_block = bytes_to_read - bytes_read;
        if (bytes_in_block > block_size_bytes) {
            bytes_in_block = block_size_bytes;
        }
        
        if (buffer != 0) {
            memcpy(buffer + bytes_read, block_buffer, bytes_in_block);
        } else {
            for (uint32_t i = 0; i < bytes_in_block; i++) {
                terminal_putchar(block_buffer[i]);
            }
        }
        
        bytes_read += bytes_in_block;
        
        if (bytes_read >= bytes_to_read) {
            break;
        }
    }
    
    if (buffer != 0 && bytes_read < max_size) {
        buffer[bytes_read] = '\0';
    }
    
    if (buffer == 0) {
        terminal_write("\n\nRead ");
        terminal_write_dec(bytes_read);
        terminal_write(" bytes from inode ");
        terminal_write_dec(inode_number);
        terminal_write("\n");
    }
}

void print_file(uint32_t inode_number) {
    read_inode(inode_number);
    
    uint16_t file_type = (inode.type_and_permissions >> 12) & 0xF;
    if (file_type != 0x8) {
        terminal_write("Error: Inode is not a regular file!\n");
        return;
    }
    
    terminal_write("\n=== File Contents (Inode ");
    terminal_write_dec(inode_number);
    terminal_write(") ===\n");
    
    if (inode.size_low == 0) {
        terminal_write("[Empty file]\n");
        return;
    }
    
    uint32_t block_size_bytes = 1024 << sb.block_size;
    uint32_t sectors_per_block = block_size_bytes / 512;
    uint8_t block_buffer[1024];
    
    uint32_t bytes_remaining = inode.size_low;
    
    for (int block_idx = 0; block_idx < 12 && bytes_remaining > 0; block_idx++) {
        uint32_t block_num = inode.block[block_idx];
        
        if (block_num == 0) {
            break;
        }
        
        uint32_t sector = block_num * sectors_per_block;
        read(sector, 1024, block_buffer);

        
        uint32_t bytes_to_print = bytes_remaining;
        if (bytes_to_print > block_size_bytes) {
            bytes_to_print = block_size_bytes;
        }
        
        for (uint32_t i = 0; i < bytes_to_print; i++) {
            terminal_putchar(block_buffer[i]);
        }
        
        bytes_remaining -= bytes_to_print;
    }
    
    terminal_write("\n=== End of File ===\n");
}

void add_directory_entry(uint32_t parent_inode, struct ext2_directory_entry *entry) {
    // Read the parent directory inode
    read_inode(parent_inode);
    
    // Check if it's actually a directory
    uint16_t file_type = (inode.type_and_permissions >> 12) & 0xF;
    if (file_type != 0x4) {
        terminal_write("Error: Parent inode is not a directory!\n");
        return;
    }
    
    uint32_t block_size_bytes = 1024 << sb.block_size;
    uint32_t sectors_per_block = block_size_bytes / 512;
    uint8_t block_buffer[1024];
    
    // Search through all direct blocks to find space
    for (int block_idx = 0; block_idx < 12; block_idx++) {
        uint32_t block_num = inode.block[block_idx];
        
        // If block is empty, we need to allocate a new one
        if (block_num == 0) {
            uint32_t group_number = find_block_group_from_inode(parent_inode);
            block_num = find_free_block(group_number);
            update_block_bitmap(group_number, block_num, 1);
            
            // Update the inode's block pointer
            inode.block[block_idx] = block_num;
            inode.size_low += block_size_bytes;
            inode.sectors_count += sectors_per_block;
            
            // Clear the new block
            for (int i = 0; i < 1024; i++) {
                block_buffer[i] = 0;
            }
            
            // Add the entry at the beginning
            memcpy(block_buffer, entry, 8 + entry->name_length);
            
            // Write the block back
            uint32_t sector = block_num * sectors_per_block;
            write(sector, 1024, block_buffer);
            
            // Write back the updated inode
            edit_inode_table(parent_inode, &inode);
            return;
        }
        
        // Read existing block
        uint32_t sector = block_num * sectors_per_block;
        read(sector, 1024, block_buffer);
        
        // Search for free space in this block
        uint32_t offset = 0;
        while (offset < block_size_bytes) {
            struct ext2_directory_entry *current = (struct ext2_directory_entry *)(block_buffer + offset);
            
            // End of entries - we can add here
            if (current->inode == 0 || current->size == 0) {
                // Calculate the space needed (must be aligned to 4 bytes)
                uint16_t entry_size = 8 + entry->name_length;
                if (entry_size % 4 != 0) {
                    entry_size += 4 - (entry_size % 4);
                }
                entry->size = entry_size;
                
                // Check if we have enough space
                if (offset + entry_size <= block_size_bytes) {
                    memcpy(block_buffer + offset, entry, 8 + entry->name_length);
                    
                    // Write the block back
                    write(sector, 1024, block_buffer);
                    
                    // Update directory size if needed
                    uint32_t new_size = offset + entry_size;
                    if (new_size > inode.size_low) {
                        inode.size_low = new_size;
                        edit_inode_table(parent_inode, &inode);
                    }
                    
                    return;
                }
                break; // Not enough space in this block
            }
            
            // Check if current entry has extra space we can steal
            uint16_t actual_size = 8 + current->name_length;
            if (actual_size % 4 != 0) {
                actual_size += 4 - (actual_size % 4);
            }
            
            if (current->size > actual_size) {
                // There's unused space after this entry
                uint16_t available = current->size - actual_size;
                uint16_t needed = 8 + entry->name_length;
                if (needed % 4 != 0) {
                    needed += 4 - (needed % 4);
                }
                
                if (available >= needed) {
                    // Shrink current entry to actual size
                    current->size = actual_size;
                    
                    // Add new entry right after
                    entry->size = available;
                    memcpy(block_buffer + offset + actual_size, entry, 8 + entry->name_length);
                    
                    // Write the block back

                    write(sector, 1024, block_buffer);
                    
                    return;
                }
            }
            
            offset += current->size;
            
            // Safety check
            if (current->size < 8) {
                break;
            }
        }
    }
    
    terminal_write("Error: No space available in directory!\n");
}

void edit_inode_table(uint32_t inode_number, struct ext2_inode *new_inode) {
    uint32_t block_group = find_block_group_from_inode(inode_number);
    
    uint32_t index_in_group = inode_number % sb.inodes_per_group;

    uint32_t inode_table_block = bgdt[block_group].inode_table;
    
    uint32_t block_size_bytes = 1024 << sb.block_size;
    uint32_t inode_size = 128;
    uint32_t inodes_per_block = block_size_bytes / inode_size;
    uint32_t block_offset = index_in_group / inodes_per_block;
    uint32_t inode_offset_in_block = (index_in_group % inodes_per_block) * inode_size;
    
    uint32_t target_block = inode_table_block + block_offset;
    
    uint32_t sectors_per_block = block_size_bytes / 512;
    uint32_t sector = target_block * sectors_per_block;
    
    uint8_t buffer[1024];
    read(sector, 1024, buffer);
    
    memcpy(buffer + inode_offset_in_block, new_inode, sizeof(struct ext2_inode));

    write(sector, 1024, buffer);
}

void update_blockgroup_descriptor(uint32_t group_number, uint32_t delta_inodes, uint32_t delta_blocks){
    bgdt[group_number].free_blocks_count += delta_blocks;
    bgdt[group_number].free_inodes_count += delta_inodes;

    uint8_t buffer[1024];

    memcpy(buffer, bgdt, sizeof(bgdt));

    write(4, 1024, buffer);
}

void update_block_bitmap(uint32_t group_number, uint32_t block_number, uint8_t new_value){
    uint32_t block_bitmap_block_address = bgdt[group_number].block_usage_bitmap;

    uint32_t block_size_bytes = 1024 << sb.block_size;
    uint32_t sectors_per_block = block_size_bytes / 512;

    uint8_t block_bitmap[1024];
    uint32_t sector = block_bitmap_block_address * sectors_per_block;
    read(sector, 1024, block_bitmap);

    uint32_t block_index = block_number % sb.blocks_per_group;  
    uint32_t byte_idx = block_index / 8;
    uint8_t bit_pos = block_index % 8;
    uint8_t byte = block_bitmap[byte_idx];
    if (new_value) {
        byte |= (1 << bit_pos);
    } else {
        byte &= ~(1 << bit_pos);
    }

    block_bitmap[byte_idx] = byte;

    write(sector, 1024, block_bitmap);

    if (new_value) {
        update_blockgroup_descriptor(group_number, 0, -1);
        update_superblock(0, -1);
    } else {
        update_blockgroup_descriptor(group_number, 0, 1);
        update_superblock(0, 1);
    }
}

void update_inode_bitmap(uint32_t group_number, uint32_t inode_number, uint8_t new_value) {
    uint32_t inode_bitmap_block_address = bgdt[group_number].inode_usage_bitmap;
    
    uint32_t block_size_bytes = 1024 << sb.block_size;
    uint32_t sectors_per_block = block_size_bytes / 512;
    
    uint8_t inode_bitmap[1024];
    uint32_t sector = inode_bitmap_block_address * sectors_per_block;
    read(sector, 1024, inode_bitmap);
    
    uint32_t inode_index = (inode_number - 1) % sb.inodes_per_group;
    
    uint32_t byte_idx = inode_index / 8;
    uint8_t bit_pos = inode_index % 8;
    
    uint8_t byte = inode_bitmap[byte_idx];
    
    if (new_value) {
        byte |= (1 << bit_pos);
    } else {
        byte &= ~(1 << bit_pos);
    }
    
    inode_bitmap[byte_idx] = byte;

    write(sector, 1024, inode_bitmap);
    
    if (new_value) {
        update_blockgroup_descriptor(group_number, -1, 0);
        update_superblock(-1, 0);
    } else {
        update_blockgroup_descriptor(group_number, 1, 0);
        update_superblock(1, 0);
    }
}


/*
void read_inode_bitmap(uint32_t group_number) {
    // Get the inode bitmap block address
    uint32_t inode_bitmap_block_address = bgdt[group_number].inode_usage_bitmap;
    
    terminal_write("\n=== Inode Bitmap for Group ");
    terminal_write_dec(group_number);
    terminal_write(" ===\n");
    terminal_write("Bitmap Block Address: ");
    terminal_write_dec(inode_bitmap_block_address);
    terminal_write("\n\n");
    
    // Read the bitmap block
    uint32_t block_size_bytes = 1024 << sb.block_size;
    uint32_t sectors_per_block = block_size_bytes / 512;
    uint8_t inode_bitmap[1024];
    
    uint32_t sector = inode_bitmap_block_address * sectors_per_block;
    ata_read_sector(sector, inode_bitmap);
    ata_read_sector(sector + 1, inode_bitmap + 512);
    
    // Print status of first 10 inodes
    terminal_write("First 10 inodes:\n");
    for (uint32_t i = 0; i < 15; i++) {
        uint32_t byte_idx = i / 8;
        uint8_t bit_pos = i % 8;
        uint8_t byte = inode_bitmap[byte_idx];
        uint8_t bit_value = (byte >> bit_pos) & 1;
        
        // Calculate actual inode number
        uint32_t inode_num = group_number * sb.inodes_per_group + i + 1;
        
        terminal_write("  Inode ");
        terminal_write_dec(inode_num);
        terminal_write(": ");
        
        if (bit_value) {
            terminal_set_color(0xFF0000);
            terminal_write("ALLOCATED");
        } else {
            terminal_set_color(0x00FF00);  // Green for free
            terminal_write("FREE");
        }
        terminal_set_color(0xFFFFFF);  // Reset to white
        terminal_write("\n");
    }
    
    terminal_write("\n");
}
*/

uint32_t find_first_free_group(void){
    for (uint32_t i = 0; i < 32; i++) {
        if (bgdt[i].free_blocks_count > 0 && bgdt[i].free_inodes_count > 0) {
            return i;
        }
    }
    return -1;
}

uint32_t find_free_block(uint32_t group_number){
    uint32_t block_bitmap_block_address = bgdt[group_number].block_usage_bitmap;
    uint32_t block_size_bytes = 1024 << sb.block_size;
    uint32_t sectors_per_block = block_size_bytes / 512;

    uint8_t block_bitmap[1024];
    uint32_t sector = block_bitmap_block_address * sectors_per_block;
    read(sector, 1024, block_bitmap);

    for (uint32_t i = 0; i < sb.blocks_per_group; i++) {
        uint32_t byte_idx = i / 8;
        uint8_t bit_pos = i % 8;
        uint8_t byte = block_bitmap[byte_idx];
        uint8_t bit_value = (byte >> bit_pos) & 1;

        if (bit_value == 0) {
            return group_number * sb.blocks_per_group + i;
        }
    }
}

uint32_t find_free_inode(uint32_t group_number){
    uint32_t inode_bitmap_block_address = bgdt[group_number].inode_usage_bitmap;
    uint32_t block_size_bytes = 1024 << sb.block_size;
    uint32_t sectors_per_block = block_size_bytes / 512;

    uint8_t inode_bitmap[1024];
    uint32_t sector = inode_bitmap_block_address * sectors_per_block;
    read(sector, 1024, inode_bitmap);

    for (uint32_t i = 0; i < sb.inodes_per_group; i++) {
        uint32_t byte_idx = i / 8;
        uint8_t bit_pos = i % 8;
        uint8_t byte = inode_bitmap[byte_idx];
        uint8_t bit_value = (byte >> bit_pos) & 1;

        if (bit_value == 0) {
            return group_number * sb.inodes_per_group + i + 1;
        }
    }
}

void read_directory_entries(uint32_t inode_number) {

    read_inode(inode_number);
    
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
    
    for (int block_idx = 0; block_idx < 12; block_idx++) {
        uint32_t block_num = inode.block[block_idx];
        
        if (block_num == 0) {
            break;
        }
        
        uint32_t sector = block_num * sectors_per_block;
        read(sector, 1024, block_buffer);
        
        uint32_t offset = 0;
        while (offset < block_size_bytes) {
            struct ext2_directory_entry *entry = (struct ext2_directory_entry *)(block_buffer + offset);
            
            if (entry->inode == 0 || entry->size == 0) {
                break;
            }
            
            terminal_write("Inode: ");
            terminal_write_dec(entry->inode);
            terminal_write("\t");
            
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
            
            for (int i = 0; i < entry->name_length && i < 255; i++) {
                terminal_putchar(entry->name[i]);
            }
            terminal_write("\n");
            
            offset += entry->size;
            
            if (entry->size < 8) {
                terminal_write("Warning: Invalid entry size, stopping\n");
                break;
            }
        }
    }
    
    if (inode.singly_indirect != 0) {
        terminal_write("\n(Note: This directory has indirect blocks - not yet implemented)\n");
    }
}

void update_superblock(uint32_t delta_inodes, uint32_t delta_blocks){

    sb.total_unallocated_blocks += delta_blocks;
    sb.total_unallocated_inodes += delta_inodes;

    uint8_t buffer[1024];

    memcpy(buffer, &sb, sizeof(struct ext2_superblock));

    write(2, 1024, buffer);
}



void parse_blockgroup_descriptors(void) {
    uint8_t buffer[1024];

    read(4, 1024, buffer);

    memcpy(bgdt, buffer, sizeof(bgdt));

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

    read(2, 1024, buffer);

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

    uint32_t block_size_bytes = 1024 << sb.block_size;
    terminal_write("Block Size: ");
    terminal_write_dec(block_size_bytes);
    terminal_write(" bytes (log2: ");
    terminal_write_dec(sb.block_size);
    terminal_write(")\n");
    
    uint32_t sectors_per_block = block_size_bytes / 512;
    terminal_write("Sectors per Block: ");
    terminal_write_dec(sectors_per_block);
    terminal_write("\n");

    uint64_t total_bytes = (uint64_t)sb.total_blocks * block_size_bytes;
    uint64_t total_mb = total_bytes / (1024 * 1024);
    terminal_write("Total Size: ");
    terminal_write_dec(total_mb);
    terminal_write(" MB\n");
    
    uint64_t free_bytes = (uint64_t)sb.total_unallocated_blocks * block_size_bytes;
    uint64_t free_mb = free_bytes / (1024 * 1024);
    terminal_write("Free Space: ");
    terminal_write_dec(free_mb);
    terminal_write(" MB\n");
    
    terminal_write("\n");

}

uint32_t find_block_group_from_inode(uint32_t inode){
    return (inode - 1) / sb.inodes_per_group;
}
