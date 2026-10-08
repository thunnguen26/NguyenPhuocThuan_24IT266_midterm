#ifndef PRINT_H
#define PRINT_H

#include "ls.h"

/* Print a single entry using the active output format. */
void print_entry(const ls_entry *entry);

/* Print every entry in list using the active output format. */
void print_entries(ls_entrylist *list);

/* Print the "total N" header for a directory listing (-l/-s). */
void print_total(const ls_entrylist *list);

#endif /* PRINT_H */
