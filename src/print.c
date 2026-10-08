#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ls.h"
#include "print.h"
#include "util.h"

typedef struct field_widths {
    int inode;
    int blocks;
    int nlink;
    int user;
    int group;
    int size;
} field_widths;

/*
 * Quote non-printable bytes as '?' when writing to a terminal (or -q);
 * -w forces raw bytes. Non-tty output defaults to raw.
 */
static int quoting_enabled(void)
{
    if (LS_OPT(LS_RAW))
        return 0;
    if (LS_OPT(LS_QUESTION))
        return 1;
    return isatty(STDOUT_FILENO) != 0;
}

static void print_name(const char *name)
{
    const unsigned char *p = (const unsigned char *)name;
    int quote = quoting_enabled();

    for (; *p != '\0'; p++) {
        if (quote && !isprint(*p))
            putchar('?');
        else
            putchar(*p);
    }
}

static char type_suffix(const struct stat *st)
{
    mode_t m = st->st_mode;

    if (S_ISDIR(m))
        return '/';
    if (S_ISLNK(m))
        return '@';
    if (S_ISSOCK(m))
        return '=';
    if (S_ISFIFO(m))
        return '|';
    if (S_ISBLK(m) || S_ISCHR(m))
        return '\0';
    if (m & (S_IXUSR | S_IXGRP | S_IXOTH))
        return '*';
    return '\0';
}

static void print_symlink_target(const ls_entry *entry)
{
#ifndef _WIN32
    char buf[1024];
    ssize_t n;

    if (!S_ISLNK(entry->st.st_mode))
        return;
    n = readlink(entry->path, buf, sizeof(buf) - 1);
    if (n < 0)
        return;
    buf[n] = '\0';
    fputs(" -> ", stdout);
    print_name(buf);
#else
    (void)entry;
#endif
}

static void compute_widths(const ls_entry *items, size_t count,
                           field_widths *w)
{
    size_t i;
    char buf[64];

    memset(w, 0, sizeof(*w));

    for (i = 0; i < count; i++) {
        const ls_entry *e = &items[i];
        int len;

        if (LS_OPT(LS_INODE)) {
            len = snprintf(buf, sizeof(buf), "%llu",
                           (unsigned long long)e->st.st_ino);
            if (len > w->inode)
                w->inode = len;
        }
        if (LS_OPT(LS_BLOCKS)) {
            format_blocks(&e->st, buf, sizeof(buf));
            len = (int)strlen(buf);
            if (len > w->blocks)
                w->blocks = len;
        }
        if (LS_LONGFORMAT) {
            len = snprintf(buf, sizeof(buf), "%lu",
                           (unsigned long)e->st.st_nlink);
            if (len > w->nlink)
                w->nlink = len;
            format_user(&e->st, buf, sizeof(buf));
            len = (int)strlen(buf);
            if (len > w->user)
                w->user = len;
            format_group(&e->st, buf, sizeof(buf));
            len = (int)strlen(buf);
            if (len > w->group)
                w->group = len;
            format_size_field(&e->st, buf, sizeof(buf));
            len = (int)strlen(buf);
            if (len > w->size)
                w->size = len;
        }
    }
}

static void print_prefix(const ls_entry *entry, const field_widths *w)
{
    char buf[64];

    if (LS_OPT(LS_INODE)) {
        snprintf(buf, sizeof(buf), "%llu",
                 (unsigned long long)entry->st.st_ino);
        printf("%*s ", w->inode, buf);
    }
    if (LS_OPT(LS_BLOCKS)) {
        format_blocks(&entry->st, buf, sizeof(buf));
        printf("%*s ", w->blocks, buf);
    }
}

static void print_suffix(const ls_entry *entry)
{
    char suffix;

    if (!LS_OPT(LS_TYPESUFFIX))
        return;
    suffix = type_suffix(&entry->st);
    if (suffix != '\0')
        putchar(suffix);
}

static void print_short(const ls_entry *entry, const field_widths *w)
{
    print_prefix(entry, w);
    print_name(entry->name);
    print_suffix(entry);
    putchar('\n');
}

static void print_long(const ls_entry *entry, const field_widths *w)
{
    char mode[11];
    char user[64];
    char group[64];
    char size[64];
    char when[64];

    format_mode(entry->st.st_mode, mode);
    format_user(&entry->st, user, sizeof(user));
    format_group(&entry->st, group, sizeof(group));
    format_size_field(&entry->st, size, sizeof(size));
    format_time(entry_time(&entry->st), when, sizeof(when));

    print_prefix(entry, w);
    printf("%s %*lu %-*s %-*s %*s %s ", mode,
           w->nlink, (unsigned long)entry->st.st_nlink,
           w->user, user,
           w->group, group,
           w->size, size,
           when);

    print_name(entry->name);
    print_suffix(entry);
    print_symlink_target(entry);
    putchar('\n');
}

void print_entry(const ls_entry *entry)
{
    field_widths w;

    compute_widths(entry, 1, &w);
    if (LS_LONGFORMAT)
        print_long(entry, &w);
    else
        print_short(entry, &w);
}

void print_entries(ls_entrylist *list)
{
    field_widths w;
    size_t i;

    compute_widths(list->items, list->count, &w);
    for (i = 0; i < list->count; i++) {
        if (LS_LONGFORMAT)
            print_long(&list->items[i], &w);
        else
            print_short(&list->items[i], &w);
    }
}

void print_total(const ls_entrylist *list)
{
    long long total = 0;
    char buf[64];
    size_t i;

    if (!LS_OPT(LS_BLOCKS) && !LS_LONGFORMAT)
        return;

    if (LS_OPT(LS_HUMAN)) {
        for (i = 0; i < list->count; i++)
            total += alloc_bytes(&list->items[i].st);
        format_size(total, buf, sizeof(buf));
    } else {
        for (i = 0; i < list->count; i++)
            total += blocks_of(&list->items[i].st);
        snprintf(buf, sizeof(buf), "%lld", total);
    }

    printf("total %s\n", buf);
}
