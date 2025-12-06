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


int execute_turn(GameState* state, int p1_command, int p2_command) {
    Player* p1 = &state->player1;
    Player* p2 = &state->player2;

    int p1_attacked = 0;
    int p2_attacked = 0;
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

    // 2. P1 커맨드 처리
    // -----------------------------------------------------------
    if (p1->hp > 0) { // HP 0 이하는 행동 불가
        // 2a. 스킬 처리 (if/else if)
        if (p1_command == CMD_POISON && p1->mp >= 4) {
            p1->mp -= 4;
            p2->poison_duration = 3;
            p1_action_taken = 1;
        }
        else if (p1_command == CMD_STRIKE && p1->mp >= 2) {
            if (get_distance(p1, p2) <= 1) {
                p1->mp -= 2;
                p2->hp -= 2;
                p1_attacked = 1;
            }
            p1_action_taken = 1;
        }
        else if (p1_command == CMD_HEAL && p1->mp >= 1) {
            p1->mp -= 1;
            p1->hp += 1;
            p1_action_taken = 1;
        }
        else if (p1_command == CMD_HEAL_ALL && p1->mp >= 2) {
            int mp_recovered = p1->mp - 2;
            p1->mp -= 2;
            p1->hp += (mp_recovered > 0 ? mp_recovered : 0);
            p1_action_taken = 1;
        }
        else if (p1_command == CMD_RANGE_ATTACK && p1->mp >= 1) {
            if (get_distance(p1, p2) == 2) {
                p1->mp -= 1;
                p2->hp -= 1;
                p1_attacked = 1;
            }
            p1_action_taken = 1;
        }
        else if (p1_command == CMD_REST) {
            p1->mp += 1;
            p1_action_taken = 1;
        }
        else if (p1_command == CMD_SELF_DESTRUCT && p1->mp >= 5 && p1->hp > 3) {
            p1->mp -= 5;
            p1->hp -= 3;
            p2->hp -= 3;
            p1_attacked = 1;
            p1_action_taken = 1;
        }
        else if (p1_command == CMD_H_ATTACK && p1->mp >= 3) {
            if (p1->y == p2->y) {
                p1->mp -= 3;
                p2->hp -= 1;
                p1_attacked = 1;
            }
            p1_action_taken = 1;
        }
        else if (p1_command == CMD_V_ATTACK && p1->mp >= 3) {
            if (p1->x == p2->x) {
                p1->mp -= 3;
                p2->hp -= 1;
                p1_attacked = 1;
            }
            p1_action_taken = 1;
        }
        // 2b. 이동 및 기본 공격 (스킬 실패 시의 폴백)
        else {
            if (p1_command >= CMD_BLINK_UP && p1_command <= CMD_BLINK_RIGHT && p1->mp >= 1) {
                // 점멸 (Blink) 처리
                int dx = 0, dy = 0;
                if (p1_command == CMD_BLINK_UP) dy = -2;
                else if (p1_command == CMD_BLINK_DOWN) dy = 2;
                else if (p1_command == CMD_BLINK_LEFT) dx = -2;
                else if (p1_command == CMD_BLINK_RIGHT) dx = 2;

                p1_next_x += dx; p1_next_y += dy;

                if (p1_next_x >= 1 && p1_next_x <= MAP_WIDTH && p1_next_y >= 1 && p1_next_y <= MAP_HEIGHT) {
                    p1->mp -= 1;
                }
                else {
                    p1_next_x = p1->x; p1_next_y = p1->y;
                }
                p1_action_taken = 1;
            }
            else if (p1_command == CMD_ATTACK) {
                // 기본 공격 처리
                if (get_distance(p1, p2) <= 1) {
                    p2->hp -= 1;
                    p1_attacked = 1;
                }
                p1_action_taken = 1;
            }
            else if (p1_command >= CMD_UP && p1_command <= CMD_RIGHT) {
                // 기본 이동
                calculate_1step_move(&p1_next_x, &p1_next_y, p1_command);
                p1_action_taken = 1;
            }
        }
    }
    // -----------------------------------------------------------


    // 3. P2 커맨드 처리 (P1과 완벽하게 대칭되도록 수정)
    // -----------------------------------------------------------
    if (p2->hp > 0) {
        // 3a. 스킬 처리 (if/else if)
        if (p2_command == CMD_POISON && p2->mp >= 4) {
            p2->mp -= 4;
            p1->poison_duration = 3;
            p2_action_taken = 1;
        }
        else if (p2_command == CMD_STRIKE && p2->mp >= 2) {
            if (get_distance(p2, p1) <= 1) {
                p2->mp -= 2;
                p1->hp -= 2;
                p2_attacked = 1;
            }
            p2_action_taken = 1;
        }
        else if (p2_command == CMD_HEAL && p2->mp >= 1) {
            p2->mp -= 1;
            p2->hp += 1;
            p2_action_taken = 1;
        }
        else if (p2_command == CMD_HEAL_ALL && p2->mp >= 2) {
            int mp_recovered = p2->mp - 2;
            p2->mp -= 2;
            p2->hp += (mp_recovered > 0 ? mp_recovered : 0);
            p2_action_taken = 1;
        }
        else if (p2_command == CMD_RANGE_ATTACK && p2->mp >= 1) {
            if (get_distance(p2, p1) == 2) {
                p2->mp -= 1;
                p1->hp -= 1;
                p2_attacked = 1;
            }
            p2_action_taken = 1;
        }
        else if (p2_command == CMD_REST) {
            p2->mp += 1;
            p2_action_taken = 1;
        }
        else if (p2_command == CMD_SELF_DESTRUCT && p2->mp >= 5 && p2->hp > 3) {
            p2->mp -= 5;
            p2->hp -= 3;
            p1->hp -= 3;
            p2_attacked = 1;
            p2_action_taken = 1;
        }
        else if (p2_command == CMD_H_ATTACK && p2->mp >= 3) {
            if (p2->y == p1->y) {
                p2->mp -= 3;
                p1->hp -= 1;
                p2_attacked = 1;
            }
            p2_action_taken = 1;
        }
        else if (p2_command == CMD_V_ATTACK && p2->mp >= 3) {
            if (p2->x == p1->x) {
                p2->mp -= 3;
                p1->hp -= 1;
                p2_attacked = 1;
            }
            p2_action_taken = 1;
        }
        // 3b. 이동 및 기본 공격 (스킬 실패 시의 폴백)
        else {
            if (p2_command >= CMD_BLINK_UP && p2_command <= CMD_BLINK_RIGHT && p2->mp >= 1) {
                // 점멸 (Blink) 처리
                int dx = 0, dy = 0;
                if (p2_command == CMD_BLINK_UP) dy = -2;
                else if (p2_command == CMD_BLINK_DOWN) dy = 2;
                else if (p2_command == CMD_BLINK_LEFT) dx = -2;
                else if (p2_command == CMD_BLINK_RIGHT) dx = 2;

                p2_next_x += dx; p2_next_y += dy;

                if (p2_next_x >= 1 && p2_next_x <= MAP_WIDTH && p2_next_y >= 1 && p2_next_y <= MAP_HEIGHT) {
                    p2->mp -= 1;
                }
                else {
                    p2_next_x = p2->x; p2_next_y = p2->y;
                }
                p2_action_taken = 1;
            }
            else if (p2_command == CMD_ATTACK) {
                // 기본 공격 처리
                if (get_distance(p2, p1) <= 1) {
                    p1->hp -= 1;
                    p2_attacked = 1;
                }
                p2_action_taken = 1;
            }
            else if (p2_command >= CMD_UP && p2_command <= CMD_RIGHT) {
                // 기본 이동
                calculate_1step_move(&p2_next_x, &p2_next_y, p2_command);
                p2_action_taken = 1;
            }
        }
    }
    // -----------------------------------------------------------


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

// ... (get_distance 함수는 생략됨)