#include "api.h"
#include <stdlib.h> // abs() 사용을 위해 포함
#include <stdio.h>

// 간단한 맨하탄 거리 계산 유틸리티 함수
static int calculate_distance(const Player* p1, const Player* p2) {
    int dx = abs(get_player_x(p1) - get_player_x(p2));
    int dy = abs(get_player_y(p1) - get_player_y(p2));
    return dx + dy;
}

// MP를 사용하지 않고 공격만 시도하는 AI 로직
int simple_killer_ai(const Player* my_info, const Player* opponent_info) {
    int distance = calculate_distance(my_info, opponent_info);

    int my_x = get_player_x(my_info);
    int my_y = get_player_y(my_info);
    int opp_x = get_player_x(opponent_info);
    int opp_y = get_player_y(opponent_info);

    // 1. 공격 판정 (MP 0인 기본 공격 CMD_ATTACK 사용)
    if (distance <= 1) {
        return CMD_ATTACK;
    }

    // 2. 추격 이동 (X축 우선, 1칸 이동)

    // X축 거리를 좁힘
    if (my_x != opp_x) {
        if (my_x < opp_x) {
            return CMD_RIGHT;
        }
        else {
            return CMD_LEFT;
        }
    }

    // Y축 추격 (X축 위치가 같을 경우)
    if (my_y != opp_y) {
        if (my_y < opp_y) {
            return CMD_DOWN;
        }
        else {
            return CMD_UP;
        }
    }

    // 3. 예외 상황 (이미 상대와 겹치거나 나란할 경우)
    return CMD_ATTACK;
}

// P1 등록 함수 (registration.h에 extern 선언됨)
void register_player1_logic() {
    int playerID = 1;
    attempt_skill_unlock(1, CMD_STRIKE, 1337);
    register_player_ai(playerID, "TEAM-ALPHA", simple_killer_ai);
}