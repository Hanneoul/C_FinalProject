#define _CRT_SECURE_NO_WARNINGS
#include "game.h" // Player 구조체의 실제 정의를 포함
#include "api.h"  // Getter 함수 선언을 포함
#include "ansi_util.h"  // Getter 함수 선언을 포함
#include <stdlib.h> 
#include <time.h>   
#include <string.h> // strncpy 사용

// game.c 파일 내부 (Static API 정의 전에 추가)



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

    // 고유 랜덤 ID 생성 및 할당 (5자리 숫자)
    state->player1.reg_key = (rand() % 90000) + 10000;
    do {
        state->player2.reg_key = (rand() % 90000) + 10000;
    } while (state->player2.reg_key == state->player1.reg_key);

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

    // 배열 0으로 초기화 (모든 스킬 잠금)
    memset(state->player1.skill_status, 0, sizeof(state->player1.skill_status));
    memset(state->player2.skill_status, 0, sizeof(state->player2.skill_status));

    // --- 기본 스킬 해금 (항상 사용 가능) ---
    // 이동 커맨드 (1 ~ 4)
    for (int i = CMD_UP; i <= CMD_RIGHT; i++) {
        state->player1.skill_status[i] = 1;
        state->player2.skill_status[i] = 1;
    }

    // 기본 공격 (5), 기본 회복 (12), 휴식 (15)
    state->player1.skill_status[CMD_ATTACK] = 1;
    state->player1.skill_status[CMD_HEAL] = 1;
    state->player1.skill_status[CMD_REST] = 1;
    // P2도 동일하게 설정
    state->player2.skill_status[CMD_ATTACK] = 1;
    state->player2.skill_status[CMD_HEAL] = 1;
    state->player2.skill_status[CMD_REST] = 1;
}

// 50% 확률로 true(1) 또는 false(0)를 반환 (우선권 결정용)
static int flip_coin() {
    return rand() % 2;
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
// ** Static API: 커맨드별 처리 함수 선언 (새로운 분류) **
// ===============================================

// 1. 위치 이동 관련 (기본 이동, 점멸)
static int HandleMove(Player* self, int command, int* next_x, int* next_y);

// 2. 체력/마나 회복 관련 (회복, 휴식)
static int HandleHeal(Player* self, int command);

// 3. 물리적 공격 관련 (기본 공격, 강타, 원거리, 자폭)
static int HandleAction(Player* self, Player* opponent, int command);

// 4. 마법/전략 공격 관련 (독, 가로/세로 공격)
static int HandleMagic(Player* self, Player* opponent, int command);



int execute_turn(GameState* state, int p1_command, int p2_command) {
    Player* p1 = &state->player1;
    Player* p2 = &state->player2;

    int p1_attacked = 0;
    int p2_attacked = 0;
    int p1_next_x = p1->x;
    int p1_next_y = p1->y;
    int p2_next_x = p2->x;
    int p2_next_y = p2->y;

    // 1. 독(DoT) 데미지 적용 (순서 유지)
    if (p1->poison_duration > 0) {
        if (p1->hp > 0) p1->hp -= 1;
        p1->poison_duration--;
    }
    if (p2->poison_duration > 0) {
        if (p2->hp > 0) p2->hp -= 1;
        p2->poison_duration--;
    }

    // 2. P1/P2 커맨드 처리 (비용 지불 및 이동 의도/회복 적용)
    // NOTE: HandleAction/Magic은 이제 데미지 적용 없이 비용만 처리합니다.
    int p1_result = handle_command_dispatch(p1, p2, p1_command, &p1_next_x, &p1_next_y);
    int p2_result = handle_command_dispatch(p2, p1, p2_command, &p2_next_x, &p2_next_y);

    int p1_wants_attack = (p1_result == ACTION_SUCCEEDED_AND_ATTACKED);
    int p2_wants_attack = (p2_result == ACTION_SUCCEEDED_AND_ATTACKED);


    // ====================================================================
    // 3. 이동 확정 (Move Phase) - 충돌 검사
    // ====================================================================
    int p1_moves = (p1->x != p1_next_x || p1->y != p1_next_y);
    int p2_moves = (p2->x != p2_next_x || p2->y != p2_next_y);

    if (p1_moves && p2_moves && p1_next_x == p2_next_x && p1_next_y == p2_next_y) {
        if (flip_coin()) { // P1 승리
            p1->x = p1_next_x; p1->y = p1_next_y;
        }
        else { // P2 승리
            p2->x = p2_next_x; p2->y = p2_next_y;
        }
    }
    else {
        p1->x = p1_next_x; p1->y = p1_next_y;
        p2->x = p2_next_x; p2->y = p2_next_y;
    }


    // ====================================================================
    // 4. 순차적 공격 적용 (Magic -> Action)
    // ====================================================================
    // ** 4.1. 공격 우선순위 결정 **
    int p1_goes_first = flip_coin();
    Player* first_attacker, * second_attacker;
    int first_command, second_command;
    int first_wants_attack, second_wants_attack;

    if (p1_goes_first) {
        first_attacker = p1; second_attacker = p2;
        first_command = p1_command; second_command = p2_command;
        first_wants_attack = p1_wants_attack; second_wants_attack = p2_wants_attack;
    }
    else {
        first_attacker = p2; second_attacker = p1;
        first_command = p2_command; second_command = p1_command;
        first_wants_attack = p2_wants_attack; second_wants_attack = p1_wants_attack;
    }

    // 공격자 플래그 업데이트를 위한 헬퍼 매크로
#define SET_ATTACKED_FLAG(player_ptr) \
        do { \
            if ((player_ptr) == p1) p1_attacked = 1; \
            else p2_attacked = 1; \
        } while (0)

    // --- 4.2. 마법 공격 (Magic Phase) ---
    // 1순위 공격자 실행
    if (first_wants_attack && (first_command == CMD_POISON || first_command == CMD_H_ATTACK || first_command == CMD_V_ATTACK) && first_attacker->hp > 0) {
        if (ApplyFinalDamage(first_attacker, second_attacker, first_command)) {
            SET_ATTACKED_FLAG(first_attacker);
        }
    }

    // 2순위 공격자 실행 (1순위 공격에 죽지 않았을 경우)
    if (second_wants_attack && (second_command == CMD_POISON || second_command == CMD_H_ATTACK || second_command == CMD_V_ATTACK) && second_attacker->hp > 0 && first_attacker->hp > 0) {
        if (ApplyFinalDamage(second_attacker, first_attacker, second_command)) {
            SET_ATTACKED_FLAG(second_attacker);
        }
    }


    // --- 4.3. 물리 공격 (Action Phase) ---
    // 1순위 공격자 실행
    if (first_wants_attack && (first_command == CMD_ATTACK || first_command == CMD_STRIKE || first_command == CMD_RANGE_ATTACK || first_command == CMD_SELF_DESTRUCT) && first_attacker->hp > 0) {
        if (ApplyFinalDamage(first_attacker, second_attacker, first_command)) {
            SET_ATTACKED_FLAG(first_attacker);
        }
    }

    // 2순위 공격자 실행 (1순위 공격에 죽지 않았을 경우)
    if (second_wants_attack && (second_command == CMD_ATTACK || second_command == CMD_STRIKE || second_command == CMD_RANGE_ATTACK || second_command == CMD_SELF_DESTRUCT) && second_attacker->hp > 0 && first_attacker->hp > 0) {
        if (ApplyFinalDamage(second_attacker, first_attacker, second_command)) {
            SET_ATTACKED_FLAG(second_attacker);
        }
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
    int p1_dead = (state->player1.hp <= 0);
    int p2_dead = (state->player2.hp <= 0);

    if (p1_dead && p2_dead) {
        // 동시 사망 시: 50% 확률로 승자 결정 (타이 브레이커)
        if (flip_coin()) {
            return 1; // P1 승리
        }
        else {
            return 2; // P2 승리
        }
    }
    if (p1_dead) {
        return 2; // P2 승리 (P1 사망)
    }
    if (p2_dead) {
        return 1; // P1 승리 (P2 사망)
    }
    return 0; // 게임 진행 중
}

// ===============================================
// ** 중앙 디스패처 구현 **
// ===============================================

// 모든 커맨드를 분배하는 중앙 디스패처 함수
static int handle_command_dispatch(Player* self, Player* opponent, int command, int* next_x, int* next_y) {
    if (self->hp <= 0) return ACTION_FAILED;


    if (command < MAX_COMMAND_ID && self->skill_status[command] == 0)
    {
        self->hp -= 2; // 페널티 2 데미지!
        return ACTION_FAILED;
    }


    // --- 1. 회복 및 휴식 (Heal Logic) ---
    if (command == CMD_HEAL || command == CMD_HEAL_ALL || command == CMD_REST) {
        return HandleHeal(self, command);
    }

    // --- 2. 이동 커맨드 (Move Logic) ---
    if ((command >= CMD_UP && command <= CMD_RIGHT) || (command >= CMD_BLINK_UP && command <= CMD_BLINK_RIGHT)) {
        return HandleMove(self, command, next_x, next_y);
    }

    // --- 3. 마법/전략 커맨드 (Magic Logic) ---
    if (command == CMD_POISON || command == CMD_H_ATTACK || command == CMD_V_ATTACK) {
        return HandleMagic(self, opponent, command);
    }

    // --- 4. 직접 공격 커맨드 (Action Logic) ---
    // (CMD_ATTACK, CMD_STRIKE, CMD_RANGE_ATTACK, CMD_SELF_DESTRUCT)
    if (command == CMD_ATTACK || command == CMD_STRIKE || command == CMD_RANGE_ATTACK || command == CMD_SELF_DESTRUCT) {
        return HandleAction(self, opponent, command);
    }

    return ACTION_FAILED; // 정의되지 않은 커맨드
}


// --- Static API: 1. HandleMove (이동) ---
static int HandleMove(Player* self, int command, int* next_x, int* next_y) {
    if (command >= CMD_UP && command <= CMD_RIGHT) {
        // 기본 1칸 이동 (MP 소모 없음)
        calculate_1step_move(next_x, next_y, command);
        return ACTION_SUCCEEDED_NO_FLASH;
    }

    if (command >= CMD_BLINK_UP && command <= CMD_BLINK_RIGHT) {
        // 점멸 (Blink) 로직: MP 1 소모, 2칸 이동
        if (self->mp < 1) return ACTION_FAILED;

        int dx = 0, dy = 0;
        if (command == CMD_BLINK_UP) dy = -2;
        else if (command == CMD_BLINK_DOWN) dy = 2;
        else if (command == CMD_BLINK_LEFT) dx = -2;
        else if (command == CMD_BLINK_RIGHT) dx = 2;

        int new_x = self->x + dx;
        int new_y = self->y + dy;

        // 경계 검사
        if (new_x >= 1 && new_x <= MAP_WIDTH && new_y >= 1 && new_y <= MAP_HEIGHT) {
            self->mp -= 1;
            *next_x = new_x;
            *next_y = new_y;
            return ACTION_SUCCEEDED_NO_FLASH;
        }
    }
    return ACTION_FAILED;
}

// --- Static API: 2. HandleHeal (회복/휴식) ---
static int HandleHeal(Player* self, int command) {
    if (command == CMD_HEAL) {
        // 회복: HP 1 회복, MP 1 소모
        if (self->mp >= 1) {
            self->mp -= 1;
            self->hp += 1;
            return ACTION_SUCCEEDED_NO_FLASH;
        }
    }
    else if (command == CMD_HEAL_ALL) {
        // 회복2: MP 2 소모, 나머지 MP 잔량만큼 HP 회복
        if (self->mp >= 2) {
            int mp_recovered = self->mp - 2;
            self->mp -= 2;
            self->hp += (mp_recovered > 0 ? mp_recovered : 0);
            return ACTION_SUCCEEDED_NO_FLASH;
        }
    }
    else if (command == CMD_REST) {
        // 휴식: MP 1 회복
        self->mp += 1;
        return ACTION_SUCCEEDED_NO_FLASH;
    }
    return ACTION_FAILED;
}

// --- Static API: 3. HandleMagic (마법/전략 공격) ---
static int HandleMagic(Player* self, Player* opponent, int command) {
    if (command == CMD_POISON) {
        // 독: MP 4 소모, DoT 3턴 부여
        if (self->mp >= 4) {
            self->mp -= 4;
            opponent->poison_duration = 3;
            return ACTION_SUCCEEDED_NO_FLASH;
        }
    }
    else if (command == CMD_H_ATTACK) {
        // 가로 공격: MP 3 소모, 같은 행 타격
        if (self->mp >= 3 && self->y == opponent->y) {
            self->mp -= 3;
            opponent->hp -= 1;
            return ACTION_SUCCEEDED_AND_ATTACKED;
        }
    }
    else if (command == CMD_V_ATTACK) {
        // 세로 공격: MP 3 소모, 같은 열 타격
        if (self->mp >= 3 && self->x == opponent->x) {
            self->mp -= 3;
            opponent->hp -= 1;
            return ACTION_SUCCEEDED_AND_ATTACKED;
        }
    }
    return ACTION_FAILED;
}

// --- Static API: 4. HandleAction (물리 공격) ---
static int HandleAction(Player* self, Player* opponent, int command) {
    if (command == CMD_ATTACK) {
        // 기본 공격: MP 0, 근접 1 데미지 -> [MP 0, 근접] 조건만 체크
        if (get_distance(self, opponent) <= 1) {
            // NOTE: opponent->hp -= 1; (삭제됨 - ApplyFinalDamage로 이동)
            return ACTION_SUCCEEDED_AND_ATTACKED;
        }
    }
    else if (command == CMD_STRIKE) {
        // 강타: MP 2 소모, 근접 2 데미지 -> [MP 2, 근접] 조건 및 MP 소모만 처리
        if (self->mp >= 2 && get_distance(self, opponent) <= 1) {
            self->mp -= 2;
            // NOTE: opponent->hp -= 2; (삭제됨)
            return ACTION_SUCCEEDED_AND_ATTACKED;
        }
    }
    else if (command == CMD_RANGE_ATTACK) {
        // 원거리 공격: MP 1 소모, 거리 2 타격 -> [MP 1, 거리 2] 조건 및 MP 소모만 처리
        if (self->mp >= 1 && get_distance(self, opponent) == 2) {
            self->mp -= 1;
            // NOTE: opponent->hp -= 1; (삭제됨)
            return ACTION_SUCCEEDED_AND_ATTACKED;
        }
    }
    else if (command == CMD_SELF_DESTRUCT) {
        // 자폭: MP 5, HP 3 소모, 상대 3 데미지 -> [MP/HP 조건] 및 자원 소모만 처리
        if (self->mp >= 5 && self->hp > 3) {
            self->mp -= 5;
            self->hp -= 3; // 자기 HP 소모는 Cost이므로 유지
            // NOTE: opponent->hp -= 3; (삭제됨)
            return ACTION_SUCCEEDED_AND_ATTACKED;
        }
    }
    return ACTION_FAILED;
}



// Static API: 5. ApplyFinalDamage (최종 피해 적용)
// 이 함수는 공격이 시도될 때, MP/HP 소모는 이미 발생했음을 전제로 합니다.
static int ApplyFinalDamage(Player* self, Player* opponent, int command) {

    // 1. 기본 공격 (CMD_ATTACK)
    if (command == CMD_ATTACK) {
        if (get_distance(self, opponent) <= 1) {
            opponent->hp -= 1;
            return 1;
        }
    }
    // 2. 강타 (CMD_STRIKE)
    else if (command == CMD_STRIKE) {
        if (get_distance(self, opponent) <= 1) {
            opponent->hp -= 2;
            return 1;
        }
    }
    // 3. 원거리 공격 (CMD_RANGE_ATTACK)
    else if (command == CMD_RANGE_ATTACK) {
        if (get_distance(self, opponent) == 2) {
            opponent->hp -= 1;
            return 1;
        }
    }
    // 4. 자폭 (CMD_SELF_DESTRUCT)
    else if (command == CMD_SELF_DESTRUCT) {
        // 자폭 비용은 이미 지불됨. 상대에게 최종 데미지만 적용함.
        opponent->hp -= 3;
        return 1;
    }
    // 5. 가로 공격 (CMD_H_ATTACK)
    else if (command == CMD_H_ATTACK) {
        if (self->y == opponent->y) {
            opponent->hp -= 1;
            return 1;
        }
    }
    // 6. 세로 공격 (CMD_V_ATTACK)
    else if (command == CMD_V_ATTACK) {
        if (self->x == opponent->x) {
            opponent->hp -= 1;
            return 1;
        }
    }
    // 독, 회복, 이동 등 비공격 커맨드는 여기서 0 반환
    return 0;
}

