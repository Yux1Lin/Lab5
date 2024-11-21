#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "word_count.h"
#include "hashmap.h"
#include "bounded_buffer.h"
#include <pthread.h>

/* max size of one bounded_buffer */
#define MAX_SIZE 5

/* arguments passed to mapper threads */
struct mapper_args
{
    int r;       // num of reducer
    int i;       // index of string to be parsed from text
    char **text; // text to parse
};

/* arguments passed to reducer threads */
struct reducer_args
{
    int m;                         // number of mappers
    struct bounded_buffer *buffer; // buffer to be reduced
};

/* bounded buffers */
struct bounded_buffer *buffers;

int print_result(char *, int);
unsigned long crc32(const unsigned char *, unsigned int);

/* Mapper parse a word from string and give it to a buffer */
void *mapper(void *ptr)
{
    struct mapper_args *args = ptr;
    int r = args->r;
    char *saveptr;
    char *text = args->text[args->i];

    /* Parse words from text and push to reducers' buffer */
    char *token = strtok_r(text, " ", &saveptr);
    while (token != NULL)
    {
        int index = crc32((unsigned char *)(token), strlen(token)) % r;
        bounded_buffer_push(buffers + index, token);
        token = strtok_r(NULL, " ", &saveptr);
    }

    for (int i = 0; i < r; i++)
    {
        char *special = calloc(2, sizeof(char));
        special[0] = '!'; // Special message
        bounded_buffer_push(buffers + i, special);
    }

    free(args);
    return NULL;
}

/* Reducer counts word in its buffer */
void *reducer(void *ptr)
{
    struct reducer_args *args = ptr;
    struct bounded_buffer *buffer = args->buffer;
    map_t local_map = hashmap_new();
    int m = 0, error;
    char *token;
    while (m != args->m)
    {
        token = bounded_buffer_pop(buffer);
        if (token[0] == '!')
        {
            m++;
            free(token);
        }
        else
        {
            int count;
            error = hashmap_get(local_map, token, &count);
            if (error == MAP_OK)
                hashmap_put(local_map, token, count + 1);
            else
                hashmap_put(local_map, token, 1);
        }
    }
    /* Print results */
    printf("Result:\n");
    hashmap_iterate(local_map, print_result);

    hashmap_free(local_map);
    free(args);
    return NULL;
}

void word_count(int m, int r, char **text)
{
    /* Each reducer has a bounded buffer */
    buffers = malloc(r * sizeof(struct bounded_buffer));
    for (int i = 0; i < r; i++)
    {
        bounded_buffer_init(buffers + i, MAX_SIZE);
    }

    /* Create mappers and reducers threads. */
    pthread_t mappers[m], reducers[r];
    for (int i = 0; i < m; i++)
    {
        struct mapper_args *args = malloc(sizeof(struct mapper_args));
        if (args == NULL)
            exit(1);
        args->r = r;
        args->i = i;
        args->text = text;
        pthread_create(&mappers[i], NULL, mapper, args);
    }
    for (int i = 0; i < r; i++)
    {
        struct reducer_args *args = malloc(sizeof(struct reducer_args));
        if (args == NULL)
            exit(1);
        args->m = m;
        args->buffer = buffers + i;
        pthread_create(&reducers[i], NULL, reducer, args);
    }

    /* Wait for all threads end */
    for (int i = 0; i < m; i++)
        pthread_join(mappers[i], NULL);
    for (int i = 0; i < r; i++)
        pthread_join(reducers[i], NULL);

    /* free bounded buffers */
    for (int i = 0; i < r; i++)
        bounded_buffer_destroy(buffers + i);
    free(buffers);    
}

int print_result(char *key, int value)
{
    printf("count of %s = %d\n", key, value);
    return 0;
}
