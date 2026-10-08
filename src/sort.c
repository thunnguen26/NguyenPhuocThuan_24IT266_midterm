#include <stdlib.h>
#include <string.h>

#include "ls.h"
#include "sort.h"
#include "util.h"

static int compare_name(const void *a, const void *b)
{
    const ls_entry *ea = a;
    const ls_entry *eb = b;

    return strcmp(ea->name, eb->name);
}

static int compare_size(const void *a, const void *b)
{
    const ls_entry *ea = a;
    const ls_entry *eb = b;

    /* Descending by size, ties broken by name. */
    if (ea->st.st_size != eb->st.st_size)
        return ea->st.st_size < eb->st.st_size ? 1 : -1;
    return compare_name(a, b);
}

static int compare_time(const void *a, const void *b)
{
    const ls_entry *ea = a;
    const ls_entry *eb = b;
    time_t ta = entry_time(&ea->st);
    time_t tb = entry_time(&eb->st);

    /* Descending by time, ties broken by name. */
    if (ta != tb)
        return ta < tb ? 1 : -1;
    return compare_name(a, b);
}

static void reverse_entries(ls_entrylist *list)
{
    size_t i, j;

    for (i = 0, j = list->count - 1; i < j; i++, j--) {
        ls_entry tmp = list->items[i];
        list->items[i] = list->items[j];
        list->items[j] = tmp;
    }
}

void sort_entries(ls_entrylist *list)
{
    int (*cmp)(const void *, const void *) = compare_name;

    if (list->count < 2)
        return;

    /* -f leaves the readdir order untouched, but -r still reverses it. */
    if (!LS_OPT(LS_NOSORT)) {
        if (LS_OPT(LS_SIZESORT))
            cmp = compare_size;
        else if (LS_OPT(LS_TIMESORT))
            cmp = compare_time;

        qsort(list->items, list->count, sizeof(*list->items), cmp);
    }

    if (LS_OPT(LS_REVERSE))
        reverse_entries(list);
}
