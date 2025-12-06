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

// 맵을 특정 배경색(flash_bg_code)으로 빠르게 다시 그림
// flash_bg_code가 0이면 일반 배경으로 그림.
void render_game_with_bg(const GameState* state, int flash_bg_code) {
    // 1. 커서 숨기기
    hide_cursor();

    // 2. 정보창 출력 (상단 정보는 깜빡이지 않으므로 한 번만 출력하거나 여기에 포함시킴)
    render_info(state);

    // 3. 맵 출력 시작
    move_cursor(1, 14);
    draw_border();

    for (int y = 1; y <= MAP_HEIGHT; y++) {
        move_cursor(1, 14 + y);
        printf("|");
        for (int x = 1; x <= MAP_WIDTH; x++) {
            int is_player1 = (state->player1.x == x && state->player1.y == y);
            int is_player2 = (state->player2.x == x && state->player2.y == y);

            // 배경색 결정: 플래시 코드 > 0 일 경우 해당 색상 사용, 아니면 기본 검정색 사용
            if (flash_bg_code != 0) {
                set_background_color(flash_bg_code);
            }
            else {
                set_background_color(ANSI_BG_BLACK);
            }

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

    move_cursor(1, 14 + MAP_HEIGHT + 2);
}

void render_game(const GameState* state) {
    // 1. 화면 지우기 및 커서 숨기기
    clear_screen();
    hide_cursor();

    // 2. 맵 출력 시작
    move_cursor(1, 14);
    draw_border();

    for (int y = 1; y <= MAP_HEIGHT; y++) {
        move_cursor(1, 14 + y);
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
    move_cursor(1, 14 + MAP_HEIGHT + 2);

    // P1의 도발 메시지 출력
    if (state->player1.secrete_message[0] != '\0') {
        set_foreground_color(ANSI_CYAN); // P1 메시지는 CYAN으로
        printf(">> %s", state->player1.secrete_message);
        reset_color();
    }

    // P2의 도발 메시지 출력
    if (state->player2.secrete_message[0] != '\0') {
        set_foreground_color(ANSI_MAGENTA); // P2 메시지는 MAGENTA로
        printf(">> %s", state->player2.secrete_message);
        reset_color();
    }
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
        printf(" >> 해금 커맨드: ");
        int count = 0;

        // CMD_UP (1) 부터 CMD_V_ATTACK (18)까지 순회
        for (int i = 1; i < MAX_COMMAND_ID; i++) {
            // 해당 CMD_ID가 해금되었을 경우 (skill_status[i] == 1)
            if (state->player1.skill_status[i] == 1) {
                // 커맨드 인덱스와 이름을 출력합니다.
                printf("%d(%s) ", i, command_names[i]);
                count++;

                // 가독성을 위해 적절한 개수마다 줄 바꿈 (예: 5개마다)
                if (count % 5 == 0) {
                    printf("\n                  "); // 들여쓰기
                }
            }
        }
        printf("\n"); // 최종 줄 바꿈

        printf(" >> 해금 커맨드: ");
        count = 0;

        // CMD_UP (1) 부터 CMD_V_ATTACK (18)까지 순회
        for (int i = 1; i < MAX_COMMAND_ID; i++) {
            // 해당 CMD_ID가 해금되었을 경우 (skill_status[i] == 1)
            if (state->player2.skill_status[i] == 1) {
                // 커맨드 인덱스와 이름을 출력합니다.
                printf("%d(%s) ", i, command_names[i]);
                count++;

                // 가독성을 위해 적절한 개수마다 줄 바꿈 (예: 5개마다)
                if (count % 5 == 0) {
                    printf("\n                  "); // 들여쓰기
                }
            }
        }
        printf("\n"); // 최종 줄 바꿈
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

// 새로운 보조 함수: 두 좌표를 포함하는 영역의 경계 계산
static void get_min_max_coords(int x1, int y1, int x2, int y2, int* min_x, int* max_x, int* min_y, int* max_y) {
    // 1칸 여백 추가 (1-tile buffer)
    *min_x = (x1 < x2 ? x1 : x2) - 1;
    *max_x = (x1 > x2 ? x1 : x2) + 1;
    *min_y = (y1 < y2 ? y1 : y2) - 1;
    *max_y = (y1 > y2 ? y1 : y2) + 1;

    // 맵 경계 클램핑 (1부터 MAP_WIDTH/HEIGHT를 벗어나지 않도록 보장)
    if (*min_x < 1) *min_x = 1;
    if (*max_x > MAP_WIDTH) *max_x = MAP_WIDTH;
    if (*min_y < 1) *min_y = 1;
    if (*max_y > MAP_HEIGHT) *max_y = MAP_HEIGHT;
}


// --- 새로운 지역화된 플래시 렌더링 함수 ---
void render_localized_flash(const GameState* state, int flash_bg_code) {
    // 공격 영역 정의 (P1과 P2를 포함하는 최소 사각형 + 1칸 여백)
    int min_x, max_x, min_y, max_y;
    get_min_max_coords(state->player1.x, state->player1.y, state->player2.x, state->player2.y,
        &min_x, &max_x, &min_y, &max_y);

    hide_cursor();

    // 맵 영역만 반복하여 다시 그림
    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            move_cursor(1 + (x * 2) - 1, 14 + y); // (x*2 - 1)은 맵 심볼의 위치 (1-based + 2칸 너비)

            int is_player1 = (state->player1.x == x && state->player1.y == y);
            int is_player2 = (state->player2.x == x && state->player2.y == y);

            // 1. 배경색 설정: 공격 플래시 (RED)
            if (flash_bg_code != 0) {
                set_background_color(ANSI_BG_RED); // 공격 플래시는 RED 배경 사용
            }
            else {
                set_background_color(ANSI_BG_BLACK);
            }

            // 2. 내용 출력
            if (is_player1) {
                set_foreground_color(ANSI_CYAN); // P1 심볼 색상
                printf(" %c", state->player1.symbol);
            }
            else if (is_player2) {
                set_foreground_color(ANSI_MAGENTA); // P2 심볼 색상
                printf(" %c", state->player2.symbol);
            }
            else {
                set_foreground_color(ANSI_WHITE);
                printf(" ."); // 빈 공간
            }

            reset_color();
        }
    }
}