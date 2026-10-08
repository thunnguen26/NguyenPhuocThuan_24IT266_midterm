#ifndef TRAVERSE_H
#define TRAVERSE_H

#include "ls.h"

/*
 * Separate operands [first, argc) into plain files and directories.
 * Returns the number of operands that could not be accessed.
 */
int split_operands(int argc, char **argv, int first,
                   ls_entrylist *files, ls_entrylist *dirs);

/* Read the names in directory path into out (unsorted). Returns 0 or -1. */
int read_directory(const char *path, ls_entrylist *out);

/* Top level driver: separate operands, list files, then each directory. */
int handle_operands(int argc, char **argv, int first);

#endif /* TRAVERSE_H */
