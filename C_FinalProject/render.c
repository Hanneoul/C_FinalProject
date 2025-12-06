#include "render.h"
#include "ansi_util.h"
#include <stdio.h>
#include <string.h> // snprintf 사용을 위해 포함됨

// 맵 외곽선을 그리는 보조 함수
static void draw_border() {
    printf("+");
    for (int i = 0; i < MAP_WIDTH; i++) {
        printf("=="); // 너비를 2칸으로 하여 정사각형처럼 보이도록 함
    }
    printf("+\n");
}

void render_game(const GameState* state) {
    // 1. 화면 지우기 및 커서 숨기기
    clear_screen();
    hide_cursor();

    // 2. 맵 출력 시작
    move_cursor(1, 10);
    draw_border();

    for (int y = 1; y <= MAP_HEIGHT; y++) {
        move_cursor(1, 10 + y);
        printf("|");
        for (int x = 1; x <= MAP_WIDTH; x++) {
            int is_player1 = (state->player1.x == x && state->player1.y == y);
            int is_player2 = (state->player2.x == x && state->player2.y == y);

            if (is_player1) {
                set_foreground_color(ANSI_RED);
                printf(" %c", state->player1.symbol); // P1 출력 ('X')
            }
            else if (is_player2) {
                set_foreground_color(ANSI_BLUE);
                printf(" %c", state->player2.symbol); // P2 출력 ('O')
            }
            else {
                set_foreground_color(ANSI_WHITE);
                printf(" ."); // 빈 공간
            }
            reset_color();
        }
        printf(" |\n");
    }
    draw_border();

    // 3. 맵 아래에 커서를 위치시켜 다음 정보를 출력할 준비
    move_cursor(1, 10 + MAP_HEIGHT + 2);
}

void render_info(const GameState* state) {
    // 맵 위에 정보 출력 (y=1)
    move_cursor(1, 1);
    printf("============================== 턴 %d ==============================\n", state->turn);

    // P1 정보 (빨간색)
    set_foreground_color(ANSI_RED);
    printf(" P1(%c) ", state->player1.symbol);
    reset_color();
    printf("HP:%d/5 | MP:%d/5 | (X,Y):%d,%d | LastCmd:%d",
        state->player1.hp, state->player1.mp, state->player1.x, state->player1.y, state->player1.last_command);

    printf("\n");

    // P2 정보 (파란색)
    set_foreground_color(ANSI_BLUE);
    printf(" P2(%c) ", state->player2.symbol);
    reset_color();
    printf("HP:%d/5 | MP:%d/5 | (X,Y):%d,%d | LastCmd:%d",
        state->player2.hp, state->player2.mp, state->player2.x, state->player2.y, state->player2.last_command);

    printf("\n");

    printf("------------------------------------------------------------------\n");

    if (state->game_over == 0) {
        printf(" >> 커맨드: 1(상), 2(하), 3(좌), 4(우), 5(공격)\n");
    }
    else {
        move_cursor(1, 5); // 승리 메시지 위치 이동
        printf("************************************************\n");
        printf("                 ");
        if (state->game_over == 1) {
            set_foreground_color(ANSI_RED);
            printf("P1(%c) 승리!", state->player1.symbol);
        }
        else if (state->game_over == 2) {
            set_foreground_color(ANSI_BLUE);
            printf("P2(%c) 승리!", state->player2.symbol);
        }
        else {
            set_foreground_color(ANSI_YELLOW);
            printf("무승부!");
        }
        reset_color();
        printf("\n");
        printf("************************************************\n");
    }

    // 커맨드 입력을 위해 커서를 이동시킴
    move_cursor(1, 7);
}