#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <time.h>

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
    printf("Part 1\n");
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