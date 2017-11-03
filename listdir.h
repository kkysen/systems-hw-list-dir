//
// Created by kkyse on 11/2/2017.
//

#ifndef SYSTEMS_LISTDIR_H
#define SYSTEMS_LISTDIR_H

#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>

off_t get_dir_size(const char *dir_path);

off_t listdir(const char *dir_path, const bool recurse);

#endif //SYSTEMS_LISTDIR_H
