#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ansi_util.h"
#include "game.h"
#include "render.h"
#include "api.h"

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
    move_cursor(1, 13);
    printf("                                                                        \n");
    move_cursor(1, 13);

    if (my_info->id == 1) set_foreground_color(ANSI_RED);
    else set_foreground_color(ANSI_BLUE);

    printf("%s(%c)> 커맨드 입력 : ", my_info->name, my_info->symbol);
    reset_color();

    if (scanf("%d", &command) != 1) {
        // 입력 버퍼 비우기
        while (getchar() != '\n');
        return 0; // 잘못된 입력 시 0(대기) 커맨드 리턴
    }
    return command;
}




// ----------------------------------------------------
//  API 구현: 스킬 해금 시도 
// ----------------------------------------------------

// ===============================================
// ** 1. QUIZ DATA DEFINITIONS **
// ===============================================

#define MAX_QUIZ_ENTRIES 30
#define MAX_NAME_LEN 30
#define MAX_ANSWER_LEN 100
#define QUIZ_FILE_NAME "quiz_data.csv"

// CSV 항목을 저장할 구조체
typedef struct {
    int cmd_id;
    char name[MAX_NAME_LEN];
    char answer[MAX_ANSWER_LEN];
} QuizEntry;

// 파일에서 로드될 퀴즈 데이터베이스 (main.c 내부에서만 접근 가능한 static)
static QuizEntry quiz_database[MAX_QUIZ_ENTRIES];
static int quiz_count = 0;


// ===============================================
// ** 2. CSV 파일 로드 함수 **
// ===============================================

// NOTE: 이 함수는 main.c 내에서만 호출 가능한 static 함수가 됩니다.
static void load_quiz_data() {
    FILE* file = fopen(QUIZ_FILE_NAME, "r");
    char line[100];
    quiz_count = 0;

    if (file == NULL) {
        printf("ERROR: Quiz data file (%s) not found!\n", QUIZ_FILE_NAME);
        return;
    }

    // 첫 줄(헤더) 건너뛰기
    if (fgets(line, sizeof(line), file) == NULL) {
        fclose(file);
        return;
    }

    // 데이터 파싱
    while (fgets(line, sizeof(line), file) != NULL && quiz_count < MAX_QUIZ_ENTRIES) {
        if (sscanf(line, "%d,%29[^,],%99[^\n]",
            &quiz_database[quiz_count].cmd_id,
            quiz_database[quiz_count].name,
            quiz_database[quiz_count].answer) == 3)
        {
            quiz_count++;
        }
    }

    fclose(file);
    printf("INFO: %d quiz entries loaded.\n", quiz_count);
}


// ** [주의] CSV 파일 로드 및 퀴즈 데이터 저장 구조체는 이 파일에 정의되어야 함. **
// (CSV 로직은 너무 길어 생략하고, find_player_by_key만 구현함.)

// Key를 사용하여 Player 포인터를 찾는 유일한 내부 함수 (static game_state에 접근)
static Player* find_player_by_key_secure_impl(int registration_key) {
    if (registration_key == game_state.player1.reg_key) {
        return &game_state.player1;
    }
    else if (registration_key == game_state.player2.reg_key) {
        return &game_state.player2;
    }
    return NULL;
}


// --- 1. 스킬 해금 시도 함수 구현 ---
void attempt_skill_unlock(int registration_key, int skill_command, const char* quiz_answer) {
    // 1. 보안 인증: Player 포인터 안전하게 획득
    Player* self = find_player_by_key_secure_impl(registration_key);

    // 2. 유효성 검사
    if (self == NULL || skill_command >= MAX_COMMAND_ID || skill_command < 1) {
        return;
    }

    // 3. 정답 찾기 로직 (CSV 데이터베이스 사용)
    const char* correct_answer = NULL;
    int target_cmd_id = skill_command;

    // 계열 스킬의 경우, 기본 ID로 정답을 찾도록 ID 통일 (CSV 중복 방지)
    // CMD_BLINK 계열은 CMD_BLINK_UP(8)만으로 찾도록 통일
    if (skill_command >= CMD_BLINK_UP && skill_command <= CMD_BLINK_RIGHT) {
        target_cmd_id = CMD_BLINK_UP;
    }
    // H/V Attack 계열은 CMD_H_ATTACK(17)만으로 찾도록 통일
    else if (skill_command == CMD_V_ATTACK) {
        target_cmd_id = CMD_H_ATTACK;
    }

    // static quiz_database 순회
    for (int i = 0; i < quiz_count; i++) {
        if (quiz_database[i].cmd_id == target_cmd_id) {
            correct_answer = quiz_database[i].answer;
            break;
        }
    }

    // 4. 정답 확인 및 스킬 상태 업데이트
    if (correct_answer != NULL && strcmp(quiz_answer, correct_answer) == 0) {
        // 정답! 해당 스킬과 계열 스킬 해금

        // Blink 계열 모두 해금 (8, 9, 10, 11)
        if (skill_command >= CMD_BLINK_UP && skill_command <= CMD_BLINK_RIGHT) {
            for (int i = CMD_BLINK_UP; i <= CMD_BLINK_RIGHT; i++) {
                if (i < MAX_COMMAND_ID) self->skill_status[i] = 1;
            }
        }
        // H/V Attack 계열 모두 해금 (17, 18)
        else if (skill_command == CMD_H_ATTACK || skill_command == CMD_V_ATTACK) {
            if (CMD_H_ATTACK < MAX_COMMAND_ID) self->skill_status[CMD_H_ATTACK] = 1;
            if (CMD_V_ATTACK < MAX_COMMAND_ID) self->skill_status[CMD_V_ATTACK] = 1;
        }
        // 나머지 단일 스킬 해금
        else {
            self->skill_status[skill_command] = 1;
        }
    }
}


// ----------------------------------------------------
// ** 2. EXTERN API: 스킬 해금 여부 확인 함수 구현 **
// ----------------------------------------------------

// 이 함수는 extern으로 선언되어 학생 파일에서 호출 가능합니다.
int is_skill_unlocked(int registration_key, int skill_command) {
    Player* self = find_player_by_key_secure_impl(registration_key);

    // 1. 보안 검사 및 커맨드 유효성 검사
    if (self == NULL || skill_command >= MAX_COMMAND_ID || skill_command < 1) {
        return 0; // 인증 실패 또는 유효하지 않은 커맨드
    }

    // 2. 해금 상태 반환
    return self->skill_status[skill_command];
}

void set_custom_secrete_message(int registration_key, const char* message) {
    // find_player_by_key_secure_impl 함수를 사용하여 target Player를 안전하게 획득
    Player* self = find_player_by_key_secure_impl(registration_key);

    if (self == NULL || message == NULL) {
        printf("ERROR: Invalid key or message for custom taunt setting.\n");
        return;
    }

    size_t buffer_size = sizeof(self->secrete_message); // 501

    // 메시지를 custom_taunt 필드에 안전하게 복사
    strncpy(self->secrete_message, message, buffer_size);
    self->secrete_message[buffer_size - 1] = '\0'; // 널 종단 보장
}



int main() {

    enable_ansi_escape_codes();
    init_game(&game_state);
    load_quiz_data();

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

    //int count = 0;

    // 3. 메인 게임 루프
    while (game_state.game_over == 0) {
        //count++;
        // [이전 턴 결과 출력] - AI 커맨드 입력 프롬프트를 보여주기 위해 유지
        render_game(&game_state);
        render_info(&game_state);

        //printf("\n\n%d 번째 턴\n\n", count);

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