#include "api.h"
#include "registration.h" // register_player2_logic 선언을 위해 필요
#include <stdlib.h> 

// --- 보조 함수: 거리 계산 (AI 내부에 정의하여 이 파일에서만 사용) ---
static int calculate_distance(const Player* p1, const Player* p2) {
    int dx = abs(get_player_x(p1) - get_player_x(p2));
    int dy = abs(get_player_y(p1) - get_player_y(p2));
    return dx + dy;
}

// --- P2 AI 로직 구현 ---
int simple_killer_ai2(const Player* my_info, const Player* opponent_info) {
    int distance = calculate_distance(my_info, opponent_info);

    int my_x = get_player_x(my_info);
    int my_y = get_player_y(my_info);
    int opp_x = get_player_x(opponent_info);
    int opp_y = get_player_y(opponent_info);

    // 1. 공격 판정
    if (distance <= 1) {
        return CMD_ATTACK;
    }

    // 2. 추격 이동 (X축 우선)
    if (my_x != opp_x) {
        if (my_x < opp_x) {
            return CMD_RIGHT;
        }
        else {
            return CMD_LEFT;
        }
    }

    // 3. Y축 추격
    if (my_y != opp_y) {
        if (my_y < opp_y) {
            return CMD_DOWN;
        }
        else {
            return CMD_UP;
        }
    }

    // 4. 예외 상황 (같은 위치에 있으나 공격 조건이 안 될 경우)
    return CMD_ATTACK;
}

// --- P2 등록 함수 구현 (main.c에서 호출됨) ---
void register_player2_logic() {
    register_player_ai(2, "TEAM-KILLER", simple_killer_ai2);
}