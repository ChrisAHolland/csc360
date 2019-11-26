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
*
*   Sources:
*   codewiki.wikidot.com/c:system-calls:fstat
*   www.tutorialspoint.com/c_standard_library/c_function_memcpy.html
*   www.tutorialsdaddy.com/courses/linux-device-driver/lessons/mmap/
*/
void diskinfo(int argc, char** argv) {
    int fd = open(argv[1], O_RDWR);
    struct stat fileStats;

    fstat(fd, &fileStats);

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
    char* dirName = argv[2];
    
    // Tokenize the input directory
    char* tokens[128];
    char* directory = argv[2];
    int dirs = 0;
    if (argc == 3) {
        char* args = strtok(directory, "/");
        int i = 0;
        while (args) {
            tokens[i++] = args;
            args =  strtok(NULL, "/");
            dirs++;
        }
        tokens[i] = NULL;
    }
    
    // Open and assemble the disk image
    int fd = open(argv[1], O_RDWR);
    struct stat fileStats;

    fstat(fd, &fileStats);

    char* data = mmap(NULL, fileStats.st_size, PROT_WRITE|PROT_READ, MAP_SHARED, fd, 0);
    struct superblock_t* superBlock;
    superBlock = (struct superblock_t*) data;

    int blockSize, rootStart = 0;
    blockSize = htons(superBlock->block_size);
    rootStart = ntohl(superBlock->root_dir_start_block) * blockSize;

    struct dir_entry_t* root_block;

    if (argc == 3 && strcmp(dirName, "/")) {
        int offSet = rootStart;
        int curr = 0;
        while(1) {
            for (int i = offSet; i < offSet + blockSize; i += 64) {
                root_block = (struct dir_entry_t*) (data+i);
                const char* name = (const char*)root_block->filename;
                if (!strcmp(name, tokens[curr])) {
                    curr++;
                    offSet = ntohl(root_block->starting_block) * blockSize;
                    if (curr == dirs || argc == 2 || !strcmp(dirName, "/")) {
                        for (int j = offSet; j < offSet + blockSize; j += 64) {
                            root_block = (struct dir_entry_t*) (data + j);
                            if (ntohl(root_block->size) == 0) continue;
                            int status = root_block->status;
                            if (status == 3) status = 'F';
                            else if (status == 5) status = 'D';
                            int size = ntohl(root_block->size);
                            unsigned char* name = root_block->filename;
                            int year = htons(root_block->modify_time.year);
                            int month = root_block->modify_time.month;
                            int day = root_block->modify_time.day;
                            int hours = root_block->modify_time.hour;
                            int minutes = root_block->modify_time.minute;
                            int seconds = root_block->modify_time.second;
                            printf("%c %10d %30s %4d/%02d/%02d %02d:%02d:%02d\n",status,size,name,year,month,day,hours,minutes,seconds);
                        }
                        return;
                    }
                    break;
                }
            }
        }
    }

    if (argc == 2 || !strcmp(dirName, "/")) {
        for (int i = rootStart; i <= rootStart+blockSize; i += 64) {
            root_block = (struct dir_entry_t*) (data+i);
            if (ntohl(root_block->size) == 0) continue;
            int status = root_block->status;
            if (status == 3) status = 'F';
            else if (status == 5) status = 'D';
            int size = ntohl(root_block->size);
            unsigned char* name = root_block->filename;
            int year = htons(root_block->modify_time.year);
            int month = root_block->modify_time.month;
            int day = root_block->modify_time.day;
            int hours = root_block->modify_time.hour;
            int minutes = root_block->modify_time.minute;
            int seconds = root_block->modify_time.second;
            printf("%c %10d %30s %4d/%02d/%02d %02d:%02d:%02d\n",status,size,name,year,month,day,hours,minutes,seconds);
        }
    }
}

/*
*   Part 3: diskget
*/
void diskget(int argc, char** argv) {
    //char* dirName = argv[2];
    char* filename = argv[3];
    
    // Tokenize the input directory
    char* tokens[128];
    char* directory = argv[2];
    int dirs = 0;
    
    char* args = strtok(directory, "/");
    int i = 0;
    while (args) {
        tokens[i++] = args;
        args =  strtok(NULL, "/");
        dirs++;
    }
    tokens[i] = NULL;
    
    
    // Open and assemble the disk image
    int fd = open(argv[1], O_RDWR);
    struct stat fileStats;

    fstat(fd, &fileStats);

    char* data = mmap(NULL, fileStats.st_size, PROT_WRITE|PROT_READ, MAP_SHARED, fd, 0);
    struct superblock_t* superBlock;
    superBlock = (struct superblock_t*) data;

    int blockSize, rootStart = 0;
    blockSize = htons(superBlock->block_size);
    rootStart = ntohl(superBlock->root_dir_start_block) * blockSize;

    struct dir_entry_t* root_block;

    int offSet = rootStart;
    int curr = 0;
    while(1) {
        for (int i = offSet; i < offSet + blockSize; i += 64) {
            root_block = (struct dir_entry_t*) (data+i);
            const char* name = (const char*)root_block->filename;
            if (!strcmp(name, tokens[curr])) {
                curr++;
                offSet = ntohl(root_block->starting_block) * blockSize;
                if (curr == dirs) {
                    for (int j = offSet; j < offSet + blockSize; j += 64) {
                        root_block = (struct dir_entry_t*) (data + offSet);
                        
                        FILE *fp;
                        fp = fopen(filename, "wb");
                        fwrite(root_block->filename-7, 1, blockSize-20, fp);
                        close(fd);
                        break;
                    }
                    return;
                }
                break;
            }
            if (curr == (dirs-1)) {
                printf("File not found.\n");
                return;
            }
        }
    }
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