#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include "stdlib.c"


typedef struct {
    u64 hashes[32];
    u64 *table;
    u16 *captured;
    u16 *queue;
    u8 *visit;
    u8 *field;
    u8 f_size;
    u8 c_ring;
} go_state;

static inline u64 splitmix64(u32 key) {
    u64 z = (u64)key + 0x9E3779B97F4A7C15ULL;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

void go_init(go_state *go, u8 size, u64 *table) {
    u16 num = size * size;
    go->captured = (u16*)xcalloc(num, sizeof(u16));
    go->queue = (u16*)xcalloc(num, sizeof(u16));
    go->field = (u8*)xcalloc(num, sizeof(u8));
    go->visit = (u8*)xcalloc(num, sizeof(u8));
    go->f_size = size;
    go->table = table;
    go->c_ring = 0;

    for (u32 i = 0; i < 32; ++i) {
        go->hashes[i] = splitmix64(i + (1<<24));
    }
}

void go_free(go_state *go) {
    free(go->visit);
    free(go->queue);
    free(go->field);
    free(go->captured);
}

u64* go_gen_table(u8 size) {
    u32 len = size * size * 2;
    u64 *ptr = xmalloc(len * sizeof(u64));
    for (u32 i = 0; i < len; ++i) {
        ptr[i] = splitmix64(i);
    }
    return ptr;
}

i32 static  bfs_dame_search(u8 *field, u8 *visit, u16 *queue, u8 size, u8 sx, u8 sy) {
    memset(visit, 0, (size_t) size * size);

    u32 color = field[sy * size + sx];
    u32 start = sy * size + sx;
    u32 left = 0, right = 0;
    if (color == 0) return 0;

    queue[right++] = start; 
    visit[start] = 1;

    while(left < right) {
        u16 cur = queue[left++];
        u8 x = cur % size;
        u8 y = cur / size;

        u8 n[4]; u8 nc = 0;

        if (x) n[nc++] = cur - 1; 
        if (y) n[nc++] = cur - size;
        if (x < (size - 1)) n[nc++] = cur + 1;
        if (y < (size - 1)) n[nc++] = cur + size;

        for (u8 i = 0; i < nc; ++i) {
            u16 c = n[i];
            if (field[c] == 0) return 1;
            if (field[c] == color && !visit[c]) {
                queue[right++] = c;
                visit[c] = 1;
            }
        }
    }

    return 0;
}

i32 static  bfs_demolish(u8 *field, u8 *visit, u16 *queue, u8 size, u8 sx, u8 sy) {
    memset(visit, 0, (size_t) size * size);

    u32 color = field[sy * size + sx];
    u32 start = sy * size + sx;
    u32 left = 0, right = 0;
    if (color == 0) return 0;

    queue[right++] = start; 
    visit[start] = 1;

    while(left < right) {
        u16 cur = queue[left++];
        u8 x = cur % size;
        u8 y = cur / size;

        field[cur] = 0;

        u8 n[4]; u8 nc = 0;

        if (x) n[nc++] = cur - 1; 
        if (y) n[nc++] = cur - size;
        if (x < (size - 1)) n[nc++] = cur + 1;
        if (y < (size - 1)) n[nc++] = cur + size;

        for(u8 i = 0; i < nc; ++i) {
            u16 c = n[i];
            if (field[c] == color && !visit[c]) {
                queue[right++] = c;
                visit[c] = 1;
            }
        }
    }

    return right;
}

i32 go_move(go_state *go, u8 x, u8 y, u8 player) {   

    u16 *captured = go->captured;
    u64 *hashes = go->hashes;
    u64 *table = go-> table;
    u16 *queue = go->queue;
    u8 *visit = go->visit;
    u8 *field = go->field;
    u8 size = go->f_size;

    if (player != 1 && player != 2) return -1;
    if (x >= size || y >= size) return -1;
    if (field[y * size + x] != 0) return -1;

    field[y * size + x] = player;
    u64 new_hash = hashes[go->c_ring] ^ table[2 * (y*size + x) + (player-1)];

    u8 n[4] = {2, 2, 2, 2};
    u8 color[4] = {3, 3, 3, 3};

    if (x) { n[0] = bfs_dame_search(field, visit, queue, size, x-1, y); color[0] = field[y * size + x - 1]; }
    if (y) { n[1] = bfs_dame_search(field, visit, queue, size, x, y-1); color[1] = field[(y-1) * size + x]; }
    if (x < (size-1)) { n[2] = bfs_dame_search(field, visit, queue, size, x+1, y); color[2] = field[y * size + x + 1]; }
    if (y < (size-1)) { n[3] = bfs_dame_search(field, visit, queue, size, x, y+1); color[3] = field[(y+1) * size + x]; }

    u8 opponent = (player == 1) ? 2 : 1;
    u8 allow = 0;

    for (u8 i = 0; i < 4; ++i) {
        if (color[i] == 0) { allow = 1; break; }
        if (n[i] && color[i] == player) { allow = 1; break; }
        if (!n[i] && color[i] == opponent) { allow = 1; break; }
    }

    if (!allow) {
        field[y * size + x] = 0;
        return -1;
    }

    i32 score = 0;
    for (u8 i = 0; i < 4; ++i) {
        if (!(!n[i] && color[i] == opponent)) continue;
        u8 cx = x, cy = y;
        switch(i) {
            case 0: cx--; break;
            case 1: cy--; break;
            case 2: cx++; break;
            case 3: cy++; break;
        }
        i32 c = bfs_demolish(field, visit, queue, size, cx, cy);
        for (u32 i = 0; i < c; ++i) {
            new_hash ^= table[(2 * queue[i]) + (opponent - 1)];
            captured[score++] = queue[i];
        }
    }

    for(u32 i = 0; i < 32; ++i) {
        if (hashes[i] == new_hash) {
            for(u32 i = 0; i < score; ++i) {
                field[captured[i]] = opponent;
            }
            field[y * size + x] = 0;
            return -1;
        }
    }

    go->c_ring = (go->c_ring + 1) % 32;
    hashes[go->c_ring] = new_hash;
    return score;
}

