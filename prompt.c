#include <pwd.h>
#include <stdio.h>
#include <unistd.h>

#include "prompt.h"

#define WORD_MAX 256

static void blue(void) {
    printf("\033[0;34m");
}

static void green(void) {
    printf("\033[0;32m");
}

static void reset(void) {
    printf("\033[0m");
}

void show_prompt(void) {
    char cwd[WORD_MAX];
    struct passwd* pass;
    pass = getpwuid(getuid());

    green();
    printf("%s", pass->pw_name);
    reset();
    printf(":");
    blue();

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s", cwd);
    }

    reset();
    printf("> ");

    fflush(stdout);
}
