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

static int next_slot_id_to_register = 1;

// API 함수 구현: extern으로 선언된 register_player_ai 함수의 실제 구현부임.
int register_player_ai(const char* team_name, CommandFn ai_function) {
    if (next_slot_id_to_register > 2) return 0; // 등록 슬롯 초과

    Player* target = NULL;
    if (next_slot_id_to_register == 1) {
        target = &game_state.player1;
    }
    else {
        target = &game_state.player2;
    }

    // AI 함수 등록 및 이름 설정
    // [1] 배열의 전체 크기(10)를 복사 길이로 사용
    size_t size = sizeof(target->name);


    // [2] strncpy로 복사 (대상 배열 크기만큼 복사 시도)
    // Note: 만약 team_name이 size보다 길면, 널 종단이 되지 않습니다!
    strncpy(target->name, team_name, size);

    // [3] 널 종단 보장: 배열의 마지막 바이트(size - 1)에 \0을 강제 삽입
    target->name[size - 1] = '\0'; // name[9]에 \0이 강제됨.
    target->get_command = ai_function;
    

    // 슬롯 카운터 증가
    next_slot_id_to_register++;

    // **가장 중요한 부분: 고유 Key를 반환**
    return target->reg_key;
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
   
}


int main() {

    enable_ansi_escape_codes();
    init_game(&game_state);

    // ********** 학생 AI 등록 (명시적 호출) **********
    // 학생들에게 미리 고지된 함수명을 사용하여 등록 함수를 호출함.
    // 이 호출을 통해 학생들의 AI 함수 포인터가 game_state에 등록됨.
    student1_ai_entry();
    student2_ai_entry();

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