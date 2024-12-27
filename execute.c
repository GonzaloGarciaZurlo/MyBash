#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "builtin.h"
#include "execute.h"
#include "tests/syscall_mock.h"

/*
 * Ejecuta un comando, reconociendo si hay archivos de entrada y/o de salida. 
 *  Crea un arreglo con los argumentos y llama a execvp y luego libera toda la memoria utilizada.
 * Requires: command!=NULL
 * 
*/
static void execute_command(scommand command) {
    int syscall_ret, in_fd, out_fd;
    char* in = scommand_get_redir_in(command);
    char* out = scommand_get_redir_out(command);

    if (in != NULL) {
        in_fd = open(in, O_RDONLY, 0);
        if (in_fd < 0) {
            fprintf(stderr, "error open in\n");
        }
        dup2(in_fd, STDIN_FILENO);
        close(in_fd);
    }
    if (out != NULL) {
        out_fd = open(out, O_WRONLY | O_TRUNC, 0);
        if (out_fd < 0) {
            fprintf(stderr, "error open out\n");
        }
        dup2(out_fd, STDOUT_FILENO);
        close(out_fd);
    }

    char** array_argumentos = scommand_to_array(command);
    syscall_ret = execvp(array_argumentos[0], array_argumentos);
    if (syscall_ret == -1) {
        fprintf(stderr, "falló la ejecucción | comando invalido\n");
    }
    free(array_argumentos);
    exit(EXIT_FAILURE);
}

/*
 *  Funcion auxiliar para ejecutar cualquier comando externo
 * Requires: apipe!=NULL 
 * 
*/
static void external_exec(pipeline apipe) {
    int length, new_pipe[2], old_pipe[2];
    bool after, before = false;
    length = pipeline_length(apipe);
    for (int i = 0; i < length; i++) {
        scommand command = pipeline_front(apipe);

        after = pipeline_length(apipe) > 1;
        if (after) {
            pipe(new_pipe);
        }
        pid_t pid = fork();
        if (pid < 0) {
            fprintf(stderr, "falló el fork\n");
        } else if (pid == 0) {
            if (after) {
                close(new_pipe[0]);
                dup2(new_pipe[1], STDOUT_FILENO);
                close(new_pipe[1]);
            }
            if (before) {
                close(old_pipe[1]);
                dup2(old_pipe[0], STDIN_FILENO);
                close(old_pipe[0]);
            }
            execute_command(command);
        }
        pipeline_pop_front(apipe);
        if (before) {
            close(old_pipe[0]);
            close(old_pipe[1]);
            wait(NULL);
        }
        if (after) {
            old_pipe[0] = new_pipe[0];
            old_pipe[1] = new_pipe[1];
        }
        before = true;
    }
}

void execute_pipeline(pipeline apipe) {
    assert(apipe != NULL);

    if (!pipeline_is_empty(apipe)) {
        if (builtin_is_internal(apipe)) {
            builtin_exec(apipe);
        } else {
            external_exec(apipe);
            if (pipeline_get_wait(apipe)) {
                wait(NULL);
            }
        }
    }
}
