#ifndef RENDER_H
#define RENDER_H

#include "game.h" // GameState 구조체 사용

// 맵을 포함한 전체 화면을 그림
void render_game(const GameState* state);

// 현재 턴 및 플레이어 HP 정보 출력
void render_info(const GameState* state);

#endif // RENDER_H