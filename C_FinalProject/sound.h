#ifndef SOUND_H
#define SOUND_H

// api.h에 정의된 CMD 상수를 사용하기 위해 포함해야 함 (또는 CMD 상수를 여기에 복사)
#include "api.h" 

// --- 공격별 Beep 소리 상수 정의 ---
// 기본 공격
#define BEEP_BASIC_ATTACK_FREQ  440 
#define BEEP_BASIC_ATTACK_DUR   50  

// 강타
#define BEEP_STRIKE_FREQ        600 
#define BEEP_STRIKE_DUR         100 

// 자폭
#define BEEP_SELF_DESTRUCT_FREQ 880 
#define BEEP_SELF_DESTRUCT_DUR  300 

// 외부 노출 함수 선언
// command에 따라 적절한 Beep음을 재생함.
void play_attack_sound(int command);

#endif // SOUND_H