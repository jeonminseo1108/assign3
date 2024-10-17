#include <stdlib.h>
#include <assert.h>
#include "chunk.h"

#define MEMALLOC_MIN 1024

/* 전역 변수들 */
static Chunk_T g_free_head = NULL; /* 자유 리스트의 첫 번째 청크 */
static void *g_heap_start = NULL, *g_heap_end = NULL;

/* 힙을 초기화하는 함수 */
static void init_heap(void) {
    g_heap_start = g_heap_end = sbrk(0);
    if (g_heap_start == (void *)-1) {
        fprintf(stderr, "sbrk(0) failed\n");
        exit(-1);
    }
}

/* 새로운 메모리를 할당하는 함수 */
static Chunk_T allocate_more_memory(size_t units) {
    if (units < MEMALLOC_MIN) {
        units = MEMALLOC_MIN;
    }

    /* 메모리 블록을 시스템에서 요청 */
    Chunk_T c = (Chunk_T)sbrk((units + 1) * CHUNK_UNIT);
    if (c == (Chunk_T)-1) {
        return NULL;
    }

    g_heap_end = sbrk(0); /* 힙의 끝 위치 업데이트 */
    chunk_set_units(c, units);
    chunk_set_status(c, CHUNK_FREE);

    return c;
}

/* 자유 리스트에서 메모리 할당 */
void *heapmgr_malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    if (g_heap_start == NULL) {
        init_heap();
    }

    size_t units = (size + CHUNK_UNIT - 1) / CHUNK_UNIT; /* 청크 단위로 변환 */

    Chunk_T prev = NULL;
    for (Chunk_T c = g_free_head; c != NULL; c = chunk_get_next_free_chunk(c)) {
        if (chunk_get_units(c) >= units) {
            if (chunk_get_units(c) > units + 1) {
                /* 블록을 나누어 사용 */
                Chunk_T new_chunk = chunk_get_next_adjacent(c, g_heap_start, g_heap_end);
                chunk_set_units(new_chunk, chunk_get_units(c) - units - 1);
                chunk_set_status(new_chunk, CHUNK_FREE);
                chunk_set_next_free_chunk(new_chunk, chunk_get_next_free_chunk(c));
                chunk_set_units(c, units);
            }
            /* 자유 리스트에서 블록 제거 */
            if (prev == NULL) {
                g_free_head = chunk_get_next_free_chunk(c);
            } else {
                chunk_set_next_free_chunk(prev, chunk_get_next_free_chunk(c));
            }

            chunk_set_status(c, CHUNK_IN_USE);
            return (void *)((char *)c + CHUNK_UNIT); /* 데이터를 위한 포인터 반환 */
        }
        prev = c;
    }

    /* 더 큰 메모리 블록 요청 */
    Chunk_T new_chunk = allocate_more_memory(units);
    if (new_chunk == NULL) {
        return NULL;
    }
    
    chunk_set_status(new_chunk, CHUNK_IN_USE);
    return (void *)((char *)new_chunk + CHUNK_UNIT);
}

/* 메모리 해제 함수 */
void heapmgr_free(void *ptr) {
    if (ptr == NULL) {
        return;
    }

    Chunk_T c = (Chunk_T)((char *)ptr - CHUNK_UNIT); /* 청크 헤더로 이동 */
    assert(chunk_get_status(c) == CHUNK_IN_USE);

    chunk_set_status(c, CHUNK_FREE);
    chunk_set_next_free_chunk(c, g_free_head);
    g_free_head = c;

    /* 인접 청크와 병합 시도 */
    Chunk_T next = chunk_get_next_adjacent(c, g_heap_start, g_heap_end);
    if (next != NULL && chunk_get_status(next) == CHUNK_FREE) {
        chunk_set_units(c, chunk_get_units(c) + chunk_get_units(next) + 1);
        chunk_set_next_free_chunk(c, chunk_get_next_free_chunk(next));
    }
}
