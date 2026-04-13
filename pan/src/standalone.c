#include "libPan.h"
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>

static void l_help(const char *exec)
{
    puts("");
    printf("Usage: %s [client|server] FILES\n", exec);
    puts("Parse binmsg & textmsg dumps");
    puts("");
    puts("Action is determined by filetype:");
    puts(" - dir    -- will search for .pan files there");
    puts(" - .pan   -- protocol defenitions will be loaded");
    puts(" - .bmsg  -- binmsg dump will be processed and dumped");
    puts(" - .tmsg  -- (TODO) textmsg dump will also be processed and dumped");
    puts("Side name can be arbitrary prefix of them, for example `c` and `s` will suffice");
    puts("");
    puts("2026, (c) Ivan Didyk, github.com/random-username-here");
    puts("Standards for binmsg & textmsg availiable at github.com/random-username-here/mipt-ded-bardak");
    puts("");
}

static void l_logger(const char *fmt, ...) {
    va_list args, copy;
    va_start(args, fmt);
    va_copy(copy, args);
    size_t size = vsnprintf(NULL, 0, fmt, copy) + 1;
    char *buf = malloc(size);
    assert(buf);
    vsnprintf(buf, size, fmt, args);
    va_end(copy);
    va_end(args);
    fputs(" -> ", stdout);
    for (const char *b = buf; *b; ++b) {
        if (*b == '\n') fputs("\n    ", stdout);
        else fputc(*b, stdout);
    }
    fputs("\n", stdout);
    free(buf);
}

static bool l_hasExt(const char *name, const char *ext)
{
    size_t len = strlen(name), elen = strlen(ext);
    if (elen > len) return false;
    return strcmp(name + len - elen, ext) == 0;
}

static char *l_readFile(const char *name, size_t *r_size)
{
    FILE *f = fopen(name, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END); // FIXME: handle failiures
    size_t size = ftell(f);
    char *buf = malloc(size);
    if (!buf) return NULL;
    fseek(f, 0, SEEK_SET);
    fread(buf, 1, size, f);
    fclose(f);
    if (r_size) *r_size = size;
    return buf;
}

static void l_process(const char *name, struct PAN *pan, enum PAN_Side side, bool isTop)
{
    struct stat st;
    stat(name, &st);

    if (S_ISDIR(st.st_mode)) {
        // TODO
    } else if (!S_ISREG(st.st_mode)) {
        if (isTop)
            printf("[x] File %s is of unknown type", name);
    } else if (l_hasExt(name, ".pan")) {
        puts("");
        printf("Loading defs from %s\n", name);
        bool ok = pan_loadDefsFromFile(pan, name);
        if (!ok) {
            printf("[x] Failed to load defs: %s\n", strerror(errno));
        } else {
            puts("Defs loaded");
        }
    } else if (l_hasExt(name, ".bmsg")) {
        puts("");
        printf("Dumping binmsg from %s\n", name);
        size_t fsz = 0;
        char *buf = l_readFile(name, &fsz);
        if (!buf) {
            printf("[x] Failed to read bmsg file: %s\n", strerror(errno));
            return;
        }
        puts("");
        size_t pos = 0;
        while (pos < fsz)
            pos += pan_binDump(pan, side, buf + pos, fsz - pos);
        free(buf);
    } else {
        printf("[x] Program doesn't know what to do with %s\n", strerror(errno));
    }
}

int main(int argc, const char *argv[])
{
    if (argc < 2) {
        l_help(argv[0]);
        return 0;
    }

    struct PAN pan;
    pan_init(&pan, l_logger, true);

    enum PAN_Side side;

    if (argv[1][0] != '\0' && strncmp(argv[1], "client", strlen(argv[1])) == 0)
        side = PAN_CLIENT;
    else if (argv[1][0] != '\0' && strncmp(argv[1], "server", strlen(argv[1])) == 0)
        side = PAN_SERVER;
    else {
        printf("Unknown side: `%s`", argv[1]);
        return -1;
    }

    for (size_t i = 2; i < argc; ++i)
        l_process(argv[i], &pan, side, true);

    pan_destroy(&pan);
    return 0;
}
