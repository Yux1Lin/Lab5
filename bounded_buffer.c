#include "bounded_buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

/* Do not use any global variables for implementation */

void bounded_buffer_init(struct bounded_buffer *buffer, int size) {
    buffer->circular_buffer = calloc(size, sizeof(void *));
    buffer->head = 0;
    buffer->tail = 0;
    buffer->count = 0;
    buffer->MAX = size;

    // Create lock and condition variables
    assert(pthread_mutex_init(&buffer->lock, NULL) == 0);
    assert(pthread_cond_init(&buffer->empty, NULL) == 0);
    assert(pthread_cond_init(&buffer->full, NULL) == 0);
}

void bounded_buffer_push(struct bounded_buffer *buffer, void *item) {
    assert(pthread_mutex_lock(&buffer->lock) == 0);

    // Wait until there is space in the buffer
    while (buffer->count == buffer->MAX) {
        assert(pthread_cond_wait(&buffer->empty, &buffer->lock) == 0);
    }

    // Add the item to the buffer
    buffer->circular_buffer[buffer->tail] = item;
    buffer->tail = (buffer->tail + 1) % buffer->MAX;
    buffer->count++;

    // Signal that the buffer is no longer empty
    assert(pthread_cond_signal(&buffer->full) == 0);
    assert(pthread_mutex_unlock(&buffer->lock) == 0);
}

void *bounded_buffer_pop(struct bounded_buffer *buffer) {
    assert(pthread_mutex_lock(&buffer->lock) == 0);

    // Wait until there is an item in the buffer
    while (buffer->count == 0) {
        assert(pthread_cond_wait(&buffer->full, &buffer->lock) == 0);
    }

    // Remove the item from the buffer
    void *item = buffer->circular_buffer[buffer->head];
    buffer->head = (buffer->head + 1) % buffer->MAX;
    buffer->count--;

    // Signal that the buffer is no longer full
    assert(pthread_cond_signal(&buffer->empty) == 0);
    assert(pthread_mutex_unlock(&buffer->lock) == 0);

    return item;
}

void bounded_buffer_destroy(struct bounded_buffer *buffer) {
    free(buffer->circular_buffer);

    // Destroy lock and condition variables
    assert(pthread_mutex_destroy(&buffer->lock) == 0);
    assert(pthread_cond_destroy(&buffer->empty) == 0);
    assert(pthread_cond_destroy(&buffer->full) == 0);
}

