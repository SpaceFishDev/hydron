#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <malloc.h>
#include <stdint.h>
#include <string.h>

char *read_whole_file(char *path);

char *find_input_file(int argc, char **argv);

#endif