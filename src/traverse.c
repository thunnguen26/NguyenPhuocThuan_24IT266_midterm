#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ls.h"
#include "print.h"
#include "sort.h"
#include "traverse.h"
#include "util.h"

int split_operands(int argc, char **argv, int first,
                   ls_entrylist *files, ls_entrylist *dirs)
{
    int errors = 0;
    int i;

    for (i = first; i < argc; i++) {
        struct stat st;

        if (lstat(argv[i], &st) != 0) {
            ls_warn(argv[i]);
            errors++;
            continue;
        }

        /* -d keeps directories in the file group instead of descending. */
        if (S_ISDIR(st.st_mode) && !LS_OPT(LS_DIRASFILE)) {
            if (entrylist_add(dirs, argv[i], argv[i], &st) != 0)
                errors++;
        } else if (entrylist_add(files, argv[i], argv[i], &st) != 0) {
            errors++;
        }
    }
    return errors;
}

/*
 * Hidden entry policy: default hides every "." name, -A hides only "." and
 * "..", -a (and -f) shows everything.
 */
static int skip_entry(const char *name)
{
    if (name[0] != '.')
        return 0;
    if (LS_OPT(LS_SHOWALMOST) &&
        (strcmp(name, ".") == 0 || strcmp(name, "..") == 0))
        return 1;
    if (!LS_SHOWHIDDEN && !LS_OPT(LS_SHOWALMOST))
        return 1;
    return 0;
}

int read_directory(const char *path, ls_entrylist *out)
{
    DIR *dir = opendir(path);
    struct dirent *de;

    if (dir == NULL) {
        ls_warn(path);
        return -1;
    }

    while ((de = readdir(dir)) != NULL) {
        size_t len;
        char *full;
        struct stat st;

        if (skip_entry(de->d_name))
            continue;

        len = strlen(path) + strlen(de->d_name) + 2;
        full = malloc(len);
        if (full == NULL)
            break;
        snprintf(full, len, "%s/%s", path, de->d_name);

        if (lstat(full, &st) != 0) {
            ls_warn(full);
            free(full);
            continue;
        }
        if (entrylist_add(out, de->d_name, full, &st) != 0) {
            free(full);
            break;
        }
        free(full);
    }
    closedir(dir);
    return 0;
}

/*
 * List one directory, optionally printing its "<dir>:" header, then recurse
 * into its subdirectories for -R. "." and ".." are never descended, which
 * also prevents looping on entries that point at an ancestor.
 */
static int list_directory(const char *path, int header)
{
    ls_entrylist list = { NULL, 0, 0 };
    int errors = 0;
    size_t i;

    if (read_directory(path, &list) != 0)
        return 1;

    if (header)
        printf("%s:\n", path);

    print_total(&list);
    sort_entries(&list);
    print_entries(&list);

    if (LS_OPT(LS_RECURSIVE)) {
        for (i = 0; i < list.count; i++) {
            const ls_entry *e = &list.items[i];

            if (!S_ISDIR(e->st.st_mode))
                continue;
            if (strcmp(e->name, ".") == 0 || strcmp(e->name, "..") == 0)
                continue;
            putchar('\n');
            errors += list_directory(e->path, 1);
        }
    }

    entrylist_free(&list);
    return errors;
}

int handle_operands(int argc, char **argv, int first)
{
    ls_entrylist files = { NULL, 0, 0 };
    ls_entrylist dirs = { NULL, 0, 0 };
    int errors = split_operands(argc, argv, first, &files, &dirs);
    int header = (files.count + dirs.count > 1) || LS_OPT(LS_RECURSIVE);
    size_t i;

    if (files.count > 0) {
        sort_entries(&files);
        print_entries(&files);
    }

    if (files.count > 0 && dirs.count > 0)
        putchar('\n');

    for (i = 0; i < dirs.count; i++) {
        if (i > 0)
            putchar('\n');
        errors += list_directory(dirs.items[i].path, header);
    }

    entrylist_free(&files);
    entrylist_free(&dirs);
    return errors;
}
