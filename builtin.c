#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "builtin.h"
#include "tests/syscall_mock.h"

bool builtin_is_exit(pipeline pipe) {
    assert(!pipeline_is_empty(pipe));
    bool is_quit = false;
    unsigned int i = 0;
    unsigned int len = pipeline_length(pipe);

    while (i < len && !is_quit) {
        is_quit = !strcmp(scommand_front(pipeline_get_nth_command(pipe, i)), "exit");
        i++;
    }
    return is_quit;
}

bool builtin_is_cd(pipeline pipe) {
    assert(!pipeline_is_empty(pipe));
    bool is_cd = false;
    is_cd = !strcmp(scommand_front(pipeline_front(pipe)), "cd");
    return is_cd;
}

bool builtin_is_internal(pipeline pipe) {
    return builtin_is_cd(pipe) || builtin_is_exit(pipe);
}

void builtin_exec(pipeline pipe) {
    if (builtin_is_cd(pipe)) {
        char** array_argumentos = scommand_to_array(pipeline_front(pipe));
        chdir(array_argumentos[1]);
    }
}
