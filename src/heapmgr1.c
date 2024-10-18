#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "chunk.h"

#define MEMALLOC_MIN 1024 /* 최소 메모리 할당 크기 */

#define NUM_FREE_LISTS 16                  /* 프리 리스트의 개수 */

static Chunk_T free_lists[NUM_FREE_LISTS] = { NULL }; /* 크기별 프리 리스트 배열 */
static void *g_heap_start = NULL, *g_heap_end = NULL; /* 힙의 시작과 끝 주소 */

void heapmgr_free(void *ptr);         /* 메모리 해제 함수 */
void *heapmgr_malloc(size_t size);    /* 메모리 할당 함수 */

static void init_heap(void);                            /* 힙 초기화 함수 */
static void add_to_free_list(Chunk_T chunk);            /* 프리 리스트에 청크 추가 */
static void remove_from_free_list(Chunk_T chunk);       /* 프리 리스트에서 청크 제거 */
static void allocate_more_memory(size_t size);          /* 더 많은 메모리 할당 */
static int get_free_list_index(size_t size);            /* 프리 리스트 인덱스 계산 */
static Chunk_T find_best_fit(size_t size);              /* 가장 적합한 프리 청크 찾기 */

static void init_heap(void) {
    /* 힙 초기화 함수 */
    g_heap_start = g_heap_end =sbrk(0);
    if (g_heap_start ==(void *)-1) {
        fprintf(stderr, "sbrk(0) failed\n");
        exit(-1);
    }
}

static int get_free_list_index(size_t size) {
    /* 프리 리스트 인덱스를 계산하여 반환 */
    int index = 0;
    size >>= 4; // 16바이트 단위로 크기 클래스를 나눔
    while (size > 0 && index < NUM_FREE_LISTS - 1) {
        size >>= 1;
        index++;
    }
    return index;
}

static void add_to_free_list(Chunk_T chunk) {
    /* 프리 리스트에 청크를 추가 */
    int index = get_free_list_index(chunk_get_size(chunk));
    chunk->next_free = free_lists[index];
    if (free_lists[index] != NULL) {
        free_lists[index]->prev_free = chunk;
    }
    chunk->prev_free = NULL;
    free_lists[index] = chunk;
}

static void remove_from_free_list(Chunk_T chunk) {
    /* 프리 리스트에서 청크를 제거 */
    int index = get_free_list_index(chunk_get_size(chunk));
    if (chunk->prev_free != NULL) {
        chunk->prev_free->next_free = chunk->next_free;
    } else {
        free_lists[index] = chunk->next_free;
    }
    if (chunk->next_free != NULL) {
        chunk->next_free->prev_free = chunk->prev_free;
    }
    chunk->next_free = NULL;
    chunk->prev_free = NULL;
}

static void allocate_more_memory(size_t size) {
    /* 시스템으로부터 더 많은 메모리를 요청하여 프리 리스트에 추가 */
    size_t total_size = align_size(size + sizeof(ChunkHeader) + sizeof(ChunkFooter));
    if (total_size < MEMALLOC_MIN) {
        total_size = MEMALLOC_MIN;
    }

    void *new_mem = sbrk(total_size);
    if (new_mem == (void *)-1) {
        fprintf(stderr, "Error: Failed to allocate_more_memory func\n");
        return;
    }

    g_heap_end = sbrk(0);

    Chunk_T c = (Chunk_T)new_mem;
    chunk_set_size_and_status(c, total_size - sizeof(ChunkHeader) - sizeof(ChunkFooter), CHUNK_FREE);
    chunk_set_footer(c);
    c->next_free = NULL;
    c->prev_free = NULL;

    /* 프리 리스트에 추가 */
    add_to_free_list(c);
}

static Chunk_T find_best_fit(size_t size) {
    /* 요청된 크기 이상의 가장 작은 프리 청크를 찾음 (Best-Fit) */
    int index = get_free_list_index(size);
    Chunk_T best_fit = NULL;
    size_t smallest_diff = (size_t)-1;

    for (int i = index; i < NUM_FREE_LISTS; i++) {
        Chunk_T c = free_lists[i];
        while (c != NULL) {
            size_t c_size = chunk_get_size(c);
            if (c_size >= size) {
                size_t diff = c_size - size;
                if (diff < smallest_diff) {
                    best_fit = c;
                    smallest_diff = diff;
                    if (diff == 0) {
                        return best_fit; // 완벽한 크기 일치
                    }
                }
            }
            c = c->next_free;
        }
        if (best_fit != NULL) {
            return best_fit;
        }
    }
    return NULL;
}

void *heapmgr_malloc(size_t size) {
    /* 메모리 할당 함수 */
    if (size == 0) {
        return NULL;
    }

    if (g_heap_start == NULL) {
        init_heap();
    }

    size_t aligned_size = align_size(size);
    Chunk_T c = find_best_fit(aligned_size);

    if (c == NULL) {
        allocate_more_memory(aligned_size);
        c = find_best_fit(aligned_size);
        if (c == NULL) {
            fprintf(stderr, "Error: find_best_fit func\n");
            return NULL;
        }
    }

    remove_from_free_list(c);
    size_t c_size = chunk_get_size(c);
    size_t remaining_size = c_size - aligned_size;

    if (remaining_size >= sizeof(ChunkHeader) + sizeof(ChunkFooter) + ALIGNMENT) {
        /* 청크 분할 */
        chunk_set_size_and_status(c, aligned_size, CHUNK_IN_USE);
        chunk_set_footer(c);

        Chunk_T new_chunk = (Chunk_T)((char *)c + sizeof(ChunkHeader) + aligned_size + sizeof(ChunkFooter));
        chunk_set_size_and_status(new_chunk, remaining_size - sizeof(ChunkHeader) - sizeof(ChunkFooter), CHUNK_FREE);
        chunk_set_footer(new_chunk);
        new_chunk->next_free = NULL;
        new_chunk->prev_free = NULL;

        add_to_free_list(new_chunk);
    } else {
        /* 남은 공간이 충분하지 않으면 전체 청크 사용 */
        chunk_set_size_and_status(c, c_size, CHUNK_IN_USE);
        chunk_set_footer(c);
    }

    return (void *)((char *)c + sizeof(ChunkHeader));
}

void heapmgr_free(void *ptr) {
    /* 메모리 해제 함수 */
    if (ptr == NULL) {
        return;
    }

    Chunk_T c = (Chunk_T)((char *)ptr - sizeof(ChunkHeader));

    if (chunk_get_status(c) != CHUNK_IN_USE) {
        fprintf(stderr, "Error: Attempt to free a chunk that is not in use\n");
        return;
    }

    chunk_set_size_and_status(c, chunk_get_size(c), CHUNK_FREE);
    chunk_set_footer(c);

    /* 이전 청크와 병합 시도 */
    if ((void *)c > g_heap_start) {
        ChunkFooter *prev_footer = (ChunkFooter *)((char *)c - sizeof(ChunkFooter));
        if ((void *)prev_footer > g_heap_start && (prev_footer->size_and_status & STATUS_MASK) == CHUNK_FREE) {
            Chunk_T prev_chunk = chunk_prev_physically(c);
            remove_from_free_list(prev_chunk);

            size_t new_size = chunk_get_size(prev_chunk) + sizeof(ChunkHeader) + sizeof(ChunkFooter) + chunk_get_size(c) + sizeof(ChunkHeader) + sizeof(ChunkFooter);
            chunk_set_size_and_status(prev_chunk, new_size - sizeof(ChunkHeader) - sizeof(ChunkFooter), CHUNK_FREE);
            c = prev_chunk;
            chunk_set_footer(c);
        }
    }

    /* 다음 청크와 병합 시도 */
    Chunk_T next_chunk = chunk_next_physically(c);
    if ((void *)next_chunk < g_heap_end && chunk_get_status(next_chunk) == CHUNK_FREE) {
        remove_from_free_list(next_chunk);

        size_t new_size = chunk_get_size(c) + sizeof(ChunkHeader) + sizeof(ChunkFooter) + chunk_get_size(next_chunk) + sizeof(ChunkHeader) + sizeof(ChunkFooter);
        chunk_set_size_and_status(c, new_size - sizeof(ChunkHeader) - sizeof(ChunkFooter), CHUNK_FREE);
        chunk_set_footer(c);
    }

    /* 프리 리스트에 추가 */
    add_to_free_list(c);
}
