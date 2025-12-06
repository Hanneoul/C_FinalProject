#define _CRT_SECURE_NO_WARNINGS
#include "game.h" // Player 구조체의 실제 정의를 포함
#include "api.h"  // Getter 함수 선언을 포함
#include "ansi_util.h"  // Getter 함수 선언을 포함
#include <stdlib.h> 
#include <time.h>   
#include <string.h> // strncpy 사용

// ===================================
// API Getter 함수 구현
// ===================================

int get_player_hp(const Player* p) {
    return p->hp;
}

int get_player_mp(const Player* p) {
    return p->mp;
}

int get_player_x(const Player* p) {
    return p->x;
}

int get_player_y(const Player* p) {
    return p->y;
}

int get_player_last_command(const Player* p) {
    return p->last_command;
}

int get_player_id(const Player* p) {
    return p->id;
}

void init_game(GameState* state) {
    srand((unsigned int)time(NULL));

    // 플레이어 1 초기화
    state->player1.id = 1;
    state->player1.x = 1;
    state->player1.y = 1;
    state->player1.hp = 5;
    state->player1.mp = 5;
    strncpy(state->player1.name, "Player X", 9);
    state->player1.symbol = 'X';
    state->player1.last_command = 0;
    state->player1.poison_duration = 0; // 독 지속 시간 초기화
    state->player1.get_command = NULL; // AI 함수 포인터를 NULL로 초기화함.

    // 플레이어 2 초기화
    state->player2.id = 2;
    state->player2.x = MAP_WIDTH;
    state->player2.y = MAP_HEIGHT;
    state->player2.hp = 5;
    state->player2.mp = 5;
    strncpy(state->player2.name, "Player O", 9);
    state->player2.symbol = 'O';
    state->player2.last_command = 0;
    state->player2.poison_duration = 0;
    state->player2.get_command = NULL; // AI 함수 포인터를 NULL로 초기화함.

    state->turn = 1;
    state->game_over = 0;
}


// 두 플레이어 간의 맨하탄 거리 계산
static int get_distance(const Player* p1, const Player* p2) {
    int dx = abs(p1->x - p2->x);
    int dy = abs(p1->y - p2->y);
    return dx + dy;
}

// 1칸 이동을 처리하고 경계를 체크하는 함수
static void calculate_1step_move(int* x, int* y, int command) {
    int next_x = *x;
    int next_y = *y;

    switch (command) {
    case CMD_UP:    next_y--; break;
    case CMD_DOWN:  next_y++; break;
    case CMD_LEFT:  next_x--; break;
    case CMD_RIGHT: next_x++; break;
    }

    // 맵 경계 체크 (1-based)
    if (next_x >= 1 && next_x <= MAP_WIDTH) {
        *x = next_x;
    }
    if (next_y >= 1 && next_y <= MAP_HEIGHT) {
        *y = next_y;
    }
}




// ** 내부 액션 상태 코드 정의 **
#define ACTION_FAILED                   0
#define ACTION_SUCCEEDED_NO_FLASH       1 // 회복, 휴식, 독 부여 등
#define ACTION_SUCCEEDED_AND_ATTACKED   2 // 기본 공격, 강타, 자폭 등 (플래시 발생)

// ===============================================
// ** Static API: 커맨드별 처리 함수 선언 **
// ===============================================

// 자원/타격형 스킬 (MP 소모 및 공격/회복)
static int handle_mp_skill(Player* self, Player* opponent, int command);
// 점멸형 스킬 (좌표 변경)
static int handle_blink(Player* self, int command, int* next_x, int* next_y);
// 기본 행동 (MP 소모 없는 공격/이동)
static int handle_basic_action(Player* self, Player* opponent, int command, int* next_x, int* next_y);

// 모든 커맨드를 분배하는 중앙 디스패처 함수
static int handle_command_dispatch(Player* self, Player* opponent, int command, int* next_x, int* next_y);

int execute_turn(GameState* state, int p1_command, int p2_command) {
    Player* p1 = &state->player1;
    Player* p2 = &state->player2;

    
    int p1_action_taken = 0;
    int p2_action_taken = 0;

    int p1_next_x = p1->x;
    int p1_next_y = p1->y;
    int p2_next_x = p2->x;
    int p2_next_y = p2->y;

    // 1. 독(DoT) 데미지 적용
    if (p1->poison_duration > 0) {
        if (p1->hp > 0) p1->hp -= 1;
        p1->poison_duration--;
    }
    if (p2->poison_duration > 0) {
        if (p2->hp > 0) p2->hp -= 1;
        p2->poison_duration--;
    }

    // 2. P1/P2 커맨드 처리
    // -----------------------------------------------------------
    int p1_result = handle_command_dispatch(p1, p2, p1_command, &p1_next_x, &p1_next_y);
    int p2_result = handle_command_dispatch(p2, p1, p2_command, &p2_next_x, &p2_next_y);
    // -----------------------------------------------------------


    // 3. 플래그 설정
    int p1_attacked = (p1_result == ACTION_SUCCEEDED_AND_ATTACKED);
    int p2_attacked = (p2_result == ACTION_SUCCEEDED_AND_ATTACKED);


    // 4. 이동 처리 (충돌 검사 및 최종 위치 반영)

    // P1의 최종 위치 반영
    if (p1_next_x != p2_next_x || p1_next_y != p2_next_y) {
        p1->x = p1_next_x;
        p1->y = p1_next_y;
    }
    // P2의 최종 위치 반영 (P1의 새로운 위치와 충돌하지 않는 경우)
    if (p2_next_x != p1->x || p2_next_y != p1->y) {
        p2->x = p2_next_x;
        p2->y = p2_next_y;
    }

    // 5. 플래시 코드 결정
    int flash_code = FLASH_NONE;
    if (p1_attacked && p2_attacked) {
        flash_code = FLASH_BOTH;
    }
    else if (p1_attacked) {
        flash_code = FLASH_P1;
    }
    else if (p2_attacked) {
        flash_code = FLASH_P2;
    }

    // 6. 게임 상태 업데이트 및 안전성 확보
    p1->last_command = p1_command;
    p2->last_command = p2_command;
    state->turn++;

    // HP와 MP는 0 이하로 내려가지 않도록 강제
    if (p1->hp < 0) p1->hp = 0;
    if (p2->hp < 0) p2->hp = 0;
    if (p1->mp < 0) p1->mp = 0;
    if (p2->mp < 0) p2->mp = 0;

    state->game_over = check_game_over(state);

    return flash_code;
}

int check_game_over(const GameState* state) {
    if (state->player1.hp <= 0 && state->player2.hp <= 0) {
        return 3; // 무승부 처리
    }
    if (state->player1.hp <= 0) {
        return 2; // P2 승리
    }
    if (state->player2.hp <= 0) {
        return 1; // P1 승리
    }
    return 0; // 게임 진행 중
}

// ===============================================
// ** 중앙 디스패처 구현 **
// ===============================================

static int handle_command_dispatch(Player* self, Player* opponent, int command, int* next_x, int* next_y) {
    if (self->hp <= 0) return ACTION_FAILED;

    // 1. 점멸 커맨드 분배 (CMD_BLINK_X: 8 ~ 11)
    if (command >= CMD_BLINK_UP && command <= CMD_BLINK_RIGHT) {
        return handle_blink(self, command, next_x, next_y);
    }

    // 2. MP 소모 스킬 분배 (CMD_POISON ~ CMD_V_ATTACK: 6, 7, 12 ~ 18)
    if (command >= CMD_POISON && command <= CMD_V_ATTACK) {
        return handle_mp_skill(self, opponent, command);
    }

    // 3. 기본 행동 분배 (CMD_UP ~ CMD_ATTACK: 1 ~ 5)
    if (command >= CMD_UP && command <= CMD_ATTACK) {
        return handle_basic_action(self, opponent, command, next_x, next_y);
    }

    return ACTION_FAILED; // 정의되지 않은 커맨드
}

// --- Static API: 점멸 처리 ---
static int handle_blink(Player* self, int command, int* next_x, int* next_y) {
    // 1. MP 소모 조건 확인
    if (self->mp < 1) {
        return ACTION_FAILED;
    }

    int dx = 0;
    int dy = 0;

    // 2. 커맨드에 따른 이동 거리 계산 (2칸 이동)
    if (command == CMD_BLINK_UP) {
        dy = -2;
    }
    else if (command == CMD_BLINK_DOWN) {
        dy = 2;
    }
    else if (command == CMD_BLINK_LEFT) {
        dx = -2;
    }
    else if (command == CMD_BLINK_RIGHT) {
        dx = 2;
    }
    else {
        return ACTION_FAILED; // 잘못된 점멸 커맨드
    }

    int new_x = self->x + dx;
    int new_y = self->y + dy;

    // 3. 경계 검사 및 이동 처리
    if (new_x >= 1 && new_x <= MAP_WIDTH && new_y >= 1 && new_y <= MAP_HEIGHT) {
        // 성공: MP 소모 및 임시 좌표 업데이트
        self->mp -= 1;
        *next_x = new_x;
        *next_y = new_y;
        return ACTION_SUCCEEDED_NO_FLASH;
    }

    // 실패: 경계 초과
    return ACTION_FAILED;
}

// --- Static API: MP 소모 스킬 처리 (독, 강타, 자폭 등) ---
static int handle_mp_skill(Player* self, Player* opponent, int command) {
    // 1. 자폭 (가장 엄격한 조건 체크)
    if (command == CMD_SELF_DESTRUCT) {
        if (self->mp >= 5 && self->hp > 3) {
            self->mp -= 5;
            self->hp -= 3;
            opponent->hp -= 3;
            return ACTION_SUCCEEDED_AND_ATTACKED;
        }
        return ACTION_FAILED;
    }

    // 2. 독 (Poison)
    else if (command == CMD_POISON) {
        if (self->mp >= 4) {
            self->mp -= 4;
            opponent->poison_duration = 3;
            return ACTION_SUCCEEDED_NO_FLASH;
        }
        return ACTION_FAILED;
    }

    // ... (나머지 강타, 회복, 원거리, 가로/세로 공격 로직도 이 함수 내에서 if/else if로 처리되어야 함)

    return ACTION_FAILED;
}

// --- Static API: 기본 행동 처리 (기본 이동 및 기본 공격) ---
static int handle_basic_action(Player* self, Player* opponent, int command, int* next_x, int* next_y) {
    if (command == CMD_ATTACK) {
        if (get_distance(self, opponent) <= 1) {
            opponent->hp -= 1;
            return ACTION_SUCCEEDED_AND_ATTACKED;
        }
        return ACTION_SUCCEEDED_NO_FLASH; // 공격 시도만으로 성공 처리 (플래시 없음)
    }

    // 기본 이동 (CMD_UP ~ CMD_RIGHT)
    if (command >= CMD_UP && command <= CMD_RIGHT) {
        calculate_1step_move(next_x, next_y, command);
        return ACTION_SUCCEEDED_NO_FLASH;
    }

    return ACTION_FAILED;
}