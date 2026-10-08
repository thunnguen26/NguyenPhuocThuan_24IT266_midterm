#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ls.h"
#include "traverse.h"

ls_options g_opts = { 0 };

static void usage(void)
{
    fprintf(stderr, "usage: ls [-AacdFfhiklnqRrSstuw] [file ...]\n");
    exit(EXIT_FAILURE);
}

/*
 * All flags take no argument. Mutually exclusive options clear the other
 * member of their pair so that the last occurrence wins, matching the
 * override rules in ARCHITECTURE.md (-w over -q, -n over -l, -u over -c,
 * -d over -R, -k over -h, -a over -A).
 */
static void parse_options(int argc, char **argv)
{
    int ch;

    while ((ch = getopt(argc, argv, "AacdFfhiklnqRrSstuw")) != -1) {
        switch (ch) {
        case 'a':
            g_opts.flags |= LS_SHOWALL;
            g_opts.flags &= ~LS_SHOWALMOST;
            break;
        case 'A':
            g_opts.flags |= LS_SHOWALMOST;
            g_opts.flags &= ~LS_SHOWALL;
            break;
        case 'c':
            g_opts.flags |= LS_CTIME;
            g_opts.flags &= ~LS_ATIME;
            break;
        case 'u':
            g_opts.flags |= LS_ATIME;
            g_opts.flags &= ~LS_CTIME;
            break;
        case 'd':
            g_opts.flags |= LS_DIRASFILE;
            g_opts.flags &= ~LS_RECURSIVE;
            break;
        case 'R':
            g_opts.flags |= LS_RECURSIVE;
            g_opts.flags &= ~LS_DIRASFILE;
            break;
        case 'F':
            g_opts.flags |= LS_TYPESUFFIX;
            break;
        case 'f':
            /* -f turns off sorting and turns on -a. */
            g_opts.flags |= LS_NOSORT | LS_SHOWALL;
            g_opts.flags &= ~LS_SHOWALMOST;
            break;
        case 'h':
            g_opts.flags |= LS_HUMAN;
            g_opts.flags &= ~LS_KILOBYTES;
            break;
        case 'k':
            g_opts.flags |= LS_KILOBYTES;
            g_opts.flags &= ~LS_HUMAN;
            break;
        case 'i':
            g_opts.flags |= LS_INODE;
            break;
        case 'l':
            g_opts.flags |= LS_LONGNAME;
            g_opts.flags &= ~LS_LONGNUM;
            break;
        case 'n':
            g_opts.flags |= LS_LONGNUM;
            g_opts.flags &= ~LS_LONGNAME;
            break;
        case 'q':
            g_opts.flags |= LS_QUESTION;
            g_opts.flags &= ~LS_RAW;
            break;
        case 'w':
            g_opts.flags |= LS_RAW;
            g_opts.flags &= ~LS_QUESTION;
            break;
        case 'r':
            g_opts.flags |= LS_REVERSE;
            break;
        case 'S':
            g_opts.flags |= LS_SIZESORT;
            break;
        case 's':
            g_opts.flags |= LS_BLOCKS;
            break;
        case 't':
            g_opts.flags |= LS_TIMESORT;
            break;
        default:
            usage();
        }
    }
}

int main(int argc, char **argv)
{
    parse_options(argc, argv);

    /* No operands: list the current directory. */
    if (optind == argc) {
        char *defargv[] = { argv[0], (char *)"." };
        return handle_operands(2, defargv, 1) == 0 ? EXIT_SUCCESS
                                                   : EXIT_FAILURE;
    }

    return handle_operands(argc, argv, optind) == 0 ? EXIT_SUCCESS
                                                    : EXIT_FAILURE;
}
