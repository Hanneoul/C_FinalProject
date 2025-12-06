#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ansi_util.h"
#include "game.h"
#include "render.h"
#include "api.h"
#include "registration.h"

// GameState를 static으로 선언하여 register_player_ai에서 접근 가능하도록 함.
static GameState game_state;

// API 함수 구현: extern으로 선언된 register_player_ai 함수의 실제 구현부임.
void register_player_ai(int player_id, const char* team_name, CommandFn ai_function) {
    if (player_id == 1 && game_state.player1.get_command == NULL) { // 한 번만 등록 허용
        strncpy(game_state.player1.name, team_name, 9);
        game_state.player1.name[9] = '\0';
        game_state.player1.get_command = ai_function;
    }
    else if (player_id == 2 && game_state.player2.get_command == NULL) { // 한 번만 등록 허용
        strncpy(game_state.player2.name, team_name, 9);
        game_state.player2.name[9] = '\0';
        game_state.player2.get_command = ai_function;
    }
    else {
        printf("등록 실패: P%d는 이미 등록되었거나 잘못된 ID임.\n", player_id);
    }
}


// AI가 등록되지 않았을 경우 사용되는 수동 입력 함수 (fallback)
static int manual_command(const Player* my_info, const Player* opponent_info) {
    int command;

    // 커서 이동 및 기존 입력부 클리어 (render.c의 move_cursor 위치와 일치시켜야 함)
    move_cursor(1, 7);
    printf("                                                                        \n");
    move_cursor(1, 7);

    if (my_info->id == 1) set_foreground_color(ANSI_RED);
    else set_foreground_color(ANSI_BLUE);

    printf("%s(%c)> 커맨드 입력 (1/2/3/4/5): ", my_info->name, my_info->symbol);
    reset_color();

    if (scanf("%d", &command) != 1) {
        // 입력 버퍼 비우기
        while (getchar() != '\n');
        return 0; // 잘못된 입력 시 0(대기) 커맨드 리턴
    }
    return command;
}


// 퀴즈 정답 데이터베이스 (예시)
#define QUIZ_ANSWER_STRIKE 1337
#define QUIZ_ANSWER_POISON 4040
// ... 다른 스킬에 대한 정답 정의 ...

// ----------------------------------------------------
// ** 학생용 API 구현: 스킬 해금 시도 **
// ----------------------------------------------------

void attempt_skill_unlock(int player_id, int skill_command, int quiz_answer) {
    // Player *self 포인터 설정
    Player* self = (player_id == 1) ? &game_state.player1 : &game_state.player2;
    unsigned int skill_flag = 0;
    int correct_answer = 0;

    // 1. 커맨드 ID를 비트 플래그로 변환 및 정답 매핑
    if (skill_command == CMD_STRIKE) {
        skill_flag = SKILL_STRIKE;
        correct_answer = QUIZ_ANSWER_STRIKE;
    }
    else if (skill_command == CMD_POISON) {
        skill_flag = SKILL_POISON;
        correct_answer = QUIZ_ANSWER_POISON;
    }
    // ... 다른 스킬 (CMD_BLINK_UP 등)에 대한 매핑 로직 추가 ...

    if (skill_flag != 0 && quiz_answer == correct_answer) {
        // 정답 확인 및 해금
        self->unlocked_skills |= skill_flag;
        // printf("P%d: Skill %d unlocked!\n", player_id, skill_command); // (디버깅용)
    }
}


int main() {

    enable_ansi_escape_codes();
    init_game(&game_state);

    // ********** 학생 AI 등록 (명시적 호출) **********
    // 학생들에게 미리 고지된 함수명을 사용하여 등록 함수를 호출함.
    // 이 호출을 통해 학생들의 AI 함수 포인터가 game_state에 등록됨.
    register_player1_logic();
    register_player2_logic();

    // ********** AI 함수 포인터 초기화 **********
    // AI가 등록되지 않았다면 수동 입력 함수를 기본값으로 설정함.
    if (game_state.player1.get_command == NULL) {
        game_state.player1.get_command = manual_command;
    }
    if (game_state.player2.get_command == NULL) {
        game_state.player2.get_command = manual_command;
    }

    // 3. 메인 게임 루프
    while (game_state.game_over == 0) {
        // [이전 턴 결과 출력] - AI 커맨드 입력 프롬프트를 보여주기 위해 유지
        render_game(&game_state);
        render_info(&game_state);

        // 1. 커맨드 입력
        int p1_command = game_state.player1.get_command(&game_state.player1, &game_state.player2);
        int p2_command = game_state.player2.get_command(&game_state.player2, &game_state.player1);

        // 2. 상태 업데이트 (플래시 코드 반환)
        int turn_flash_code = execute_turn(&game_state, p1_command, p2_command);

        // 3. 업데이트된 상태 렌더링 (공격, 이동, HP 변화 반영)
        // 이 렌더링이 먼저 되어야 플레이어가 변화된 화면을 봄.
        render_game(&game_state);
        render_info(&game_state);

        // 4. 이펙트 적용 (재렌더링 플래시)
        if (turn_flash_code != FLASH_NONE && game_state.game_over == 0) {
            // 플래시는 RED로 고정
            int bg_code = ANSI_BG_RED;
            
            // 1단계: 공격 영역을 RED 배경으로 그림 (Flash ON)
            render_localized_flash(&game_state, bg_code);
            Sleep(100);

            // 2단계: 공격 영역을 다시 기본 BLACK 배경으로 그림 (Flash OFF)
            render_localized_flash(&game_state, 0);
        }

        // 5. 턴 지연
        if (game_state.game_over == 0) {
            Sleep(300);
        }
    }

    // 4. 게임 종료 후 처리
    render_game(&game_state);
    render_info(&game_state);

    show_cursor();
    reset_color();

    printf("\n\n게임이 종료되었음. 엔터를 눌러 종료 바람.\n");
    getchar();
    getchar();

    return 0;
}