#ifndef CHUNK_H
#define CHUNK_H

#include <stddef.h>

#define CHUNK_UNIT sizeof(size_t)
#define ALIGNMENT CHUNK_UNIT
#define align_size(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

#define CHUNK_IN_USE 1           /* 청크 사용 중 상태 */
#define CHUNK_FREE 0             /* 청크 프리 상태 */
#define STATUS_MASK 1            /* 상태를 추출하기 위한 마스크 */
#define SIZE_MASK (~STATUS_MASK) /* 크기를 추출하기 위한 마스크 */

/* 청크 헤더 구조체 */
typedef struct ChunkHeader {
    size_t size_and_status;       /* 청크의 크기와 상태를 저장 (하위 비트에 상태 저장) */
    struct ChunkHeader *next_free; /* 다음 프리 청크를 가리키는 포인터 */
    struct ChunkHeader *prev_free; /* 이전 프리 청크를 가리키는 포인터 */
} ChunkHeader, *Chunk_T;

/* 청크 푸터 구조체 */
typedef struct {
    size_t size_and_status; /* 청크의 크기와 상태를 저장 */
} ChunkFooter;

/* 함수 프로토타입 선언 */
void chunk_set_size_and_status(Chunk_T chunk, size_t size, int status); /* 청크의 크기와 상태 설정 */
size_t chunk_get_size(Chunk_T chunk);    /* 청크의 크기 가져오기 */
int chunk_get_status(Chunk_T chunk);     /* 청크의 상태 가져오기 */
void chunk_set_footer(Chunk_T chunk);    /* 청크의 푸터 설정 */
ChunkFooter *chunk_get_footer(Chunk_T chunk); /* 청크의 푸터 가져오기 */
Chunk_T chunk_next_physically(Chunk_T chunk); /* 다음 물리적 청크 가져오기 */
Chunk_T chunk_prev_physically(Chunk_T chunk); /* 이전 물리적 청크 가져오기 */

#endif
