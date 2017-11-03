//
// Created by kkyse on 11/2/2017.
//

#include "file_stats.h"

#include <stdio.h>
#include <string.h>

#include "listdir.h"

static char file_perms_str[10 + 1] = {0};

#define bit(n, i) (((n) >> (i)) & 1)

static inline void one_file_perm(const mode_t mode, const uint8_t index) {
    file_perms_str[index + 0] = (char) (bit(mode, 2) ? 'r' : '-');
    file_perms_str[index + 1] = (char) (bit(mode, 1) ? 'w' : '-');
    file_perms_str[index + 2] = (char) (bit(mode, 0) ? 'x' : '-');
}

const char *file_perms(const mode_t mode) {
//    for (int i = sizeof(mode) * 8 - 1; i >= 0; i--) {
//        printf("%d, ", bit(mode, i));
//    }
    file_perms_str[0] = (char) (bit(mode, 15) ? '-' : 'd');
    one_file_perm(mode >> 0, 7);
    one_file_perm(mode >> 3, 4);
    one_file_perm(mode >> 6, 1);
    return file_perms_str;
}

static char format_size_str[4 + 3 + 1 + 2 + 1] = {0};

static const char *prefixes[] = {" B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB"};

const char *format_size(const off_t size) {
//    printf("%zd\n", size);
    if (size == 0) {
        return "0.00  B";
    }
    const size_t n = (const size_t) size;
    uint8_t i = 0;
    for (; i < sizeof(n) * 8; i += 10) {
        if ((n >> i) == 0) {
            i -= 10;
            break;
        }
    }
//    const uint16_t sig_figs = (uint16_t) (n >> i);
    const float sig_figs = ((float) n) / (1u << i);
    sprintf(format_size_str, "%4.3g %s", sig_figs, prefixes[i / 10]);
    return format_size_str;
}

// same as asctime (used in ctime), except no newline
char time_str_buf[3 + 1 + 3 + 1 + 20 + 1 + 20 + 1 + 20 + 1 + 20 + 1 + 20 + 1] = {0};

char *time_str(const time_t time) {
    struct tm *tm = localtime(&time);
    strftime(time_str_buf, sizeof(time_str_buf), "%c", tm);
    return time_str_buf;
}

bool is_dir(const struct stat *const stats) {
    return !bit(stats->st_mode, 15);
}

off_t file_size(const char *const path) {
    struct stat stats;
    if (stat(path, &stats) == -1) {
        perror("stat");
        return -1;
    }
    return stats.st_size;
}

off_t print_file_stats(const char *const path, const struct stat *const stats) {
    const char *const filename = strrchr(path, '/');
    const char null = 0;
    printf("%s %s %s %s\n",
           file_perms(stats->st_mode),
           format_size(stats->st_size),
           time_str(stats->st_atime),
           filename ? filename + 1 : &null
    );
    return 0;
}

off_t print_file_stats_recursive(const char *const path, const struct stat *const stats, const bool find_dir_size) {
    char dir_size_str[sizeof(format_size_str)];
    off_t dir_size = -1;
    if (find_dir_size && is_dir(stats)) {
        dir_size = get_dir_size(path);
        const char *const tmp = format_size(dir_size);
        strcpy(dir_size_str, tmp);
    } else {
        strcpy(dir_size_str, "       ");  // 7 spaces
    }
    
    const char *const filename = strrchr(path, '/');
    const char null = 0;
    
    printf("%s %s %s %s %s\n",
           file_perms(stats->st_mode),
           format_size(stats->st_size),
           dir_size_str,
           time_str(stats->st_atime),
           filename ? filename + 1 : &null
    );
    
    return dir_size == -1 ? 0 : dir_size;
}

off_t print_file_stats_for_path(const char *const path) {
    struct stat stats;
    if (stat(path, &stats) == -1) {
        perror("stat");
        return -1;
    }
    return print_file_stats_recursive(path, &stats, -1);
}