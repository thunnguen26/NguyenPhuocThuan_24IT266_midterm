# Midterm Project: UNIX ls(1) Implementation

> **Author:** Nguyễn Phước Thuần
> **Student ID:** 24IT266
> **Course:** Lập Trình Hệ Thống
> **Repository:** https://github.com/thunnguen26/NguyenPhuocThuan_24IT266_midterm
> **Submission Date:** 2026-10-08

A modular re-implementation of the UNIX `ls(1)` utility in C (C99 / POSIX.1-2008),
modeled on the NetBSD manual specification and built strictly without warnings.

---

## 1. Quickstart & Installation (Hướng dẫn tải và biên dịch)

### Bước 1 — Tải mã nguồn về máy (Clone repository)

```sh
git clone https://github.com/thunnguen26/NguyenPhuocThuan_24IT266_midterm.git
cd NguyenPhuocThuan_24IT266_midterm
```

### Bước 2 — Biên dịch (Build)

```sh
make clean && make
```

The build uses `-Wall -Wextra -pedantic -std=c99` and produces the `ls` binary in
the project root without any warnings or errors.

**Requirements:** a C99 compiler (GCC or Clang) and a POSIX.1-2008 environment
(Linux, macOS, or BSD). Windows builds are supported through MinGW for convenience.

**Other targets:**

```sh
make        # build (incremental)
make clean  # remove object files, dependency files and the binary
```

---

## 2. Usage & Sample Output (Cách sử dụng & Ví dụ)

The examples below use a small project directory; output is illustrative and
follows the program's actual formatting.

### Long listing of all entries (`-la`)

```sh
$ ./ls -la
total 52
drwxr-xr-x  6 thuan thuan  4096 Oct  8 14:20 .
drwxr-xr-x 12 thuan thuan  4096 Oct  8 13:50 ..
-rw-r--r--  1 thuan thuan   312 Oct  7 09:15 .gitignore
-rw-r--r--  1 thuan thuan   706 Oct  7 09:15 Makefile
-rw-r--r--  1 thuan thuan   907 Oct  7 09:15 README.md
drwxr-xr-x  2 thuan thuan  4096 Oct  8 14:04 build
drwxr-xr-x  2 thuan thuan  4096 Oct  8 14:04 include
-rwxr-xr-x  1 thuan thuan 28704 Oct  8 14:07 ls
-rw-r--r--  1 thuan thuan  3304 Oct  8 14:07 main.c
drwxr-xr-x  2 thuan thuan  4096 Oct  8 14:04 src
-rw-r--r--  1 thuan thuan  6592 Oct  8 14:07 util.c
```

### Size-sorted, human-readable long listing (`-lSh`)

```sh
$ ./ls -lSh
total 1.1M
-rwxr-xr-x 1 thuan thuan 287K Oct  8 14:07 ls
-rw-r--r-- 1 thuan thuan 6.6K Oct  8 14:07 util.c
-rw-r--r-- 1 thuan thuan 5.5K Oct  8 14:07 print.c
-rw-r--r-- 1 thuan thuan 3.3K Oct  8 14:07 main.c
-rw-r--r-- 1 thuan thuan 2.6K Oct  8 14:04 include/ls.h
-rw-r--r-- 1 thuan thuan 1.2K Oct  8 14:04 include/util.h
-rw-r--r-- 1 thuan thuan  706 Oct  7 09:15 Makefile
```

### Recursive listing (`-R`)

```sh
$ ./ls -R src
main.c
print.c
sort.c
traverse.c
util.c
```

With multiple directories (or under `-R`), each directory is introduced by a
`<dir>:` header and separated by a blank line:

```sh
$ ./ls -R src include
src:
main.c
print.c
sort.c
traverse.c
util.c

include:
ls.h
print.h
sort.h
traverse.h
util.h
```

### Exit status check (Kiểm tra mã trả về)

A missing or unreadable operand produces a diagnostic on **stderr** and a non-zero
exit status, while remaining operands are still processed:

```sh
$ ./ls /root
ls: /root: Permission denied
$ echo $?
1
```

A fully successful run exits `0`:

```sh
$ ./ls Makefile > /dev/null
$ echo $?
0
```

### Environment variable (`BLOCKSIZE`)

```sh
$ BLOCKSIZE=1k ./ls -s include    # block counts in 1K units
```

---

## 3. Features — Supported Options (Tính năng)

All 19 required options are implemented.

| Option | Description |
|--------|-------------|
| `-A` | List all entries except `.` and `..`. |
| `-a` | List all entries, including those beginning with `.` (and `.`/`..`). |
| `-c` | Use the status-change time (ctime) for sorting (`-t`) and display (`-l`). |
| `-d` | Treat directory operands as ordinary files; do not list their contents. |
| `-F` | Append a type indicator: `/` directory, `*` executable, `@` symlink, `=` socket, `\|` FIFO. |
| `-f` | Do not sort; implies `-a`. |
| `-h` | Human-readable sizes (powers of 1024, e.g. `1.5K`). |
| `-i` | Print the inode number of each entry. |
| `-k` | Report blocks in 1024-byte units instead of 512-byte units. |
| `-l` | Long format: mode, links, owner, group, size, timestamp, name. |
| `-n` | As `-l`, but print numeric UID/GID instead of names. |
| `-q` | Replace non-printable filename characters with `?`. |
| `-R` | Recursively list subdirectories. |
| `-r` | Reverse the sort order. |
| `-S` | Sort by file size, largest first. |
| `-s` | Print the number of allocated blocks for each entry. |
| `-t` | Sort by timestamp, most recent first. |
| `-u` | Use the access time (atime) for sorting and display. |
| `-w` | Print non-printable characters raw. |

**Environment variable:** `BLOCKSIZE` selects the unit used for block counts
(`-s`, `total`) when `-k` is not given. It accepts a byte count with an optional
`k`/`M`/`G` suffix (`512`, `1024`, `2k`, `1M`), defaulting to 512-byte units.

---

## 4. Option Precedence (Quy tắc ưu tiên)

Mutually exclusive options are resolved during parsing, so **the option appearing
last on the command line wins**:

| Pair | Meaning |
|------|---------|
| `-q` ↔ `-w` | non-printable handling: `?` vs. raw |
| `-l` ↔ `-n` | long format with names vs. numeric IDs |
| `-c` ↔ `-u` | ctime vs. atime selection |
| `-R` ↔ `-d` | recurse vs. treat directories as files |
| `-h` ↔ `-k` | human sizes vs. kilobyte blocks |
| `-a` ↔ `-A` | include vs. exclude `.`/`..` |

Additionally, `-f` implies `-a`.

---

## 5. Architecture (Kiến trúc)

The code is split into small, single-purpose translation units; shared types and
state live in headers.

```
.
├── Makefile              # build: all, clean
├── .gitignore            # build artifacts, binaries, temp files
├── README.md             # project report
├── include/
│   ├── ls.h              # option bitmask, entry structures, global config
│   ├── traverse.h        # operand handling and directory walking
│   ├── sort.h            # ordering
│   ├── print.h           # short/long output
│   └── util.h            # formatting helpers, error reporting
└── src/
    ├── main.c            # getopt() parsing, precedence resolution, dispatch
    ├── traverse.c        # operand split, opendir/readdir, -d, -R
    ├── sort.c            # name / size / time comparators, -r
    ├── print.c           # short and long formats, alignment, suffixes
    └── util.c            # mode string, owner/group, time, block/size formatting
```

**Design notes**

- **Option state** is a single bitmask (`enum ls_flag`) in `include/ls.h`, exposed
  through the global `g_opts`. Parsing clears the loser of each exclusive pair.
- **Entry storage** is a growable array (`ls_entrylist`) of `ls_entry` records;
  each entry owns its `name`, `path`, and optional symlink `target` strings.
- **Ownership** is explicit: `entrylist_add()` copies its inputs, and every list
  is released by `entrylist_free()`, including on error and early-return paths.

---

## 6. Edge Cases & Loop Safety (Xử lý biên & an toàn đệ quy)

- **Symlinks are not followed** — entries and operands are inspected with
  `lstat(2)`, so links are reported as links (`l` type, and `name -> target` in
  long format) instead of being traversed.
- **Recursion safety (`-R`)** — `.` and `..` are never descended into. Combined
  with `lstat` (which does not follow links), a symbolic link pointing at an
  ancestor cannot cause an infinite descent.
- **SUID/SGID/sticky bits** — the execute letter of the affected triplet becomes
  `s`/`S` (set-user/group-ID) or `t`/`T` (sticky), with the uppercase form used
  when the execute bit is absent.
- **Owner/group** — names are looked up in the passwd/group database and fall back
  to numeric IDs when no entry exists or when `-n` is active.
- **Non-printable characters** — bytes failing `isprint()` are shown as `?` when
  output is a terminal or `-q` is given, and printed raw under `-w` or when output
  is redirected (BSD default).
- **Device files** — character/block devices print `major, minor` in the size
  column instead of a byte count.
- **Errors are non-fatal** — a missing file, an unreadable directory, or a failed
  `stat` prints `ls: <path>: <error>` on stderr and processing continues with the
  remaining operands.
- **Exit status** — `0` when all operands and directories were processed without
  error; non-zero (`EXIT_FAILURE`) if any error occurred.
