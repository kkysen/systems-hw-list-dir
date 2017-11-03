//
// Created by kkyse on 11/2/2017.
//

#include "listdir.h"

#include <dirent.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "file_stats.h"

#ifndef NAME_MAX
    #define NAME_MAX 256 - 1
#endif

static inline bool justify_dir_path(char *const dir_path) {
    const size_t len = strlen(dir_path);
    if (dir_path[len - 1] == '/') {
        dir_path[len - 1] = 0;
        return true;
    }
    return false;
}

static inline void restore_dir_path(char *const dir_path, const bool modified) {
    if (!modified) {
        return;
    }
    const size_t len = strlen(dir_path);
    dir_path[len] = '/';
}

static inline bool is_child_dir(const char *const filename) {
    const char *const fn = filename;
    return !(fn[0] == '.' && (fn[1] == 0 || (fn[1] == '.' && fn[2] == 0)));
}

typedef off_t (*const FileWalker)(
        const char *const file_path,
        const struct stat *const stats,
        const struct dirent *const file
);

off_t walk_dir(const char *const dir_path, const FileWalker walker) {
    const bool modified_dir_path = justify_dir_path((char *const) dir_path);
    DIR *const dir = opendir(dir_path);
    if (!dir) {
        perror("opendir");
        goto no_free_error;
    }
    
    const size_t dir_path_len = strlen(dir_path);
    char *const file_path = (char *) malloc((dir_path_len + NAME_MAX + 1 + 1) * sizeof(char));
    if (!file_path) {
        perror("malloc");
        goto error;
    }
    
    memcpy(file_path, dir_path, dir_path_len * sizeof(char));
    file_path[dir_path_len] = '/';
    char *const file_path_append = file_path + ((dir_path_len + 1) * sizeof(char));
    off_t size = 0;
    for (;;) {
        const struct dirent *const file = readdir(dir);
        if (!file) {
            if (errno) {
                perror("readdir");
                goto error;
            }
            break;
        }
        strcpy(file_path_append, file->d_name);
        struct stat stats;
        if (stat(file_path, &stats) == -1) {
            perror("stat");
            goto error;
        }
//        printf("adding size of %s to %s\n", file_path, dir_path);
        const bool is_child = is_child_dir(file->d_name);
        if (is_child) {
            size += stats.st_size; // don't add size of . and .. dirs
        }
        const off_t inner_size = walker(file_path, &stats, file);
        if (inner_size == -1) {
            perror("walker");
            goto error;
        }
        if (is_child) {
            size += inner_size;
        }
    }
    
    if (closedir(dir) == -1) {
        perror("closedir");
        goto error;
    }
    
    free(file_path);
    restore_dir_path((char *const) dir_path, modified_dir_path);
    return size;
    
    error:
    free(file_path);
    
    no_free_error:
    restore_dir_path((char *const) dir_path, modified_dir_path);
    return -1;
}

off_t recurse_dir_size(const char *const file_path, const struct stat *const stats, const struct dirent *const file) {
    if (is_dir(stats) && is_child_dir(file->d_name)) {
        return get_dir_size(file_path);
    }
    return 0;
}

off_t get_dir_size(const char *dir_path) {
    return walk_dir(dir_path, &recurse_dir_size);
}

off_t list_file(const char *const file_path, const struct stat *const stats, const struct dirent *const file) {
    return print_file_stats(file_path, stats);
}

off_t list_file_recursive(const char *const file_path, const struct stat *const stats, const struct dirent *const file) {
    const off_t size = print_file_stats_recursive(file_path, stats, is_child_dir(file->d_name));
    if (size == -1) {
        perror("print_file_stats_recursive");
    }
    return size;
}

off_t listdir(const char *const dir_path, const bool recurse) {
    printf("Listing directory info for directory \"%s\" (%s):\n\n", dir_path, recurse ? "recursive" : "flat");
    const off_t dir_size = walk_dir(dir_path, recurse ? &list_file_recursive : &list_file);
    if (dir_size == -1) {
        perror("walk_dir");
        return -1;
    }
    printf("\nTotal Directory Size of \"%s\": %s\n", dir_path, format_size(dir_size));
    return dir_size;
}

bool has_flag(const int argc, const char *const *const argv, const char *const flag) {
    if (argc <= 1) {
        return false;
    }
    return strcmp(flag, argv[1]) == 0;
}

int main(const int argc, const char *const *const argv) {
    if (has_flag(argc, argv, "-help")) {
        printf("Usage: ./listdir [-r] <directory>\n");
        printf("    -r: list directory sizes recursively\n");
        return 0;
    }
    const bool recurse = has_flag(argc, argv, "-r");
    const uint32_t dir_path_index = recurse ? 2 : 1;
    const char *const dir_path = argc <= dir_path_index ? "." : argv[dir_path_index];
    listdir(dir_path, recurse);
}