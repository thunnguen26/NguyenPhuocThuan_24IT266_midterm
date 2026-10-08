#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <grp.h>
#include <pwd.h>
#include <sys/sysmacros.h>
#endif

#include "ls.h"
#include "util.h"

static char *dup_string(const char *s)
{
    size_t n = strlen(s) + 1;
    char *copy = malloc(n);

    if (copy != NULL)
        memcpy(copy, s, n);
    return copy;
}

int entrylist_add(ls_entrylist *list, const char *name, const char *path,
                  const struct stat *st)
{
    ls_entry *entry;

    if (list->count == list->cap) {
        size_t cap = list->cap ? list->cap * 2 : 32;
        ls_entry *items = realloc(list->items, cap * sizeof(*items));

        if (items == NULL)
            return -1;
        list->items = items;
        list->cap = cap;
    }

    entry = &list->items[list->count];
    entry->name = dup_string(name);
    entry->path = dup_string(path);
    entry->target = NULL;
    entry->statted = 0;

    if (entry->name == NULL || entry->path == NULL) {
        free(entry->name);
        free(entry->path);
        return -1;
    }

    if (st != NULL) {
        entry->st = *st;
        entry->statted = 1;
    }
    list->count++;
    return 0;
}

void entrylist_free(ls_entrylist *list)
{
    size_t i;

    for (i = 0; i < list->count; i++) {
        free(list->items[i].name);
        free(list->items[i].path);
        free(list->items[i].target);
    }
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->cap = 0;
}

time_t entry_time(const struct stat *st)
{
    if (LS_OPT(LS_CTIME))
        return st->st_ctime;
    if (LS_OPT(LS_ATIME))
        return st->st_atime;
    return st->st_mtime;
}

/*
 * Block report unit. -k forces 1K; otherwise BLOCKSIZE may select a unit
 * (a byte count with an optional k/M/G suffix), defaulting to 512 bytes.
 * Computed once; the options are fully parsed before any listing.
 */
static long long report_blocksize(void)
{
    static int cached = 0;
    static long long size = 512;
    const char *env;
    char *end;
    long long v;

    if (cached)
        return size;
    cached = 1;

    if (LS_OPT(LS_KILOBYTES)) {
        size = 1024;
        return size;
    }

    env = getenv("BLOCKSIZE");
    if (env == NULL || *env == '\0')
        return size;

    v = strtoll(env, &end, 10);
    if (end == env)
        return size;

    switch (*end) {
    case 'k': case 'K': v *= 1024; break;
    case 'm': case 'M': v *= 1024 * 1024; break;
    case 'g': case 'G': v *= 1024LL * 1024 * 1024; break;
    default: break;
    }

    if (v > 0)
        size = v;
    return size;
}

/* Bytes allocated to st. Windows stat() has no st_blocks. */
long long alloc_bytes(const struct stat *st)
{
#ifdef LS_NO_ST_BLOCKS
    return ((long long)st->st_size + 511) / 512 * 512;
#else
    return (long long)st->st_blocks * 512;
#endif
}

long long blocks_of(const struct stat *st)
{
    long long bs = report_blocksize();
    long long bytes = alloc_bytes(st);

    return (bytes + bs - 1) / bs;
}

void format_size(long long bytes, char *buf, size_t bufsize)
{
    static const char units[] = "KMGTPE";
    double value = (double)bytes;
    int u = -1;

    while (value >= 1024.0 && u < 5) {
        value /= 1024.0;
        u++;
    }

    if (u < 0)
        snprintf(buf, bufsize, "%lld", bytes);
    else if (value < 10.0)
        snprintf(buf, bufsize, "%.1f%c", value, units[u]);
    else
        snprintf(buf, bufsize, "%.0f%c", value, units[u]);
}

void format_mode(mode_t mode, char *buf)
{
    static const char rwx[] = "rwxrwxrwx";
    int i;

    if (S_ISDIR(mode))
        buf[0] = 'd';
    else if (S_ISLNK(mode))
        buf[0] = 'l';
    else if (S_ISBLK(mode))
        buf[0] = 'b';
    else if (S_ISCHR(mode))
        buf[0] = 'c';
    else if (S_ISFIFO(mode))
        buf[0] = 'p';
    else if (S_ISSOCK(mode))
        buf[0] = 's';
    else
        buf[0] = '-';

    for (i = 0; i < 9; i++)
        buf[i + 1] = (mode & (1u << (8 - i))) ? rwx[i] : '-';

    /* SUID/SGID/sticky replace the execute letter in their triplet. */
    if (mode & S_ISUID)
        buf[3] = (mode & S_IXUSR) ? 's' : 'S';
    if (mode & S_ISGID)
        buf[6] = (mode & S_IXGRP) ? 's' : 'S';
    if (mode & S_ISVTX)
        buf[9] = (mode & S_IXOTH) ? 't' : 'T';

    buf[10] = '\0';
}

void format_user(const struct stat *st, char *buf, size_t bufsize)
{
#ifndef _WIN32
    if (!LS_OPT(LS_LONGNUM)) {
        struct passwd *pw = getpwuid(st->st_uid);

        if (pw != NULL) {
            snprintf(buf, bufsize, "%s", pw->pw_name);
            return;
        }
    }
#endif
    snprintf(buf, bufsize, "%lu", (unsigned long)st->st_uid);
}

void format_group(const struct stat *st, char *buf, size_t bufsize)
{
#ifndef _WIN32
    if (!LS_OPT(LS_LONGNUM)) {
        struct group *gr = getgrgid(st->st_gid);

        if (gr != NULL) {
            snprintf(buf, bufsize, "%s", gr->gr_name);
            return;
        }
    }
#endif
    snprintf(buf, bufsize, "%lu", (unsigned long)st->st_gid);
}

/*
 * Recent files show the time of day; anything older than roughly six months
 * (or dated in the future) shows the year instead, matching BSD ls.
 */
void format_time(time_t t, char *buf, size_t bufsize)
{
    static const char *months[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    struct tm *tm = localtime(&t);
    time_t now = time(NULL);
    const char *mon;

    if (tm == NULL) {
        snprintf(buf, bufsize, "???");
        return;
    }
    mon = months[tm->tm_mon];

    if (t > now || now - t > (time_t)15778476)
        snprintf(buf, bufsize, "%s %2d  %04d", mon, tm->tm_mday,
                 tm->tm_year + 1900);
    else
        snprintf(buf, bufsize, "%s %2d %02d:%02d", mon, tm->tm_mday,
                 tm->tm_hour, tm->tm_min);
}

void format_blocks(const struct stat *st, char *buf, size_t bufsize)
{
    if (LS_OPT(LS_HUMAN))
        format_size(alloc_bytes(st), buf, bufsize);
    else
        snprintf(buf, bufsize, "%lld", blocks_of(st));
}

void format_size_field(const struct stat *st, char *buf, size_t bufsize)
{
#ifndef _WIN32
    if (S_ISCHR(st->st_mode) || S_ISBLK(st->st_mode)) {
        snprintf(buf, bufsize, "%u, %u", (unsigned)major(st->st_rdev),
                 (unsigned)minor(st->st_rdev));
        return;
    }
#endif
    if (LS_OPT(LS_HUMAN))
        format_size((long long)st->st_size, buf, bufsize);
    else
        snprintf(buf, bufsize, "%lld", (long long)st->st_size);
}

void ls_warn(const char *path)
{
    fprintf(stderr, "ls: %s: %s\n", path, strerror(errno));
}
