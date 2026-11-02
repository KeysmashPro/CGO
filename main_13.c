#include "cgo.c"

i32 main(void) {
    u8 size = 13;
    u64 *table = go_gen_table(255);
    go_state state;
    go_init(&state, size, table);

    printf("\033[2J\033[H\n");
    for (i32 i = size - 1; i >= 0; --i) {
        if (i < 9) printf("    0%d  ", i + 1);
        else  printf("    %d  ", i + 1);
        for(i32 j = 0; j < size; ++j) {
            if (state.field[i * size + j] == 0) printf("· ");
            if (state.field[i * size + j] == 1) printf("◯ ");
            if (state.field[i * size + j] == 2) printf("● ");
        }
        printf("\n");
    }
    printf("\n        ");
    for(u32 i = 0; i < size; ++i) {
        printf("%c ", i + 65);
    }
    printf("\n\n");

    u8 x, y, p = 1;
    for(;;) {
        fflush(stdout);
        printf("\r\033[19C");
        scanf("%c", &x);
        if (x == 'q' || x == 'Q') {
            go_free(&state);
            printf("\n");
            return 0;
        }
        scanf("%hhu", &y);
        fflush(stdout);
        while (getchar() != '\n' && !feof(stdin));
        printf("\033[2J\033[H\n");
        if (x >= 97) x -= 32;
        i32 r = go_move(&state, x - 65, y - 1, p);
        if (r != -1) p = (p == 1) ? 2 : 1;

        for (i32 i = size - 1; i >= 0; --i) {
            if (i < 9) printf("    0%d  ", i + 1);
            else  printf("    %d  ", i + 1);
            for(i32 j = 0; j < size; ++j) {
                if (state.field[i * size + j] == 0) printf("· ");
                if (state.field[i * size + j] == 1) printf("◯ ");
                if (state.field[i * size + j] == 2) printf("● ");
            }
            printf("\n");
        }
        printf("\n        ");
        for(u32 i = 0; i < size; ++i) {
            printf("%c ", i + 65);
        }
        if (r == -1) {
            printf("\n");
            printf("           [ Forbidden move! ]");
            printf("\n");
        } else {
            printf("\n\n");
        }
    }

    return 0;
}

