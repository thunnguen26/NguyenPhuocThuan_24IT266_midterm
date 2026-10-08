#ifndef LS_H
#define LS_H

#include <sys/stat.h>
#include <stddef.h>

/*
 * MinGW provides no lstat() and its struct stat lacks st_blocks; map onto
 * the available equivalents so the tree builds there. On POSIX targets
 * these are inert and the real interfaces are used.
 */
#if defined(_WIN32) && !defined(lstat)
#define lstat stat
#define LS_NO_ST_BLOCKS 1
#endif

/*
 * MinGW does not expose every POSIX mode macro under -std=c99 / _POSIX_C_SOURCE.
 * Supply the missing ones so mode formatting and -F suffixes stay portable.
 */
#ifndef S_IFMT
#define S_IFMT   0170000
#endif
#ifndef S_IFLNK
#define S_IFLNK  0120000
#endif
#ifndef S_IFSOCK
#define S_IFSOCK 0140000
#endif
#ifndef S_ISLNK
#define S_ISLNK(m) (((m) & S_IFMT) == S_IFLNK)
#endif
#ifndef S_ISSOCK
#define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)
#endif
#ifndef S_ISUID
#define S_ISUID 0004000
#endif
#ifndef S_ISGID
#define S_ISGID 0002000
#endif
#ifndef S_ISVTX
#define S_ISVTX 0001000
#endif

/*
 * Option flags. At most one member of each mutually exclusive pair is ever
 * set: parse_options() clears the loser so the last option on the command
 * line wins (see the precedence matrix in ARCHITECTURE.md).
 */
enum ls_flag {
    LS_SHOWALL    = 1u << 0,   /* -a */
    LS_SHOWALMOST = 1u << 1,   /* -A */
    LS_CTIME      = 1u << 2,   /* -c */
    LS_DIRASFILE  = 1u << 3,   /* -d */
    LS_TYPESUFFIX = 1u << 4,   /* -F */
    LS_NOSORT     = 1u << 5,   /* -f, implies -a */
    LS_HUMAN      = 1u << 6,   /* -h */
    LS_INODE      = 1u << 7,   /* -i */
    LS_KILOBYTES  = 1u << 8,   /* -k */
    LS_LONGNAME   = 1u << 9,   /* -l */
    LS_LONGNUM    = 1u << 10,  /* -n, implies -l */
    LS_QUESTION   = 1u << 11,  /* -q */
    LS_RECURSIVE  = 1u << 12,  /* -R */
    LS_REVERSE    = 1u << 13,  /* -r */
    LS_SIZESORT   = 1u << 14,  /* -S */
    LS_BLOCKS     = 1u << 15,  /* -s */
    LS_TIMESORT   = 1u << 16,  /* -t */
    LS_ATIME      = 1u << 17,  /* -u */
    LS_RAW        = 1u << 18   /* -w */
};

#define LS_OPT(flag)  ((g_opts.flags & (flag)) != 0u)
#define LS_LONGFORMAT (LS_OPT(LS_LONGNAME) || LS_OPT(LS_LONGNUM))
#define LS_SHOWHIDDEN (LS_OPT(LS_SHOWALL) || LS_OPT(LS_NOSORT))

typedef struct ls_entry {
    char       *name;    /* name as displayed */
    char       *path;    /* path used to stat() the entry */
    char       *target;  /* symlink target, resolved on demand */
    struct stat st;
    int         statted; /* st is meaningful only when non-zero */
} ls_entry;

typedef struct ls_entrylist {
    ls_entry *items;
    size_t    count;
    size_t    cap;
} ls_entrylist;

typedef struct ls_options {
    unsigned int flags;
} ls_options;

extern ls_options g_opts;

#endif /* LS_H */
