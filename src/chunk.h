#ifndef _CHUNK_H_
#define _CHUNK_H_

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#define CHUNK_UNIT 16

/* chunk struct */
typedef struct Chunk *Chunk_T;

enum {
    CHUNK_FREE,     /* 자유 상태 */
    CHUNK_IN_USE    /* 사용 중 */
};

/* chuckbase function */
int chunk_get_status(Chunk_T c);
void chunk_set_status(Chunk_T c, int status);
int chunk_get_units(Chunk_T c);
void chunk_set_units(Chunk_T c, int units);
Chunk_T chunk_get_next_free_chunk(Chunk_T c);
void chunk_set_next_free_chunk(Chunk_T c, Chunk_T next);
Chunk_T chunk_get_next_adjacent(Chunk_T c, void *start, void *end);

#endif
