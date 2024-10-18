#include "chunk.h"

void chunk_set_size_and_status(Chunk_T chunk, size_t size, int status) {
    /* 청크의 크기와 상태를 설정 */
    chunk->size_and_status = (size & SIZE_MASK) | (status & STATUS_MASK);
}

size_t chunk_get_size(Chunk_T chunk) {
    /* 청크의 크기를 반환 */
    return chunk->size_and_status & SIZE_MASK;
}

int chunk_get_status(Chunk_T chunk) {
    /* 청크의 상태를 반환 */
    return chunk->size_and_status & STATUS_MASK;
}

void chunk_set_footer(Chunk_T chunk) {
    /* 청크의 푸터를 설정 */
    ChunkFooter *footer = (ChunkFooter *)((char *)chunk + sizeof(ChunkHeader) + chunk_get_size(chunk));
    footer->size_and_status = chunk->size_and_status;
}

ChunkFooter *chunk_get_footer(Chunk_T chunk) {
    /* 청크의 푸터를 반환 */
    return (ChunkFooter *)((char *)chunk + sizeof(ChunkHeader) + chunk_get_size(chunk));
}

Chunk_T chunk_next_physically(Chunk_T chunk) {
    /* 다음 물리적 청크를 반환 */
    return (Chunk_T)((char *)chunk + sizeof(ChunkHeader) + chunk_get_size(chunk) + sizeof(ChunkFooter));
}

Chunk_T chunk_prev_physically(Chunk_T chunk) {
    /* 이전 물리적 청크를 반환 */
    ChunkFooter *prev_footer = (ChunkFooter *)((char *)chunk - sizeof(ChunkFooter));
    size_t prev_size = prev_footer->size_and_status & SIZE_MASK;
    return (Chunk_T)((char *)chunk - sizeof(ChunkFooter) - prev_size - sizeof(ChunkHeader));
}
