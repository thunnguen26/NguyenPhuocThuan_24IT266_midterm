#ifndef UTIL_H
#define UTIL_H

#include <time.h>

#include "ls.h"

/* Append copies of name/path (st may be NULL) to list. Returns 0 or -1. */
int  entrylist_add(ls_entrylist *list, const char *name, const char *path,
                   const struct stat *st);
void entrylist_free(ls_entrylist *list);

/* Timestamp selected by -c/-u, defaulting to the modification time. */
time_t entry_time(const struct stat *st);

/* Bytes allocated to st (st_blocks scaled, or a size fallback on Windows). */
long long alloc_bytes(const struct stat *st);

/* Block count for st in the active report unit (-k / BLOCKSIZE / 512). */
long long blocks_of(const struct stat *st);

/* Format a byte count into buf as a human readable size for -h. */
void format_size(long long bytes, char *buf, size_t bufsize);

/* Long-format columns. */
void format_mode(mode_t mode, char *buf);
void format_user(const struct stat *st, char *buf, size_t bufsize);
void format_group(const struct stat *st, char *buf, size_t bufsize);
void format_time(time_t t, char *buf, size_t bufsize);
void format_blocks(const struct stat *st, char *buf, size_t bufsize);
void format_size_field(const struct stat *st, char *buf, size_t bufsize);

/* Report "ls: <path>: <errno>" on stderr. */
void ls_warn(const char *path);

#endif /* UTIL_H */
