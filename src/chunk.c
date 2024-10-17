#include "chunk.h"
#include <assert.h>

struct Chunk {
    Chunk_T next;   /* next chunk */
    int units;      /* chunck size */
    int status;     /* CHUNK_FRE or CHUNK_IN_USE */
};

int chunk_get_status(Chunk_T c) {
    return c->status;
}

void chunk_set_status(Chunk_T c, int status) {
    c->status = status;
}

int chunk_get_units(Chunk_T c) {
    return c->units;
}

void chunk_set_units(Chunk_T c, int units) {
    c->units = units;
}

Chunk_T chunk_get_next_free_chunk(Chunk_T c) {
    return c->next;
}

void chunk_set_next_free_chunk(Chunk_T c, Chunk_T next) {
    c->next = next;
}

Chunk_T chunk_get_next_adjacent(Chunk_T c, void *start, void *end) {
    Chunk_T next = c + c->units + 1;
    if ((void *)next >= end) {
        return NULL;
    }
    return next;
}
