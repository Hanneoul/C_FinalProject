#ifndef RENDER_H
#define RENDER_H

#include "game.h" // GameState 구조체 사용

// 맵을 포함한 전체 화면을 그림
void render_game(const GameState* state);

// 현재 턴 및 플레이어 HP 정보 출력
void render_info(const GameState* state);

// render.c 파일 상단 (Static 정의 부분에 추가)

// 총 커맨드 수 (0번 인덱스는 사용하지 않거나 Wait로 가정, 1~18 사용)
#define CMD_COUNT 20 

static const char* command_names[CMD_COUNT] = {
    "N/A", // 0번 인덱스 (사용 안 함)
    "상", // 1: CMD_UP
    "하", // 2: CMD_DOWN
    "좌", // 3: CMD_LEFT
    "우", // 4: CMD_RIGHT
    "기본공격", // 5: CMD_ATTACK
    "독", // 6: CMD_POISON
    "강타", // 7: CMD_STRIKE
    "점멸(상)", // 8: CMD_BLINK_UP
    "점멸(하)", // 9: CMD_BLINK_DOWN
    "점멸(좌)", // 10: CMD_BLINK_LEFT
    "점멸(우)", // 11: CMD_BLINK_RIGHT
    "회복", // 12: CMD_HEAL
    "회복2", // 13: CMD_HEAL_ALL
    "원거리공격", // 14: CMD_RANGE_ATTACK
    "휴식", // 15: CMD_REST
    "폭주", // 16: CMD_SELF_DESTRUCT
    "가로공격", // 17: CMD_H_ATTACK
    "세로공격",  // 18: CMD_V_ATTACK
    "비밀"    // 19: CMD_SECRETE
};

#endif // RENDER_H