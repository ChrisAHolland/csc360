#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>

// Supper Block Struct
struct __attribute__((__packed__)) superblock_t {
    uint8_t fs_id[8];
    uint16_t block_size;
    uint32_t file_system_block_count;
    uint32_t fat_start_block;
    uint32_t fat_block_count;
    uint32_t root_dir_start_block;
    uint32_t root_dir_block_count;
};

// Time and Date Entry Struct
struct __attribute__((__packed__)) dir_entry_timedate_t {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

// Directory Entry Struct
struct __attribute__((__packed__)) dir_entry_t {
    uint8_t status;
    uint32_t starting_block;
    uint32_t block_count;
    uint32_t size;
    struct dir_entry_timedate_t create_time;
    struct dir_entry_timedate_t modify_time;
    uint8_t filename[31];
    uint8_t unused[6];
};

/*
*   Part 1: diskinfo
*/
void diskinfo(int argc, char** argv) {
    if (argc < 2) return 1;
    
    int fd = open(argv[1], O_RDWR);
    struct stat fileStats;

    if (fstat(fd, &fileStats) < 0) return 1;

    char* data = mmap(NULL, fileStats.st_size, PROT_WRITE|PROT_READ, MAP_SHARED, fd, 0);
    struct superblock_t* superBlock;
    superBlock = (struct superblock_t*) data;

    int blockSize = htons(superBlock->block_size);
    int fatStart = ntohl(superBlock->fat_start_block);
    int fatCount = ntohl(superBlock->fat_block_count);

    printf("Super block information:\n");
    printf("Block size: %d\n", blockSize);
    printf("Block count: %d\n", ntohl(superBlock->file_system_block_count));
    printf("FAT starts: %d\n", fatStart);
    printf("FAT blocks: %d\n", fatCount);
    printf("Root directory start: %d\n", ntohl(superBlock->root_dir_start_block));
    printf("Root directory blocks: %d\n\n", ntohl(superBlock->root_dir_block_count));

    int startBlock = fatStart * blockSize;
    int endBlock = fatCount * blockSize;

    int freeBlocks, reservedBlocks, allocBlocks = 0;

    for (int i = startBlock; i < startBlock + endBlock; i += 4) {
        int temp = 0;
        memcpy(&temp, data + i, 4);
        temp = htonl(temp);
        if(temp == 0) {
            freeBlocks++;
        } else if (temp == 1) {
            reservedBlocks++;
        } else {
            allocBlocks++;
        }
    }

    printf("FAT information:\n");
    printf("Free Blocks: %d\n", freeBlocks);
    printf("Reserved Blocks: %d\n", reservedBlocks);
    printf("Allocated Blocks: %d\n", allocBlocks);

    close(fd);
}

/*
*   Part 2: disklist
*/
void disklist(int argc, char** argv) {
    printf("Part 2\n");
}

/*
*   Part 3: diskget
*/
void diskget(int argc, char** argv) {
    printf("Part 3\n");
}

/*
*   Part 4: diskput
*/
void diskput(int argc, char** argv) {
    printf("Part 4\n");
}

/*
*   Part 5: diskfix
*/
void diskfix(int argc, char** argv) {
    printf("Part 5\n");
}

int main(int argc, char* argv[]) {
    #if defined(PART1)
        diskinfo(argc, argv);
    #elif defined(PART2)
        disklist(argc, argv);
    #elif defined(PART3)
        diskget(argc, argv);
    #elif defined(PART4)
        diskput(argc, argv);
    #elif defined(PART5)
        diskfix(argc, argv);
    #else
    #   error "PART[12345] must be defined"
    #endif
        return 0;
}