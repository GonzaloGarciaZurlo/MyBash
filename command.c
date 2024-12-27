#include <assert.h>
#include <glib.h>
#include <stdio.h>

#include "command.h"
#include "strextra.h"

/********** COMANDO SIMPLE **********/

/* Estructura correspondiente a un comando simple.
 * Es una 3-upla del tipo ([char*], char* , char*).
 */

struct scommand_s {
    GQueue* args;
    char* redir_in;
    char* redir_out;
};

scommand scommand_new(void) {
    scommand self = calloc(1, sizeof(struct scommand_s));
    self->args = g_queue_new();
    self->redir_in = NULL;
    self->redir_out = NULL;
    assert(self != NULL);
    return self;
}

scommand scommand_destroy(scommand self) {
    assert(self != NULL);
    g_queue_free_full(self->args, free);
    free(self->redir_in);
    free(self->redir_out);
    self->args = NULL;
    self->redir_in = NULL;
    self->redir_out = NULL;
    free(self);
    return NULL;
}

void scommand_push_back(scommand self, char* argument) {
    assert(self != NULL && argument != NULL);
    g_queue_push_tail(self->args, argument);
}

void scommand_pop_front(scommand self) {
    assert(self != NULL && !scommand_is_empty(self));
    gpointer temp = g_queue_pop_head(self->args);
    free(temp);
}

void scommand_set_redir_in(scommand self, char* filename) {
    assert(self != NULL);
    if (self->redir_in != NULL) {
        free(self->redir_in);
    }
    self->redir_in = filename;
}

void scommand_set_redir_out(scommand self, char* filename) {
    assert(self != NULL);
    if (self->redir_out != NULL) {
        free(self->redir_out);
    }
    self->redir_out = filename;
}

bool scommand_is_empty(const scommand self) {
    assert(self != NULL);
    return g_queue_is_empty(self->args);
}

unsigned int scommand_length(const scommand self) {
    assert(self != NULL);
    return g_queue_get_length(self->args);
}

char* scommand_front(const scommand self) {
    assert(self != NULL && !scommand_is_empty(self));
    return g_queue_peek_head(self->args);
}

char* scommand_get_redir_in(const scommand self) {
    assert(self != NULL);
    return self->redir_in;
}

char* scommand_get_redir_out(const scommand self) {
    assert(self != NULL);
    return self->redir_out;
}

char* scommand_to_string(const scommand self) {
    assert(self != NULL);
    char* result = "";
    if (!g_queue_is_empty(self->args)) {
        result = strmerge(result, g_queue_peek_nth(self->args, 0));
        for (unsigned int i = 1; i < scommand_length(self); ++i) {
            result = strmerge_and_free_first(result, " ");
            result = strmerge_and_free_first(result, g_queue_peek_nth(self->args, i));
        }

        if (NULL != self->redir_in) {
            result = strmerge_and_free_first(result, " < ");
            result = strmerge_and_free_first(result, self->redir_in);
        }

        if (NULL != self->redir_out) {
            result = strmerge_and_free_first(result, " > ");
            result = strmerge_and_free_first(result, self->redir_out);
        }
    } else {
        result = strdup("");
    }
    return result;
}

scommand scommand_copy(scommand self) {
    scommand copy = scommand_new();
    for (unsigned int i = 0; i < scommand_length(self); i++) {
        g_queue_push_tail(copy->args, strdup(g_queue_peek_nth(self->args, i)));
    }
    if (self->redir_in != NULL) {
        copy->redir_in = strdup(self->redir_in);
    }
    if (self->redir_out != NULL) {
        copy->redir_out = strdup(self->redir_out);
    }
    return copy;
}

char** scommand_to_array(scommand command) {
    unsigned int length = scommand_length(command);
    char** argv = calloc(length + 1, sizeof(char*));
    scommand copy = scommand_copy(command);
    for (unsigned int i = 0; i < length; ++i) {
        argv[i] = strdup(scommand_front(copy));
        scommand_pop_front(copy);
    }
    argv[length] = NULL;
    scommand_destroy(copy);
    return argv;
}

/********** COMANDO PIPELINE **********/

/* Estructura correspondiente a un comando pipeline.
 * Es un 2-upla del tipo ([scommand], bool)
 */

struct pipeline_s {
    GQueue* scmds;
    bool wait;
};

pipeline pipeline_new(void) {
    pipeline self = calloc(1, sizeof(struct pipeline_s));
    self->scmds = g_queue_new();
    self->wait = true;
    assert(self != NULL);
    return self;
}

pipeline pipeline_destroy(pipeline self) {
    assert(self != NULL);
    while (!g_queue_is_empty(self->scmds)) {
        pipeline_pop_front(self);
    }
    g_queue_free(self->scmds);
    self->scmds = NULL;
    free(self);
    return NULL;
}

void pipeline_push_back(pipeline self, scommand sc) {
    assert(self != NULL && sc != NULL);
    g_queue_push_tail(self->scmds, sc);
}

void pipeline_pop_front(pipeline self) {
    assert(self != NULL && !pipeline_is_empty(self));
    scommand temp = g_queue_pop_head(self->scmds);
    scommand_destroy(temp);
}

void pipeline_set_wait(pipeline self, const bool w) {
    assert(self != NULL);
    self->wait = w;
}

bool pipeline_is_empty(const pipeline self) {
    assert(self != NULL);
    return g_queue_is_empty(self->scmds);
}

unsigned int pipeline_length(const pipeline self) {
    assert(self != NULL);
    return g_queue_get_length(self->scmds);
}

scommand pipeline_front(const pipeline self) {
    assert(self != NULL && !pipeline_is_empty(self));
    return g_queue_peek_head(self->scmds);
}

bool pipeline_get_wait(const pipeline self) {
    assert(self != NULL);
    return self->wait;
}

char* pipeline_to_string(const pipeline self) {
    assert(self != NULL);

    char* result = "";
    if (!g_queue_is_empty(self->scmds)) {
        char* temp = scommand_to_string(g_queue_peek_nth(self->scmds, 0));
        result = strmerge(result, temp);
        free(temp);
        for (unsigned int i = 1; i < pipeline_length(self); ++i) {
            result = strmerge_and_free_first(result, " | ");
            result = strmerge_and_free(result, scommand_to_string(g_queue_peek_nth(self->scmds, i)));
        }
    } else {
        result = strdup("");
    }
    if (!self->wait) {
        result = strmerge_and_free_first(result, " &");
    }

    return result;
}

scommand pipeline_get_nth_command(pipeline self, unsigned int n) {
    assert(self != NULL && n < pipeline_length(self));
    scommand nth;
    nth = g_queue_peek_nth(self->scmds, n);
    return nth;
}