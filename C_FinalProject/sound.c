// sound.c

#include "sound.h"
#include <windows.h> // Beep 함수 사용을 위해 필수

// command에 따라 Beep 소리를 재생하는 함수 구현
void play_attack_sound(int command) {
    if (command == CMD_ATTACK) {
        Beep(BEEP_BASIC_ATTACK_FREQ, BEEP_BASIC_ATTACK_DUR);
    }
    else if (command == CMD_STRIKE) {
        Beep(BEEP_STRIKE_FREQ, BEEP_STRIKE_DUR);
    }
    // 기본 공격에 대한 처리는 CMD_ATTACK이 5이므로, CMD_ATTACK일 때 실행되도록 함.
    // CMD_ATTACK은 5, CMD_STRIKE는 7을 전제함.

    // 이외의 공격 커맨드(원거리, 가로, 세로 공격 등)에 대한 로직을 여기에 추가할 수 있음.
    // else if (command == CMD_RANGE_ATTACK) { Beep(550, 75); }
}