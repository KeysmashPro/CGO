#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t  i8;

typedef double f64;
typedef float  f32;


/* --------------------------- Allocators-------------------------- */

static void *xmalloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr && size) {
        fprintf(stderr, "System error, out of memory, couldn`t malloc: %zu bytes", size);
        abort();
    }
    return ptr;
}

static void *xcalloc(size_t nmemb, size_t size) {
    void *ptr = calloc(nmemb, size);
    if (!ptr && size && nmemb) {
        fprintf(stderr, "System error, out of memory, couldn`t calloc: %zu bytes", size);
        abort();
    }
    return ptr;
}

static void *xrealloc(void *p, size_t size) {
    void *ptr = realloc(p, size);
    if (!ptr && size) {
        fprintf(stderr, "System error, out of memory, couldn`t realloc: %zu bytes", size);
        abort();
    }
    return ptr;
}

/* -----------------------------Vector----------------------------- */

typedef struct {
    u64 *data;
    size_t capacity;
    size_t size;
} vec_u64;

void vec_u64_resize(vec_u64 *v, size_t size)
{
    if (v->capacity >= size) {
        v->size = size;
        return;
    }

    v->data = realloc((void*)v->data, size * sizeof(u64));
    v->capacity = size;
    v->size = size;
}

void vec_u64_reserve(vec_u64 *v, size_t cap)
{
    if (v->capacity >= cap) return;
    v->data = realloc((void*)v->data, cap * sizeof(u64));
    v->capacity = cap;
}


void vec_u64_growth(vec_u64 *v)
{
    size_t new_size = 2 * (v->capacity + 1);
    v->data = realloc((void*)v->data, new_size * sizeof(u64));
    v->capacity = new_size;
    printf("Alloc %zu bytes\n", new_size);
}

void vec_u64_push(vec_u64 *v, u64 x)
{
    if (v-> capacity == v->size) {
        vec_u64_growth(v);
    }
    v->data[v->size++] = x;
}

void vec_u64_init(vec_u64 *v, size_t size) {
    v-> data = NULL;
    v-> capacity = 0;
    v-> size = 0;
    vec_u64_reserve(v, size); 
}
/* ---------------------------------------------------------------- */
