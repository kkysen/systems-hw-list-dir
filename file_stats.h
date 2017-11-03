//
// Created by kkyse on 11/2/2017.
//

#ifndef SYSTEMS_FILE_STATS_H
#define SYSTEMS_FILE_STATS_H

#include <time.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdbool.h>
#include <sys/stat.h>

const char *file_perms(mode_t mode);

const char *format_size(off_t size);

off_t file_size(const char *path);

bool is_dir(const struct stat *stats);

off_t print_file_stats(const char *path, const struct stat *stats);

off_t print_file_stats_recursive(const char *path, const struct stat *stats, bool find_dir_size);

off_t print_file_stats_for_path(const char *path);

#endif //SYSTEMS_FILE_STATS_H